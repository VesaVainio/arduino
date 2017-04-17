#pragma once

#include "EepromInterface.h"

class SettingsMenu : public DisplayHandler {
private:
	char const* menuItems[3] = { "BACKLIGHT", "SOIL LIMIT", "EXIT" };
	char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };
	int itemIndex = 0;

	DisplayHandler* _MainMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("SET");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
		_lcd->print(' ');
		switch (itemIndex) {
		case 0:
			_lcd->print(backlightOptions[getBacklightMode()]);
		}
	};

public:
	SettingsMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		BacklightMode backlightMode = getBacklightMode(); // cannot initialize inside switch

		switch (itemIndex) {
		case 0:
			backlightMode = static_cast<BacklightMode>((((byte)backlightMode) + 1) % 3);
			putBacklightMode(backlightMode);
			printMenuOnLcd();
			break;
		case 1:
			break;
		case 2:
			return _MainMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};
	virtual void updateLcd() {};
};
