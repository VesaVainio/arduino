#pragma once

#include "EepromInterface.h"
#include "Types.h"

class SettingsMenu : public DisplayHandler {
private:
	char const* menuItems[8] = { "ENABLED", "BACKLIGHT", "TEMP LOW LIMIT", "TEMP HIGH LIMIT", "STEP TIME UP", "STEP TIME DOWN", "RESET SETTINGS", "EXIT" };
	char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };

	int itemIndex = 0;

	DisplayHandler* _MainMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	int wateringSeries = 0;
	int resetConfirms = 0;

	void printMenuOnLcd() {
		Settings settings = getSettings();
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("SETTINGS");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
		_lcd->setCursor(0, 2);
		switch (itemIndex) {
		case 0:
			_lcd->print(settings.enabled);
			break;
		case 1:
			_lcd->print(backlightOptions[settings.backlightMode]);
			break;
		case 2:
			_lcd->print(settings.tempLowLimit);
			break;
		case 3:
			_lcd->print(settings.tempHighLimit);
			break;
		case 4:
			_lcd->print(settings.stepTimeUp);
			break;
		case 5:
			_lcd->print(settings.stepTimeDown);
			break;
		}
	};

public:
	SettingsMenu(LiquidCrystal_I2C* lcd, DisplayHandler* mainMenu) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 8;
		printMenuOnLcd();
		resetConfirms = 0;
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		Settings settings = getSettings();

		switch (itemIndex) {
		case 0:
			settings.enabled = !settings.enabled;
			break;
		case 1:
			settings.backlightMode = static_cast<BacklightMode>((((byte)settings.backlightMode) + 1) % 3);
			break;
		case 2:
			settings.tempLowLimit = increaseSetting(15, 35, 1, settings.tempLowLimit);
			break;
		case 3:
			settings.tempHighLimit = increaseSetting(15, 35, 1, settings.tempHighLimit);
			break;
		case 4:
			settings.stepTimeUp = increaseSetting(500, 3000, 100, settings.stepTimeUp);
			break;
		case 5:
			settings.stepTimeDown = increaseSetting(500, 3000, 100, settings.stepTimeDown);
			break;
		case 6:
			if (resetConfirms < 3) {
				resetConfirms++;
				_lcd->setCursor(0, 3);
				_lcd->print("Press again " + String(3 - resetConfirms) + " times");
			}
			else {
				settings.enabled = true;
				settings.backlightMode = Auto;
				settings.tempLowLimit = 24;
				settings.tempHighLimit = 28;
				settings.stepTimeUp = 2000;
				settings.stepTimeDown = 1000;
			}
			
		case 7:
			return _MainMenuLocal;
		}

		putSettings(settings);
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button3Pressed() {
		Settings settings = getSettings();

		switch (itemIndex) {
		case 0:
			settings.enabled = !settings.enabled;
			break;
		case 1:
			settings.backlightMode = static_cast<BacklightMode>((((byte)settings.backlightMode) - 1) >= 0 ? (((byte)settings.backlightMode) - 1) : 2);
			break;
		case 2:
			settings.tempLowLimit = decreaseSetting(15, 35, 1, settings.tempLowLimit);
			break;
		case 3:
			settings.tempHighLimit = decreaseSetting(15, 35, 1, settings.tempHighLimit);
			break;
		case 4:
			settings.stepTimeUp = decreaseSetting(500, 3000, 100, settings.stepTimeUp);
			break;
		case 5:
			settings.stepTimeDown = decreaseSetting(500, 3000, 100, settings.stepTimeDown);
			break;
		case 6:
			return _MainMenuLocal;
		}

		putSettings(settings);
		printMenuOnLcd();
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
};
