#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int trigPin = 6;
const int echoPin = 7;

const int motor1enable = 10;
const int motor1forward = 9;
const int motor1backward  = 8;


const int motor2enable = 5;
const int motor2forward = 4;
const int motor2backward = 3;


unsigned long lastScreenUpdate;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

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

  analogWrite(motor1enable, 150);

  analogWrite(motor2enable, 150);


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
  
  delay(5000);
  digitalWrite(motor1forward, HIGH);
  digitalWrite(motor2forward, LOW);  
 
  delay(5000);
  digitalWrite(motor1forward, LOW);
  digitalWrite(motor2forward, HIGH);  

  delay(5000);
  digitalWrite(motor1forward, LOW);
  digitalWrite(motor2forward, LOW);  
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
