#pragma once

class WateringHistoryRoller : public DisplayHandler {
private:
	int const delayOptions[3] = { 400, 800, 1600 };
	int delayOptionIndex = 1;
	DisplayHandler* _ParentMenu;
	LiquidCrystal_I2C* _lcd;
	RTC_DS3231* _rtc;
	int _wateringSeries;

	byte dispMode;
	unsigned long lcdUpdatedMillis;
public:
	WateringHistoryRoller(LiquidCrystal_I2C* lcd, RTC_DS3231* rtc, DisplayHandler* parentMenu, int wateringSeries) {
		_lcd = lcd;
		_rtc = rtc;
		_ParentMenu = parentMenu;
		_wateringSeries = wateringSeries;
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

			int wateringIndex = getWateringRecordIndex(_wateringSeries) - dispMode;
			if (wateringIndex < 0)
			{
				wateringIndex += wateringSeriesItems;
			}

			WateringRecord record = getWateringRecord(_wateringSeries, wateringIndex);

			if (record.time == 0) {
				_lcd->print("N/A");
			}
			else {
				DateTime recordTime = DateTime(record.time);
				DateTime now = _rtc->now();
				TimeSpan timeAgo = now - recordTime;

				_lcd->print(String(timeAgo.days()) + "d" + String(timeAgo.hours()) + "h hmd " + String(record.moistureAtStart));
				_lcd->setCursor(0, 1);
				_lcd->print("ba " + String(record.baseAmount) + " ta " + String(record.totalAmount));
			}

			dispMode += 1;
			if (dispMode > wateringSeriesItems - 1)
			{
				dispMode = 0;
			}
			lcdUpdatedMillis = currentMillis;
		}
	};
};
