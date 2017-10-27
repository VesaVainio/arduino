#include "EepromInterface.h"
#include "Types.h"
#include <EEPROM.h>

const int startOfMinuteSamples = 200;
const int oneMinuteSeriesBytes = minuteSeriesItems * sizeof(float);

const int startOfHourSamples = startOfMinuteSamples + seriesCount * oneMinuteSeriesBytes;
const int oneHourSeriesBytes = 24 * sizeof(float);

const int wateringSeriesCount = 2;
const int wateringSeriesInUse = 1;

const int startOfWateringRecords = startOfHourSamples + seriesCount * oneHourSeriesBytes;
const int oneWateringRecordSeriesBytes = wateringSeriesItems * sizeof(WateringRecord);

const int startOfWateringSettings = 20;
const int startOfWateringStatus = 100;

void clearAllSamples() {
	for (int i = startOfMinuteSamples; i < 4096; i++) {
		EEPROM.put(i, 0);
	}
}

void putMinuteSample(int series, int index, float value)
{
	int address = startOfMinuteSamples + series * oneMinuteSeriesBytes + index * sizeof(float);
	EEPROM.put(address, value);
}

float getMinuteSample(int series, int index)
{
	int address = startOfMinuteSamples + series * oneMinuteSeriesBytes + index * sizeof(float);
	float value;
	EEPROM.get(address, value);
	return value;
}

void putHourSample(int series, int index, float value)
{
	int address = startOfHourSamples + series * oneHourSeriesBytes + index * sizeof(float);
	EEPROM.put(address, value);
}

float getHourSample(int series, int index)
{
	int address = startOfHourSamples + series * oneHourSeriesBytes + index * sizeof(float);
	float value;
	EEPROM.get(address, value);
	return value;
}

float getNHoursAvg(int series, int n)
{
	int index = getHourIndex();
	float avg = 0;
	for (int i = 0; i < n; i++)
	{
		index--;
		if (index < 0)
		{
			index = 23;
		}
		avg += getHourSample(series, index);
	}

	avg /= n;
	return avg;
}

void putWateringRecord(int series, int index, WateringRecord wateringRecord) {
	int address = startOfWateringRecords + series * oneWateringRecordSeriesBytes + index * sizeof(WateringRecord);
	EEPROM.put(address, wateringRecord);
}

WateringRecord getWateringRecord(int series, int index) {
	int address = startOfWateringRecords + series * oneWateringRecordSeriesBytes + index * sizeof(WateringRecord);
	WateringRecord record;
	EEPROM.get(address, record);
	return record;
}

void putMinuteIndex(byte index) {
	EEPROM.put(0, index);
}

byte getMinuteIndex() {
	byte index;
	EEPROM.get(0, index);

	if (index >= minuteSeriesItems) {
		return 0;
	}

	return index;
}

// reserve empty spaces for minute index, to allow it to span multiple memory location, to not exceed the write limit for one memory location so easily

void putHourIndex(byte index) {
	EEPROM.put(5, index);
}

byte getHourIndex() {
	byte index;
	EEPROM.get(5, index);

	if (index >= 24) {
		return 0;
	}

	return index;
}

byte getWateringRecordIndex(int series) {
	byte index;
	EEPROM.get(6 + series, index);
	return index;
}

void putWateringRecordIndex(int series, int index) {
	EEPROM.put(6 + (series * 2), index);
}

BacklightMode getBacklightMode()
{
	BacklightMode mode;
	EEPROM.get(14, mode);
	if (mode < 0 || mode > 2) {
		return On;
	}

	return mode;
}

void putBacklightMode(BacklightMode backlightMode)
{
	EEPROM.put(10, backlightMode);
}

WateringSettings getWateringSettings(int index) {
	WateringSettings settings;
	EEPROM.get(startOfWateringSettings + index * sizeof(WateringSettings), settings);
	return settings;
}

void putWateringSettings(int index, WateringSettings settings) {
	EEPROM.put(startOfWateringSettings + index * sizeof(WateringSettings), settings);
} // needs 3 * 9 bytes (3 * 1 bytes to spare) = 30 bytes

WateringStatus getWateringStatus(int index) {
	WateringStatus status;
	EEPROM.get(startOfWateringStatus + index * sizeof(WateringStatus), status);
	return status;
}

void putWateringStatus(int index, WateringStatus status) {
	EEPROM.put(startOfWateringStatus + index * sizeof(WateringStatus), status);
} // needs 14 bytes
