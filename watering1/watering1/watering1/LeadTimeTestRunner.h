#pragma once

#include <Arduino.h>
#include "Utils.h"

class LeadTimeTestRunner : public DisplayHandler {
private:
	enum Mode {
		Info,
		Running
	};

	bool pumpRunning = false;

	const int TEST_INTERVAL = 5000;

	Mode mode;
	unsigned long lastTestStartMillis;

	DisplayHandler* _TestMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;

	int wateringSeries = 0;
	WateringPins* wateringPins;
	int secondsLeftOnScreen = 10;
	int indexToTest = 0;

	void printMenuOnLcd() {
		WateringSettings settings = getWateringSettings(wateringSeries);
		int secondsLeft = ((lastTestStartMillis + TEST_INTERVAL) - millis()) / 1000;
		secondsLeftOnScreen = secondsLeft;

		_lcd->clear();
		_lcd->setCursor(0, 0);
		switch (mode)
		{
		case LeadTimeTestRunner::Info:
			_lcd->print("Test every 5s");
			_lcd->setCursor(0, 1);
			_lcd->print("CANCEL     START");
			break;
		case LeadTimeTestRunner::Running:
			_lcd->print("Test " + String(settings.leadTime) + " in " + String(secondsLeft) + "s");
			_lcd->setCursor(0, 1);
			_lcd->print("STOP    INCREASE");
			break;
		default:
			break;
		}
	};

public:
	LeadTimeTestRunner(LiquidCrystal_I2C* lcd, DisplayHandler* testMenu, WateringPins* pinsArray) {
		_lcd = lcd;
		_TestMenuLocal = testMenu;
		wateringPins = pinsArray;
	}

	void setIndex(int index) {
		indexToTest = index;
	}

	virtual DisplayHandler* button1Pressed() {
		stopPump(wateringPins, indexToTest);
		pumpRunning = false;
		return _TestMenuLocal;
	}

	virtual DisplayHandler* button2Pressed() {
		WateringSettings settings = getWateringSettings(wateringSeries);

		switch (mode)
		{
		case LeadTimeTestRunner::Info:
			mode = Running;
			return this;
			break;
		case LeadTimeTestRunner::Running:
			settings.leadTime = increaseSetting(50, 2000, 50, settings.leadTime);
			break;
		default:
			break;
		}

		putWateringSettings(wateringSeries, settings);
		printMenuOnLcd();
		return this;
	}

	virtual void activate() {
		mode = Info;
		lastTestStartMillis = millis();
		printMenuOnLcd();
	};

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		WateringSettings settings = getWateringSettings(wateringSeries);
		int secondsLeft = ((lastTestStartMillis + TEST_INTERVAL) - millis()) / 1000;
		if (pumpRunning == true && lastTestStartMillis + settings.leadTime < currentMillis) {
			stopPump(wateringPins, indexToTest);
			pumpRunning = false;
		}
		else if (pumpRunning == false && lastTestStartMillis + TEST_INTERVAL < currentMillis) {
			startPump(wateringPins, indexToTest, settings.leadPower);
			pumpRunning = true;
			lastTestStartMillis = currentMillis;
		}
		else if (mode == Running && secondsLeft != secondsLeftOnScreen) {
			printMenuOnLcd();
		}
	};
};
