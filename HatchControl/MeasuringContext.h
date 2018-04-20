#pragma once

#include <Arduino.h>
#include <dht.h>

class MeasuringContext
{
private:
	unsigned long dhtUpdatedMillis = 0;

	int currentTemperature = 0;
	int currentHumidity = 0;

public:
	MeasuringContext() {
	}

	int getCurrentTemperature() {
		return currentTemperature;
	}

	int getCurrentAirHumidity() {
		return currentHumidity;
	}

	bool updateDht(dht DHT, int dhtPin)
	{
		unsigned long currentMillis = millis();
		bool error = false;
		if (currentMillis > dhtUpdatedMillis + 500)
		{
			int chk = DHT.read11(dhtPin);
			if (chk < 0) {
				error = true;
				Serial.println("DHT11 error");
			}
			else {
				currentTemperature = DHT.temperature;
				currentHumidity = DHT.humidity;
				//Serial.println("Temp " + String(currentTemperature) + " humid " + String(currentHumidity));
			}
			dhtUpdatedMillis = currentMillis;
		}

		return error;
	}

};
