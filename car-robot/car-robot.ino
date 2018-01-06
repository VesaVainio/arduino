#include <Arduino.h>

#define LEFT 12
#define RIGHT 11
#define FORWARD 10
#define BACK 9

#define BUTTON_START 8
#define BUTTON_FORWARD 7
#define BUTTON_BACKWARD 5
#define BUTTON_LEFT 4
#define BUTTON_RIGHT 2

#define MOTOR_ENABLE 6

#define LIGHTS 3

#define NORMAL_POWER 120
#define LOWER_POWER 90

#define PHASE_MS 1000


void setup() {
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);
	pinMode(FORWARD, OUTPUT);
	pinMode(BACK, OUTPUT);

	pinMode(MOTOR_ENABLE, OUTPUT);
	pinMode(LIGHTS, OUTPUT);

	pinMode(BUTTON_START, INPUT);
	pinMode(BUTTON_FORWARD, INPUT);
	pinMode(BUTTON_BACKWARD, INPUT);
	pinMode(BUTTON_LEFT, INPUT);
	pinMode(BUTTON_RIGHT, INPUT);

	Serial.begin(9600);
}

void loop() {
	if (digitalRead(BUTTON_START) == HIGH) {
		Serial.println("Button start pressed at " + String(millis()));
		delay(500);

		//analogWrite(MOTOR_ENABLE, NORMAL_POWER);
		//Serial.println("Forward 1");
		//digitalWrite(FORWARD, HIGH);
		//delay(PHASE_MS);

		//Serial.println("Forward-left");
		//digitalWrite(LEFT, HIGH);
		//digitalWrite(RIGHT, LOW);
		//delay(PHASE_MS);

		//Serial.println("Forward 2");
		//analogWrite(MOTOR_ENABLE, LOWER_POWER);
		//digitalWrite(LEFT, LOW);
		//digitalWrite(RIGHT, LOW);
		//delay(PHASE_MS);
		//analogWrite(MOTOR_ENABLE, NORMAL_POWER);

		//Serial.println("Forward-right");
		//digitalWrite(LEFT, LOW);
		//digitalWrite(RIGHT, HIGH);
		//delay(PHASE_MS * 2);

		//Serial.println("Forward 3");
		//analogWrite(MOTOR_ENABLE, LOWER_POWER);
		//digitalWrite(LEFT, LOW);
		//digitalWrite(RIGHT, LOW);
		//delay(PHASE_MS);
		//analogWrite(MOTOR_ENABLE, NORMAL_POWER);

		//Serial.println("Forward-left 2");
		//digitalWrite(LEFT, HIGH);
		//digitalWrite(RIGHT, LOW);
		//delay(PHASE_MS);

		//Serial.println("Backward");
		//digitalWrite(LEFT, LOW);
		//digitalWrite(RIGHT, LOW);
		//digitalWrite(FORWARD, LOW);
		//digitalWrite(BACK, HIGH);
		//delay(PHASE_MS);

		//Serial.println("Stop");
		//stop();
	}

	if (digitalRead(BUTTON_FORWARD) == HIGH) {
		Serial.println("Button forward pressed at " + String(millis()));
		delay(500);/*
		analogWrite(MOTOR_ENABLE, 240);

		for (int i = 0; i < 5; i++) {
			digitalWrite(LEFT, HIGH);
			digitalWrite(RIGHT, LOW);
			digitalWrite(FORWARD, HIGH);
			digitalWrite(BACK, LOW);
			delay(750);

			digitalWrite(LEFT, LOW);
			digitalWrite(RIGHT, HIGH);
			digitalWrite(FORWARD, LOW);
			digitalWrite(BACK, HIGH);
			delay(750);
		}

		stop();*/
	}

	if (digitalRead(BUTTON_LEFT) == HIGH) {
		Serial.println("Button left pressed at " + String(millis()));
		delay(500);
	}

	if (digitalRead(BUTTON_RIGHT) == HIGH) {
		Serial.println("Button right pressed at " + String(millis()));
		delay(500);
	}

	if (digitalRead(BUTTON_BACKWARD) == HIGH) {
		Serial.println("Button backward pressed at " + String(millis()));
		delay(500);
	}

}

void stop() {
	digitalWrite(LEFT, LOW);
	digitalWrite(RIGHT, LOW);
	digitalWrite(FORWARD, LOW);
	digitalWrite(BACK, LOW);
	analogWrite(MOTOR_ENABLE, 0);
}

