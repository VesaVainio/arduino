#pragma once

#include <Arduino.h>

void startPump(WateringPins(*wateringPins)[4], int index, int power) {
	if (wateringPins[index]->pumpOnOffPin != wateringPins[index]->pumpPwmPin) {
		int onOffPin = wateringPins[index]->pumpOnOffPin;
		Serial.println("Setting pin " + String(onOffPin) + " high");
		digitalWrite(onOffPin, HIGH);
	}

	int pwmPin = wateringPins[index]->pumpPwmPin;

	Serial.println("Setting pin " + String(pwmPin) + " to " + String(power));
	analogWrite(pwmPin, power);
}

void stopPump(WateringPins(*wateringPins)[4], int index) {
	if (wateringPins[index]->pumpOnOffPin != wateringPins[index]->pumpPwmPin) {
		digitalWrite(wateringPins[index]->pumpOnOffPin, LOW);
	}

	analogWrite(wateringPins[index]->pumpPwmPin, 0);
}
