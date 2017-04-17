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

/* Misc variables to add:
- backlightMode
- hour index (persistent cumulative running hours)
- hour of day
- watering record index
- watering status?
*/

ErrorMode errorMode = Ok;

bool backlightOn = true;

int pump1Power = 80;

unsigned long nextMinuteSampleMillis = 0;
unsigned long buttonUpdatedMillis = 0;

int button1State = LOW;
int button2State = LOW;
bool buttonStateChanging = false;

MeasuringContext measuringContext;

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
TestMenu* _Test = new TestMenu(&lcd, _MainMenu, PUMP1_PIN, pump1Power);

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
			Serial.println("Button 1 pressed");
			newCurrentHandler = currentHandler->button1Pressed();
			buttonPressed = true;
		}

		if (button2State == LOW && button2NewState == HIGH)
		{
			Serial.println("Button 2 pressed");
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

void setup()
{
	Serial.begin(9600);
	Serial.println("Starting setup");

	lcd.begin(16, 2);

	pinMode(MOISTURE_PIN1, OUTPUT);
	pinMode(MOISTURE_PIN2, OUTPUT);

	pinMode(BUTTON1_PIN, INPUT);
	pinMode(BUTTON2_PIN, INPUT);

	delay(10);

	for (int i = 0; i < 3; i++)
	{
		measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
		lcd.noBacklight();
		delay(250);

		measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
		digitalWrite(MOISTURE_PIN2, HIGH);
		lcd.backlight();
		delay(250);
	}

	measuringContext.moistureInterval = 15000;

	_MainMenu->Init(&lcd, _InfoRoller, _HistoryRoller, _Settings, _Test);
	currentHandler = _InfoRoller;

	Serial.println("setup finished");
}

void loop()
{
	ErrorMode errorMode = measuringContext.updateDht(DHT, DHT11_PIN);
	setErrorMode(errorMode);

	measuringContext.updateMoisture(MOISTURE_PIN1, MOISTURE_PIN2);
	updateButtonsWithDebounce();
	updateBacklight();

	if (errorMode == Ok)
	{
		doSampling();
		currentHandler->updateLcd();
	}

	delay(10);
}
