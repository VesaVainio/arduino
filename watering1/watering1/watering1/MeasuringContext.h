#pragma once

#include <Arduino.h>

class MeasuringContext
{
private:
	WateringPins (*wateringPins)[4]; // pointer to an array

	unsigned long dhtUpdatedMillis = 0;
	unsigned long moistureUpdatedMillis = 0;

	int currentTemperature = 0;
	int currentHumidity = 0;
	int currentSoilValues[2] = {0, 0};

	byte moistureReadingState = 0;
	int moistureInterval = 0;
	int wateringCount = 0;

public:
	MeasuringContext(int wateringCount_, WateringPins (*pinsArray)[4]) {
		wateringCount = wateringCount_;
		wateringPins = pinsArray;
	}

	void setMoistureInterval(int interval) {
		moistureInterval = interval;
	}

	int getCurrentTemperature() {
		return currentTemperature;
	}

	int getCurrentAirHumidity() {
		return currentHumidity;
	}

	int getCurrentSoil(int wateringIndex) {
		return currentSoilValues[wateringIndex];
	}

	void updateMoisture()
	{
		unsigned long currentMillis = millis();
		for (int i=0; i<wateringCount; i++) {
			if (moistureReadingState == 0 && currentMillis > moistureUpdatedMillis + moistureInterval)
			{
				moistureReadingState = 1;
				digitalWrite((*wateringPins)[i].moisturePin1, HIGH);
				digitalWrite((*wateringPins)[i].moisturePin2, LOW);
				moistureUpdatedMillis = currentMillis;
			}
			else if (moistureReadingState == 1 && currentMillis > moistureUpdatedMillis + 200)
			{
				currentSoilValues[i] = analogRead((*wateringPins)[i].moistureAnalog); // actually get the reading

				moistureReadingState = 2;
				digitalWrite((*wateringPins)[i].moisturePin1, LOW);
				digitalWrite((*wateringPins)[i].moisturePin2, HIGH);
				moistureUpdatedMillis = currentMillis;
			}
			else if (moistureReadingState == 2 && currentMillis > moistureUpdatedMillis + 200)
			{
				moistureReadingState = 0;
				digitalWrite((*wateringPins)[i].moisturePin1, LOW);
				digitalWrite((*wateringPins)[i].moisturePin1, LOW);
				moistureUpdatedMillis = currentMillis;
			}
		}
	}

	ErrorMode updateDht(dht DHT, int dhtPin)
	{
		unsigned long currentMillis = millis();
		ErrorMode errorMode = Ok;
		if (currentMillis > dhtUpdatedMillis + 200)
		{
			int chk = DHT.read11(dhtPin);
			if (chk < 0) {
				errorMode = DhtError;
			}
			else {
				currentTemperature = DHT.temperature;
				currentHumidity = DHT.humidity;
			}
			dhtUpdatedMillis = currentMillis;
		}

		return errorMode;
	}

};
