#pragma once

#include <Arduino.h>
#include "EepromInterface.h"

class TestMenu : public DisplayHandler {
private:
	const char* menuItems[3] = { "TEST UP", "TEST DOWN", "EXIT" };
	int itemIndex = 0;
	unsigned long motorTestStart = 0;
	bool motorTestRunning = false;

	DisplayHandler* _InfoDisplay = 0;
	LiquidCrystal_I2C* _lcd;

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
		motorTestRunning = true;
		digitalWrite(_enable, HIGH);
		_lcd->clear();
	}

public:
	TestMenu(LiquidCrystal_I2C* lcd, DisplayHandler* InfoDisplay, int enable, int motorUp, int motorDown) {
		_lcd = lcd;
		_InfoDisplay = InfoDisplay;
		_enable = enable;
		_motorUp = motorUp;
		_motorDown = motorDown;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		int hatchPosition = getHatchPosition();
		switch (itemIndex) {
		case 0:
			if (hatchPosition < 5) {
				startTestCommon();
				_lcd->print("TEST UP");
				digitalWrite(_motorUp, HIGH);
				putHatchPosition(hatchPosition + 1);
			}
			return this;
		case 1:
			if (hatchPosition > 0) {
				startTestCommon();
				_lcd->print("TEST DOWN");
				digitalWrite(_motorDown, HIGH);
				putHatchPosition(hatchPosition - 1);
			}
			return this;
		case 2:
			return _InfoDisplay;
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
			if (runningTime > 1500) {
				digitalWrite(_enable, LOW);
				digitalWrite(_motorUp, LOW);
				digitalWrite(_motorDown, LOW);
				motorTestRunning = false;
				printMenuOnLcd();
			}
		}
	};
};
