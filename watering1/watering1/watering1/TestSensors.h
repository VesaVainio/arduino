#pragma once

#include <Arduino.h>
#include "Utils.h"

class TestSensors : public DisplayHandler {
private:
	unsigned long lastUpdated = 0;
	int _wateringCount = 0;

	DisplayHandler* _TestMenuLocal = 0;
	LiquidCrystal_I2C* _lcd;
	MeasuringContext* _measuringContext = 0;

public:
	TestSensors(LiquidCrystal_I2C* lcd, DisplayHandler* testMenu, int wateringCount_, MeasuringContext* measuringContext)
	{
		_lcd = lcd;
		_TestMenuLocal = testMenu;
		_wateringCount = wateringCount_;
		_measuringContext = measuringContext;
	}

	virtual DisplayHandler* button2Pressed() {
		_measuringContext->setMoistureInterval(15000);
		return _TestMenuLocal;
	}

	virtual void activate() {
		_lcd->clear();
		_lcd->setCursor(0, 0);
		_lcd->print("TEST SENSORS");
		_measuringContext->setMoistureInterval(1000);
	};

	virtual void updateLcd() {
		unsigned long currentMillis = millis();
		if (currentMillis > lastUpdated + 500) {
			_lcd->setCursor(0, 1);
			_lcd->print("tmp    " + String(_measuringContext->getCurrentTemperature()));
			_lcd->setCursor(10, 1);
			_lcd->print("hmd    " + String(_measuringContext->getCurrentAirHumidity()));
			_lcd->setCursor(0, 2);
			_lcd->print("soil1 " + padIntNumber(_measuringContext->getCurrentSoil(0), true, ' '));
			if (_wateringCount >= 2) {
				_lcd->setCursor(10, 2);
				_lcd->print("soil2 " + padIntNumber(_measuringContext->getCurrentSoil(1), true, ' '));
			}
			if (_wateringCount >= 3) {
				_lcd->setCursor(0, 3);
				_lcd->print("soil3 " + padIntNumber(_measuringContext->getCurrentSoil(2), true, ' '));
			}
			if (_wateringCount >= 4) {
				_lcd->setCursor(10, 3);
				_lcd->print("soil4 " + padIntNumber(_measuringContext->getCurrentSoil(3), true, ' '));
			}
			lastUpdated = currentMillis;
		}
	};
};