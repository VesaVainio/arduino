
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <dht.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
dht DHT;

#define DHT11_PIN 2

#define MODE_OK 1
#define MODE_ERROR -1

struct minuteSample
{
  int temperature;
  int humidity;
};

struct hourSample
{
  float temperature;
  float humidity;
};

int mode = MODE_OK;

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
  updateDht();
  if (mode == MODE_OK) 
  {
      lcd.setCursor(0,0);
      lcd.print("t:" + String(currentTemperature) + " t1:" + String(hourSamples[hourSampleIndex].temperature, 1));
      
      lcd.setCursor(0,1);
      lcd.print("h:" + String(currentHumidity) + " t1:" + String(hourSamples[hourSampleIndex].humidity, 1));
  }

  delay(200);
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

