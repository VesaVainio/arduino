#pragma once

class InfoDisplay : public DisplayHandler {
private:
	LiquidCrystal_I2C * _lcd;
	MeasuringContext* _measuringContext = 0;
	DisplayHandler* _testMenu = 0;
	unsigned long lcdUpdatedMillis;

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
			_lcd->print("tmp    " + String(_measuringContext->getCurrentTemperature()));
			_lcd->setCursor(10, 0);
			_lcd->print("hmd    " + String(_measuringContext->getCurrentAirHumidity()));

			lcdUpdatedMillis = currentMillis;
		}
	};
};
