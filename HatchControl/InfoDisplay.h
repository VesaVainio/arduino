#pragma once

#include "Types.h"
#include "EepromInterface.h"
#include "Utils.h"

class InfoDisplay : public DisplayHandler {
private:
	LiquidCrystal_I2C * _lcd;
	MeasuringContext* _measuringContext = 0;
	DisplayHandler* _MainMenu = 0;
	unsigned long lcdUpdatedMillis;
	HourInfo summary;
	int pauseSecs = 0;

public:
	InfoDisplay(LiquidCrystal_I2C* lcd, MeasuringContext* measuringContext, DisplayHandler* mainMenu) {
		_lcd = lcd;
		_measuringContext = measuringContext;
		_MainMenu = mainMenu;
		lcdUpdatedMillis = 0;
	}

	virtual void activate() {
		_lcd->clear();
		lcdUpdatedMillis = 0;
	};

	virtual DisplayHandler* button1Pressed() {
		return _MainMenu;
	}

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 1000)
		{
			_lcd->setCursor(0, 0);
			_lcd->print("tmp " + padIntNumber(_measuringContext->getCurrentTemperature()));
			_lcd->setCursor(7, 0);
			_lcd->print("hmd " + padIntNumber(_measuringContext->getCurrentAirHumidity()));
			_lcd->setCursor(14, 0);
			_lcd->print("pos " + String(getHatchPosition()));

			_lcd->setCursor(0, 1);
			_lcd->print("12h temp  " + padIntNumber(summary.minTemp) + "-" + padIntNumber(summary.maxTemp));

			_lcd->setCursor(0, 2);
			_lcd->print("12h hatch " + padIntNumber(summary.minHatch) + "-" + padIntNumber(summary.maxHatch));

			_lcd->setCursor(0, 3);
			_lcd->print("12h moves " + padIntNumber(summary.hatchMoves));

			_lcd->setCursor(14, 3);
			if (pauseSecs > 0) {
				_lcd->print("P " + String(pauseSecs) + "s");
			}
			else {
				_lcd->print("      ");
			}

			lcdUpdatedMillis = currentMillis;
		}
	};

	virtual void updateSummary(HourInfo newSummary) {
		summary = newSummary;
	}

	virtual void setPauseSecs(int secs) {
		pauseSecs = secs;
	}
};
