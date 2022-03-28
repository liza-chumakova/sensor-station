/*
*    The MIT License (MIT)
*
*    Copyright (c) 2022 Liza Chumakova
*
*    Permission is hereby granted, free of charge, to any person obtaining a copy
*    of this software and associated documentation files (the "Software"), to deal
*    in the Software without restriction, including without limitation the rights
*    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*    copies of the Software, and to permit persons to whom the Software is
*    furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*    THE SOFTWARE.
*/
#include <BMP085.h>
#include <Wire.h>
#include <RTC.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

const int Key_1 = A1;

LiquidCrystal_I2C lcd(0x27,20,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
static unsigned long m_next_update = 0;

static ezButton Button1(Key_1);

class SensorInterface
{
public:
  static void init();

  virtual void update() = 0;
  virtual void print() = 0;
  virtual void handlekey() = 0;

protected:
  static DS3231 m_rtc;
  static BMP085 m_barometer;
  static //объявление жатчика для остального
};
DS3231 SensorInterface::m_rtc;
BMP085 SensorInterface::m_barometer;
//тоже 

class PressureSensor : public SensorInterface
{
public:
  void update();
  void print();
  void handlekey();

private:
  float m_pressure;
};

class HumiditySensor : public SensorInterface
{
public:
  void update();
  void print();
  void handlekey();

private:
  float m_humidity;
};

class TemperatureSensor : public SensorInterface
{
public:
    void update();
    void print();
    void handlekey();

private:
    float m_temperature;
};

class Time : public SensorInterface
{
public:
    void update();
    void print();
    void handlekey();
    static void printtime(int value);

private:
    float m_time;
};

/*
class RadiationSensor : public SensorInterface
{
public:
    void update();
    void print();
    void handlekey();

private:
    float m_radiation;
};

class COSensor : public SensorInterface
{
public:
    void update();
    void print();
    void handlekey();

private:
    float m_CO;
};
*/

Time TheTime;

//COSensor TheCOSensor;

HumiditySensor TheHumiditySensor;

PressureSensor ThePressureSensor;

//RadiationSensor TheRadiationSensor;

TemperatureSensor TheTemperatureSensor;

static SensorInterface *TheActiveSensor = NULL;

void setup()
{
  pinMode(Key_1, INPUT);
  Button1.setDebounceTime(50);
  
  TheActiveSensor = &TheTime;

  Serial.begin(9600);
  while (!Serial);

  lcd.init();// initialize the lcd 
  lcd.backlight();

  SensorInterface::init();
}

void loop()
{
  Button1.loop();
  if (Button1.isPressed())
  {
    TheActiveSensor->handlekey();
  } 

  const unsigned long time_now = millis();
  
  if (time_now >= m_next_update || !m_next_update) 
  {
    TheActiveSensor->update();
    TheActiveSensor->print();
    m_next_update = time_now + 1000;
  }
}

void SensorInterface::init()
{
  m_barometer.init();
  //сделать тоже для других датчиков
  /*
  m_rtc.begin();
  m_rtc.setHourMode(CLOCK_H24);
  
  m_rtc.setDay(27);
  m_rtc.setMonth(3);
  m_rtc.setYear(2022);

  m_rtc.setHours(13);
  m_rtc.setMinutes(01);
  m_rtc.setSeconds(40);
  */
}

void Time::printtime(int value)
{
  if (value<10)
  {
    lcd.print("0");
    lcd.print(value);
  }
  else
  {
    lcd.print(value);
  }
}

void Time::update()
{
}

void Time::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Time");
  lcd.setCursor(0,1);
  printtime(m_rtc.getHours());
  lcd.print(":");
  printtime(m_rtc.getMinutes());
  lcd.print(":");
  printtime(m_rtc.getSeconds());
}

void Time::handlekey()
{
  Serial.println("In time sensor");
  TheActiveSensor = &TheTemperatureSensor;
  m_next_update = 0;
}

void TemperatureSensor::update()
{
  m_temperature = m_barometer.bmp085GetTemperature(m_barometer.bmp085ReadUT());
  m_barometer.bmp085GetPressure(m_barometer.bmp085ReadUP())/133.32239f;
}

void TemperatureSensor::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temperature");
  lcd.setCursor(0,1);
  lcd.print(m_temperature, 1);
  lcd.print(" C");
}

void TemperatureSensor::handlekey()
{
  Serial.println("In temperature sensor");
  TheActiveSensor = &TheHumiditySensor;
  m_next_update = 0;
}

void HumiditySensor::update()
{
  this->m_humidity = 50;
}

void HumiditySensor::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Humidity");
  lcd.setCursor(0,1);
  lcd.print(m_humidity, 0);
  lcd.print(" %");
}

void HumiditySensor::handlekey()
{
  Serial.println("In humidity sensor");
  TheActiveSensor = &ThePressureSensor;
  m_next_update = 0;
}

void PressureSensor::update()
{
  m_barometer.bmp085GetTemperature(m_barometer.bmp085ReadUT()); //Get the temperature, bmp085ReadUT MUST be called first
  m_pressure = m_barometer.bmp085GetPressure(m_barometer.bmp085ReadUP())/133.32239f;
}

void PressureSensor::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Pressure");
  lcd.setCursor(0,1);
  lcd.print(m_pressure, 1);
  lcd.print(" Pa");
}

void PressureSensor::handlekey()
{
  Serial.println("In pressure sensor");
  TheActiveSensor = &TheTime;
  m_next_update = 0;
}

/*
void RadiationSensor::update()
{
  //берем данные
}

void RadiationSensor::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Radiation");
  lcd.setCursor(0,1);
  lcd.print(m_radiation, 1);
  lcd.print(" Mz");
}

void RadiationSensor::handlekey()
{
  Serial.println("In radiation sensor");
  TheActiveSensor = &TheCOSensor;
  m_next_update = 0;
}

void COSensor::update()
{
  //берем данные
}

void COSensor::print()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CO");
  lcd.setCursor(0,1);
  lcd.print(m_CO, 1);
  lcd.print(" ppn");
}

void COSensor::handlekey()
{
  Serial.println("In CO sensor");
  TheActiveSensor = &TheCOSensor;
  m_next_update = 0;
}
*/
