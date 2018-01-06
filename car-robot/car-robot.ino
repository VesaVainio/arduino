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

#define MAX_STEPS 200

#define STEP_DURATION 750


enum StepType {
	Wait,
	Move_Forward,
	Move_Forward_Left,
	Move_Forward_Right,
	Move_Backward,
	Move_Backward_Right,
	Move_Backward_Left
};

enum Mode {
	Programming,
	Playing,
	Waiting
};

StepType steps[MAX_STEPS];

Mode currentMode = Programming;

int firstFreeIndex = 0;
int currentPlayIndex = 0;

unsigned long leftPressStart = 0;
unsigned long rightPressStart = 0;
unsigned long forwardPressStart = 0;
unsigned long backwardPressStart = 0;
unsigned long startPressStart = 0;

unsigned long stepPlayStart = 0;

boolean leftPressed = false;
boolean rightPressed = false;
boolean forwardPressed = false;
boolean backwardPressed = false;
boolean startPressed = false;

boolean leftTimeouted = false;
boolean rightTimeouted = false;

boolean programmingLightsGoingUp = true;
int programmingLightsPower = 0;
unsigned long programmingLightsChanged = 0;

boolean playLightsOn = false;
unsigned long playLightsSwitched = 0;

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

	//Serial.begin(9600);

	initSteps();
}

void initSteps() {
	steps[0] = Wait;
	firstFreeIndex = 1;
	stepPlayStart = 0;
	currentPlayIndex = 0;
}

void initPlaying(unsigned long currentMillis) {
	currentPlayIndex = 0;
	stepPlayStart = currentMillis;
	currentMode = Playing;
	playLightsOn = false;
	playLightsSwitched = 0;
}

void loop() {
	unsigned long currentMillis = millis();
	
	if (currentMode == Programming) {
		if (firstFreeIndex >= MAX_STEPS - 1) {
			initPlaying(currentMillis);
			return;
		}

		if (!leftPressed && !rightPressed && !forwardPressed && !backwardPressed) {
			if (currentMillis - programmingLightsChanged > 10) {
				if (programmingLightsGoingUp) {
					programmingLightsPower += 1;
					if (programmingLightsPower > 128) { // pulsing doesn't go to full brightness to avoid confusion with the button presses
						programmingLightsPower = 128;
						programmingLightsGoingUp = false;
					}
				}
				else {
					programmingLightsPower -= 1;
					if (programmingLightsPower < 0) {
						programmingLightsPower = 0;
						programmingLightsGoingUp = true;
					}
				}
				programmingLightsChanged = currentMillis;
				analogWrite(LIGHTS, programmingLightsPower);
			}
		} else {
			programmingLightsPower = 0;
			programmingLightsGoingUp = true;
		}

		boolean leftPressedState = digitalRead(BUTTON_LEFT) == HIGH;
		boolean rightPressedState = digitalRead(BUTTON_RIGHT) == HIGH;

		// LEFT BUTTON LOGIC
		if (!leftPressed && leftPressedState && !leftTimeouted) {
			leftPressed = true;
			digitalWrite(LIGHTS, HIGH);
			leftPressStart = currentMillis;
		}

		if (leftPressed && !leftPressedState) {
			leftPressed = false;
			digitalWrite(LIGHTS, LOW);
		}

		if (leftTimeouted && !leftPressedState) {
			leftTimeouted = false;
		}

		if (leftPressed && leftPressedState && currentMillis - leftPressStart > 2000) {
			for (int i = 0; i < 5; i++) {
				if (firstFreeIndex >= MAX_STEPS - 3) {
					initPlaying(currentMillis);
					return;
				}
				steps[firstFreeIndex++] = Move_Forward_Left;
				steps[firstFreeIndex++] = Move_Backward_Right;
			}
			leftPressed = false;
			leftTimeouted = true;
			digitalWrite(LIGHTS, LOW);
		}

		// RIGHT BUTTON LOGIC
		if (!rightPressed && rightPressedState && !rightTimeouted) {
			rightPressed = true;
			digitalWrite(LIGHTS, HIGH);
			rightPressStart = currentMillis;
		}

		if (rightPressed && !rightPressedState) {
			rightPressed = false;
			digitalWrite(LIGHTS, LOW);
		}

		if (rightTimeouted && !rightPressedState) {
			rightTimeouted = false;
		}

		if (rightPressed && rightPressedState && currentMillis - rightPressStart > 2000) {
			for (int i = 0; i < 5; i++) {
				if (firstFreeIndex >= MAX_STEPS - 3) {
					initPlaying(currentMillis);
					return;
				}
				steps[firstFreeIndex++] = Move_Forward_Right;
				steps[firstFreeIndex++] = Move_Backward_Left;
			}
			rightPressed = false;
			rightTimeouted = true;
			digitalWrite(LIGHTS, LOW);
		}

		// FORWARD BUTTON LOGIC
		if (!forwardPressed && digitalRead(BUTTON_FORWARD) == HIGH) {
			forwardPressed = true;
			digitalWrite(LIGHTS, HIGH);
			forwardPressStart = currentMillis;
			return;
		}

		if (forwardPressed && digitalRead(BUTTON_FORWARD) == LOW) {
			forwardPressed = false;
			if (currentMillis - forwardPressStart > 50) {
				if (leftPressed) {
					steps[firstFreeIndex++] = Move_Forward_Left;
					leftPressed = false; // force updating leftPressStart on next cycle to not trigger the long press timeout for left button
				}
				else if (rightPressed) {
					steps[firstFreeIndex++] = Move_Forward_Right;
					rightPressed = false;
				}
				else {
					steps[firstFreeIndex++] = Move_Forward;
					digitalWrite(LIGHTS, LOW);
				}
			}
			
			return;
		}

		// BACKWARD BUTTON LOGIC
		if (!backwardPressed && digitalRead(BUTTON_BACKWARD) == HIGH) {
			backwardPressed = true;
			digitalWrite(LIGHTS, HIGH);
			backwardPressStart = currentMillis;
			return;
		}

		if (backwardPressed && digitalRead(BUTTON_BACKWARD) == LOW) {
			backwardPressed = false;
			if (currentMillis - backwardPressStart > 50) {
				if (leftPressed) {
					steps[firstFreeIndex++] = Move_Backward_Left;
					leftPressed = false;
				}
				else if (rightPressed) {
					steps[firstFreeIndex++] = Move_Backward_Right;
					rightPressed = false;
				}
				else {
					steps[firstFreeIndex++] = Move_Backward;
					digitalWrite(LIGHTS, LOW);
				}
			}
			
			return;
		}

		// START BUTTON LOGIC
		if (!startPressed && digitalRead(BUTTON_START) == HIGH) {
			startPressed = true;
			startPressStart = currentMillis;
		}

		if (startPressed && digitalRead(BUTTON_START) == LOW) {
			startPressed = false;
			if (currentMillis - startPressStart > 50) {
				initPlaying(currentMillis);
			}
		}
	}
	else if (currentMode == Playing) { // mode: playing
		if (currentMillis - playLightsSwitched > 1000) {
			if (playLightsOn) {
				digitalWrite(LIGHTS, LOW);
				playLightsOn = false;
			}
			else {
				digitalWrite(LIGHTS, HIGH);
				playLightsOn = true;
			}
			playLightsSwitched = currentMillis;
		}

		if (currentMillis - stepPlayStart > STEP_DURATION) {
			if (currentPlayIndex + 1 == firstFreeIndex) {
				stop();
				currentMode = Waiting;
				return;
			}

			stepPlayStart += STEP_DURATION;
			currentPlayIndex++;

			if (steps[currentPlayIndex] == steps[currentPlayIndex - 1]) {
				return;
			}

			switch (steps[currentPlayIndex])
			{
			case Wait:
				digitalWrite(LEFT, LOW);
				digitalWrite(RIGHT, LOW);
				digitalWrite(FORWARD, LOW);
				digitalWrite(BACK, LOW);
				analogWrite(MOTOR_ENABLE, 0);
				break;
			case Move_Forward:
				digitalWrite(LEFT, LOW);
				digitalWrite(RIGHT, LOW);
				digitalWrite(FORWARD, HIGH);
				digitalWrite(BACK, LOW);
				analogWrite(MOTOR_ENABLE, LOWER_POWER);
				break;
			case Move_Forward_Left:
				digitalWrite(LEFT, HIGH);
				digitalWrite(RIGHT, LOW);
				digitalWrite(FORWARD, HIGH);
				digitalWrite(BACK, LOW);
				analogWrite(MOTOR_ENABLE, NORMAL_POWER);
				break;
			case Move_Forward_Right:
				digitalWrite(LEFT, LOW);
				digitalWrite(RIGHT, HIGH);
				digitalWrite(FORWARD, HIGH);
				digitalWrite(BACK, LOW);
				analogWrite(MOTOR_ENABLE, NORMAL_POWER);
				break;
			case Move_Backward:
				digitalWrite(LEFT, LOW);
				digitalWrite(RIGHT, LOW);
				digitalWrite(FORWARD, LOW);
				digitalWrite(BACK, HIGH);
				analogWrite(MOTOR_ENABLE, LOWER_POWER);
				break;
			case Move_Backward_Left:
				digitalWrite(LEFT, HIGH);
				digitalWrite(RIGHT, LOW);
				digitalWrite(FORWARD, LOW);
				digitalWrite(BACK, HIGH);
				analogWrite(MOTOR_ENABLE, NORMAL_POWER);
				break;
			case Move_Backward_Right:
				digitalWrite(LEFT, LOW);
				digitalWrite(RIGHT, HIGH);
				digitalWrite(FORWARD, LOW);
				digitalWrite(BACK, HIGH);
				analogWrite(MOTOR_ENABLE, NORMAL_POWER);
				break;
			}
		}
	}
	else if (currentMode == Waiting) {
		if (digitalRead(BUTTON_START) == HIGH) {
			initPlaying(currentMillis);
			return;
		}

		if (digitalRead(BUTTON_LEFT) == HIGH || digitalRead(BUTTON_RIGHT) == HIGH || digitalRead(BUTTON_FORWARD) == HIGH || digitalRead(BUTTON_BACKWARD) == HIGH) {
			initSteps();
			forwardPressed = false;
			backwardPressed = false;
			leftPressed = false;
			rightPressed = false;
			currentMode = Programming;
		}
	}

	delay(1);
	//if (digitalRead(BUTTON_START) == HIGH) {
	//	Serial.println("Button start pressed at " + String(millis()));
	//	delay(500);
	//}

	//if (digitalRead(BUTTON_FORWARD) == HIGH) {
	//	Serial.println("Button forward pressed at " + String(millis()));
	//	delay(500);
	//}

	//if (digitalRead(BUTTON_LEFT) == HIGH) {
	//	Serial.println("Button left pressed at " + String(millis()));
	//	delay(500);
	//}

	//if (digitalRead(BUTTON_RIGHT) == HIGH) {
	//	Serial.println("Button right pressed at " + String(millis()));
	//	delay(500);
	//}

	//if (digitalRead(BUTTON_BACKWARD) == HIGH) {
	//	Serial.println("Button backward pressed at " + String(millis()));
	//	delay(500);
	//}

}

void stop() {
	digitalWrite(LEFT, LOW);
	digitalWrite(RIGHT, LOW);
	digitalWrite(FORWARD, LOW);
	digitalWrite(BACK, LOW);
	analogWrite(MOTOR_ENABLE, 0);
}

