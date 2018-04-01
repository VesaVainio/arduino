#pragma once

#include <Arduino.h>
#include "EepromInterface.h"

class ManualWateringMenu : public FlexMenuHandler {
private:
	const char* menuItems[2] = { "Run pump", "Exit" };

	DisplayHandler* _WateringMenu = 0;
	LiquidCrystal_I2C* _lcd;
	MeasuringContext* _measuringContext = 0;

	void printMenuOnLcd() {
		int menuIndex = getMenuIndex(itemIndex);
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("MANUAL WATERING");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[menuIndex]);
		_lcd->print(' ');
		switch (menuIndex) {
		case 0:
			_lcd->print(String(getFlexItemIndex(itemIndex) + 1));
			_lcd->setCursor(0, 2);
			_lcd->print("Moisture " + String(_measuringContext->getCurrentSoil(getFlexItemIndex(itemIndex))));
			break;
		}
	};

public:
	ManualWateringMenu(LiquidCrystal_I2C* lcd, DisplayHandler* wateringMenu, int wateringCount, MeasuringContext* measuringContext)
		: FlexMenuHandler(2, 0, wateringCount)
	{
		_lcd = lcd;
		_WateringMenu = wateringMenu;
		_measuringContext = measuringContext;
	}

	virtual DisplayHandler* button2Pressed() {
		int menuIndex = getMenuIndex(itemIndex);
		int pumpIndex = getFlexItemIndex(itemIndex);
		switch (menuIndex) {
		case 0:
			// TODO
			break;
		case 1:
			return _WateringMenu;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
};
