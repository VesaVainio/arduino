#pragma once

#include "EepromInterface.h"
#include "WateringSettingsMenu.h"
#include "ResetMenu.h"
#include "DateTimeSettingsMenu.h"
#include "FlexMenuHandler.h"

class SettingsMenu : public FlexMenuHandler {
private:
	char const* menuItems[5] = { "BACKLIGHT", "PLANT", "DATE & TIME", "RESET ALL", "EXIT" };
	char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };

	DisplayHandler* _MainMenuLocal = 0;
	WateringSettingsMenu* _WateringSettingsMenu = 0;
	DisplayHandler* _DateTimeSettingsMenu = 0;
	DisplayHandler* _ResetMenu = 0;
	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		int menuIndex = getMenuIndex(itemIndex);
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("SET");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[menuIndex]);
		_lcd->print(' ');
		switch (menuIndex) {
			case 0:
				_lcd->print(backlightOptions[getBacklightMode()]);
				break;
			case 1:
				_lcd->print(String(getFlexItemIndex(itemIndex) + 1));
				break;
		}
	};

public:
	SettingsMenu(LiquidCrystal_I2C* lcd, MainMenu* mainMenu, RTC_DS3231* rtc, int plantCount)
		: FlexMenuHandler(5, 1, plantCount)
	{
		_lcd = lcd;
		_MainMenuLocal = mainMenu;
		_WateringSettingsMenu = new WateringSettingsMenu(lcd, this);
		_DateTimeSettingsMenu = new DateTimeSettingsMenu(lcd, this, rtc);
		_ResetMenu = new ResetMenu(lcd, this);
	}

	virtual DisplayHandler* button2Pressed() {
		BacklightMode backlightMode = getBacklightMode(); // cannot initialize inside switch

		switch (getMenuIndex(itemIndex)) {
			case 0:
				backlightMode = static_cast<BacklightMode>((((byte)backlightMode) + 1) % 3);
				putBacklightMode(backlightMode);
				break;
			case 1:
				_WateringSettingsMenu->setWateringSeries(getFlexItemIndex(itemIndex));
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
