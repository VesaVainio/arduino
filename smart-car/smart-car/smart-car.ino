#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int trigPin = 6;
const int echoPin = 2;

const int motor1enable = 10;
const int motor1forward = 8;
const int motor1backward  = 9;

const int motor2enable = 5;
const int motor2forward = 3;
const int motor2backward = 4;

const int buttonPin = 7;

unsigned long lastScreenUpdate;

unsigned long lastTriggerMicros;
volatile unsigned long lastEchoMicros;

volatile float distance = 100.0f;
float lastDistance = 100.0f;

int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

enum Mode {
  Fast,
  Slow,
  Stop,
  Parked
};

Mode currentMotorMode = Parked;

void setup()
{
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
  pinMode(motor1enable, OUTPUT);
  pinMode(motor1forward, OUTPUT);
  pinMode(motor1backward, OUTPUT);
  pinMode(motor2enable, OUTPUT);
  pinMode(motor2forward, OUTPUT);
  pinMode(motor2backward, OUTPUT); 

  pinMode(buttonPin, INPUT);
  
  digitalWrite(motor1forward, LOW);
  digitalWrite(motor2forward, LOW);  
  digitalWrite(motor1backward, LOW);
  digitalWrite(motor2backward, LOW);
  digitalWrite(motor1enable, LOW);
  digitalWrite(motor2enable, LOW);

  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

  for (int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(250); 
    lcd.noBacklight();
    delay(250);
  }
  lcd.noBacklight(); // finish with backlight on 

  lcd.setCursor(0,0);
  lcd.print("Distance: ");

  lcd.setCursor(0,1);
  lcd.print("Parked");
  
  digitalWrite(motor1forward, HIGH);
  digitalWrite(motor2forward, HIGH);
  digitalWrite(motor1backward, LOW);
  digitalWrite(motor2backward, LOW); 

  attachInterrupt(digitalPinToInterrupt(echoPin), echoReceived, FALLING);
  //Serial.println("Motor mode: " + String(currentMotorMode));
}

void loop()
{
  unsigned long currentMicros = micros();

  updateButton(currentMicros);

  updateDistance(currentMicros);

  noInterrupts();
  float localDistance = distance;
  interrupts();

  if (localDistance != lastDistance && currentMotorMode != Parked) {
    Mode newMode;
  
    if (localDistance > 50) {
      newMode = Fast;
    } else if (localDistance > 15) {
      newMode = Slow;
    } else {
      newMode = Stop;
    }
  
    if (newMode != currentMotorMode) {
      updateMotors(newMode);
      currentMotorMode = newMode;
      Serial.println("Motor mode: " + String(currentMotorMode) + " motors updated");
    }
    lastDistance = localDistance;
  }

  updateDisplay(currentMicros, localDistance);
}

void echoReceived() {
  lastEchoMicros = micros();
  long duration = lastEchoMicros - lastTriggerMicros;
  distance = (duration * 0.034 / 2.0) - 10;
}

void updateMotors(Mode mode) {
  switch (mode) {
    case Fast: {
        /*digitalWrite(motor1enable, HIGH);
        digitalWrite(motor2enable, HIGH);*/
        analogWrite(motor1enable, 180);
        analogWrite(motor2enable, 180);
        lcd.setCursor(0,1);
        lcd.print("Fast  ");
        break;
    }
    case Slow: {
        /*digitalWrite(motor1enable, LOW);
        digitalWrite(motor2enable, LOW);*/
        analogWrite(motor1enable, 140);
        analogWrite(motor2enable, 140);
        lcd.setCursor(0,1);
        lcd.print("Slow  ");
        break;
    }
    case Stop: {
        /*digitalWrite(motor1enable, LOW);
        digitalWrite(motor2enable, LOW);*/
        analogWrite(motor1enable, 0);
        analogWrite(motor2enable, 0);
        lcd.setCursor(0,1);
        lcd.print("Stop  ");
        break;
    }
    case Parked: {
        /*digitalWrite(motor1enable, LOW);
        digitalWrite(motor2enable, LOW);*/
        analogWrite(motor1enable, 0);
        analogWrite(motor2enable, 0);
        lcd.setCursor(0,1);
        lcd.print("Parked");
        break;
    }
  }
}

void updateButton(unsigned long currentMicros) {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = currentMicros;
  }

  if ((currentMicros - lastDebounceTime) > 50) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH) {
        if (currentMotorMode == Parked) {
          currentMotorMode = Stop;
        } else {
          currentMotorMode = Parked;
          updateMotors(currentMotorMode);
        }
        Serial.println("Motor mode: " + String(currentMotorMode) + " by button");
      }
    }
  }

  lastButtonState = reading;
}

void updateDistance(unsigned long currentMicros) {
  if (currentMicros - lastTriggerMicros > 100000UL) {

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    noInterrupts();
    lastTriggerMicros = micros();
    interrupts();
  }
}

void updateDisplay(unsigned long currentMicros, float localDistance) {
  if (currentMicros - lastScreenUpdate > 500000UL) {
    // Prints the distance on the Serial Monitor
    lcd.setCursor(10,0);
    lcd.print(padFloatNumber(localDistance, true, ' ') + " ");
    //Serial.println(String(localDistance));
    lastScreenUpdate = currentMicros;
  }
}

String padFloatNumber(float number, bool padHundreds, char padChar) {
  String padding = "";
  if (padHundreds && number < 100) {
    padding = padding + padChar;
  }

  if (number < 10) {
    padding = padding + padChar;
  }

  return padding + String(number, 1);
}

