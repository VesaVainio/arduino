#pragma once

#include "Utils.h"

class MeasurementHistoryRoller : public DisplayHandler {
private:
	int const delayOptions[3] = { 400, 800, 1600 };
	int delayOptionIndex = 1;
	int _wateringCount;
	DisplayHandler* _ParentMenu;
	LiquidCrystal_I2C* _lcd;

	byte dispMode;
	unsigned long lcdUpdatedMillis;
public:
	MeasurementHistoryRoller(LiquidCrystal_I2C* lcd, DisplayHandler* parentMenu, int wateringCount) {
		_lcd = lcd;
		_ParentMenu = parentMenu;
		_wateringCount = wateringCount;
	}

	virtual DisplayHandler* button1Pressed() { return _ParentMenu; }
	virtual DisplayHandler* button2Pressed() { 
		delayOptionIndex = (delayOptionIndex + 1) % 3;
		return this; 
	}

	virtual void activate() {
		dispMode = 0;
		lcdUpdatedMillis = 0;
	};

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + delayOptions[delayOptionIndex])
		{
			_lcd->clear();
			_lcd->setCursor(0, 0);
			if (dispMode <= (minuteSeriesItems - 1))
			{
				int minuteIndex = getMinuteIndex() - 1 - dispMode;
				if (minuteIndex < 0)
				{
					minuteIndex += minuteSeriesItems;
				}

				int minutes = (dispMode + 1) * 10;
				if (minutes < 10)
				{
					_lcd->print(" ");
				}

				_lcd->print(String(minutes) + "m  " + padFloatNumber(getMinuteSample(0, minuteIndex), false, ' ') + "   " + padFloatNumber(getMinuteSample(1, minuteIndex), false, ' '));
				_lcd->setCursor(0, 1);
				_lcd->print("    " + padFloatNumber(getMinuteSample(2, minuteIndex), true, ' '));
				if (_wateringCount > 1) {
					_lcd->print("  " + padFloatNumber(getMinuteSample(3, minuteIndex), true, ' '));
				}
			}
			else {
				int hourIndex = getHourIndex() - 1 - (dispMode - minuteSeriesItems);
				if (hourIndex < 0)
				{
					hourIndex += 24;
				}

				int hours = (dispMode - (minuteSeriesItems - 1));
				if (hours < 10)
				{
					_lcd->print(" ");
				}

				_lcd->print(String(hours) + "h  " + padFloatNumber(getHourSample(0, hourIndex), false, ' ') + "   " + padFloatNumber(getHourSample(1, hourIndex), false, ' '));
				_lcd->setCursor(0, 1);
				_lcd->print("    " + padFloatNumber(getHourSample(2, hourIndex), true, ' '));
				if (_wateringCount > 1) {
					_lcd->print("  " + padFloatNumber(getHourSample(3, hourIndex), true, ' '));
				}
			}

			dispMode += 1;
			if (dispMode > (24 + minuteSeriesItems - 1))
			{
				dispMode = 0;
			}
			lcdUpdatedMillis = currentMillis;
		}
	};
};
