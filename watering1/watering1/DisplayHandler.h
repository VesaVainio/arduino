#pragma once

class DisplayHandler {
public:
	virtual DisplayHandler* button1Pressed() { return this; }
	virtual DisplayHandler* button2Pressed() { return this; }
	virtual void activate() {};
	virtual void updateLcd() {};
};

