#include <LiquidCrystal_I2C.h>
#include <dht.h>

#include "EepromInterface.h"
#include "Types.h"
#include "MeasuringContext.h"

#include "DisplayHandler.h"
#include "MainMenu.h"
#include "InfoRoller.h"
#include "HistoryRoller.h"
#include "SettingsMenu.h"
#include "TestMenu.h"

#define DHT11_PIN 2

#define MOISTURE_PIN1 4
#define MOISTURE_PIN2 5

#define BUTTON1_PIN 13
#define BUTTON2_PIN 12

#define PUMP1_PIN 11

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;

ErrorMode errorMode = Ok;

bool backlightOn = true;

int pump1Power = 80;

unsigned long nextMinuteSampleMillis = 0;
unsigned long buttonUpdatedMillis = 0;

int button1State = LOW;
int button2State = LOW;
bool buttonStateChanging = false;

MeasuringContext measuringContext;

byte wateringMode = Idle;

void setErrorMode(ErrorMode newMode)
{
	if (newMode != errorMode) {
		lcd.clear();
		switch (newMode)
		{
			case DhtError:
				lcd.setCursor(0, 0);
				lcd.print("DHT11 ERROR");
				break;
			default:
				break;
		}
	}
	errorMode = newMode;
}

void doSampling()
{
	unsigned long currentMillis = millis();

	if (currentMillis < nextMinuteSampleMillis)
	{
		return;
	}

	int minuteIndex = getMinuteIndex();

	nextMinuteSampleMillis += 300000ul;
	putMinuteSample(0, minuteIndex, (float)measuringContext.getCurrentTemperature());
	putMinuteSample(1, minuteIndex, (float)measuringContext.getCurrentAirHumidity());
	putMinuteSample(2, minuteIndex, (float)measuringContext.getCurrentSoil());
	minuteIndex++;

	if (minuteIndex == minuteSeriesItems)
	{
		minuteIndex = 0;
		putMinuteIndex(minuteIndex);

		int hourIndex = getHourIndex();

		for (int s = 0; s < seriesInUse; s++) {
			float avgValue = 0;

			for (int i = 0; i < minuteSeriesItems; i++) {
				avgValue += getMinuteSample(s, i);
			}

			avgValue /= minuteSeriesItems;

			putHourSample(s, hourIndex, avgValue);
		}

		hourIndex++;
		if (hourIndex == 24)
		{
			hourIndex = 0;
		}
		putHourIndex(hourIndex);
	}
	else
	{
		putMinuteIndex(minuteIndex);
	}
}

MainMenu* _MainMenu = new MainMenu();
InfoRoller* _InfoRoller = new InfoRoller(&lcd, &measuringContext, _MainMenu);
HistoryRoller* _HistoryRoller = new HistoryRoller(&lcd, _MainMenu);
SettingsMenu* _Settings = new SettingsMenu(&lcd, _MainMenu);
TestMenu* _Test = new TestMenu(&lcd, _MainMenu, PUMP1_PIN);

DisplayHandler* currentHandler;

void updateButtonsWithDebounce()
{
	int button1NewState = digitalRead(BUTTON1_PIN);
	int button2NewState = digitalRead(BUTTON2_PIN);

	unsigned long currentMillis = millis();
	if (buttonStateChanging == false && (button1NewState != button1State || button2NewState != button2State))
	{
		buttonUpdatedMillis = currentMillis;
		buttonStateChanging = true;
	}
	else if (buttonStateChanging == true && currentMillis > buttonUpdatedMillis + 50)
	{
		DisplayHandler* oldCurrentHandler = currentHandler;
		DisplayHandler* newCurrentHandler;
		bool buttonPressed = false;

		if (button1State == LOW && button1NewState == HIGH)
		{
			Serial.println("1 pressed");
			newCurrentHandler = currentHandler->button1Pressed();
			buttonPressed = true;
		}

		if (button2State == LOW && button2NewState == HIGH)
		{
			Serial.println("2 pressed");
			newCurrentHandler = currentHandler->button2Pressed();
			buttonPressed = true;
		}

		if (buttonPressed == true && oldCurrentHandler != newCurrentHandler) {
			currentHandler = newCurrentHandler;
			currentHandler->activate();
		}

		button1State = button1NewState;
		button2State = button2NewState;
		buttonStateChanging = false;
	}
}

void updateBacklight() {
	BacklightMode backlightMode = getBacklightMode();

	if (backlightMode == On && backlightOn == false) {
		lcd.backlight();
		backlightOn = true;
	}
	else if (backlightMode == Off && backlightOn == true) {
		lcd.noBacklight();
		backlightOn = false;
	}
	else if (backlightMode == Auto) {
		unsigned long currentMillis = millis();
		if (currentMillis < buttonUpdatedMillis + 30000 && backlightOn == false) {
			lcd.backlight();
			backlightOn = true;
		}
		else if (currentMillis > buttonUpdatedMillis + 30000 && backlightOn == true) {
			lcd.noBacklight();
			backlightOn = false;
		}
	}
}

void updateWatering() {
	WateringSettings settings = getWateringSettings(0);
	unsigned long currentMillis = millis();
	word currentSoil = abs(measuringContext.getCurrentSoil());

	if (currentSoil < 20) {
		// moisture reading abnormal
		analogWrite(PUMP1_PIN, 0);
		return;
	}

	if (wateringMode == Idle) {
		if (settings.enabled && currentSoil < settings.moistureLimit) {
			WateringStatus status;
			status.wateringSeriesIndex = 0;
			word baseAmount = getBaseAmount(settings, 0);
			word totalAmount = calculateTargetAmount(settings, 0, baseAmount);
			word dose = totalAmount / 3;
			totalAmount = dose * 3;
			status.targetAmount = totalAmount;
			status.previousCycleMoisture = currentSoil;
			status.previousCycleStartMillis = currentMillis;
			status.dose = dose;
			putWateringStatus(status);
			storeWateringRecord(baseAmount, totalAmount, currentSoil, 0);
			Serial.println("Starting pump, lead power " + String(settings.leadPower));
			analogWrite(PUMP1_PIN, settings.leadPower);
			wateringMode = PumpRunningLead;
		}
	}
	else if (wateringMode == PumpRunningLead)
	{
		WateringStatus status = getWateringStatus();
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime) {
			Serial.println("Setting pump power " + String(settings.pumpPower));
			analogWrite(PUMP1_PIN, settings.pumpPower);
			wateringMode = PumpRunningWatering;
		}
	}
	else if (wateringMode == PumpRunningWatering) {
		WateringStatus status = getWateringStatus();
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime + status.dose) {
			analogWrite(PUMP1_PIN, 0);
			status.usedAmount = status.usedAmount + (currentMillis - status.previousCycleStartMillis - settings.leadTime);
			Serial.println("Pump stopped, usedAmount " + String(status.usedAmount));
			status.previousCycleStartMillis = currentMillis;
			putWateringStatus(status);
			if (status.usedAmount >= status.targetAmount) {
				wateringMode = Idle;
				storeWateringResult(Success);
				Serial.println("Wmode Idle");
			}
			else {
				wateringMode = Interval;
				Serial.println("Wmode Interval");
			}
		}
	}
	else if (wateringMode == Interval) {
		WateringStatus status = getWateringStatus();
		if (currentMillis > status.previousCycleStartMillis + 35000) {
			Serial.println("Interval elapsed, moisture " + String(currentSoil));
			if (currentSoil > status.previousCycleMoisture + 10 || status.phaseNumber > 0) { // only check previousCycleMoisture on 1st phase
				status.previousCycleStartMillis = currentMillis;
				status.previousCycleMoisture = currentSoil;
				status.phaseNumber = status.phaseNumber + 1;
				putWateringStatus(status);
				Serial.println("Starting pump");
				analogWrite(PUMP1_PIN, settings.pumpPower);
				wateringMode = PumpRunningWatering;
			}
			else {
				Serial.println("Moisture not increased, Wmode Idle");
				storeWateringResult(MoistureNotIncreased);
				wateringMode = Idle;
			}
		}
	}
}

void storeWateringRecord(word baseAmount, word totalAmount, word moistureAtStart, byte series) {
	WateringRecord newRecord;
	newRecord.baseAmount = baseAmount;
	newRecord.totalAmount = totalAmount;
	newRecord.moistureAtStart = measuringContext.getCurrentSoil();
	// TODO set hour for newRecord
	int index = (getWateringRecordIndex(series) + 1) % wateringSeriesItems;
	putWateringRecord(series, index, newRecord);
	putWateringRecordIndex(series, index);
}

void storeWateringResult(WateringResult result) {
	byte index = getWateringRecordIndex(0);
	WateringRecord record = getWateringRecord(0, index);
	record.result = result;
	putWateringRecord(0, index, record);
}

word getBaseAmount(WateringSettings settings, byte series) {
	WateringRecord previousRecord = getWateringRecord(series, getWateringRecordIndex(0));
	word baseAmount;
	if (previousRecord.baseAmount > 0) {
		baseAmount = previousRecord.baseAmount;
	}
	else {
		// TODO make the factor a setting
		baseAmount = settings.potSqCm * 60;
	}
	return baseAmount;
}

word calculateTargetAmount(WateringSettings settings, byte series, word baseAmount) {
	Serial.println("Base: " + String(baseAmount));

	int moistureDifference = settings.moistureLimit - measuringContext.getCurrentSoil();
	Serial.println("Mdiff: " + String(moistureDifference));
	// TODO make a setting
	float moistureDifferencePart = (float)(moistureDifference) / 200.0 * baseAmount;
	if (moistureDifferencePart > baseAmount * 0.5) {
		moistureDifferencePart = baseAmount * 0.5;
		Serial.println("Mdiff cutoff");
	}

	Serial.println("Mdiff part: " + String(moistureDifferencePart));


	// TODO make settings
	float twelveHoursAvgTemp = getNHoursAvg(0, 12);
	Serial.println("12h avg temp: " + String(twelveHoursAvgTemp));
	float tempCoefficient = 0;
	if (twelveHoursAvgTemp > 24.0) {
		tempCoefficient = (twelveHoursAvgTemp - 24.0) * 0.1;
	}
	else if (twelveHoursAvgTemp < 15.0) {
		tempCoefficient = (twelveHoursAvgTemp - 15.0) * 0.05;
	}
	Serial.println("Temp coeff: " + String(tempCoefficient));
	float tempPart = baseAmount * tempCoefficient;
	Serial.println("Temp part: " + String(tempPart));

	word totalAmount = (word)(baseAmount + moistureDifferencePart + tempPart);
	Serial.println("W/o adj: " + String(totalAmount));

	totalAmount = (word)(totalAmount * ((float)settings.adjustPercentage / 100.0));
	Serial.println("Adj amount: " + String(totalAmount));

	return totalAmount;
}

void setup()
{
	Serial.begin(9600);
	Serial.println("Setup");

	lcd.begin(16, 2);

	pinMode(MOISTURE_PIN1, OUTPUT);
	pinMode(MOISTURE_PIN2, OUTPUT);

	pinMode(PUMP1_PIN, OUTPUT);

	pinMode(BUTTON1_PIN, INPUT);
	pinMode(BUTTON2_PIN, INPUT);

	delay(10);

	for (int i = 0; i < 3; i++)
	{
		measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
		lcd.noBacklight();
		delay(250);

		measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
		lcd.backlight();
		delay(250);
	}

	measuringContext.moistureInterval = 15000;

	_MainMenu->Init(&lcd, _InfoRoller, _HistoryRoller, _Settings, _Test);
	currentHandler = _InfoRoller;

	//WateringStatus status = getWateringStatus();
	//if (status.usedAmount < status.targetAmount) {
	//	wateringMode = Interval;
	//}

	Serial.println("Done");
}

void loop()
{
	ErrorMode errorMode = measuringContext.updateDht(DHT, DHT11_PIN);
	setErrorMode(errorMode);

	measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
	updateButtonsWithDebounce();
	updateBacklight();
	updateWatering();

	if (errorMode == Ok)
	{
		doSampling();
		currentHandler->updateLcd();
	}

	delay(10);
}
