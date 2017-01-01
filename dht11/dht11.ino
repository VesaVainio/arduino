#include <dht.h>

dht DHT;

#define DHT11_PIN 2
#define GREEN_LED_1 3
#define YELLOW_LED_1 5
#define RED_LED_1 6

#define LED_B_1 9
#define LED_B_2 10
#define LED_B_3 11

#define BUTTON_PIN 13

#define MODE_FULL 1
#define MODE_BLINK 2
#define MODE_BLINK_QUARTER 3

int mode = MODE_BLINK_QUARTER;

int cycle = 0;
unsigned long lastUpdateTime;

int lastState = LOW;
int primedForChange = 0;
unsigned long lastChangeTime;

void setup() {
  Serial.begin(9600);

  pinMode(GREEN_LED_1, OUTPUT);
  pinMode(YELLOW_LED_1, OUTPUT);
  pinMode(RED_LED_1, OUTPUT);

  pinMode(LED_B_1, OUTPUT);
  pinMode(LED_B_2, OUTPUT);
  pinMode(LED_B_3, OUTPUT);

  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
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
    primedForChange = 0;
    lastUpdateTime = 0;
    cycle = 0;
  }
  lastState = state;

  //Serial.print(mode);
  //Serial.print(",\t");
  //Serial.println(state);

  int newCycle = cycle;

  if (cycle == 0 && currentMillis - lastUpdateTime > 1500)
  {
    //Serial.println("cycle 0 update");

    int brightness = 255;
    if (mode == MODE_BLINK_QUARTER) {
      brightness = 16;
    }
    
    int chk = DHT.read11(DHT11_PIN);
    if (chk < 0) {
      analogWrite(GREEN_LED_1, brightness);
      analogWrite(YELLOW_LED_1, brightness);
      analogWrite(RED_LED_1, brightness);

      analogWrite(LED_B_1, brightness);
      analogWrite(LED_B_2, brightness);
      analogWrite(LED_B_3, brightness);
      return;
    }

    Serial.print("chk ");
    Serial.println(chk);
    
    //Serial.print(DHT.humidity, 1);
    //Serial.print(",\t");
    //Serial.println(DHT.temperature, 1);
  
    //Serial.print("brighness ");
    //Serial.println(brightness);

    if (DHT.temperature < 25) {
      analogWrite(GREEN_LED_1, brightness);
      analogWrite(YELLOW_LED_1, 0);
      analogWrite(RED_LED_1, 0);
    } else if (DHT.temperature >= 25 && DHT.temperature < 27) {
      analogWrite(GREEN_LED_1, 0);
      analogWrite(YELLOW_LED_1, brightness);
      analogWrite(RED_LED_1, 0);
    } else {
      analogWrite(GREEN_LED_1, 0);
      analogWrite(YELLOW_LED_1, 0);
      analogWrite(RED_LED_1, brightness);
    }
  
    if (DHT.humidity < 35) {
      analogWrite(LED_B_1, brightness);
      analogWrite(LED_B_2, 0);
      analogWrite(LED_B_3, 0);
    } else if (DHT.humidity >= 35 && DHT.humidity < 45) {
      analogWrite(LED_B_1, 0);
      analogWrite(LED_B_2, brightness);
      analogWrite(LED_B_3, 0);
    } else {
      analogWrite(LED_B_1, 0);
      analogWrite(LED_B_2, 0);
      analogWrite(LED_B_3, brightness);
    }

    newCycle = 1;
    lastUpdateTime = currentMillis;
  }

  if (mode != MODE_FULL && cycle == 1 && currentMillis - lastUpdateTime > 1000) {
    //Serial.println("cycle 1 update");
    analogWrite(GREEN_LED_1, 0);
    analogWrite(YELLOW_LED_1, 0);
    analogWrite(RED_LED_1, 0);
  
    analogWrite(LED_B_1, 0);
    analogWrite(LED_B_2, 0);
    analogWrite(LED_B_3, 0);

    newCycle = 0;
    lastUpdateTime = currentMillis;
  }

  cycle = newCycle;

  if (mode == MODE_FULL) {
    cycle = 0;
  }
  
  delay(10);
}

