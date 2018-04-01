#pragma once

#include <Arduino.h>
#include "TriggerWateringMenu.h"
#include "ManualWateringMenu.h"

class WateringMenu : public DisplayHandler {
private:
	const char* menuItems[3] = { "Trigger manual", "Manual watering", "Exit" };
	int itemIndex = 0;
	unsigned long pumpTestStart = 0;
	bool pumpTestRunning = false;
	
	DisplayHandler* _MainMenuLocal = 0;
	DisplayHandler* _TriggerWateringMenu = 0;
	DisplayHandler* _ManualWateringMenu = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("WATERING");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
	};

public:
	WateringMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu, int wateringCount, WateringPins* pinsArray, MeasuringContext* measuringContext) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_TriggerWateringMenu = new TriggerWateringMenu(lcd, this, wateringCount, measuringContext);
		_ManualWateringMenu = new ManualWateringMenu(lcd, this, wateringCount, measuringContext);
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		switch (itemIndex) {
		case 0:
			return _TriggerWateringMenu;
		case 1:
			return _ManualWateringMenu;
		case 2:
			return _MainMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
};
