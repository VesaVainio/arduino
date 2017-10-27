#pragma once

#include "EepromInterface.h"

class ResetMenu : public DisplayHandler {
private:
	const int requiredClicks = 5;

	int itemIndex = 0;

	DisplayHandler* _SettingsMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	int wateringSeries = 0;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("RESET EVERYTHING");
		_lcd->setCursor(0, 1);
		_lcd->print(itemIndex);
		_lcd->print("/");
		_lcd->print(requiredClicks);
		_lcd->print(" PRESS ");
		if (itemIndex % 2 == 0) {
			_lcd->print("LEFT");
		} else {
			_lcd->print("RIGHT");
		}
	};

	void resetEverything() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("*** RESET ALL IN");
		_lcd->setCursor(0, 1);
		_lcd->print("PROGRESS ***");

		putMinuteIndex(0);
		putHourIndex(0);
		putBacklightMode(On);

		WateringSettings settings;
		WateringStatus status;

		for (int i=0; i<4; i++) {
			putWateringRecordIndex(i, 0);
			putWateringSettings(i, settings);
			putWateringStatus(i, status);
		}

		clearAllSamples();
		delay(1000);
	}

public:
	ResetMenu(LiquidCrystal_I2C* lcd, DisplayHandler* settingsMenu) {
		_lcd = lcd;
		_SettingsMenuLocal = settingsMenu;
	}

	virtual DisplayHandler* button1Pressed() {
		if (itemIndex % 2 == 0) {
			itemIndex++;
			if (itemIndex == 5) {
				resetEverything();
				return _SettingsMenuLocal;
			}
			else {
				printMenuOnLcd();
			}
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
			else {
				printMenuOnLcd();
			}
			return this;
		}
		else {
			return _SettingsMenuLocal;
		}
	}

	virtual void activate() {
		itemIndex = 0;
		printMenuOnLcd();
	};

	virtual void updateLcd() {};

};
