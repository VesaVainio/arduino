#include "EepromInterface.h"
#include "Types.h"
#include <EEPROM.h>

const int hourInfoBytes = 8; // currently 5, rest reserved
const int hourInfoStart = 10;

void putHatchPosition(byte position) {
	EEPROM.put(0, position);
}

byte getHatchPosition() {
	byte index;
	EEPROM.get(0, index);
	return index;
}

void putHourIndex(byte index) {
	EEPROM.put(2, index);
}

byte getHourIndex() {
	byte index;
	EEPROM.get(2, index);
	return index;
}

void putHourInfo(byte index, HourInfo info) {
	EEPROM.put(hourInfoStart + index * hourInfoBytes, info);
}

HourInfo getHourInfo(byte index) {
	HourInfo info;
	EEPROM.get(hourInfoStart + index * hourInfoBytes, info);
	return info;
}
