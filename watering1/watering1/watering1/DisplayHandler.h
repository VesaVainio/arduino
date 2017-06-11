#pragma once

#include <Arduino.h>

class DisplayHandler {
protected:
	word increaseSetting(word lowerLimit, word higherLimit, word step, word current) {
		current = current + step;
		if (current > higherLimit) {
			current = lowerLimit;
		}
		return current;
	}

public:
	virtual DisplayHandler* button1Pressed() { return this; }
	virtual DisplayHandler* button2Pressed() { return this; }
	virtual void activate() {};
	virtual void updateLcd() {};
};

