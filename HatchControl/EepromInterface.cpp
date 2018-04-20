#include "EepromInterface.h"
#include <EEPROM.h>

void putHatchPosition(byte position) {
	EEPROM.put(0, position);
}

byte getHatchPosition() {
	byte index;
	EEPROM.get(0, index);
	return index;
}