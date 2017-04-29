#pragma once

class LeadTimeTestMenu : public DisplayHandler {
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
	int _pumpPin = 0;
	int secondsLeftOnScreen = 10;

	void printMenuOnLcd() {
		WateringSettings settings = getWateringSettings(wateringSeries);
		int secondsLeft = ((lastTestStartMillis + TEST_INTERVAL) - millis()) / 1000;
		secondsLeftOnScreen = secondsLeft;

		_lcd->clear();
		_lcd->setCursor(0, 0);
		switch (mode)
		{
		case LeadTimeTestMenu::Info:
			_lcd->print("Test every 10s");
			_lcd->setCursor(0, 1);
			_lcd->print("CANCEL     START");
			break;
		case LeadTimeTestMenu::Running:
			_lcd->print("Test " + String(settings.leadTime) + " in " + String(secondsLeft) + "s");
			_lcd->setCursor(0, 1);
			_lcd->print("STOP    INCREASE");
			break;
		default:
			break;
		}
	};

public:
	LeadTimeTestMenu(LiquidCrystal_I2C* lcd, DisplayHandler* testMenu, int pumpPin) {
		_lcd = lcd;
		_TestMenuLocal = testMenu;
		_pumpPin = pumpPin;
	}

	virtual DisplayHandler* button1Pressed() {
		analogWrite(_pumpPin, 0);
		pumpRunning = false;
		return _TestMenuLocal;
	}

	virtual DisplayHandler* button2Pressed() {
		WateringSettings settings = getWateringSettings(wateringSeries);

		switch (mode)
		{
		case LeadTimeTestMenu::Info:
			mode = Running;
			return this;
			break;
		case LeadTimeTestMenu::Running:
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
			analogWrite(_pumpPin, 0);
			pumpRunning = false;
		}
		else if (pumpRunning == false && lastTestStartMillis + TEST_INTERVAL < currentMillis) {
			analogWrite(_pumpPin, settings.leadPower);
			pumpRunning = true;
			lastTestStartMillis = currentMillis;
		}
		else if (mode == Running && secondsLeft != secondsLeftOnScreen) {
			printMenuOnLcd();
		}
	};
};
