#include <dht.h>

dht DHT;

#define DHT11_PIN 2

#define MODE_UP 1
#define MODE_DOWN 2
#define MODE_BIDI 3

#define BUTTON_PIN 13

int pins[6];
int brightnesses[6];

int direction = 1;
int leadPin = 0;

int lastState = LOW;
int primedForChange = 0;
unsigned long lastChangeTime;

int mode = MODE_BIDI;

void setup() {
  Serial.begin(9600);
  
  pins[0] = 3;
  pins[1] = 5;
  pins[2] = 6;
  pins[3] = 9;
  pins[4] = 10;
  pins[5] = 11;
  
  for (int i=0; i < 6; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void loop() {
  int chk = DHT.read11(DHT11_PIN);

  int state = digitalRead(BUTTON_PIN);
  unsigned long currentMillis = millis();
  if (state == HIGH && lastState == LOW) {
    lastChangeTime = currentMillis;
    primedForChange = 1;
  }
  if (state == HIGH && primedForChange == 1 && currentMillis - lastChangeTime > 50)
  {
    mode++;
    if (mode == 4) {
      mode = 1;
    }

    if (mode == MODE_DOWN) {
      direction = -1;
    }
    if (mode == MODE_UP) {
      direction = 1;
    }
    
    primedForChange = 0;
  }
  lastState = state;

  int temperature = DHT.temperature;
  Serial.print("temperature ");
  Serial.print(temperature);

  int change = (temperature - 20) * 4;

  Serial.print(" change ");
  Serial.println(change);

  
  brightnesses[leadPin] += change;
  if (brightnesses[leadPin] > 255) {
    brightnesses[leadPin] = 255;
    leadPin += direction;
  }

  if (leadPin == -1) {
    if (mode == MODE_DOWN) {
      leadPin = 5;
    } else {
      leadPin = 1;
      direction = 1;
    }
  } else if (leadPin == 6) {
    if (mode == MODE_UP) {
      leadPin = 0;
    } else {
      leadPin = 5;
      direction = -1;
    }
  }

  for (int i = 0; i < 6; i++) {
    if (i != leadPin) {
      brightnesses[i] -= change / 2;
      if (brightnesses[i] < 0) {
        brightnesses[i] = 0;
      }
    }
    analogWrite(pins[i], brightnesses[i]);
  }
}
