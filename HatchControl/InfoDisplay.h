#pragma once

#include "Types.h"
#include "EepromInterface.h"

class InfoDisplay : public DisplayHandler {
private:
	LiquidCrystal_I2C * _lcd;
	MeasuringContext* _measuringContext = 0;
	DisplayHandler* _testMenu = 0;
	unsigned long lcdUpdatedMillis;
	HourInfo summary;

	String padIntNumber(int number) {
		String padding = "";

		if (number < 10) {
			padding = padding + ' ';
		}

		return padding + String(number);
	}

public:
	InfoDisplay(LiquidCrystal_I2C* lcd, MeasuringContext* measuringContext) {
		_lcd = lcd;
		_measuringContext = measuringContext;
		lcdUpdatedMillis = 0;
	}

	virtual void Init(DisplayHandler* testMenu) {
		_testMenu = testMenu;
	}

	virtual void activate() {
		_lcd->clear();
		lcdUpdatedMillis = 0;
	};

	virtual DisplayHandler* button1Pressed() {
		return _testMenu;
	}

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 2000)
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

			lcdUpdatedMillis = currentMillis;
		}
	};

	virtual void updateSummary(HourInfo newSummary) {
		summary = newSummary;
	}
};
