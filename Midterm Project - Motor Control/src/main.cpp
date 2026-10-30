
//////////////////////////////////////////////////////// DISCLAIMER ////////////////////////////////////////////////////////
/*
1.) Connect computer to each micro controller (one at a time), force reset the micro controller, and upload the new code
2.) Once both micro controllers have the new code uploaded, connect both of them to computer (1 via USB and 1 via port) for power
3.) Connect external power supply to the breadboard (for the motors) and set the voltage to 5-6 [V]
4.) With everything connected and powered on, the joystick should now control the mechanism

NOTE: For the final testing, the breadboard will be zip-tied upside-down to the bottom of the mechanism housing (for easier access)
*/

//////////////////////////////////////////////////////// SETUP ////////////////////////////////////////////////////////
//Libraries
#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>

//Define misc values
void setReports();
#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET -1

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

bool isMainBoard = true; // Upload with the line below commented out to the accelerometer board.
//bool isMainBoard = false; // Upload with the line above commented out to the motor board.

String pcbuffer = "";
String boardbuffer = "";

int pinRx = 5;
int pinTx = 6;

long rateToPc = 115200;
long rateToBoard = 9600;

//Main setup function
void setup() {
  //Board 2 board setup
  if (isMainBoard) {
    Serial.begin(rateToPc);
    while (!Serial) {
      ;
    }
  }
  Serial1.setPins(pinRx, pinTx);
  Serial1.begin(9600);

  //Accelerometer setup
    Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("Adafruit BNO08x test!");

  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");

  setReports();
}

//"setReports" function
void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
}

//Second board function
void RunSecondBoardLoop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      boardbuffer.trim();
      if (boardbuffer.length() > 0) {
        //Call in values from first board
        float alpha = boardbuffer.substring(0,5).toFloat()*100.; //Reads 5 decimal places of first value (alpha)
        float beta = boardbuffer.substring(7).toFloat()*100.; //Skips comma (6) and reads 5 decimal places of second value (beta)
  
        //Convert angles (in radians) to motor volatage outputs (-250 to 250)
        float fb_Ctrl = 4.*alpha*250;
        float rl_Ctrl = 4.*beta*250;

        Serial.print("FB_ctrl:");
        Serial.println(fb_Ctrl);
        Serial.print("RL_ctrl:");
        Serial.println(rl_Ctrl);


        //Define motor voltage outputs (-250 to 250) for forward/backwards motion and define boundary conditions
        if (fb_Ctrl >= 0 && fb_Ctrl <= 250) {
            analogWrite(9, fb_Ctrl); 
        } else if (fb_Ctrl >= -250 && fb_Ctrl < 0) {
            analogWrite(10, -fb_Ctrl);
        } else if (fb_Ctrl > 250) {
            analogWrite(9, 250);
        } else if (fb_Ctrl < -250) {
            analogWrite(10, 250);
        } 
  
        // Define motor voltage outputs (-250 to 250) for right/left motion and define boundary conditions
        if (rl_Ctrl >= 0 && rl_Ctrl <= 250) {
            analogWrite(A0, rl_Ctrl);
        } else if (rl_Ctrl >= -250 && rl_Ctrl < 0) {
            analogWrite(A1, -rl_Ctrl);
        } else if (rl_Ctrl > 250) {
            analogWrite(A0, 250);
        } else if (rl_Ctrl < -250) {
            analogWrite(A1, 250);
        }

      }
      boardbuffer = "";
    } else if (c != '\r') {
      boardbuffer += c;
    }
  }
}

//////////////////////////////////////////////////////// MAIN CODE ////////////////////////////////////////////////////////
void loop() {
  // \r \n
  if (isMainBoard) {

    //If the board was reset, print a message
    if (bno08x.wasReset()) {
        Serial.print("sensor was reset ");
        setReports();
    }

    //If the board was NOT reset (it shouldn't have been), then run the main code
    if (bno08x.getSensorEvent(&sensorValue)) {
        //Read accelerometer raw data
        float x = sensorValue.un.accelerometer.x;
        float y = sensorValue.un.accelerometer.y;
        float z = sensorValue.un.accelerometer.z;

        //Convert accelerometer data to angles
        float alpha = atan2(z, sqrt(x*x + y*y));
        float beta = atan2(y, sqrt(x*x + z*z));

        //Print angle values into single line to feed into second board 
        Serial.print(alpha/100.,5); //Divide by 100 to convert values into decimals < 1; 5 indicates # of decimal places
        Serial.print(",");
        Serial.println(beta/100.,5); //Divide by 100 to convert values into decimals < 1; 5 indicates # of decimal places
    }
    
  } else {
    RunSecondBoardLoop();
  }
}