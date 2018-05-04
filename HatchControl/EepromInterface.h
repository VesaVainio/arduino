#pragma once

#include "Types.h"
#include "Arduino.h"

const int hourCount = 24;

void putHatchPosition(byte position);
byte getHatchPosition();

void putHourIndex(byte index);
byte getHourIndex();

void putHourInfo(int index, HourInfo info);
HourInfo getHourInfo(int index);

void putSettings(Settings settings);
Settings getSettings();