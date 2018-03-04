#pragma once

#include <Arduino.h>
#include "LeadTimeTestRunner.h"

class LeadTimeTestMenu : public FlexMenuHandler {
private:
	const char* menuItems[2] = { "LEAD TIME PUMP", "EXIT" };

	DisplayHandler* _TestMenuLocal = 0;
	LeadTimeTestRunner* _LeadTimeTestRunner = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		int menuIndex = getMenuIndex(itemIndex);
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST LEAD TIME");
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
	LeadTimeTestMenu(LiquidCrystal_I2C* lcd, DisplayHandler* testMenu, int wateringCount_, WateringPins* pinsArray)
		: FlexMenuHandler(2, 0, wateringCount_)
	{
		_lcd = lcd;
		_TestMenuLocal = testMenu;
		_LeadTimeTestRunner = new LeadTimeTestRunner(lcd, this, pinsArray);
	}

	virtual DisplayHandler* button2Pressed() {
		int menuIndex = getMenuIndex(itemIndex);
		int pumpIndex = getFlexItemIndex(itemIndex);
		
		switch (menuIndex) {
		case 0:
			_LeadTimeTestRunner->setIndex(pumpIndex);
			return _LeadTimeTestRunner;
		case 1:
			return _TestMenuLocal;
		}

		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
};