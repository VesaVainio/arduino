
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;

#define DHT11_PIN 2

#define MOISTURE_PIN1 4
#define MOISTURE_PIN2 5

#define BUTTON1_PIN 13
#define BUTTON2_PIN 12

#define DHT_MODE_OK 1
#define DHT_MODE_ERROR -1

#define DISP_MODE_CURRENT 1
#define DISP_MODE_1H 2
#define DISP_MODE_6H 3
#define DISP_MODE_STATS 4

#define MENU_MODE_SHOW_HISTORY 1
#define MENU_MODE_EXIT 99

#define MODE_1_DISP_CYCLE 1
#define MODE_2_DISP_MENU 2
#define MODE_3_DISP_HISTORY 3

const int seriesCount = 6;

const int seriesInUse = 3;

const int startOfMinuteSamples = 24; // 24 bytes for misc variables
const int oneMinuteSeriesBytes = 12 * sizeof(float);

const int startOfHourSamples = startOfMinuteSamples + seriesCount * oneMinuteSeriesBytes;
const int oneHourSeriesBytes = 24 * sizeof(float);

byte dhtMode = DHT_MODE_OK;

byte dispMode = DISP_MODE_STATS;
unsigned long lcdUpdatedMillis = 0;

int currentTemperature = 0;
int currentHumidity = 0;
int currentSoil = 0;
unsigned long dhtUpdatedMillis = 0;

unsigned long nextMinuteSampleMillis = 0;

unsigned long moistureUpdatedMillis = 0;
byte moistureReadingState = 0;

unsigned long buttonUpdatedMillis = 0;
int button1State = 0;
int button2State = 0;
bool buttonStateChanging = false;

int mode = MODE_1_DISP_CYCLE;

int menuMode = MENU_MODE_SHOW_HISTORY;


void setup()
{
  Serial.begin(9600);

  lcd.begin(16,2);

  pinMode(MOISTURE_PIN1, OUTPUT);
  pinMode(MOISTURE_PIN2, OUTPUT);

  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  
  for(int i = 0; i< 3; i++)
  {
    digitalWrite(MOISTURE_PIN1, HIGH);
    digitalWrite(MOISTURE_PIN2, LOW);
    lcd.noBacklight();
    delay(250);

    currentSoil = analogRead(0); // get the moisture reading
    
    digitalWrite(MOISTURE_PIN1, LOW);
    digitalWrite(MOISTURE_PIN2, HIGH);
    lcd.backlight();
    delay(250);
  }

  digitalWrite(MOISTURE_PIN1, LOW);
  digitalWrite(MOISTURE_PIN2, LOW);
}

void loop()
{
  updateDht();
  updateMoisture();
  updateButtonsWithDebounce();
  
  if (dhtMode == DHT_MODE_OK) 
  {
      doSampling();
      updateLcd();
  }

  delay(5);
}

void updateButtonsWithDebounce()
{
  int button1NewState = digitalRead(BUTTON1_PIN);
  int button2NewState = digitalRead(BUTTON2_PIN);

  unsigned long currentMillis = millis();
  if (buttonStateChanging == false && (button1NewState != button1State || button2NewState != button2State))
  {
    buttonUpdatedMillis == currentMillis;
    buttonStateChanging = true;
  }
  else if (buttonStateChanging = true && currentMillis > buttonUpdatedMillis + 50)
  {
    if (button1State == LOW && button1NewState == HIGH)
    {
      button1Pressed();
    }

    if (button2State == LOW && button2NewState == HIGH)
    {
      button2Pressed();
    }

    button1State = button1NewState;
    button2State = button2NewState;
    buttonStateChanging = false;
  }
}

void button1Pressed()
{
  if (mode == MODE_1_DISP_CYCLE)
  {
    mode = MODE_2_DISP_MENU;
    menuMode = MENU_MODE_SHOW_HISTORY;
    renderMenu();
  }
  else if (mode == MODE_2_DISP_MENU)
  {
    menuNext();
    renderMenu();
  }
  else if (mode == MODE_3_DISP_HISTORY)
  {
    mode = MODE_1_DISP_CYCLE;
    lcdUpdatedMillis = 0;
    dispMode = DISP_MODE_STATS;
  }
}

void button2Pressed()
{
  menuChoose();
}

void renderMenu()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MENU");
  lcd.setCursor(0,1);
  if (menuMode == MENU_MODE_SHOW_HISTORY)
  {
    lcd.print("    SHOW HISTORY");
  }
  else if (menuMode == MENU_MODE_EXIT)
  {
    lcd.print("    EXIT");
  }
}

void menuNext()
{
  if (menuMode == MENU_MODE_SHOW_HISTORY)
  {
    menuMode = MENU_MODE_EXIT;
  }
  else if (menuMode == MENU_MODE_EXIT)
  {
    menuMode = MENU_MODE_SHOW_HISTORY;
  }
}

void menuChoose()
{
  if (menuMode == MENU_MODE_EXIT)
  {
    mode = MODE_1_DISP_CYCLE;
    lcdUpdatedMillis = 0;
    dispMode = DISP_MODE_STATS;
  }
  else if (menuMode == MENU_MODE_SHOW_HISTORY)
  {
    mode = MODE_3_DISP_HISTORY;
    lcdUpdatedMillis = 0;
    dispMode = 0;
  }
}

void updateMoisture()
{
  unsigned long currentMillis = millis();
  if (moistureReadingState == 0 && currentMillis > moistureUpdatedMillis + 30000)
  {
    moistureReadingState = 1;
    digitalWrite(MOISTURE_PIN1, HIGH);
    digitalWrite(MOISTURE_PIN2, LOW);
    moistureUpdatedMillis = currentMillis;
  }
  else if (moistureReadingState == 1 && currentMillis > moistureUpdatedMillis + 200)
  {
    currentSoil = analogRead(0); // actually get the reading

    moistureReadingState = 2;
    digitalWrite(MOISTURE_PIN1, LOW);
    digitalWrite(MOISTURE_PIN2, HIGH);
    moistureUpdatedMillis = currentMillis;
  }
  else if (moistureReadingState == 2 && currentMillis > moistureUpdatedMillis + 200)
  {
    moistureReadingState = 0;
    digitalWrite(MOISTURE_PIN1, LOW);
    digitalWrite(MOISTURE_PIN2, LOW);
    moistureUpdatedMillis = currentMillis;
  }
}

void updateLcd()
{
  if (mode == MODE_1_DISP_CYCLE)
  {
    updateLcdWithDisplayCycle();
  }
  else if (mode == MODE_3_DISP_HISTORY)
  {
    updateLcdWithHistory();
  }
}

void updateLcdWithHistory()
{
  unsigned long currentMillis = millis();
  if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 400)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    if (dispMode <= 11)
    {
      int minuteIndex = getMinuteIndex() - 1 - dispMode;
      if (minuteIndex < 0)
      {
        minuteIndex += 12;
      }

      int minutes = (dispMode + 1) * 5;
      if (minutes < 10)
      {
        lcd.print(" ");
      }
      
      lcd.print(String(minutes) + "m " + String(getMinuteSample(0, minuteIndex), 1) + " " + String(getMinuteSample(1, minuteIndex), 1));
      lcd.setCursor(0,1);
      lcd.print("    " + String(getMinuteSample(2, minuteIndex), 1));
    }
    else {
      int hourIndex = getHourIndex() - 1 - (dispMode - 12);
      if (hourIndex < 0)
      {
        hourIndex += 24;  
      }

      int hours = (dispMode - 11);
      if (hours < 10)
      {
        lcd.print(" ");
      }

      lcd.print(String(hours) + "h " + String(getHourSample(0, hourIndex), 1) + " " + String(getHourSample(1, hourIndex), 1));
      lcd.setCursor(0,1);
      lcd.print("    " + String(getHourSample(2, hourIndex), 1));
    }
    
    dispMode += 1;
    if (dispMode > 35)
    {
      dispMode = 0;
    }
    lcdUpdatedMillis = currentMillis;
  }
}

void updateLcdWithDisplayCycle()
{
  unsigned long currentMillis = millis();
  if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 2000)
  {
    lcd.clear();
    lcd.setCursor(0,0);

    if (dispMode == DISP_MODE_STATS)
    {
      dispMode = DISP_MODE_CURRENT;
      lcd.print("tmp  " + String(currentTemperature) + " hmd " + String(currentHumidity));
      lcd.setCursor(0,1);
      lcd.print("soil " + String(currentSoil));
    } 
    else if (dispMode == DISP_MODE_CURRENT)
    {
      dispMode = DISP_MODE_1H;
      lcd.print("1h " + String(getNHoursAvg(0, 1), 1) + " " + String(getNHoursAvg(1, 1), 1));
      lcd.setCursor(0,1);
      lcd.print("   " + String(getNHoursAvg(2, 1), 1) + "  ");
    }
    else if (dispMode == DISP_MODE_1H)
    {
      dispMode = DISP_MODE_6H;
      lcd.print("6h " + String(getNHoursAvg(0, 6), 1) + " " + String(getNHoursAvg(1, 6), 1));
      lcd.setCursor(0,1);
      lcd.print("   " + String(getNHoursAvg(2, 6), 1));
    }
    else if (dispMode == DISP_MODE_6H)
    {
      dispMode = DISP_MODE_STATS;
      lcd.print("run " + String(currentMillis/(1000*60ul*60ul)) + "h " + String((currentMillis % (1000*60ul*60ul)) / (1000*60ul)) + "min");
      lcd.setCursor(0,1);
      lcd.print(String(currentMillis));
    }
    lcdUpdatedMillis = currentMillis;
  }
}

void doSampling()
{
  unsigned long currentMillis = millis();

  if (currentMillis < nextMinuteSampleMillis)
  {
    return;
  }
  
  int minuteIndex = getMinuteIndex();

  nextMinuteSampleMillis += 300000ul;
  putMinuteSample(0, minuteIndex, (float)currentTemperature);
  putMinuteSample(1, minuteIndex, (float)currentHumidity);
  putMinuteSample(2, minuteIndex, (float)currentSoil);
  minuteIndex++;

  if (minuteIndex == 12)
  {
    minuteIndex = 0;
    putMinuteIndex(minuteIndex);

    int hourIndex = getHourIndex();

    for (int s = 0; s < seriesInUse; s++) {
      float avgValue = 0;
  
      for (int i = 0; i < 12; i++) {
        avgValue += getMinuteSample(s, i);
      }
  
      avgValue /= 12;

      putHourSample(s, hourIndex, avgValue);
    }

    hourIndex++;
    if (hourIndex == 24)
    {
      hourIndex = 0;
    }
    putHourIndex(hourIndex);
  }
  else 
  {
    putMinuteIndex(minuteIndex);
  }
}

void updateDht()
{
  unsigned long currentMillis = millis();
  if (currentMillis > dhtUpdatedMillis + 200)
  {
    int chk = DHT.read11(DHT11_PIN);
    if (chk < 0) {
      setMode(DHT_MODE_ERROR);
      lcd.setCursor(0,0);
      lcd.print("DHT11 ERROR ");
      lcd.print(chk);
    }
    else {
      currentTemperature = DHT.temperature;
      currentHumidity = DHT.humidity;
      setMode(DHT_MODE_OK);
    }
    dhtUpdatedMillis = currentMillis;
  }
}

void setMode(int newMode)
{
  if (newMode != dhtMode) {
    lcd.clear();
  }
  dhtMode = newMode;
}

float getNHoursAvg(int series, int n)
{
  int index = getHourIndex();
  float avg = 0;
  for (int i=0; i<n; i++)
  {
    index--;
    if (index < 0)
    {
      index = 23;
    }
    avg += getHourSample(series, index);
  }

  avg /= n;
  return avg;
}

void putMinuteSample(int series, int index, float value)
{
  int address = startOfMinuteSamples + series * oneMinuteSeriesBytes + index * sizeof(float);
  EEPROM.put(address, value);
}

float getMinuteSample(int series, int index)
{
  int address = startOfMinuteSamples + series * oneMinuteSeriesBytes + index * sizeof(float);
  float value;
  EEPROM.get(address, value);  
  return value;
}

void putHourSample(int series, int index, float value)
{
  int address = startOfHourSamples + series * oneHourSeriesBytes + index * sizeof(float);
  EEPROM.put(address, value);
}

float getHourSample(int series, int index)
{
  int address = startOfHourSamples + series * oneHourSeriesBytes + index * sizeof(float);
  float value;
  EEPROM.get(address, value);  
  return value;
}

void putMinuteIndex(int index) {
  EEPROM.put(0, index);
}

int getMinuteIndex() {
  int index;
  EEPROM.get(0, index);
  return index;
}

void putHourIndex(int index) {
  EEPROM.put(2, index);
}

int getHourIndex() {
  int index;
  EEPROM.get(2, index);
  return index;
}

