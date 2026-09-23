/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_ST7789.h>

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

enum hvacState {
  Heating, // 0
  Cooling, // 1
  hCount// 2
};

enum menuState {
  TemperatureMenu, //0
  OperationMenu, //1
  UnitMenu, //2
  mCount //3
};

enum tempState {
  C, //0
  F, //1
  tCount //2
};

hvacState opMode = Heating;
menuState menuMode = TemperatureMenu;
tempState tempMode = C;

volatile long prevChangeTime = 0;
volatile long prevChangeTimeTwo = 0;
volatile long targetTemp = 27;
long debounceTime = 50;
volatile bool changeButtonFlag = false;
volatile bool menuButtonFlag = false;

void IRAM_ATTR buttonToChangeThings() {
  long now = millis();
  if(now > prevChangeTime + debounceTime) {
    changeButtonFlag = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeMenu() {
  long now = millis();
  if(now > prevChangeTimeTwo + debounceTime) {
    menuButtonFlag = true;
    prevChangeTimeTwo = now;
  }
}

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

float getCurrentTemp() {
  if(tempMode == tempState::C) { 
    return bme.temperature;
  }
  if(tempMode == tempState::F) {
     return bme.temperature * 9./5. + 32.;
  }
  return -1100.;
}

void setup() {

  //Display setup
  display.init(135, 240);
    display.setRotation(3);
    canvas.setTextColor(ST77XX_BLUE);
    canvas.fillScreen(ST77XX_ORANGE);
    canvas.setCursor(0, 65);
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, 1);


  //Sensor Setup
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    canvas.print("Could not find a valid BME680 sensor!");
    while (1);
  }

  pinMode(1, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(1), buttonToChangeThings, RISING);

  pinMode(2, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(2), buttonToChangeMenu, RISING);

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_2X);

  // bme.setHumidityOversampling(BME680_OS_2X);
  // bme.setPressureOversampling(BME680_OS_4X);
  // bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  // bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  canvas.fillScreen(ST77XX_WHITE);
  canvas.setCursor(0, 65);
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    canvas.print("Failed to begin reading!");
    return;
  }

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    canvas.print("Failed to complete reading!");
    return;
  }
  
  float currentTemp = getCurrentTemp();
  // Display on Serial monitor
  Serial.print("Temperature = ");
  Serial.print(currentTemp);
  if (tempMode == tempState::C) {
    Serial.print(" *C");
  }
  if (tempMode == tempState::F) {
    Serial.print(" *F");
  }
  Serial.print(" with target ");
  Serial.print(targetTemp);
  if (tempMode == tempState::C) {
    Serial.print(" *C");
  }
  if (tempMode == tempState::F) {
    Serial.print(" *F");
  }
  Serial.print(" operating in mode ");
  Serial.print((int)opMode);
  Serial.print(" in menu ");
  Serial.println((int)menuMode);

  //Display on Arduino Monitor
  canvas.print("Temperature = ");
  canvas.print(currentTemp);
  if (tempMode == tempState::C) {
    canvas.print(" *C");
  }
  if (tempMode == tempState::F) {
    canvas.print(" *F");
  }
  canvas.print(" with target ");
  canvas.print(targetTemp);
  if (tempMode == tempState::C) {
    canvas.print(" *C");
  }
  if (tempMode == tempState::F) {
    canvas.print(" *F");
  }
  canvas.print(" operating in mode ");
  canvas.print((int)opMode);
  canvas.print(" in menu ");
  canvas.println((int)menuMode);

  if (opMode == Heating) {
    if (currentTemp < targetTemp) {
      Serial.println("Heater is on now!");
      canvas.setCursor(0,100);
      canvas.println("Heater is on now!");
    }
     
  } else if (opMode == Cooling)
  {
    if (currentTemp > targetTemp) {
      Serial.println("AC is on now!");
      canvas.setCursor(0,100);
      canvas.println("AC is on now!");
    }
  }

  if (menuButtonFlag) {
    menuButtonFlag = false;
    menuMode = (menuState)(((int)menuMode + 1) % (int)menuState::mCount);
    Serial.print("Moving to menu: ");
    Serial.println((int)menuMode);
  }

  if (changeButtonFlag) {
     if (menuMode == TemperatureMenu) {
        targetTemp += 1.0;
       if (tempMode == C) {
        if (targetTemp > 30.0); {
          targetTemp = targetTemp - 10.;
        }
       }
       if (tempMode == F) {
        if (targetTemp > 86.0); {
          targetTemp = targetTemp - 50.;
       }
      }
    } 
    if (menuMode == OperationMenu) {
      opMode = (hvacState)(((int)opMode + 1) % (int)hvacState::hCount);
    }
    if (menuMode == UnitMenu) {
      tempMode = (tempState)(((int)tempMode + 1) % (int)tempState::tCount);
      if (tempMode == C) {
        targetTemp = (targetTemp - 32.) * 5./9;
      }
      if (tempMode == F) {
        targetTemp = targetTemp * 9./5 + 32.;
      }
      Serial.println();
      Serial.print("Switching to ");
      Serial.print((int)tempMode);
      Serial.println(" units");
    }
    changeButtonFlag = false;
  }

  // Serial.print(F("Pressure = "));
  // Serial.print(bme.pressure / 100.0);
  // Serial.println(F(" hPa"));

  // Serial.print(F("Humidity = "));
  // Serial.print(bme.humidity);
  // Serial.println(F(" %"));



  Serial.println();
  delay(100);
}