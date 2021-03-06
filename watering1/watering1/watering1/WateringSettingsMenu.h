#pragma once

#include "EepromInterface.h"
#include "Types.h"

class WateringSettingsMenu : public DisplayHandler {
private:
	char const* menuItems[9] = { "ENABLED", "MOIST LIMIT", "POT CM2", "TRIGGER", "ADJUST %", "PUMP POWER", "LEAD TIME", "HOUR", "EXIT" };
	char const* triggerTypeOptions[3] = { "Moist limit", "Time of day", "Manual" };

	int itemIndex = 0;

	DisplayHandler* _SettingsMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	int wateringSeries = 0;

	void printMenuOnLcd() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("PLANT");
		_lcd->print(' ');
		_lcd->print(String(wateringSeries + 1));
		_lcd->setCursor(0, 1);
		_lcd->print(menuItems[itemIndex]);
		_lcd->print(' ');
		switch (itemIndex) {
		case 0:
			_lcd->print(getWateringSettings(wateringSeries).enabled);
			break;
		case 1:
			_lcd->print(getWateringSettings(wateringSeries).moistureLimit);
			break;
		case 2:
			_lcd->print(getWateringSettings(wateringSeries).potSqCm);
			break;
		case 3:
			_lcd->print(triggerTypeOptions[getWateringSettings(wateringSeries).triggerType]);
			break;
		case 4:
			_lcd->print(getWateringSettings(wateringSeries).adjustPercentage);
			break;
		case 5:
			_lcd->print(getWateringSettings(wateringSeries).pumpPower);
			break;
		case 6:
			_lcd->print(getWateringSettings(wateringSeries).leadTime);
			break;
		case 7:
			_lcd->print(getWateringSettings(wateringSeries).startHour);
			break;
		}
	};

public:
	WateringSettingsMenu(LiquidCrystal_I2C* lcd, DisplayHandler* settingsMenu) {
		_lcd = lcd;
		_SettingsMenuLocal = settingsMenu;
	}

	virtual DisplayHandler* button1Pressed() {
		itemIndex = (itemIndex + 1) % 9;
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button2Pressed() {
		WateringSettings settings = getWateringSettings(wateringSeries);

		switch (itemIndex) {
		case 0:
			settings.enabled = !settings.enabled;
			break;
		case 1:
			settings.moistureLimit = increaseSetting(50, 350, 10, settings.moistureLimit);
			break;
		case 2:
			settings.potSqCm = increaseSetting(75, 1000, 25, settings.potSqCm);
			break;
		case 3:
			settings.triggerType = static_cast<TriggerType>((((byte)settings.triggerType) + 1) % 3);
			break;
		case 4:
			settings.adjustPercentage = increaseSetting(25, 200, 5, settings.adjustPercentage);
			break;
		case 5:
			settings.pumpPower = increaseSetting(50, 250, 10, settings.pumpPower);
			break;
		case 6:
			settings.leadTime = increaseSetting(50, 2000, 50, settings.leadTime);
			break;
		case 7:
			settings.startHour = increaseSetting(1, 24, 1, settings.startHour);
			break;
		case 8:
			return _SettingsMenuLocal;
		}

		putWateringSettings(wateringSeries, settings);
		printMenuOnLcd();
		return this;
	}

	virtual DisplayHandler* button3Pressed() {
		WateringSettings settings = getWateringSettings(wateringSeries);

		switch (itemIndex) {
		case 0:
			settings.enabled = !settings.enabled;
			break;
		case 1:
			settings.moistureLimit = decreaseSetting(50, 350, 10, settings.moistureLimit);
			break;
		case 2:
			settings.potSqCm = decreaseSetting(75, 1000, 25, settings.potSqCm);
			break;
		case 3:
			settings.triggerType = static_cast<TriggerType>((((byte)settings.triggerType) - 1) >= 0 ? (((byte)settings.triggerType) - 1) : 2);
			break;
		case 4:
			settings.adjustPercentage = decreaseSetting(25, 200, 5, settings.adjustPercentage);
			break;
		case 5:
			settings.pumpPower = decreaseSetting(50, 250, 10, settings.pumpPower);
			break;
		case 6:
			settings.leadTime = decreaseSetting(50, 2000, 50, settings.leadTime);
			break;
		case 7:
			settings.startHour = decreaseSetting(1, 24, 1, settings.startHour);
			break;
		case 8:
			return _SettingsMenuLocal;
		}

		putWateringSettings(wateringSeries, settings);
		printMenuOnLcd();
		return this;
	}

	virtual void activate() {
		printMenuOnLcd();
	};

	void setWateringSeries(int seriesIndex) {
		wateringSeries = seriesIndex;
	}
};
