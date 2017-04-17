#include "EepromInterface.h"
#include "Types.h"
#include <EEPROM.h>


const int startOfMinuteSamples = 24; // 24 bytes for misc variables
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

void putMinuteIndex(byte index) {
	EEPROM.put(0, index);
}

byte getMinuteIndex() {
	byte index;
	EEPROM.get(0, index);
	return index;
}

void putHourIndex(byte index) {
	EEPROM.put(1, index);
}

byte getHourIndex() {
	byte index;
	EEPROM.get(1, index);
	return index;
}

BacklightMode getBacklightMode()
{
	BacklightMode mode;
	EEPROM.get(2, mode);
	return mode;
}

void putBacklightMode(BacklightMode backlightMode)
{
	EEPROM.put(2, backlightMode);
}

WateringSettings getWateringSettings(int index) {
	WateringSettings settings;
	EEPROM.get(6 + index * 8, settings);
	return settings;
}

void putWateringSettings(int index, WateringSettings settings) {
	EEPROM.put(6 + index * 8, settings);
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
