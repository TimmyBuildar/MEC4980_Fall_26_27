#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>

void setReports();
// For SPI mode, we need a CS pin
#define BNO08X_CS 10
#define BNO08X_INT 9

// For SPI mode, we also need a RESET
//#define BNO08X_RESET 5
// but not for I2C or UART
#define BNO08X_RESET -1
// Set sensitivity values for rotation input to voltage output conversion
int sens_FB = 2.25;
int sens_RL = 2.5;

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

bool isMainBoard = false;

String pcbuffer = "";
String boardbuffer = "";

int pinRx = 5;
int pinTx = 6;

long rateToPc = 115200;
long rateToBoard = 9600;

int DoSomeMath(int a) {
  return a * a;
}

void setup() {
  if (isMainBoard) {
    Serial.begin(rateToPc);
    while (!Serial) {
      ;
    }
  }
  Serial1.setPins(pinRx, pinTx);
  Serial1.begin(9600);

}

void RunSecondBoardLoop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      boardbuffer.trim();
      if (boardbuffer.length() > 0) {
        //Serial1.println(boardbuffer);
        boardbuffer.substring()
        
      }
      boardbuffer = "";
    } else if (c != '\r') {
      boardbuffer += c;
    }
  }
}

void loop() {
  // \r \n
  if (isMainBoard) {
    // Record accelerometer data
    float x = sensorValue.un.accelerometer.x;
    float y = sensorValue.un.accelerometer.y;
    float z = sensorValue.un.accelerometer.z;

    // Convert data to angles
    float alpha = atan2(z, sqrt(x*x + y*y));
    float beta = atan2(y, sqrt(x*x + z*z));

    Serial.print(alpha);
    Serial.print(",");
    Serial.println(beta); 
  } else {
    RunSecondBoardLoop();

  }

}