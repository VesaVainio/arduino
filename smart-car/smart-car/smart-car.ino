#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int trigPin = 6;
const int echoPin = 7;

unsigned long lastScreenUpdate;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

void setup()
{
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

  for (int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); // finish with backlight on 

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  lcd.setCursor(0,0);
  lcd.print("Distance: ");
}

void loop()
{
  unsigned long currentMicros = micros();

  updateDistance(currentMicros);
}

void updateDistance(unsigned long currentMicros) {
  if (currentMicros - lastScreenUpdate > 500000UL) {
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    float distance = duration * 0.034 / 2.0;
    // Prints the distance on the Serial Monitor
    lcd.setCursor(10,0);
    lcd.print(padFloatNumber(distance, true, ' ') + ' ');
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

