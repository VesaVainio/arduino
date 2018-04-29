#pragma once

class MainMenu : public DisplayHandler {
private:
	char const* menuItems[4] = { "HISTORY", "SETTINGS", "TEST", "EXIT" };
	int itemIndex = 0;

	DisplayHandler* _InfoDisplayLocal = 0;
	DisplayHandler* _HistoryRollerLocal = 0;
	DisplayHandler* _SettingsLocal = 0;
	DisplayHandler* _TestLocal = 0;

	LiquidCrystal_I2C* _lcd;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("MENU");
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
	};

public:
	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 4;
		printMenuOnLcd();
		return this;
	};

	virtual DisplayHandler* button2Pressed() {
		switch (itemIndex) {
		case 0:
			return _HistoryRollerLocal;
		case 1:
			return _SettingsLocal;
		case 2:
			return _TestLocal;
		case 3:
			return _InfoDisplayLocal;
		}

		return this;
	};

	virtual void activate() {
		printMenuOnLcd();
	}

	virtual void updateLcd() { };

	void Init(LiquidCrystal_I2C* lcd, DisplayHandler* infoDisplay, DisplayHandler* historyMenu, DisplayHandler* settings, DisplayHandler* test) {
		_lcd = lcd;
		_InfoDisplayLocal = infoDisplay;
		_HistoryRollerLocal = historyMenu;
		_SettingsLocal = settings;
		_TestLocal = test;
	}
};
