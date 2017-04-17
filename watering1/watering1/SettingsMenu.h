#pragma once

#include "EepromInterface.h"
#include "WateringSettingsMenu.h"

class SettingsMenu : public DisplayHandler {
private:
	char const* menuItems[5] = { "BACKLIGHT", "HOUR OF DAY", "PLANT 1", "RESET ALL", "EXIT" };
	char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };
	int itemIndex = 0;

	DisplayHandler* _MainMenuLocal = 0;
	DisplayHandler* _WateringSettingsMenu = 0;
	DisplayHandler* _ResetMenu = 0;
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
			break;
		case 1:
			_lcd->print(getHourOfDay());
			break;
		}
	};

public:
	SettingsMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_WateringSettingsMenu = new WateringSettingsMenu(lcd, this);
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 3;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		BacklightMode backlightMode = getBacklightMode(); // cannot initialize inside switch
		byte hourOfDay = getHourOfDay();

		switch (itemIndex) {
			case 0:
				backlightMode = static_cast<BacklightMode>((((byte)backlightMode) + 1) % 3);
				putBacklightMode(backlightMode);
				printMenuOnLcd();
				break;
			case 1:
				putHourOfDay(increaseSetting(1, 24, 1, hourOfDay));
				break;
			case 2:
				return _WateringSettingsMenu;
			case 3:
				return _ResetMenu;
			case 4:
				return _MainMenuLocal;
		}
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {};
};
