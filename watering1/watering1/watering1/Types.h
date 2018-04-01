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

/* Trigger types:
 * MoistureLimit - starts watering when ever measured moisture level goes below a set limit
 * TimeOfDay     - starts watering always at a given time of day (may also specify an "emergency level" to start watering at any time, if moisture level goes below this level
 * Manual        - starts watering only manually
 */
enum TriggerType : byte {
	MoistureLimit,
	TimeOfDay,
	Manual
};

enum AmountMethod : byte {
	Automatic
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
	word potSqCm; // cm^2 of the pot, used to calculate the initial baseAmount
	byte adjustPercentage; // manual adjustment of watering amount, 100 = 100% = 1
	byte pumpPower; // the pump power for PWM, doesn't affect amounts (adjusted for different pump types, lift height etc)
	byte startHour;
	word leadTime; // how long to run power at start of pumping phase
	byte leadPower;
	byte doses;
	bool enabled;
	TriggerType triggerType;
	AmountMethod amountMethod;

	WateringSettings() {
		moistureLimit = 200;
		potSqCm = 75;
		adjustPercentage = 100;
		pumpPower = 60;
		startHour = 19;
		leadTime = 300;
		leadPower = 255;
		doses = 3;
		enabled = false;
		triggerType = Manual;
		amountMethod = Automatic;
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
};
