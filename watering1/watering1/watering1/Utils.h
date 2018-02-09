#pragma once

#include <Arduino.h>

void startPump(WateringPins(*wateringPins)[4], int index, int power) {
	if (wateringPins[index]->pumpOnOffPin != wateringPins[index]->pumpPwmPin) {
		digitalWrite(wateringPins[index]->pumpOnOffPin, HIGH);
	}

	analogWrite(wateringPins[index]->pumpPwmPin, power);
}

void stopPump(WateringPins(*wateringPins)[4], int index) {
	if (wateringPins[index]->pumpOnOffPin != wateringPins[index]->pumpPwmPin) {
		digitalWrite(wateringPins[index]->pumpOnOffPin, LOW);
	}

	analogWrite(wateringPins[index]->pumpPwmPin, 0);
}
