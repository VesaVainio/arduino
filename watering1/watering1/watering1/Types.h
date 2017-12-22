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
	PumpRunningLead,
	PumpRunningWatering,
	Interval
};

enum WateringResult : byte {
	Success,
	MoistureNotIncreased
};

enum TriggerType : byte {
	MoistureLimit,
	TimeOfDay
};

// Results of watering
// store N (e.g 10 pcs) of these per plant
struct WateringRecord // 11 bytes
{
	long time; // unix time
	word moistureAtStart;
	word baseAmount; // base amount, not considering temp adjustment or adjustPercentage
	word totalAmount; // actual amount
	WateringResult result;
};

// Settings for determining: 1. when to trigger watering 2. the amount to pump 3. how to split the amount to doses
// settings set by user, one record for each pot/pump
struct WateringSettings // 16 bytes
{
	word moistureLimit; // moisture limit for starting watering
	word emergencyLimit; // moisture limit for starting watering at any time in TimeOfDay trigger mode
	word potSqCm; // cm^2 of the pot, used to calculate the initial baseAmount
	byte growthFactor; // how much should the baseAmount increase per 24h as per assumed growth of the plant (may be corrected for temp)
	byte adjustPercentage; // manual adjustment of watering amount, 100 = 100% = 1
	byte pumpPower; // the pump power for PWM, doesn't affect amounts (adjusted for different pump types, lift height etc)
	byte startHour;
	word leadTime; // how long to run power at start of pumping phase
	byte leadPower;
	byte doses;
	bool enabled;
	TriggerType triggerType;

	WateringSettings() {
		moistureLimit = 200;
		emergencyLimit = 140;
		potSqCm = 75;
		growthFactor = 0;
		adjustPercentage = 100;
		pumpPower = 60;
		startHour = 19;
		leadTime = 300;
		leadPower = 255;
		doses = 3;
		enabled = false;
		triggerType = MoistureLimit;
	}
};

struct WateringStatus // 14 bytes
{
	word targetAmount;
	word usedAmount;
	word dose;
	byte phaseNumber;
	word previousCycleMoisture;
	unsigned long previousCycleStartMillis;

	WateringStatus() {
		targetAmount = 0;
		usedAmount = 0;
		dose = 0;
		previousCycleMoisture = 0;
		previousCycleStartMillis = 0;
		phaseNumber = 0;
	}
};

struct WateringPins
{
	byte moisturePin1;
	byte moisturePin2;
	byte moistureAnalog;
	byte pumpOnOffPin;
	byte pumpPwmPin;

/*	WateringPins() {
		moisturePin1 = 0;
		moisturePin2 = 0;
		pumpEnablePin = 0;
		pumpPwmPin = 0;
	}*/
};
