#pragma once

#include <Arduino.h>
#include "EepromInterface.h"
#include "LeadTimeTestMenu.h"
#include "TestPumpMenu.h"

class TestMenu : public DisplayHandler {
private:
	const char* menuItems[3] = { "TEST PUMP ONCE", "TEST LEAD TIME", "EXIT" };
	int itemIndex = 0;
	unsigned long pumpTestStart = 0;
	bool pumpTestRunning = false;
	
	DisplayHandler* _MainMenuLocal = 0;
	DisplayHandler* _LeadTimeTestMenu = 0;
	DisplayHandler* _TestPumpMenu = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
	};

public:
	TestMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu, int wateringCount, WateringPins* pinsArray) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_TestPumpMenu = new TestPumpMenu(lcd, this, wateringCount, pinsArray);
		WateringPins pins1 = pinsArray[0];
		_LeadTimeTestMenu = new LeadTimeTestMenu(lcd, this, wateringCount, pinsArray);
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		int pump1Power = getWateringSettings(0).pumpPower;
		switch (itemIndex) {
		case 0:
			return _TestPumpMenu;
		case 1:
			return _LeadTimeTestMenu;
		case 2:
			return _MainMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
};
