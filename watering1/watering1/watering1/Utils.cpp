#include "Utils.h"

void startPump(WateringPins *wateringPins, int index, int power) {
	if (wateringPins[index].pumpOnOffPin != wateringPins[index].pumpPwmPin) {
		int onOffPin = wateringPins[index].pumpOnOffPin;
		Serial.println("Setting pin " + String(onOffPin) + " high");
		digitalWrite(onOffPin, HIGH);
	}

	int pwmPin = wateringPins[index].pumpPwmPin;

	Serial.println("Setting pin " + String(pwmPin) + " to " + String(power));
	analogWrite(pwmPin, power);
}

void stopPump(WateringPins *wateringPins, int index) {
	if (wateringPins[index].pumpOnOffPin != wateringPins[index].pumpPwmPin) {
		digitalWrite(wateringPins[index].pumpOnOffPin, LOW);
	}

	analogWrite(wateringPins[index].pumpPwmPin, 0);
}

String padIntNumber(int number, bool padHundreds, char padChar) {
	String padding = "";
	if (padHundreds && number < 100) {
		padding = padding + padChar;
	}

	if (number < 10) {
		padding = padding + padChar;
	}

	return padding + String(number);
}

String padFloatNumber(float number, bool padHundreds, char padChar) {
	String padding = "";
	if (padHundreds && number < 100) {
		padding = padding + padChar;
	}

	if (number < 10) {
		padding = padding + padChar;
	}

	return padding + String(number, 1);
}
