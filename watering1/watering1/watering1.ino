
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <dht.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;

#define DHT11_PIN 2

#define MODE_OK 1
#define MODE_ERROR -1

#define DISP_MODE_CURRENT 1
#define DISP_MODE_1H 2
#define DISP_MODE_6H 3
#define DISP_MODE_STATS 4

struct minuteSample
{
  byte temperature;
  byte humidity;
};

struct hourSample
{
  float temperature;
  float humidity;
};

byte mode = MODE_OK;

byte dispMode = DISP_MODE_STATS;
unsigned long lcdUpdatedMillis = 0;

int currentTemperature = 0;
int currentHumidity = 0;
unsigned long dhtUpdatedMillis = 0;

unsigned long nextMinuteSampleMillis = 0;

minuteSample minuteSamples[60];
int minuteSampleIndex = 0;

hourSample hourSamples[24];
int hourSampleIndex = 0;

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
  Serial.print("Loop ");
  Serial.println(dispMode);
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
      lcd.print("1h av tmp " + String(getNHoursAvgTemp(1), 1));
      lcd.setCursor(0,1);
      lcd.print("1h av hmd " + String(getNHoursAvgHumid(1), 1));
    }
    else if (dispMode == DISP_MODE_1H)
    {
      dispMode = DISP_MODE_6H;
      lcd.setCursor(0,0);
      lcd.print("6h av tmp " + String(getNHoursAvgTemp(6), 1));
      lcd.setCursor(0,1);
      lcd.print("6h av hmd " + String(getNHoursAvgHumid(6), 1));
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
  if (nextMinuteSampleMillis == 0)
  {
    nextMinuteSampleMillis = currentMillis + 60*1000ul;
    minuteSamples[minuteSampleIndex].temperature = currentTemperature;
    minuteSamples[minuteSampleIndex].humidity = currentHumidity;
    minuteSampleIndex++;
  }
  else if (currentMillis > nextMinuteSampleMillis)
  {
    nextMinuteSampleMillis += 60*1000ul;
    minuteSamples[minuteSampleIndex].temperature = currentTemperature;
    minuteSamples[minuteSampleIndex].humidity = currentHumidity;
    minuteSampleIndex++;
  }

  if (minuteSampleIndex == 60)
  {
    float avgTemperature = 0;
    float avgHumidity = 0;

    for (int i=0; i<60; i++) {
      avgTemperature += minuteSamples[i].temperature;
      avgHumidity += minuteSamples[i].humidity;
    }

    avgTemperature /= 60;
    avgHumidity /= 60;

    hourSamples[hourSampleIndex].temperature = avgTemperature;
    hourSamples[hourSampleIndex].humidity = avgHumidity;
    hourSampleIndex++;
    if (hourSampleIndex == 24)
    {
      hourSampleIndex = 0;
    }
    
    minuteSampleIndex = 0;
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

float getNHoursAvgTemp(int n)
{
  int index = hourSampleIndex;
  float avg = 0;
  for (int i=0; i<n; i++)
  {
    index--;
    if (index < 0)
    {
      index = 23;
    }
    avg += hourSamples[index].temperature;
  }

  avg /= n;
  return avg;
}

float getNHoursAvgHumid(int n)
{
  int index = hourSampleIndex;
  float avg = 0.0f;
  for (int i=0; i<n; i++)
  {
    index--;
    if (index < 0)
    {
      index = 23;
    }
    avg += hourSamples[index].humidity;
  }

  avg /= n;
  return avg;
}

