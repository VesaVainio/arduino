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

#define PUMP1_PIN 11

#define DHT_MODE_OK 1
#define DHT_MODE_ERROR -1

enum BacklightMode : byte {
	Off = 0,
	Auto = 1,
	On = 2
};

// store N (e.g 10 pcs) of these per plant
struct WateringRecord // 8 bytes
{
	word hour; // cumulative running hours
	word moistureAtStart;
	word baseAmount; // base amount, not considering temp adjustment or adjustPercentage
	word amount; // actual amount
};

// settings set by user, one record for each pot/pump
struct WateringSettings // 8 bytes
{
	word moistureLimit; // moisture limit for starting watering
	word potSqCm; // cm^2 of the pot, used to calculate the initial baseAmount
	byte growthFactor; // how much should the baseAmount increase per 24h as per assumed growth of the plant (may be corrected for temp)
	byte adjustPercentage; // manual adjustment of watering amount, 100 = 100% = 1
	byte pumpPower; // the pump power for PWM, doesn't affect amounts (adjusted for different pump types, lift height etc)
	byte startHour;
};

// needed only once, as only 1 watering can be in process at once
struct WateringStatus // 11 bytes
{
	byte wateringSeriesIndex; // points to the series, 0 or 1
	word targetAmount;
	word usedAmount;
	word previousCycleMoisture;
	unsigned long previousCycleStartMillis;
};

/* Misc variables to add:
- backlightMode
- hour index (persistent cumulative running hours)
- hour of day
- watering record index
- watering status?
*/

const int seriesCount = 4;
const int seriesInUse = 3;
const int minuteSeriesItems = 6;

const int startOfMinuteSamples = 24; // 24 bytes for misc variables
const int oneMinuteSeriesBytes = minuteSeriesItems * sizeof(float);

const int startOfHourSamples = startOfMinuteSamples + seriesCount * oneMinuteSeriesBytes;
const int oneHourSeriesBytes = 24 * sizeof(float);

const int wateringSeriesCount = 2;
const int wateringSeriesInUse = 1;
const int wateringSeriesItems = 10;

const int startOfWateringRecords = startOfHourSamples + seriesCount * oneHourSeriesBytes;
const int oneWateringRecordSeriesBytes = wateringSeriesItems * sizeof(WateringRecord);

byte dhtMode = DHT_MODE_OK;

BacklightMode backlightMode = On;
bool backlightOn = true;

int pump1Power = 100;

int currentTemperature = 0;
int currentHumidity = 0;
int currentSoil = 0;
unsigned long dhtUpdatedMillis = 0;

unsigned long nextMinuteSampleMillis = 0;

unsigned long moistureUpdatedMillis = 0;
byte moistureReadingState = 0;

unsigned long buttonUpdatedMillis = 0;
int button1State = LOW;
int button2State = LOW;
bool buttonStateChanging = false;

void updateMoisture()
{
  unsigned long currentMillis = millis();
  if (moistureReadingState == 0 && currentMillis > moistureUpdatedMillis + 15000)
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

  if (minuteIndex == minuteSeriesItems)
  {
    minuteIndex = 0;
    putMinuteIndex(minuteIndex);

    int hourIndex = getHourIndex();

    for (int s = 0; s < seriesInUse; s++) {
      float avgValue = 0;
  
      for (int i = 0; i < minuteSeriesItems; i++) {
        avgValue += getMinuteSample(s, i);
      }
  
      avgValue /= minuteSeriesItems;

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

void putMinuteIndex(byte index) {
  EEPROM.put(0, index);
}

byte getMinuteIndex() {
  byte index;
  EEPROM.get(0, index);
  return index;
}

void putHourIndex(byte index) {
  EEPROM.put(1, index);
}

byte getHourIndex() {
  byte index;
  EEPROM.get(1, index);
  return index;
}

//void putBacklightMode(byte backlightMode)
//{
//	EEPROM.put(2, backlightMode);
//}
//
//BacklightMode getBacklightMode()
//{
//	byte mode;
//	EEPROM.get(2, mode);
//	return static_cast<BacklightMode>(mode);
//}

//WateringSettings getWateringSettings(int index) {
//	WateringSettings settings;
//	EEPROM.get(6 + index * 8, settings);
//	return settings;
//}

class DisplayHandler {
  public:
    virtual DisplayHandler* button1Pressed() { return this; }
    virtual DisplayHandler* button2Pressed() { return this; }
    virtual void activate() {};
    virtual void updateLcd() {};
};

class HistoryRoller;
class InfoRoller;
class Settings;

class MainMenu : public DisplayHandler {
  private:
    char const* menuItems[4] = { "SHOW HISTORY", "SETTINGS", "TEST", "EXIT" };
    int itemIndex = 0;

    DisplayHandler* _InfoRollerLocal = 0;
    DisplayHandler* _HistoryRollerLocal = 0;
    DisplayHandler* _SettingsLocal = 0;
    DisplayHandler* _TestLocal = 0;
    
    void printMenuOnLcd() {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MENU");
      lcd.setCursor(0,1);
      lcd.print(menuItems[itemIndex]);
    };
      
  public:
    virtual DisplayHandler* button1Pressed() { 
      itemIndex = (itemIndex + 1) % 4;
      printMenuOnLcd();
      return this;
    };
    
    virtual DisplayHandler* button2Pressed() { 
      switch (itemIndex) {
        case 0:
          return _HistoryRollerLocal;
        case 1:
          return _SettingsLocal;
        case 2:
          return _TestLocal;
        case 3:
          return _InfoRollerLocal;      
	  }

	  return this;
    };

    virtual void activate() {
      printMenuOnLcd();
    }
    
    virtual void updateLcd() { };

    void Init(DisplayHandler* infoRoller, DisplayHandler* historyRoller, DisplayHandler* settings, DisplayHandler* test) {
      _InfoRollerLocal = infoRoller;
      _HistoryRollerLocal = historyRoller;
      _SettingsLocal = settings;
      _TestLocal = test;
    }
};

class InfoRoller : public DisplayHandler {
  private:
    MainMenu* _MainMenu;

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
    MainMenu* _MainMenuLocal;
    byte dispMode;
    unsigned long lcdUpdatedMillis;
  public:
    HistoryRoller(MainMenu* mainMenu) {
      _MainMenuLocal = mainMenu;
    }

    virtual DisplayHandler* button1Pressed() { return _MainMenuLocal; }
    virtual DisplayHandler* button2Pressed() { return this; }

    virtual void activate() {
      dispMode = 0;
      lcdUpdatedMillis = 0;
    };

    virtual void updateLcd() {
      unsigned long currentMillis = millis();
      if (lcdUpdatedMillis == 0 || currentMillis > lcdUpdatedMillis + 400)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        if (dispMode <= (minuteSeriesItems - 1))
        {
          int minuteIndex = getMinuteIndex() - 1 - dispMode;
          if (minuteIndex < 0)
          {
            minuteIndex += minuteSeriesItems;
          }
    
          int minutes = (dispMode + 1) * 10;
          if (minutes < 10)
          {
            lcd.print(" ");
          }
          
          lcd.print(String(minutes) + "m " + String(getMinuteSample(0, minuteIndex), 1) + " " + String(getMinuteSample(1, minuteIndex), 1));
          lcd.setCursor(0,1);
          lcd.print("    " + String(getMinuteSample(2, minuteIndex), 1));
        }
        else {
          int hourIndex = getHourIndex() - 1 - (dispMode - minuteSeriesItems);
          if (hourIndex < 0)
          {
            hourIndex += 24;  
          }
    
          int hours = (dispMode - (minuteSeriesItems - 1));
          if (hours < 10)
          {
            lcd.print(" ");
          }
    
          lcd.print(String(hours) + "h " + String(getHourSample(0, hourIndex), 1) + " " + String(getHourSample(1, hourIndex), 1));
          lcd.setCursor(0,1);
          lcd.print("    " + String(getHourSample(2, hourIndex), 1));
        }
        
        dispMode += 1;
        if (dispMode > (24 + minuteSeriesItems - 1))
        {
          dispMode = 0;
        }
        lcdUpdatedMillis = currentMillis;
      }
    };
};

class Settings : public DisplayHandler {
  private:
    char const* menuItems[3] = { "BACKLIGHT", "SOIL LIMIT", "EXIT" };
    char const* backlightOptions[3] = { "OFF", "AUTO", "ON" };
    int itemIndex = 0;

    DisplayHandler* _MainMenuLocal = 0;
    
    void printMenuOnLcd() {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SET");
      lcd.setCursor(0,1);
      lcd.print(menuItems[itemIndex]);
      lcd.print(' ');
      switch (itemIndex) {
        case 0:
          lcd.print(backlightOptions[backlightMode]);
      }
    };    
    
  public:
    Settings(MainMenu* mainMenu) {
      _MainMenuLocal = mainMenu;
    }
    
    virtual DisplayHandler* button1Pressed() { 
      itemIndex = (itemIndex + 1) % 3;
      printMenuOnLcd();
      return this; 
    }
    
    virtual DisplayHandler* button2Pressed() { 
      switch (itemIndex) {
        case 0:
          backlightMode = static_cast<BacklightMode>((((byte)backlightMode) + 1) % 3);
          printMenuOnLcd();
          break;
        case 1:
          break;
        case 2:
          return _MainMenuLocal;
      }
      return this; 
    }
    
    virtual void activate() {
      printMenuOnLcd();  
    };
    virtual void updateLcd() {};
};

class Test : public DisplayHandler {
  private:
    const char* menuItems[2] = { "TEST PUMP ONCE", "EXIT" };
    int itemIndex = 0;
    unsigned long pumpTestStart = 0;
    bool pumpTestRunning = false;

    DisplayHandler* _MainMenuLocal = 0;
    
    void printMenuOnLcd() {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("TEST");
      lcd.setCursor(0,1);
      lcd.print(menuItems[itemIndex]);
    };    
    
  public:
    Test(MainMenu* mainMenu) {
      _MainMenuLocal = mainMenu;
    }
    
    virtual DisplayHandler* button1Pressed() { 
      itemIndex = (itemIndex + 1) % 2;
      printMenuOnLcd();
      return this; 
    }
    
    virtual DisplayHandler* button2Pressed() { 
      switch (itemIndex) {
        case 0:
          pumpTestStart = millis();
          analogWrite(PUMP1_PIN, pump1Power);
          pumpTestRunning = true;
          break;
        case 1:
          return _MainMenuLocal;
      }
      return this; 
    }
    
    virtual void activate() {
      printMenuOnLcd();  
    };
    
    virtual void updateLcd() {
      if (pumpTestRunning == true) {
        unsigned long runningTime = millis() - pumpTestStart;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("PUMP 1 TEST " + String(runningTime));
        if (runningTime > 3000) {
          analogWrite(PUMP1_PIN, 0);
          pumpTestRunning = false;
          printMenuOnLcd();
        }
      }
    };
};

MainMenu* _MainMenu = new MainMenu();
InfoRoller* _InfoRoller = new InfoRoller(_MainMenu);
HistoryRoller* _HistoryRoller = new HistoryRoller(_MainMenu);
Settings* _Settings = new Settings(_MainMenu);
Test* _Test = new Test(_MainMenu);

DisplayHandler* currentHandler;

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting setup");
  
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

  _MainMenu->Init(_InfoRoller, _HistoryRoller, _Settings, _Test);
  currentHandler = _InfoRoller;

  Serial.println("setup finished");
}

void updateButtonsWithDebounce()
{
  int button1NewState = digitalRead(BUTTON1_PIN);
  int button2NewState = digitalRead(BUTTON2_PIN);

  unsigned long currentMillis = millis();
  if (buttonStateChanging == false && (button1NewState != button1State || button2NewState != button2State))
  {
    buttonUpdatedMillis = currentMillis;
    buttonStateChanging = true;
  }
  else if (buttonStateChanging == true && currentMillis > buttonUpdatedMillis + 50)
  {
    DisplayHandler* oldCurrentHandler = currentHandler;
    DisplayHandler* newCurrentHandler;
    bool buttonPressed = false;

    if (button1State == LOW && button1NewState == HIGH)
    {
      Serial.println("Button 1 pressed");
      newCurrentHandler = currentHandler->button1Pressed();
      buttonPressed = true;
    }

    if (button2State == LOW && button2NewState == HIGH)
    {
      Serial.println("Button 2 pressed");
      newCurrentHandler = currentHandler->button2Pressed();
      buttonPressed = true;
    }

    if (buttonPressed == true && oldCurrentHandler != newCurrentHandler) {
      currentHandler = newCurrentHandler;
      currentHandler->activate();
    }

    button1State = button1NewState;
    button2State = button2NewState;
    buttonStateChanging = false;
  }
}

void updateBacklight() {
  if (backlightMode == On && backlightOn == false) {
    lcd.backlight();
    backlightOn = true;
  } else if (backlightMode == Off && backlightOn == true) {
    lcd.noBacklight();
    backlightOn = false;
  } else if (backlightMode == Auto) {
    unsigned long currentMillis = millis();
    if (currentMillis < buttonUpdatedMillis + 30000 && backlightOn == false) {
      lcd.backlight();
      backlightOn = true;
    } else if (currentMillis > buttonUpdatedMillis + 30000 && backlightOn == true) {
      lcd.noBacklight();
      backlightOn = false;
    }
  }
}

void loop()
{
  updateDht();
  updateMoisture();
  updateButtonsWithDebounce();
  updateBacklight();
  
  if (dhtMode == DHT_MODE_OK) 
  {
      doSampling();
      currentHandler->updateLcd();
  }

  delay(10);
}

