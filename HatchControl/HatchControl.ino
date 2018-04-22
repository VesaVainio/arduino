#pragma once

#include <Wire.h>
#include <Arduino.h>

#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>

#include "MeasuringContext.h"
#include "DisplayHandler.h"
#include "InfoDisplay.h"
#include "TestMenu.h"
#include "EepromInterface.h"

#define MOTOR_ENABLE_PIN 2
#define MOTOR_UP_PIN 3
#define MOTOR_DOWN_PIN 4

#define BUTTON1_PIN 8
#define BUTTON2_PIN 9

#define DHT11_PIN 11

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
dht DHT;
MeasuringContext* measuringContext = new MeasuringContext();

InfoDisplay* _InfoDisplay = new InfoDisplay(&lcd, measuringContext);
TestMenu* _TestMenu = new TestMenu(&lcd, _InfoDisplay, MOTOR_ENABLE_PIN, MOTOR_UP_PIN, MOTOR_DOWN_PIN);

DisplayHandler* currentHandler = _InfoDisplay;

unsigned long nextHourFull = 60 * 60 * 1000;
HourInfo currentHour;
HourInfo summary12h;

unsigned long buttonUpdatedMillis = 0;

int button1State = LOW;
int button2State = LOW;
bool buttonStateChanging = false;


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
			Serial.println("1 pressed at " + String(currentMillis));
			newCurrentHandler = currentHandler->button1Pressed();
			buttonPressed = true;
		}

		if (button2State == LOW && button2NewState == HIGH)
		{
			Serial.println("2 pressed at " + String(currentMillis));
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

unsigned long hatchChangedMillis = 0;

void hatchMoveCommon(int pin, int newPosition) {
	digitalWrite(MOTOR_ENABLE_PIN, HIGH);
	digitalWrite(pin, HIGH);
	delay(1500);
	digitalWrite(MOTOR_ENABLE_PIN, LOW);
	digitalWrite(pin, LOW);
	putHatchPosition(newPosition);
	hatchChangedMillis = millis();
	currentHandler->activate();
	currentHour.updateHatch(newPosition);
	currentHour.addMove();
}

void updateHatch() {
	unsigned long currentMillis = millis();
	int hatchPosition = getHatchPosition();
	int currentTemp = measuringContext->getCurrentTemperature();
	if (currentMillis > hatchChangedMillis + 3 * 60 * 1000) {
		if (currentTemp > 27 && hatchPosition < 5) {
			lcd.clear();
			lcd.print("Raising to " + String(hatchPosition + 1) + "/5");
			hatchMoveCommon(MOTOR_UP_PIN, hatchPosition + 1);
		}
		else if (currentTemp < 24 && hatchPosition > 0) {
			lcd.clear();
			lcd.print("Lowering to " + String(hatchPosition - 1) + "/5");
			hatchMoveCommon(MOTOR_DOWN_PIN, hatchPosition - 1);
		}
	}
}

void setup()
{
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);
	pinMode(MOTOR_UP_PIN, OUTPUT);
	pinMode(MOTOR_DOWN_PIN, OUTPUT);

	pinMode(BUTTON1_PIN, INPUT);
	pinMode(BUTTON2_PIN, INPUT);
	
	pinMode(DHT11_PIN, INPUT);

	Serial.begin(9600);
	Serial.println("Setup");

	lcd.begin(20, 4);

	for (int i = 0; i < 3; i++)
	{
		lcd.noBacklight();
		delay(250);

		lcd.backlight();
		delay(250);
	}

	_InfoDisplay->Init(_TestMenu);

	bool dhtError = measuringContext->updateDht(DHT, DHT11_PIN);
	int hatch = getHatchPosition();

	if (!dhtError) {
		int temp = measuringContext->getCurrentTemperature();
		currentHour = HourInfo(temp, hatch);
	}
	else {
		currentHour = HourInfo(0, hatch);
		Serial.println("DHT error during setup!");
	}

	updateSummary(getHourIndex());

	Serial.println("Setup done");
}

void updateSummary(int index) {
	summary12h = currentHour;

	for (int i = 0; i < 12; i++) {
		index -= 1;
		if (index < 0) {
			index = hourCount - 1;
		}
		HourInfo info = getHourInfo(index);
		if (info.maxTemp > summary12h.maxTemp) {
			summary12h.maxTemp = info.maxTemp;
		}
		if (info.minTemp < summary12h.minTemp) {
			summary12h.minTemp = info.minTemp;
		}
		if (info.maxHatch > summary12h.maxHatch) {
			summary12h.maxHatch = info.maxHatch;
		}
		if (info.minHatch < summary12h.minHatch) {
			summary12h.minHatch = info.minHatch;
		}
		summary12h.hatchMoves += info.hatchMoves;
	}

	_InfoDisplay->updateSummary(summary12h);
}

void updateHourInfo() {
	unsigned long currentMillis = millis();
	if (currentMillis > nextHourFull) {
		int index = getHourIndex();
		putHourInfo(index, currentHour);

		index += 1;
		if (index >= hourCount) {
			index = 0;
		}
		putHourIndex(index);

		currentHour = HourInfo(measuringContext->getCurrentTemperature(), getHatchPosition());

		nextHourFull += 60 * 60 * 1000;
	}
}

void loop()
{
	bool dhtError = measuringContext->updateDht(DHT, DHT11_PIN);
	if (!dhtError) {
		currentHour.updateTemp(measuringContext->getCurrentTemperature());
	}

	updateButtonsWithDebounce();

	updateHatch();

	updateHourInfo();

	currentHandler->updateLcd();

	delay(20);
}
