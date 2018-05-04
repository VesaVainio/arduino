#pragma once

#include "HatchContext.h"
#include <Arduino.h>

class HatchContext
{
private:
	unsigned long _hatchChangedMillis = 0; 

public:

	unsigned long getHatchChanged() {
		return _hatchChangedMillis;
	}

	void updateHatchChanged() {
		_hatchChangedMillis = millis();
	}
};