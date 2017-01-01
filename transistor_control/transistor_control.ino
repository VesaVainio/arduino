int buttonPin = 2;
int motorControl = 11;

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin, INPUT);
  pinMode(motorControl, OUTPUT);
}

void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    //Serial.println("Button state is HIGH, starting watering");
    for (int i=0; i<3; i++) {
      /*Serial.print("Watering cycle ");
      Serial.print(i+1);
      Serial.println();*/
      analogWrite(motorControl, 255);
      delay(4000);
  
      analogWrite(motorControl, 0);
      delay(10000);
    }
    Serial.println("Watering finished");
  }
  else {
    //Serial.println("Button state is LOW");
    delay(10);
  }
}
