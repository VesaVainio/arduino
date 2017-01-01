
const int buttonPin = 2;
const int yellowLedPin = 7;
const int pinkLedPin = 13;

const int MODE_RECORD_WAIT = 0;
const int MODE_RECORD = 1;
const int MODE_PLAYBACK = 2;

struct time_state
{
  unsigned long changeTime;
  int state;
};

time_state changes[200];
int firstFreeIndex = 0;
int changesIndex = 0;
int previousState = 0;
unsigned long previousChange = 0;
unsigned long millisOffset = 0;

String indexLabel = "index: ";
String timeLabel = " time: ";
String stateLabel = " state: ";

int mode = MODE_RECORD_WAIT;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(yellowLedPin, OUTPUT);
  pinMode(pinkLedPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (mode) {
    case MODE_RECORD_WAIT:
      digitalWrite(yellowLedPin, HIGH);
      digitalWrite(pinkLedPin, LOW);
      if (digitalRead(buttonPin) == HIGH) {
        for (int i=0; i<4; i++) {
          digitalWrite(yellowLedPin, LOW);
          delay(250);    
          digitalWrite(yellowLedPin, HIGH);
          delay(250);
        }
        startRecording();
        digitalWrite(yellowLedPin, LOW);
      }
      break;
    case MODE_RECORD:
      handleRecordCase();
      break;
    case MODE_PLAYBACK:
      if ((millis() - millisOffset) > changes[changesIndex].changeTime) {
        Serial.println(indexLabel + changesIndex + timeLabel + changes[changesIndex].changeTime + stateLabel + changes[changesIndex].state);
        digitalWrite(pinkLedPin, changes[changesIndex].state);
        changesIndex++;
      }
      if (changesIndex == firstFreeIndex)
      {
        mode = MODE_RECORD_WAIT;
        digitalWrite(yellowLedPin, LOW);
      }
      break;
  }
}

void handleRecordCase()
{
  int currentState = digitalRead(buttonPin);
  unsigned long currentMillis = millis();
  if (currentState != previousState && currentMillis - previousChange > 50)
  {
    recordChange(currentState, currentMillis);
  }
  if (firstFreeIndex == 200) {
    Serial.println("firstFreeIndex == 200");
    startPlayback();
  }
  if (currentMillis - previousChange > 3000)
  {
    Serial.println("currentMillis - previousChange > 3000");
    startPlayback();
  }
}

void recordChange(int currentState, unsigned long currentMillis)
{
  changes[firstFreeIndex].changeTime = currentMillis - millisOffset;
  changes[firstFreeIndex].state = currentState;
  previousState = currentState;
  previousChange = currentMillis;

  Serial.println(indexLabel + firstFreeIndex + timeLabel + changes[firstFreeIndex].changeTime + stateLabel + currentState);
  firstFreeIndex++;
  digitalWrite(pinkLedPin, currentState);
  
}

void startRecording()
{
  Serial.println("Start recording");
  unsigned long currentMillis = millis();
  mode = MODE_RECORD;
  millisOffset = currentMillis;
  firstFreeIndex = 0;
  recordChange(digitalRead(buttonPin), currentMillis);
}

void startPlayback()
{
  for (int i=0; i<4; i++) {
    digitalWrite(yellowLedPin, LOW);
    delay(250);    
    digitalWrite(yellowLedPin, HIGH);
    delay(250);
  }
  millisOffset = millis();
  changesIndex = 1;
  mode = MODE_PLAYBACK;
  digitalWrite(pinkLedPin, changes[0].state); // play index 0 here
  Serial.println("Start playback");
}


