#pragma once

#include <Arduino.h>
#include "EepromInterface.h"
#include "HatchContext.h"

class TestMenu : public DisplayHandler {
private:
	const char* menuItems[5] = { "TEST UP", "TEST DOWN", "Calibrate up", "Calibrate down", "EXIT" };
	int itemIndex = 0;
	unsigned long motorTestStart = 0;
	bool motorTestRunning = false;

	DisplayHandler* _ParentDisplay = 0;
	LiquidCrystal_I2C* _lcd;
	HatchContext* _hatchContext = 0;

	int timeToRun = 0;

	int _enable;
	int _motorUp;
	int _motorDown;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
		_lcd->setCursor(0, 3);
		_lcd->print("Position " + String(getHatchPosition()) + "/5");
	};

	void startTestCommon() {
		motorTestStart = millis();
		_hatchContext->updateHatchChanged();
		motorTestRunning = true;
		digitalWrite(_enable, HIGH);
		_lcd->clear();
	}

public:
	TestMenu(LiquidCrystal_I2C* lcd, DisplayHandler* ParentDisplay, HatchContext* hatchContext, int enable, int motorUp, int motorDown) {
		_lcd = lcd;
		_ParentDisplay = ParentDisplay;
		_hatchContext = hatchContext;
		_enable = enable;
		_motorUp = motorUp;
		_motorDown = motorDown;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 5;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		int hatchPosition = getHatchPosition();
		Settings settings = getSettings();
		switch (itemIndex) {
		case 0:
			if (hatchPosition < 5) {
				startTestCommon();
				_lcd->print("TEST UP");
				digitalWrite(_motorUp, HIGH);
				timeToRun = settings.stepTimeUp;
				putHatchPosition(hatchPosition + 1);
			}
			return this;
		case 1:
			if (hatchPosition > 0) {
				startTestCommon();
				_lcd->print("TEST DOWN");
				digitalWrite(_motorDown, HIGH);
				timeToRun = settings.stepTimeDown; 
				putHatchPosition(hatchPosition - 1);
			}
			return this;
		case 2:
			digitalWrite(_motorUp, HIGH);
			digitalWrite(_enable, HIGH);
			delay(200);
			digitalWrite(_motorUp, LOW);
			digitalWrite(_enable, LOW);
			return this;
		case 3:
			digitalWrite(_motorDown, HIGH);
			digitalWrite(_enable, HIGH);
			delay(200);
			digitalWrite(_motorDown, LOW);
			digitalWrite(_enable, LOW);
			return this;
		case 4:

			return _ParentDisplay;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {
		if (motorTestRunning == true) {
			unsigned long runningTime = millis() - motorTestStart;
			_lcd->setCursor(10, 0);
			_lcd->print(String(runningTime));
			if (runningTime > timeToRun) {
				digitalWrite(_enable, LOW);
				digitalWrite(_motorUp, LOW);
				digitalWrite(_motorDown, LOW);
				motorTestRunning = false;
				printMenuOnLcd();
			}
		}
	};
};
