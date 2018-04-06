#pragma once

#include <Wire.h>
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
#include "WateringMenu.h"
//#include "Utils.h"
#include "WateringContext.h"

#define DHT11_PIN 4

#define BUTTON1_PIN 29
#define BUTTON2_PIN 28
#define BUTTON3_PIN 30

int const wateringCount = 2;

WateringPins wateringPins[] = {
		{ 24, 25, 0, 38, 5 },
		{ 34, 35, 1, 39, 5 },
		{ 46, 47, 2, 42, 6 },
		{ 50, 51, 3, 43, 6 }
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
WateringContext* wateringContext = new WateringContext(wateringCount, wateringPins, &rtc, measuringContext);

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
HistoryMenu* _HistoryMenu = new HistoryMenu(&lcd, &rtc, _MainMenu, wateringCount);
SettingsMenu* _Settings = new SettingsMenu(&lcd, _MainMenu, &rtc, wateringCount);
TestMenu* _Test = new TestMenu(&lcd, _MainMenu, wateringCount, wateringPins, measuringContext);
WateringMenu* _WateringMenu = new WateringMenu(&lcd, _MainMenu, wateringCount, wateringPins, measuringContext);

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

	_MainMenu->Init(&lcd, _InfoRoller, _HistoryMenu, _Settings, _Test, _WateringMenu);
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
	wateringContext->updateWatering();

	if (errorMode == Ok)
	{
		doSampling();
		currentHandler->updateLcd();
	}

	delay(10);
}
