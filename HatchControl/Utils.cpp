#include <Arduino.h>

String padIntNumber(int number) {
	String padding = "";

	if (number < 10) {
		padding = padding + ' ';
	}

	return padding + String(number);
}
