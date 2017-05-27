#pragma once

#include <RTClib.h>

class DateTimeSettingsMenu : public DisplayHandler {
private:
	char const* menuItems[7] = { "Year", "Month", "Day", "Hour", "Minute", "Second", "EXIT" };
	int itemIndex = 0;

	DisplayHandler* _SettingsMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;
	RTC_DS3231* _rtc;

	void printMenuOnLcd() {
		DateTime now = _rtc->now();
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("SET");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
		_lcd->print(' ');
		switch (itemIndex) {
		case 0:
			_lcd->print(now.year());
			break;
		case 1:
			_lcd->print(now.month());
			break;
		case 2:
			_lcd->print(now.day());
			break;
		case 3:
			_lcd->print(now.hour());
			break;
		case 4:
			_lcd->print(now.minute());
			break;
		case 5:
			_lcd->print(now.second());
			break;
		}
	};

public:
	DateTimeSettingsMenu(LiquidCrystal_I2C* lcd, DisplayHandler* settingsMenu, RTC_DS3231* rtc) {
		_lcd = lcd;
		_SettingsMenuLocal = settingsMenu;
		_rtc = rtc;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 7;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		DateTime now = _rtc->now();

		switch (itemIndex) {
		case 0:
			now = DateTime(increaseSetting(2017, 2027, 1, now.year()), now.month(), now.day(), now.hour(), now.minute(), now.second());
			break;
		case 1:
			now = DateTime(now.year(), increaseSetting(1, 12, 1, now.month()), now.day(), now.hour(), now.minute(), now.second());
			break;
		case 2:
			now = DateTime(now.year(), now.month(), increaseSetting(1, 31, 1, now.day()), now.hour(), now.minute(), now.second());
			break;
		case 3:
			now = DateTime(now.year(), now.month(), now.day(), increaseSetting(0, 23, 1, now.hour()), now.minute(), now.second());
			break;
		case 4:
			now = DateTime(now.year(), now.month(), now.day(), now.hour(), increaseSetting(0, 59, 1, now.minute()), now.second());
			break;
		case 5:
			now = DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), increaseSetting(0, 59, 1, now.second()));
			break;
		case 6:
			return _SettingsMenuLocal;
		}

		_rtc->adjust(now);
		printMenuOnLcd();
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	virtual void updateLcd() {};
};
