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

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

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

}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
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

  // Record accelerometer data
  float x = sensorValue.un.accelerometer.x;
  float y = sensorValue.un.accelerometer.y;
  float z = sensorValue.un.accelerometer.z;

  // Convert data to angles
  float alpha = atan2(z, sqrt(x*x + y*y));
  float beta = atan2(y, sqrt(x*x + z*z));

  // Convert angles to motor volatage outputs
  float fb_Ctrl = sens_FB*alpha*255;
  float rl_Ctrl = sens_RL*beta*255;
  
  
  // Print debug angles and voltage values
  Serial.print(" alpha: ");
  Serial.println(alpha*RAD_TO_DEG);
  Serial.print(" beta: ");
  Serial.println(beta*RAD_TO_DEG);
  Serial.print(" F/B: ");
  Serial.println(fb_Ctrl);
  Serial.print(" R/L ");
  Serial.println(rl_Ctrl);


  // Define motor voltage outputs for forward/backwards motion and define boundary conditions
  if (fb_Ctrl >= 0 && fb_Ctrl <= 255) {
  analogWrite(9, fb_Ctrl); 
  } else if (fb_Ctrl >= -255 && fb_Ctrl < 0) {
  analogWrite(10, -fb_Ctrl);
  } else if (fb_Ctrl > 255) {
  analogWrite(9, 255);
  } else if (fb_Ctrl < -255) {
  analogWrite(10, 255);
  } 
  
  // Define motor voltage outputs for right/left motion and define boundary conditions
  if (rl_Ctrl >= 0 && rl_Ctrl <= 255) {
  analogWrite(A0, rl_Ctrl);
  } else if (rl_Ctrl >= -255 && rl_Ctrl < 0) {
  analogWrite(A1, -rl_Ctrl);
  } else if (rl_Ctrl > 255) {
  analogWrite(A0, 255);
  } else if (rl_Ctrl < -255) {
  analogWrite(A1, 255);
  }
}