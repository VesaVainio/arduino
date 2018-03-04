#pragma once

#include <RTClib.h>
#include "EepromInterface.h"
#include "Utils.h"

class InfoRoller : public DisplayHandler {
private:
	LiquidCrystal_I2C* _lcd;
	RTC_DS3231* _rtc;
	MeasuringContext* _measuringContext;
	MainMenu* _MainMenu;

	enum Mode {
		Hours1,
		Hours6,
		Stats
	};

	Mode mode;
	unsigned long lcdUpdatedMillis;

public:
	InfoRoller(LiquidCrystal_I2C* lcd, RTC_DS3231* rtc, MeasuringContext* measuringContext, MainMenu* mainMenu) {
		_lcd = lcd;
		_rtc = rtc;
		_measuringContext = measuringContext;
		_MainMenu = mainMenu;
		mode = Hours1;
		lcdUpdatedMillis = 0;
	}

	virtual DisplayHandler* button1Pressed() {
		return _MainMenu;
	}

	virtual void activate() {
		lcdUpdatedMillis = 0;
	};

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 2000)
		{
			_lcd->setCursor(0, 0);
			_lcd->print("tmp    " + String(_measuringContext->getCurrentTemperature()));
			_lcd->setCursor(10, 0);
			_lcd->print("hmd    " + String(_measuringContext->getCurrentAirHumidity()));
			_lcd->setCursor(0, 1);
			_lcd->print("soil1 " + padIntNumber(_measuringContext->getCurrentSoil(0), true, ' '));
			_lcd->setCursor(10, 1);
			_lcd->print("soil2 " + padIntNumber(_measuringContext->getCurrentSoil(1), true, ' '));

			_lcd->setCursor(0, 2);

			if (mode == Stats)
			{
				mode = Hours1;
				printForNumHours(1);
			}
			else if (mode == Hours1)
			{
				mode = Hours6;
				printForNumHours(6);
			}
			else if (mode == Hours6)
			{
				mode = Stats;
				DateTime now = _rtc->now();
				DateTime testTime = DateTime(now.unixtime());
				_lcd->print("run " + String(currentMillis / (1000 * 60ul * 60ul)) + "h " + String((currentMillis % (1000 * 60ul * 60ul)) / (1000 * 60ul)) + "min    ");
				
				_lcd->setCursor(0, 3);
				_lcd->print("clock " + padIntNumber(testTime.hour(), false, '0') + ':' + padIntNumber(testTime.minute(), false, '0') + ':' + padIntNumber(testTime.second(), false, '0') + "  ");
			}

			lcdUpdatedMillis = currentMillis;
		}
	};

	void printForNumHours(int hours) {
		_lcd->print(String(hours) + "h  " + padFloatNumber(getNHoursAvg(0, hours), true, ' ') + " ");
		_lcd->setCursor(10, 2);
		_lcd->print(padFloatNumber(getNHoursAvg(1, hours), true, ' '));
		_lcd->setCursor(0, 3);
		_lcd->print("    " + padFloatNumber(getNHoursAvg(2, hours), true, ' ') + " ");
		_lcd->setCursor(10, 3);
		_lcd->print(padFloatNumber(getNHoursAvg(3, hours), true, ' '));
	}
};
