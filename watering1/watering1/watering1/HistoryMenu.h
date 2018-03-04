#pragma once

#include "MeasurementHistoryRoller.h"
#include "WateringHistoryRoller.h"

class HistoryMenu : public DisplayHandler {
private:
	char const* menuItems[3] = { "Measurements", "Watering 1", "EXIT" };
	int itemIndex = 0;
	int _wateringCount;

	DisplayHandler* _MainMenuLocal = 0;
	DisplayHandler* _MeasurementRoller = 0;
	DisplayHandler* _Watering1Roller = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("SHOW");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
	};

public:
	HistoryMenu(LiquidCrystal_I2C* lcd, RTC_DS3231* rtc, MainMenu* mainMenu, int wateringCount) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_wateringCount = wateringCount;
		_MeasurementRoller = new MeasurementHistoryRoller(lcd, this, wateringCount);
		_Watering1Roller = new WateringHistoryRoller(lcd, rtc, this, 0);
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		switch (itemIndex) {
		case 0:
			return _MeasurementRoller;
		case 1:
			return _Watering1Roller;
		case 2:
			return _MainMenuLocal;
		}

		printMenuOnLcd();
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {};
};
