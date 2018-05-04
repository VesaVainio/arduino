#pragma once

#include "EepromInterface.h"
#include "Utils.h"

class HistoryRoller : public DisplayHandler {
private:
	int const delayOptions[3] = { 400, 800, 1600 };
	int delayOptionIndex = 1;
	DisplayHandler* _ParentMenu;
	LiquidCrystal_I2C* _lcd;
	unsigned long lcdUpdatedMillis;
	byte infoIndex = 0;

public:
	HistoryRoller(LiquidCrystal_I2C* lcd, DisplayHandler* parentMenu) {
		_lcd = lcd;
		_ParentMenu = parentMenu;
	}

	virtual DisplayHandler* button1Pressed() { return _ParentMenu; }
	
	virtual DisplayHandler* button2Pressed() {
		delayOptionIndex = (delayOptionIndex + 1) % 3;
		return this;
	}

	virtual void activate() {
		lcdUpdatedMillis = 0;
		infoIndex = 0;
	};

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + delayOptions[delayOptionIndex])
		{
			_lcd->clear();
			_lcd->setCursor(0, 0);
			int hourIndex = getHourIndex() - 1 - infoIndex;
			if (hourIndex < 0)
			{
				hourIndex += hourCount;
			}

			if (infoIndex < 10)
			{
				_lcd->print(" ");
			}

			_lcd->print(String(infoIndex) + "h ago");

			HourInfo hourInfo = getHourInfo(hourIndex);
			_lcd->setCursor(0, 1);
			_lcd->print("Temp  " + padIntNumber(hourInfo.minTemp) + "-" + padIntNumber(hourInfo.maxTemp));

			_lcd->setCursor(0, 2);
			_lcd->print("Hatch " + padIntNumber(hourInfo.minHatch) + "-" + padIntNumber(hourInfo.maxHatch));

			_lcd->setCursor(0, 3);
			_lcd->print("Moves " + padIntNumber(hourInfo.hatchMoves));

			infoIndex += 1;
			if (infoIndex > (hourCount - 1))
			{
				infoIndex = 0;
			}
			lcdUpdatedMillis = currentMillis;
		}
	};
};
