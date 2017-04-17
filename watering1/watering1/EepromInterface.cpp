#include "EepromInterface.h"
#include "Types.h"
#include <EEPROM.h>


const int startOfMinuteSamples = 60; // 24 bytes for misc variables
const int oneMinuteSeriesBytes = minuteSeriesItems * sizeof(float);

const int startOfHourSamples = startOfMinuteSamples + seriesCount * oneMinuteSeriesBytes;
const int oneHourSeriesBytes = 24 * sizeof(float);

const int wateringSeriesCount = 2;
const int wateringSeriesInUse = 1;
const int wateringSeriesItems = 10;

const int startOfWateringRecords = startOfHourSamples + seriesCount * oneHourSeriesBytes;
const int oneWateringRecordSeriesBytes = wateringSeriesItems * sizeof(WateringRecord);


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

void putMinuteIndex(byte index) {
	EEPROM.put(0, index);
}

byte getMinuteIndex() {
	byte index;
	EEPROM.get(0, index);
	return index;
}

// reserve empty spaces for minute index, to allow it to span multiple memory location, to not exceed the write limit for one memory location so easily

void putHourIndex(byte index) {
	EEPROM.put(5, index);
}

byte getHourIndex() {
	byte index;
	EEPROM.get(5, index);
	return index;
}

byte getWateringRecordIndex(int series) {
	byte index;
	EEPROM.get(6 + series, index);
	return index;
}

void putWateringRecordIndex(int series, int index) {
	EEPROM.put(6 + series, index);
}

BacklightMode getBacklightMode()
{
	BacklightMode mode;
	EEPROM.get(9, mode);
	return mode;
}

void putBacklightMode(BacklightMode backlightMode)
{
	EEPROM.put(9, backlightMode);
}

WateringSettings getWateringSettings(int index) {
	WateringSettings settings;
	EEPROM.get(10 + index * 8, settings);
	return settings;
}

void putWateringSettings(int index, WateringSettings settings) {
	EEPROM.put(10 + index * 8, settings);
} // needs 3 * 8 bytes = 24 bytes

WateringStatus getWateringStatus() {
	WateringStatus status;
	EEPROM.get(34, status);
	return status;
}

void putWateringStatus(WateringStatus status) {
	EEPROM.put(34, status);
} // nneds 11 bytes

word getCumulativeRunningHour() {
	word runningHour;
	EEPROM.get(45, runningHour);
	return runningHour;
}

void putCumulativeRunningHour(word runningHour) {
	EEPROM.put(45, runningHour);
}

word getHourOfDay() {
	word hourOfDay;
	EEPROM.get(46, hourOfDay);
	return hourOfDay;
}

void putHourOfDay(word hourOfDay) {
	EEPROM.put(46, hourOfDay);
}
