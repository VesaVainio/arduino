#pragma once

#include "EepromInterface.h"
#include <RTClib.h>

class InfoRoller : public DisplayHandler {
private:
	LiquidCrystal_I2C* _lcd;
	RTC_DS3231* _rtc;
	MeasuringContext* _measuringContext;
	MainMenu* _MainMenu;

	enum Mode {
		Current,
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
		mode = Current;
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
			_lcd->clear();
			_lcd->setCursor(0, 0);

			if (mode == Stats)
			{
				mode = Current;
				_lcd->print("tmp  " + String(_measuringContext->getCurrentTemperature()) + " hmd " + String(_measuringContext->getCurrentAirHumidity()));
				_lcd->setCursor(0, 1);
				_lcd->print("soil " + String(_measuringContext->getCurrentSoil()));
			}
			else if (mode == Current)
			{
				mode = Hours1;
				_lcd->print("1h " + String(getNHoursAvg(0, 1), 1) + " " + String(getNHoursAvg(1, 1), 1));
				_lcd->setCursor(0, 1);
				_lcd->print("   " + String(getNHoursAvg(2, 1), 1) + "  ");
			}
			else if (mode == Hours1)
			{
				mode = Hours6;
				_lcd->print("6h " + String(getNHoursAvg(0, 6), 1) + " " + String(getNHoursAvg(1, 6), 1));
				_lcd->setCursor(0, 1);
				_lcd->print("   " + String(getNHoursAvg(2, 6), 1));
			}
			else if (mode == Hours6)
			{
				mode = Stats;
				DateTime now = _rtc->now();
				DateTime testTime = DateTime(now.unixtime());
				_lcd->print("run " + String(currentMillis / (1000 * 60ul * 60ul)) + "h " + String((currentMillis % (1000 * 60ul * 60ul)) / (1000 * 60ul)) + "min");
				_lcd->setCursor(0, 1);
				_lcd->print(String(testTime.hour()) + ":" + String(testTime.minute()) + ":" + String(testTime.second()));
			}

			lcdUpdatedMillis = currentMillis;
		}
	};
};

