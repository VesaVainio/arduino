#pragma once

#include <Arduino.h>
#include "Utils.h"

class TestPumpMenu : public FlexMenuHandler {
private:
	const char* menuItems[2] = { "TEST PUMP", "EXIT" };

	unsigned long pumpTestStart = 0;
	bool pumpTestRunning = false;

	WateringPins(*wateringPins)[4]; // pointer to an array

	DisplayHandler* _TestMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		int menuIndex = getMenuIndex(itemIndex);
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST PUMP");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[menuIndex]);
		_lcd->print(' ');
		switch (menuIndex) {
		case 0:
			_lcd->print(String(getFlexItemIndex(itemIndex) + 1));
			break;
		}
	};

public:
	TestPumpMenu(LiquidCrystal_I2C* lcd, DisplayHandler* testMenu, int wateringCount_, WateringPins(*pinsArray)[4])
		: FlexMenuHandler(2, 0, wateringCount_)
	{
		_lcd = lcd;
		_TestMenuLocal = testMenu;
		wateringPins = pinsArray;
	}

	virtual DisplayHandler* button2Pressed() {
		int menuIndex = getMenuIndex(itemIndex);
		int pumpIndex = getFlexItemIndex(itemIndex);
		int pumpPower = getWateringSettings(pumpIndex).pumpPower;
		switch (menuIndex) {
		case 0:
			pumpTestStart = millis();
			startPump(wateringPins, pumpIndex, pumpPower);
			pumpTestRunning = true;
			break;
		case 1:
			return _TestMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {
		if (pumpTestRunning == true) {
			unsigned long runningTime = millis() - pumpTestStart;
			int pumpIndex = getFlexItemIndex(itemIndex);
			_lcd->clear();
			_lcd->setCursor(0, 0);
			_lcd->print("PUMP " + String(pumpIndex + 1) + " TEST " + String(runningTime));
			if (runningTime > 3000) {
				stopPump(wateringPins, pumpIndex);
				pumpTestRunning = false;
				printMenuOnLcd();
			}
		}
	};
};