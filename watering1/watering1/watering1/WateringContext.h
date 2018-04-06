#pragma once

#include <Arduino.h>
#include <RTClib.h>
#include "Types.h"
#include "MeasuringContext.h"

class WateringContext
{
private:
	WateringPins * wateringPins; // pointer to an array
	int wateringCount;
	RTC_DS3231* rtc;
	MeasuringContext* measuringContext;
	WateringMode wateringModes[4] = { Idle, Idle, Idle, Idle };

	void pumpStart(int index, int power);

	void pumpStop(int index);

	bool updateWateringForPump(int index, WateringSettings settings, bool pumpRunning);

	bool shouldStartWatering(int index, WateringSettings settings, word currentSoil, DateTime now);

	bool hasPreviousWateringsWithin(int index, int hours, int maxCount, DateTime now);

	void storeWateringRecord(word baseAmount, word totalAmount, word moistureAtStart, byte series);

	void storeWateringResult(WateringResult result, byte series);

	word getBaseAmount(WateringSettings settings, byte series);

	word calculateTargetAmount(WateringSettings settings, byte series, word baseAmount);


public:
	WateringContext(int wateringCount_, WateringPins* pinsArray, RTC_DS3231* rtc_, MeasuringContext* measuringContext_);

	void updateWatering();
};
