#pragma once

#include "Arduino.h"

enum BacklightMode : byte {
	Off = 0,
	Auto = 1,
	On = 2
};

enum ErrorMode : byte {
	Ok,
	DhtError
};

enum WateringMode : byte {
	Idle,
	PumpRunning,
	Interval
};

enum WateringResult : byte {
	Success,
	MoistureNotIncreased
};

// store N (e.g 10 pcs) of these per plant
struct WateringRecord // 8 bytes
{
	word hour; // cumulative running hours
	word moistureAtStart;
	word baseAmount; // base amount, not considering temp adjustment or adjustPercentage
	word totalAmount; // actual amount
	WateringResult result;
};

// settings set by user, one record for each pot/pump
struct WateringSettings // 8 bytes
{
	word moistureLimit; // moisture limit for starting watering
	word potSqCm; // cm^2 of the pot, used to calculate the initial baseAmount
	byte growthFactor; // how much should the baseAmount increase per 24h as per assumed growth of the plant (may be corrected for temp)
	byte adjustPercentage; // manual adjustment of watering amount, 100 = 100% = 1
	byte pumpPower; // the pump power for PWM, doesn't affect amounts (adjusted for different pump types, lift height etc)
	byte startHour;
	bool enabled;

	WateringSettings() {
		moistureLimit = 200;
		potSqCm = 75;
		growthFactor = 0;
		adjustPercentage = 100;
		pumpPower = 80;
		startHour = 19;
		enabled = false;
	}
};

// needed only once, as only 1 watering can be in process at once
struct WateringStatus // 11 bytes
{
	byte wateringSeriesIndex; // points to the series, 0 or 1
	word targetAmount;
	word usedAmount;
	word dose;
	word previousCycleMoisture;
	unsigned long previousCycleStartMillis;

	WateringStatus() {
		wateringSeriesIndex = 0;
		targetAmount = 0;
		usedAmount = 0;
		previousCycleMoisture = 0;
		previousCycleStartMillis = 0;
	}
};
