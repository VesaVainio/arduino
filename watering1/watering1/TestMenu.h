#pragma once

#include <Arduino.h>
#include "EepromInterface.h"

class TestMenu : public DisplayHandler {
private:
	const char* menuItems[2] = { "TEST PUMP ONCE", "EXIT" };
	int itemIndex = 0;
	unsigned long pumpTestStart = 0;
	bool pumpTestRunning = false;
	
	int _pump1Pin = 0;

	DisplayHandler* _MainMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
	};

public:
	TestMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu, int pump1Pin) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_pump1Pin = pump1Pin;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 2;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		int pump1Power = getWateringSettings(0).pumpPower;
		switch (itemIndex) {
		case 0:
			pumpTestStart = millis();
			analogWrite(_pump1Pin, pump1Power);
			pumpTestRunning = true;
			break;
		case 1:
			return _MainMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {
		if (pumpTestRunning == true) {
			unsigned long runningTime = millis() - pumpTestStart;
			_lcd->clear();
			_lcd->setCursor(0, 0);
			_lcd->print("PUMP 1 TEST " + String(runningTime));
			if (runningTime > 3000) {
				analogWrite(_pump1Pin, 0);
				pumpTestRunning = false;
				printMenuOnLcd();
			}
		}
	};
};
