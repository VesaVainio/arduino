#pragma once

#include <Arduino.h>

class MeasuringContext
{
private:
	unsigned long dhtUpdatedMillis = 0;
	unsigned long moistureUpdatedMillis = 0;

	int currentTemperature = 0;
	int currentHumidity = 0;
	int currentSoil = 0;

	byte moistureReadingState = 0;

public:

	int moistureInterval = 0;

	int getCurrentTemperature() {
		return currentTemperature;
	}

	int getCurrentAirHumidity() {
		return currentHumidity;
	}

	int getCurrentSoil() {
		return currentSoil;
	}

	void updateMoisture(int moisturePin1, int moisturePin2)
	{
		unsigned long currentMillis = millis();
		if (moistureReadingState == 0 && currentMillis > moistureUpdatedMillis + moistureInterval)
		{
			moistureReadingState = 1;
			digitalWrite(moisturePin1, HIGH);
			digitalWrite(moisturePin2, LOW);
			moistureUpdatedMillis = currentMillis;
		}
		else if (moistureReadingState == 1 && currentMillis > moistureUpdatedMillis + 200)
		{
			currentSoil = analogRead(0); // actually get the reading

			moistureReadingState = 2;
			digitalWrite(moisturePin1, LOW);
			digitalWrite(moisturePin2, HIGH);
			moistureUpdatedMillis = currentMillis;
		}
		else if (moistureReadingState == 2 && currentMillis > moistureUpdatedMillis + 200)
		{
			moistureReadingState = 0;
			digitalWrite(moisturePin1, LOW);
			digitalWrite(moisturePin2, LOW);
			moistureUpdatedMillis = currentMillis;
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

