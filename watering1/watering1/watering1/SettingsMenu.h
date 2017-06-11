#pragma once

#include "EepromInterface.h"
#include "WateringSettingsMenu.h"
#include "ResetMenu.h"
#include "DateTimeSettingsMenu.h"

class SettingsMenu : public DisplayHandler {
private:
	char const* menuItems[6] = { "BACKLIGHT", "PLANT 1", "DATE & TIME", "RESET ALL", "EXIT" };
	char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };
	int itemIndex = 0;

	DisplayHandler* _MainMenuLocal = 0;
	DisplayHandler* _WateringSettingsMenu = 0;
	DisplayHandler* _DateTimeSettingsMenu = 0;
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
		}
	};

public:
	SettingsMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu, RTC_DS3231* rtc) {
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_WateringSettingsMenu = new WateringSettingsMenu(lcd, this);
		_DateTimeSettingsMenu = new DateTimeSettingsMenu(lcd, this, rtc);
		_ResetMenu = new ResetMenu(lcd, this);
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 5;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		BacklightMode backlightMode = getBacklightMode(); // cannot initialize inside switch

		switch (itemIndex) {
			case 0:
				backlightMode = static_cast<BacklightMode>((((byte)backlightMode) + 1) % 3);
				putBacklightMode(backlightMode);
				break;
			case 1:
				return _WateringSettingsMenu;
			case 2: 
				return _DateTimeSettingsMenu;
			case 3:
				return _ResetMenu;
			case 4:
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
