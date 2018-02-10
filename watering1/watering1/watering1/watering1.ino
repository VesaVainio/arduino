#include <Arduino.h>

#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>

#include "EepromInterface.h"
#include "Types.h"
#include "MeasuringContext.h"

#include "DisplayHandler.h"
#include "MainMenu.h"
#include "InfoRoller.h"
#include "HistoryMenu.h"
#include "SettingsMenu.h"
#include "TestMenu.h"
#include "Utils.h"

#define DHT11_PIN 4

#define BUTTON1_PIN 29
#define BUTTON2_PIN 28
#define BUTTON3_PIN 30

int const wateringCount = 2;
WateringMode wateringModes[] = { Idle, Idle, Idle, Idle };

WateringPins wateringPins[] = {
		{ 24, 25, 0, 38, 5 },
		{ 34, 35, 1, 39, 5 },
		{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 }
};

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;
RTC_DS3231 rtc;

ErrorMode errorMode = Ok;

bool backlightOn = true;

int pump1Power = 80;

unsigned long nextMinuteSampleMillis = 0;
unsigned long buttonUpdatedMillis = 0;

int button1State = LOW;
int button2State = LOW;
int button3State = LOW;
bool buttonStateChanging = false;

MeasuringContext* measuringContext = new MeasuringContext(wateringCount, wateringPins);

void setErrorMode(ErrorMode newMode)
{
	if (newMode != errorMode) {
		lcd.clear();
		switch (newMode)
		{
			case DhtError:
				lcd.setCursor(0, 0);
				lcd.print("DHT11 ERROR " + String(newMode));
				delay(50);
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
	putMinuteSample(0, minuteIndex, (float)measuringContext->getCurrentTemperature());
	putMinuteSample(1, minuteIndex, (float)measuringContext->getCurrentAirHumidity());
	for (int i=0; i<wateringCount; i++) {
		putMinuteSample(2 + i, minuteIndex, (float)measuringContext->getCurrentSoil(i));
	}

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
InfoRoller* _InfoRoller = new InfoRoller(&lcd, &rtc, measuringContext, _MainMenu);
HistoryMenu* _HistoryMenu = new HistoryMenu(&lcd, &rtc, _MainMenu);
SettingsMenu* _Settings = new SettingsMenu(&lcd, _MainMenu, &rtc, wateringCount);
TestMenu* _Test = new TestMenu(&lcd, _MainMenu, wateringCount, wateringPins);

DisplayHandler* currentHandler;

void updateButtonsWithDebounce()
{
	int button1NewState = digitalRead(BUTTON1_PIN);
	int button2NewState = digitalRead(BUTTON2_PIN);
	int button3NewState = digitalRead(BUTTON3_PIN);

	unsigned long currentMillis = millis();
	if (buttonStateChanging == false && (button1NewState != button1State || button2NewState != button2State ||
			button3NewState != button3State))
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

		if (button3State == LOW && button3NewState == HIGH)
		{
			Serial.println("3 pressed");
			newCurrentHandler = currentHandler->button3Pressed();
			buttonPressed = true;
		}

		if (buttonPressed == true && oldCurrentHandler != newCurrentHandler) {
			currentHandler = newCurrentHandler;
			currentHandler->activate();
		}

		button1State = button1NewState;
		button2State = button2NewState;
		button3State = button3NewState;
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
	bool pumpRunning = false;
	for (int i=0; i<wateringCount; i++) {
		WateringSettings settings = getWateringSettings(i);
		if (settings.enabled) {
			bool currentPumpRunning = updateWateringForPump(i, settings, pumpRunning);
			if (currentPumpRunning) {
				pumpRunning = true;
			}
		}
	}
}

void startPump(int index, int power) {
	startPump(wateringPins, index, power);
}

void stopPump(int index) {
	stopPump(wateringPins, index);
}

bool updateWateringForPump(int index, WateringSettings settings, bool pumpRunning) {
	unsigned long currentMillis = millis();
	word currentSoil = abs(measuringContext->getCurrentSoil(index));

	if (currentSoil < 20) {
		// moisture reading abnormal
		stopPump(index);
		return false;
	}

	if (wateringModes[index] == Idle) {
		DateTime now = rtc.now();
		if (!pumpRunning && shouldStartWatering(index, settings, currentSoil, now)) {
			WateringStatus status;
			word baseAmount = getBaseAmount(settings, index);
			word totalAmount = calculateTargetAmount(settings, index, baseAmount);
			word dose = totalAmount / 3;
			totalAmount = dose * 3;
			status.targetAmount = totalAmount;
			status.previousCycleMoisture = currentSoil;
			status.previousCycleStartMillis = currentMillis;
			status.dose = dose;
			putWateringStatus(index, status);
			storeWateringRecord(baseAmount, totalAmount, currentSoil, index);
			Serial.println("Starting pump " + String(index + 1) + ", lead power " + String(settings.leadPower));
			wateringModes[index] = PumpRunningLead;
			startPump(index, settings.leadPower);
			return true;
		}
	}
	else if (wateringModes[index] == PumpRunningLead)
	{
		WateringStatus status = getWateringStatus(index);
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime) {
			Serial.println("Setting pump " + String(index + 1) + " power " + String(settings.pumpPower));
			wateringModes[index] = PumpRunningWatering;
			startPump(index, settings.pumpPower);
		}
		return true;
	}
	else if (wateringModes[index] == PumpRunningWatering) {
		WateringStatus status = getWateringStatus(index);
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime + status.dose) {
			stopPump(index);
			status.usedAmount = status.usedAmount + (currentMillis - status.previousCycleStartMillis - settings.leadTime);
			Serial.println("Pump " + String(index + 1) + " stopped, usedAmount " + String(status.usedAmount));
			status.previousCycleStartMillis = currentMillis;
			putWateringStatus(index, status);
			if (status.usedAmount >= status.targetAmount) {
				wateringModes[index] = Idle;
				storeWateringResult(Success);
				Serial.println("Wmode Idle");
			}
			else {
				wateringModes[index] = Interval;
				Serial.println("Wmode Interval");
			}
		} else {
			return true;
		}
	}
	else if (wateringModes[index] == Interval) {
		WateringStatus status = getWateringStatus(index);
		if (!pumpRunning && currentMillis > status.previousCycleStartMillis + 35000) {
			Serial.println("Interval elapsed, moisture " + String(currentSoil));
			if (currentSoil > status.previousCycleMoisture + 10 || status.phaseNumber > 0) { // only check previousCycleMoisture on 1st phase
				status.previousCycleStartMillis = currentMillis;
				status.previousCycleMoisture = currentSoil;
				status.phaseNumber = status.phaseNumber + 1;
				putWateringStatus(index, status);
				Serial.println("Starting pump " + String(index + 1));
				startPump(index, settings.leadPower);
				wateringModes[index] = PumpRunningLead;
				return true;
			}
			else {
				Serial.println("Moisture not increased, Wmode Idle");
				storeWateringResult(MoistureNotIncreased);
				wateringModes[index] = Idle;
			}
		}
	}

	return false;
}

bool shouldStartWatering(int index, WateringSettings settings, word currentSoil, DateTime now) {
	if (settings.triggerType == MoistureLimit) {
		if (currentSoil < settings.moistureLimit) {
			return true;
		}
	} else {
		if (now.hour() == settings.startHour && currentSoil < settings.moistureLimit) {
			return true;
		}
		if (currentSoil < settings.emergencyLimit) {
			return true;
		}
	}

	return false;
}

void storeWateringRecord(word baseAmount, word totalAmount, word moistureAtStart, byte series) {
	WateringRecord newRecord;
	newRecord.baseAmount = baseAmount;
	newRecord.totalAmount = totalAmount;
	newRecord.moistureAtStart = measuringContext->getCurrentSoil(series);
	newRecord.time = rtc.now().unixtime();
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
	Serial.println("Base amount: " + String(baseAmount));

	int moistureDifference = settings.moistureLimit - measuringContext->getCurrentSoil(series);
	Serial.println("Moist diff: " + String(moistureDifference));
	// TODO make a setting
	float moistureDifferencePart = (float)(moistureDifference) / 200.0 * baseAmount;
	if (moistureDifferencePart > baseAmount * 0.5) {
		moistureDifferencePart = baseAmount * 0.5;
		Serial.println("Moist diff cutoff");
	}

	Serial.println("Moist diff part: " + String(moistureDifferencePart));


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
	Serial.println("Total w/o adj: " + String(totalAmount));

	totalAmount = (word)(totalAmount * ((float)settings.adjustPercentage / 100.0));
	Serial.println("Adj amount: " + String(totalAmount));

	return totalAmount;
}

void setup()
{
	Serial.begin(9600);
	Serial.println("Setup");

	lcd.begin(20, 4);

	if (rtc.lostPower()) {
		Serial.println("RTC has lost power, initializing");
		// following line sets the RTC to the date & time this sketch was compiled
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}

	rtc.begin();

	for (int i = 0; i < wateringCount; i++) {
		Serial.println("Setting moisturePin1 " + String(wateringPins[i].moisturePin1) + " as output");
		pinMode(wateringPins[i].moisturePin1, OUTPUT);
		Serial.println("Setting moisturePin2 " + String(wateringPins[i].moisturePin2) + " as output");
		pinMode(wateringPins[i].moisturePin2, OUTPUT);
		Serial.println("Setting pumpOnOffPin " + String(wateringPins[i].pumpOnOffPin) + " as output");
		pinMode(wateringPins[i].pumpOnOffPin, OUTPUT);
		Serial.println("Setting pumpPwmPin " + String(wateringPins[i].pumpPwmPin) + " as output");
		pinMode(wateringPins[i].pumpPwmPin, OUTPUT);
	}

	pinMode(BUTTON1_PIN, INPUT);
	pinMode(BUTTON2_PIN, INPUT);
	pinMode(BUTTON3_PIN, INPUT);

	delay(10);

	for (int i = 0; i < 3; i++)
	{
		measuringContext->updateMoisture();
		lcd.noBacklight();
		delay(250);

		measuringContext->updateMoisture();
		lcd.backlight();
		delay(250);
	}

	measuringContext->setMoistureInterval(15000);

	_MainMenu->Init(&lcd, _InfoRoller, _HistoryMenu, _Settings, _Test);
	currentHandler = _InfoRoller;

	Serial.println("Done");
}

void loop()
{
	ErrorMode errorMode = measuringContext->updateDht(DHT, DHT11_PIN);
	setErrorMode(errorMode);

	measuringContext->updateMoisture();
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
