#pragma once

class MainMenu : public DisplayHandler {
private:
	char const* menuItems[5] = { "HISTORY", "SETTINGS", "TEST", "WATERING", "EXIT" };
	int itemIndex = 0;

	DisplayHandler* _InfoRollerLocal = 0;
	DisplayHandler* _HistoryMenuLocal = 0;
	DisplayHandler* _SettingsLocal = 0;
	DisplayHandler* _TestLocal = 0;
	DisplayHandler* _WateringMenuLocal = 0;

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
		itemIndex = (itemIndex + 1) % 5;
		printMenuOnLcd();
		return this;
	};

	virtual DisplayHandler* button2Pressed() {
		switch (itemIndex) {
		case 0:
			return _HistoryMenuLocal;
		case 1:
			return _SettingsLocal;
		case 2:
			return _TestLocal;
		case 3:
			return _WateringMenuLocal;
		case 4:
			return _InfoRollerLocal;
		}

		return this;
	};

	virtual void activate() {
		printMenuOnLcd();
	}

	virtual void updateLcd() { };

	void Init(LiquidCrystal_I2C* lcd, DisplayHandler* infoRoller, DisplayHandler* historyMenu, DisplayHandler* settings, DisplayHandler* test, DisplayHandler* wateringMenu) {
		_lcd = lcd;
		_InfoRollerLocal = infoRoller;
		_HistoryMenuLocal = historyMenu;
		_SettingsLocal = settings;
		_TestLocal = test;
		_WateringMenuLocal = wateringMenu;
	}
};
