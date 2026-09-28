#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <AceButton.h>
#include <Adafruit_ST7789.h>
using namespace ace_button;

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(240, 135);

void setReports();
// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

// For SPI mode, we also need a RESET
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1

enum menuState {
  StepsMenu, //0
  TravelMenu, //1
  StrideMenu, //2
  RawAccelMenu, //2
  mCount //4
};

menuState curMenu = StepsMenu;

volatile long prevChangeTime = 0;
volatile long prevChangeTimeTwo = 0;
volatile long prevChangeTimeThree = 0;
volatile long strideLength = 1;
long debounceTime = 50;
volatile bool changeButtonFlag = false;
volatile bool changeBackButtonFlag = false;
volatile bool menuButtonFlag = false;

void IRAM_ATTR buttonToChangeThings() {
  long now = millis();
  if(now > prevChangeTime + debounceTime) {
    changeButtonFlag = true;
    prevChangeTime = now;
  }
}

void IRAM_ATTR buttonToChangeBack() {
  long now = millis();
  if(now > prevChangeTimeTwo + debounceTime) {
    changeBackButtonFlag = true;
    prevChangeTimeTwo = now;
  }
}

void IRAM_ATTR buttonToChangeMenu() {
  long now = millis();
  if(now > prevChangeTimeThree + debounceTime) {
    menuButtonFlag = true;
    prevChangeTimeThree = now;
  }
}

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console open  
  Serial.println("Adafruit BNO08x test!");

  // Try to initialize!
  if (!bno08x.begin_I2C()) {
    // if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte
    // UART buffer! if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  setReports();

  //Display setup
   display.init(135, 240);
      display.setRotation(3);
      canvas.setTextColor(ST77XX_BLACK);
      canvas.fillScreen(ST77XX_WHITE);
      pinMode(TFT_BACKLITE, OUTPUT);
      digitalWrite(TFT_BACKLITE, 1);

}

void loop() {
  delay(10);

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    setReports();
  }

  if (!bno08x.getSensorEvent(&sensorValue)) {
    return;
  }

  float x = sensorValue.un.accelerometer.x;
  float y = sensorValue.un.accelerometer.y;
  float z = sensorValue.un.accelerometer.z;

  float alpha = atan2(x, sqrt(y*y + z*z)) * RAD_TO_DEG;
  float beta = atan2(y, z) * RAD_TO_DEG;
  
  if (menuButtonFlag) {
    menuButtonFlag = false;
    curMenu = (menuState)(((int)curMenu + 1) % (int)menuState::mCount);
    Serial.print("Moving to menu: ");
    Serial.println((int)curMenu);
    if (curMenu == StepsMenu) {
        switch (sensorValue.sensorId) {

        case SH2_STEP_COUNTER:
          Serial.print("Step Counter - steps: ");
          Serial.print(sensorValue.un.stepCounter.steps);
          Serial.print(" latency: ");
          Serial.println(sensorValue.un.stepCounter.latency);
          canvas.setCursor(0,60);
          canvas.print("Step Counter - steps: ");
          canvas.println(sensorValue.un.stepCounter.steps);
      }
    }
    if (curMenu == TravelMenu){
      Serial.print("Approximate Distance Traveled: ");
      Serial.println(strideLength*sensorValue.un.stepCounter.steps);
      canvas.setCursor(0,60);
      canvas.print("Approximate Distance Traveled: ");
      canvas.println(strideLength*sensorValue.un.stepCounter.steps);
    }
    if (curMenu == StrideMenu){
      Serial.print("Stride Legnth: ");
      Serial.println(strideLength);
      canvas.setCursor(0,60);
      canvas.print("Stride Legnth: ");
      canvas.println(strideLength);
    }
    if (curMenu == RawAccelMenu){
      Serial.print("Accelerometer - x: ");
      Serial.print(x);
      Serial.print(" y: ");
      Serial.print(y);
      Serial.print(" z: ");
      Serial.print(z);
      canvas.setCursor(0,60);
      canvas.print("Accelerometer - x: ");
      canvas.print(x);
      canvas.print(" y: ");
      canvas.print(y);
      canvas.print(" z: ");
      canvas.print(z);
    }
  }


  if (changeButtonFlag) {
    if (curMenu == StrideMenu){
      long strideLength = strideLength + 1;
    }
    changeButtonFlag = false;
  }

  if (changeBackButtonFlag) {
    if (curMenu == StrideMenu){
      long strideLength = strideLength - 1;
    }
    changeBackButtonFlag = false;
  }
}

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
}