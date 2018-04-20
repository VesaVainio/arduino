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

void updateHatch() {
	unsigned long currentMillis = millis();
	int hatchPosition = getHatchPosition();
	int currentTemp = measuringContext->getCurrentTemperature();
	if (currentMillis > hatchChangedMillis + 3 * 60 * 1000) {
		if (currentTemp > 27 && hatchPosition < 5) {
			lcd.clear();
			lcd.print("Raising to " + String(hatchPosition + 1) + "/5");
			digitalWrite(MOTOR_ENABLE_PIN, HIGH);
			digitalWrite(MOTOR_UP_PIN, HIGH);
			delay(1500);
			digitalWrite(MOTOR_ENABLE_PIN, LOW);
			digitalWrite(MOTOR_UP_PIN, LOW);
			putHatchPosition(hatchPosition + 1);
			hatchChangedMillis = currentMillis;
			currentHandler->activate();
		}
		else if (currentTemp < 24 && hatchPosition > 0) {
			lcd.clear();
			lcd.print("Lowering to " + String(hatchPosition - 1) + "/5");
			digitalWrite(MOTOR_ENABLE_PIN, HIGH);
			digitalWrite(MOTOR_DOWN_PIN, HIGH);
			delay(1500);
			digitalWrite(MOTOR_ENABLE_PIN, LOW);
			digitalWrite(MOTOR_DOWN_PIN, LOW);
			putHatchPosition(hatchPosition - 1);
			hatchChangedMillis = currentMillis;
			currentHandler->activate();
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

	Serial.println("Setup done");
}

void loop()
{
	bool dhtError = measuringContext->updateDht(DHT, DHT11_PIN);

	updateButtonsWithDebounce();

	updateHatch();

	currentHandler->updateLcd();

	delay(20);
}
