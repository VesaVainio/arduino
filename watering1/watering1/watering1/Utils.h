#pragma once

#include <Arduino.h>
#include "Types.h"

void startPump(WateringPins *wateringPins, int index, int power);

void stopPump(WateringPins *wateringPins, int index);

String padIntNumber(int number, bool padHundreds, char padChar);

String padFloatNumber(float number, bool padHundreds, char padChar);
