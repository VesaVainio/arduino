#pragma once

#include <Arduino.h>
#include <dht.h>

class MeasuringContext
{
private:
	WateringPins* wateringPins; // pointer to an array

	unsigned long dhtUpdatedMillis = 0;
	unsigned long moistureUpdatedMillis[4] = { 0, 0, 0, 0 };

	int currentTemperature = 0;
	int currentHumidity = 0;
	int currentSoilValues[4] = { 0, 0, 0, 0 };

	byte moistureReadingState = 0;
	int moistureInterval = 0;
	int wateringCount = 0;
	int index = 0;

public:
	MeasuringContext(int wateringCount_, WateringPins* pinsArray) {
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
		if (moistureReadingState == 0 && currentMillis > moistureUpdatedMillis[index] + moistureInterval)
		{
			moistureReadingState = 1;
			digitalWrite(wateringPins[index].moisturePin1, HIGH);
			digitalWrite(wateringPins[index].moisturePin2, LOW);
			moistureUpdatedMillis[index] = currentMillis;
		}
		else if (moistureReadingState == 1 && currentMillis > moistureUpdatedMillis[index] + 100)
		{
			moistureReadingState = 2;
			currentSoilValues[index] = analogRead(wateringPins[index].moistureAnalog); // actually get the reading

			digitalWrite(wateringPins[index].moisturePin1, LOW);
			digitalWrite(wateringPins[index].moisturePin2, HIGH);
			moistureUpdatedMillis[index] = currentMillis;
		}
		else if (moistureReadingState == 2 && currentMillis > moistureUpdatedMillis[index] + 100)
		{
			moistureReadingState = 0;
			digitalWrite(wateringPins[index].moisturePin1, LOW);
			digitalWrite(wateringPins[index].moisturePin1, LOW);
			moistureUpdatedMillis[index] = currentMillis;

			index += 1;
			if (index >= wateringCount) {
				index = 0;
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
