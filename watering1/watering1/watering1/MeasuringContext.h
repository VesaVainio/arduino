#pragma once

#include <Arduino.h>

class MeasuringContext
{
private:
	int const M_PIN1 = 0;
	int const M_PIN2 = 1;
	int const M_ANALOG = 2;
	int const MOISTURE_PINS[2][3] = { { 24, 25, 0}, { 32, 33, 2}};

	unsigned long dhtUpdatedMillis = 0;
	unsigned long moistureUpdatedMillis = 0;

	int currentTemperature = 0;
	int currentHumidity = 0;
	int currentSoilValues[2] = {0, 0};

	byte moistureReadingState = 0;
	int moistureInterval = 0;
	int wateringCount = 0;

public:
	MeasuringContext(int wateringCount_) {
		wateringCount = wateringCount_;
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
				digitalWrite(MOISTURE_PINS[i][M_PIN1], HIGH);
				digitalWrite(MOISTURE_PINS[i][M_PIN2], LOW);
				moistureUpdatedMillis = currentMillis;
			}
			else if (moistureReadingState == 1 && currentMillis > moistureUpdatedMillis + 200)
			{
				currentSoilValues[i] = analogRead(MOISTURE_PINS[i][M_ANALOG]); // actually get the reading

				moistureReadingState = 2;
				digitalWrite(MOISTURE_PINS[i][M_PIN1], LOW);
				digitalWrite(MOISTURE_PINS[i][M_PIN2], HIGH);
				moistureUpdatedMillis = currentMillis;
			}
			else if (moistureReadingState == 2 && currentMillis > moistureUpdatedMillis + 200)
			{
				moistureReadingState = 0;
				digitalWrite(MOISTURE_PINS[i][M_PIN1], LOW);
				digitalWrite(MOISTURE_PINS[i][M_PIN2], LOW);
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
