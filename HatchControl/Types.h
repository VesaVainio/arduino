#pragma once

#include <Arduino.h>

enum BacklightMode : byte {
	Off = 0,
	Auto = 1,
	On = 2
};

struct HourInfo
{
	byte maxTemp;
	byte minTemp;
	byte maxHatch;
	byte minHatch;
	byte hatchMoves;

	HourInfo() { }

	HourInfo(byte temp, byte hatch) {
		maxTemp = temp;
		minTemp = temp;
		maxHatch = hatch;
		minHatch = hatch;
		hatchMoves = 0;
	}

	void updateHatch(byte position) {
		if (position < minHatch) {
			minHatch = position;
		}
		else if (position > maxHatch) {
			maxHatch = position;
		}
	}

	void updateTemp(byte temp) {
		if (temp < minTemp) {
			minTemp = temp;
		}
		else if (temp > maxTemp) {
			maxTemp = temp;
		}
	}

	void addMove() {
		hatchMoves += 1;
	}
};

struct Settings
{
	bool enabled;
	BacklightMode backlightMode;
	byte tempLowLimit;
	byte tempHighLimit;
	int stepTimeUp;
	int stepTimeDown;
};
