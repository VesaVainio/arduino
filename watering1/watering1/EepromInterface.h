#pragma once

#include "Types.h"
#include "Arduino.h"

const int seriesCount = 4;
const int seriesInUse = 3;
const int minuteSeriesItems = 6;

void clearAllSamples();

void putMinuteSample(int series, int index, float value);
float getMinuteSample(int series, int index);

void putHourSample(int series, int index, float value);
float getHourSample(int series, int index);

float getNHoursAvg(int series, int n);

void putMinuteIndex(byte index);
byte getMinuteIndex();

void putHourIndex(byte index);
byte getHourIndex();

byte getWateringRecordIndex(int series);
void putWateringRecordIndex(int series, int index);

BacklightMode getBacklightMode();
void putBacklightMode(BacklightMode backlightMode);

WateringSettings getWateringSettings(int index);
void putWateringSettings(int index, WateringSettings settings);

WateringStatus getWateringStatus();
void putWateringStatus(WateringStatus status);

word getCumulativeRunningHour();
void putCumulativeRunningHour(word runningHour);

byte getHourOfDay();
void putHourOfDay(byte hourOfDay);
