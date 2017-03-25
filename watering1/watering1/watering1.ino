
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

byte dispMode = 0;
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

class DisplayHandler {
  public:
    virtual DisplayHandler* button1Pressed() { return this; }
    virtual DisplayHandler* button2Pressed() { return this; }
    virtual void activate() {};
    virtual void updateLcd() {};
};

class HistoryRoller;
class InfoRoller;

class MainMenu : public DisplayHandler {
  private:
    char *menuItems[2] = { "SHOW HISTORY", "EXIT" };
    int itemIndex = 0;

    void printMenuOnLcd() {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MENU");
      lcd.setCursor(0,1);
      lcd.print(menuItems[itemIndex]);
    };
      
  public:
    DisplayHandler *_InfoRoller;
    DisplayHandler *_HistoryRoller;    
    
    virtual DisplayHandler* button1Pressed() { 
      itemIndex = itemIndex++ % 2;
      printMenuOnLcd();
      return this;
    };
    
    virtual DisplayHandler* button2Pressed() { 
      switch (itemIndex) {
        case 0:
          return this;
        case 1:
          return _HistoryRoller;
      }
    };

    virtual void activate() {
      printMenuOnLcd();
    }
    
    virtual void updateLcd() { };

    void Init(DisplayHandler* infoRoller, DisplayHandler* historyRoller) {
      _InfoRoller = infoRoller;
      _HistoryRoller = historyRoller;
    }
};

class InfoRoller : public DisplayHandler {
  private:
    MainMenu *_MainMenu;

    enum Mode {
      Current,
      Hours1,
      Hours6,
      Stats
    };

    Mode mode;
    unsigned long lcdUpdatedMillis;
  
  public:
    InfoRoller(MainMenu* mainMenu) {
      _MainMenu = mainMenu;
      mode = Current;
      lcdUpdatedMillis = 0;
    }
    
    virtual DisplayHandler* button1Pressed() {
      return _MainMenu;
    }
    
    virtual DisplayHandler* button2Pressed() { }

    virtual void activate() {
      lcdUpdatedMillis = 0;
    };

    virtual void updateLcd() { 
      unsigned long currentMillis = millis();
      if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 2000)
      {
        lcd.clear();
        lcd.setCursor(0,0);
    
        if (mode == Stats)
        {
          mode = Current;
          lcd.print("tmp  " + String(currentTemperature) + " hmd " + String(currentHumidity));
          lcd.setCursor(0,1);
          lcd.print("soil " + String(currentSoil));
        } 
        else if (mode == Current)
        {
          mode = Hours1;
          lcd.print("1h " + String(getNHoursAvg(0, 1), 1) + " " + String(getNHoursAvg(1, 1), 1));
          lcd.setCursor(0,1);
          lcd.print("   " + String(getNHoursAvg(2, 1), 1) + "  ");
        }
        else if (mode == Hours1)
        {
          mode = Hours6;
          lcd.print("6h " + String(getNHoursAvg(0, 6), 1) + " " + String(getNHoursAvg(1, 6), 1));
          lcd.setCursor(0,1);
          lcd.print("   " + String(getNHoursAvg(2, 6), 1));
        }
        else if (mode == Hours6)
        {
          mode = Stats;
          lcd.print("run " + String(currentMillis/(1000*60ul*60ul)) + "h " + String((currentMillis % (1000*60ul*60ul)) / (1000*60ul)) + "min");
          lcd.setCursor(0,1);
          lcd.print(String(currentMillis));
        }
        
        lcdUpdatedMillis = currentMillis;
      }  
    };
};

class HistoryRoller : public DisplayHandler {
  private:
    MainMenu *_MainMenuLocal;
    unsigned long lcdUpdatedMillis;
  public:
    HistoryRoller(MainMenu* mainMenu) {
      _MainMenuLocal = mainMenu;
    }

    virtual DisplayHandler* button1Pressed() { return _MainMenuLocal; }
    virtual DisplayHandler* button2Pressed() { return this; }

    virtual void activate() {
      lcdUpdatedMillis = 0;
    };

    virtual void updateLcd() {
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
    };
};

MainMenu *_MainMenu = new MainMenu();
InfoRoller *_InfoRoller = new InfoRoller(_MainMenu);
HistoryRoller *_HistoryRoller = new HistoryRoller(_MainMenu);

DisplayHandler *currentHandler;

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

  _MainMenu->Init(_InfoRoller, _HistoryRoller);
  currentHandler = _InfoRoller;
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
    DisplayHandler* oldCurrentHandler = currentHandler;
    DisplayHandler* newCurrentHandler;

    if (button1State == LOW && button1NewState == HIGH)
    {
      newCurrentHandler = currentHandler->button1Pressed();
    }

    if (button2State == LOW && button2NewState == HIGH)
    {
      newCurrentHandler = currentHandler->button2Pressed();
    }

    if (oldCurrentHandler != newCurrentHandler) {
      currentHandler = newCurrentHandler;
      currentHandler->activate();
    }

    button1State = button1NewState;
    button2State = button2NewState;
    buttonStateChanging = false;
  }
}

void loop()
{
  updateDht();
  updateMoisture();
  updateButtonsWithDebounce();
  
  if (dhtMode == DHT_MODE_OK) 
  {
      doSampling();
      currentHandler->updateLcd();
  }

  delay(5);
}

