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
#define MOTOR_DRIVE_1 3
#define MOTOR_DRIVE_2 4

#define BUTTON1_PIN 8
#define BUTTON2_PIN 9

#define DHT11_PIN 11

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
dht DHT;
MeasuringContext* measuringContext = new MeasuringContext();

InfoDisplay* _InfoDisplay = new InfoDisplay(&lcd, measuringContext);
TestMenu* _TestMenu = new TestMenu(&lcd, _InfoDisplay, MOTOR_ENABLE_PIN, MOTOR_DRIVE_1, MOTOR_DRIVE_2);

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

void setup()
{
	pinMode(MOTOR_ENABLE_PIN, OUTPUT);
	pinMode(MOTOR_DRIVE_1, OUTPUT);
	pinMode(MOTOR_DRIVE_2, OUTPUT);

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

	currentHandler->updateLcd();

	delay(20);
}
