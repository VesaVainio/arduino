#pragma once

#include "WateringContext.h"
#include <Arduino.h>
#include <RTClib.h>
#include "Types.h"
#include "EepromInterface.h"
#include "MeasuringContext.h"
#include "Utils.h"


void WateringContext::pumpStart(int index, int power) {
	::startPump(wateringPins, index, power);
}

void WateringContext::pumpStop(int index) {
	::stopPump(wateringPins, index);
}

bool WateringContext::updateWateringForPump(int index, WateringSettings settings, bool pumpRunning) {
	unsigned long currentMillis = millis();
	word currentSoil = abs(measuringContext->getCurrentSoil(index));

	if (currentSoil < 20) {
		// moisture reading abnormal
		pumpStop(index);
		return false;
	}

	if (wateringModes[index] == Idle) {
		DateTime now = rtc->now();
		if (!pumpRunning && shouldStartWatering(index, settings, currentSoil, now)) {
			WateringStatus status;
			word baseAmount = getBaseAmount(settings, index);
			word totalAmount = calculateTargetAmount(settings, index, baseAmount);
			word dose = totalAmount / 3;
			totalAmount = dose * 3;
			status.targetAmount = totalAmount;
			status.previousCycleMoisture = currentSoil;
			status.previousCycleStartMillis = currentMillis;
			status.dose = dose;
			putWateringStatus(index, status);
			storeWateringRecord(baseAmount, totalAmount, currentSoil, index);
			Serial.println("Starting pump " + String(index + 1) + ", lead power " + String(settings.leadPower));
			wateringModes[index] = PumpRunningLead;
			pumpStart(index, settings.leadPower);
			return true;
		}
	}
	else if (wateringModes[index] == PumpRunningLead)
	{
		WateringStatus status = getWateringStatus(index);
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime) {
			Serial.println("Setting pump " + String(index + 1) + " power " + String(settings.pumpPower));
			wateringModes[index] = PumpRunningWatering;
			pumpStart(index, settings.pumpPower);
		}
		return true;
	}
	else if (wateringModes[index] == PumpRunningWatering) {
		WateringStatus status = getWateringStatus(index);
		if (currentMillis > status.previousCycleStartMillis + settings.leadTime + status.dose) {
			pumpStop(index);
			status.usedAmount = status.usedAmount + (currentMillis - status.previousCycleStartMillis - settings.leadTime);
			Serial.println("Pump " + String(index + 1) + " stopped, usedAmount " + String(status.usedAmount));
			status.previousCycleStartMillis = currentMillis;
			putWateringStatus(index, status);
			if (status.usedAmount >= status.targetAmount) {
				wateringModes[index] = Idle;
				storeWateringResult(Success, index);
				Serial.println("Wmode Idle");
			}
			else {
				wateringModes[index] = Interval;
				Serial.println("Wmode Interval");
			}
		}
		else {
			return true;
		}
	}
	else if (wateringModes[index] == Interval) {
		WateringStatus status = getWateringStatus(index);
		if (!pumpRunning && currentMillis > status.previousCycleStartMillis + 35000) {
			Serial.println("Interval elapsed, moisture " + String(currentSoil));
			if (currentSoil > status.previousCycleMoisture + 10 || status.phaseNumber > 0) { // only check previousCycleMoisture on 1st phase
				status.previousCycleStartMillis = currentMillis;
				status.previousCycleMoisture = currentSoil;
				status.phaseNumber = status.phaseNumber + 1;
				putWateringStatus(index, status);
				Serial.println("Starting pump " + String(index + 1));
				pumpStart(index, settings.leadPower);
				wateringModes[index] = PumpRunningLead;
				return true;
			}
			else {
				Serial.println("Moisture not increased, Wmode Idle");
				storeWateringResult(MoistureNotIncreased, index);
				wateringModes[index] = Idle;
			}
		}
	}

	return false;
}

bool WateringContext::shouldStartWatering(int index, WateringSettings settings, word currentSoil, DateTime now) {

	if (settings.triggerType == MoistureLimit) {
		if (currentSoil < settings.moistureLimit && !hasPreviousWateringsWithin(index, 24, 2, now)) { // max twice per 24 hours
			return true;
		}
	}
	else if (settings.triggerType == TimeOfDay) {
		if (now.hour() == settings.startHour && !hasPreviousWateringsWithin(index, 12, 1, now)) { // max once in the last 12 hours
			return true;
		}
		if (currentSoil < settings.moistureLimit && !hasPreviousWateringsWithin(index, 24, 2, now)) {
			return true;
		}
	}

	return false;
}

bool WateringContext::hasPreviousWateringsWithin(int index, int hours, int maxCount, DateTime now) {
	int recordIndex = getWateringRecordIndex(index);
	int recentCount = 0;
	unsigned long unixTimeDayAgo = now.unixtime() - hours * 60 * 60;
	for (int i = 0; i < maxCount; i++) {
		WateringRecord oldRecord = getWateringRecord(index, recordIndex);
		if (oldRecord.time > unixTimeDayAgo) {
			recentCount++;
			if (recentCount == maxCount) {
				Serial.println("Pump " + String(index) + " already has " + String(recentCount) + " waterings in last " + String(hours) + " hours");
				return true;
			}
		}
		recordIndex--;
		if (recordIndex < 0) {
			recordIndex = wateringSeriesItems - 1;
		}
	}

	return false;
}

void WateringContext::storeWateringRecord(word baseAmount, word totalAmount, word moistureAtStart, byte series) {
	WateringRecord newRecord;
	newRecord.baseAmount = baseAmount;
	newRecord.totalAmount = totalAmount;
	newRecord.moistureAtStart = measuringContext->getCurrentSoil(series);
	newRecord.time = rtc->now().unixtime();
	int index = (getWateringRecordIndex(series) + 1) % wateringSeriesItems;
	putWateringRecord(series, index, newRecord);
	putWateringRecordIndex(series, index);
}

void WateringContext::storeWateringResult(WateringResult result, byte series) {
	byte index = getWateringRecordIndex(series);
	WateringRecord record = getWateringRecord(series, index);
	record.result = result;
	putWateringRecord(series, index, record);
}

word WateringContext::getBaseAmount(WateringSettings settings, byte series) {
	WateringRecord previousRecord = getWateringRecord(series, getWateringRecordIndex(0));
	word baseAmount;
	if (previousRecord.baseAmount > 0) {
		baseAmount = previousRecord.baseAmount;
	}
	else {
		// TODO make the factor a setting
		baseAmount = settings.potSqCm * 60;
	}
	return baseAmount;
}

word WateringContext::calculateTargetAmount(WateringSettings settings, byte series, word baseAmount) {
	Serial.println("Base amount: " + String(baseAmount));

	int moistureDifference = settings.moistureLimit - measuringContext->getCurrentSoil(series);
	Serial.println("Moist diff: " + String(moistureDifference));
	// TODO make a setting
	float moistureDifferencePart = (float)(moistureDifference) / 200.0 * baseAmount;
	if (moistureDifferencePart > baseAmount * 0.5) {
		moistureDifferencePart = baseAmount * 0.5;
		Serial.println("Moist diff cutoff");
	}

	Serial.println("Moist diff part: " + String(moistureDifferencePart));


	// TODO make settings
	float twelveHoursAvgTemp = getNHoursAvg(0, 12);
	Serial.println("12h avg temp: " + String(twelveHoursAvgTemp));
	float tempCoefficient = 0;
	if (twelveHoursAvgTemp > 24.0) {
		tempCoefficient = (twelveHoursAvgTemp - 24.0) * 0.1;
	}
	else if (twelveHoursAvgTemp < 15.0) {
		tempCoefficient = (twelveHoursAvgTemp - 15.0) * 0.05;
	}
	Serial.println("Temp coeff: " + String(tempCoefficient));
	float tempPart = baseAmount * tempCoefficient;
	Serial.println("Temp part: " + String(tempPart));

	word totalAmount = (word)(baseAmount + moistureDifferencePart + tempPart);
	Serial.println("Total w/o adj: " + String(totalAmount));

	totalAmount = (word)(totalAmount * ((float)settings.adjustPercentage / 100.0));
	Serial.println("Adj amount: " + String(totalAmount));

	return totalAmount;
}

WateringContext::WateringContext(int wateringCount_, WateringPins* pinsArray, RTC_DS3231* rtc_, MeasuringContext* measuringContext_) {
	wateringCount = wateringCount_;
	wateringPins = pinsArray;
	rtc = rtc_;
	measuringContext = measuringContext_;
}

void WateringContext::updateWatering() {
	bool pumpRunning = false;
	for (int i = 0; i<wateringCount; i++) {
		WateringSettings settings = getWateringSettings(i);
		if (settings.enabled) {
			bool currentPumpRunning = updateWateringForPump(i, settings, pumpRunning);
			if (currentPumpRunning) {
				pumpRunning = true;
			}
		}
	}
}
