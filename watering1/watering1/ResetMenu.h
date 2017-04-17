#pragma once

#include "EepromInterface.h"

class ResetMenu : public DisplayHandler {
private:
	int itemIndex = 0;

	SettingsMenu* _SettingsMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	int wateringSeries = 0;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("RESET EVERYTHING");
		_lcd->setCursor(0, 1);
		_lcd->print(itemIndex);
		_lcd->print("/5 PRESS ");
		if (itemIndex % 2 == 0) {
			_lcd->print("LEFT");
		} else {
			_lcd->print("RIGHT");
		}
	};

public:
	ResetMenu(LiquidCrystal_I2C* lcd, SettingsMenu* settingsMenu) {
		_lcd = lcd;
		_SettingsMenuLocal = settingsMenu;
	}

	virtual DisplayHandler* button1Pressed() {
		if (itemIndex % 2 == 0) {
			itemIndex++;
			return this;
		}
		else {
			return _SettingsMenuLocal;
		}
	}

	virtual DisplayHandler* button2Pressed() {
		if (itemIndex % 2 == 1) {
			itemIndex++;
			if (itemIndex == 5) {
				resetEverything();
				return _SettingsMenuLocal;
			}
			return this;
		}
		else {
			return _SettingsMenuLocal;
		}
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {};

	void resetEverything() {
		putMinuteIndex(0);
		putHourIndex(0);
		putWateringRecordIndex(0, 0);
		putWateringRecordIndex(1, 0);
		putWateringRecordIndex(2, 0);
		putBacklightMode(On);
		WateringSettings settings;
		putWateringSettings(0, settings);
		putWateringSettings(1, settings);
		putWateringSettings(2, settings);
		clearAllSamples();
	}
};
