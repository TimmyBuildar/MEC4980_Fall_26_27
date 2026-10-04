#include <Arduino.h>
#include <string.h>
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
 //initialize the variables we're linked to
  Input = analogRead(A0);
  Setpoint = 100;

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

void loop() {
  Input = analogRead(A0);
  myPID.Compute();
  analogWrite(9, 255 - Input);
  delay(10);
}
