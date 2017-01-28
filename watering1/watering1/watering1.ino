
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <dht.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;

#define DHT11_PIN 2

#define MODE_OK 1
#define MODE_ERROR -1

#define DISP_MODE_CURRENT 1
#define DISP_MODE_1H 2
#define DISP_MODE_6H 3
#define DISP_MODE_STATS 4

const int seriesCount = 6;

const int seriesInUse = 2;

const int startOfMinuteSamples = 24; // 24 bytes for misc variables
const int oneMinuteSeriesBytes = 12 * sizeof(float);

const int startOfHourSamples = startOfMinuteSamples + seriesCount * oneMinuteSeriesBytes;
const int oneHourSeriesBytes = 24 * sizeof(float);

byte mode = MODE_OK;

byte dispMode = DISP_MODE_STATS;
unsigned long lcdUpdatedMillis = 0;

int currentTemperature = 0;
int currentHumidity = 0;
unsigned long dhtUpdatedMillis = 0;

unsigned long nextMinuteSampleMillis = 0;

void setup()
{
  Serial.begin(9600);

  lcd.begin(16,2);

  for(int i = 0; i< 3; i++)
  {
    lcd.noBacklight();
    delay(250);
    lcd.backlight();
    delay(250);
  }
}

void loop()
{
  updateDht();
  doSampling();
  if (mode == MODE_OK) 
  {
      updateLcd();
  }

  delay(200);
}

void updateLcd()
{
  unsigned long currentMillis = millis();
  if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 2000)
  {
    if (dispMode == DISP_MODE_STATS)
    {
      dispMode = DISP_MODE_CURRENT;
      lcd.setCursor(0,0);
      lcd.print("Curr  tmp " + String(currentTemperature) + "  ");
      lcd.setCursor(0,1);
      lcd.print("Curr  hmd " + String(currentHumidity) + "  ");
    } 
    else if (dispMode == DISP_MODE_CURRENT)
    {
      dispMode = DISP_MODE_1H;
      lcd.setCursor(0,0);
      lcd.print("1h av tmp " + String(getNHoursAvg(0, 1), 1));
      lcd.setCursor(0,1);
      lcd.print("1h av hmd " + String(getNHoursAvg(1, 1), 1));
    }
    else if (dispMode == DISP_MODE_1H)
    {
      dispMode = DISP_MODE_6H;
      lcd.setCursor(0,0);
      lcd.print("6h av tmp " + String(getNHoursAvg(0, 6), 1));
      lcd.setCursor(0,1);
      lcd.print("6h av hmd " + String(getNHoursAvg(1, 6), 1));
    }
    else if (dispMode == DISP_MODE_6H)
    {
      dispMode = DISP_MODE_STATS;
      lcd.clear();
      lcd.setCursor(0,0);
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
      setMode(MODE_ERROR);
      lcd.setCursor(0,0);
      lcd.print("DHT11 ERROR ");
      lcd.print(chk);
    }
    else {
      currentTemperature = DHT.temperature;
      currentHumidity = DHT.humidity;
      setMode(MODE_OK);
    }
    dhtUpdatedMillis = currentMillis;
  }
}

void setMode(int newMode)
{
  if (newMode != mode) {
    lcd.clear();
  }
  mode = newMode;
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

