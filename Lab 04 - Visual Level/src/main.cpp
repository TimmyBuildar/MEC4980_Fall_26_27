#include <Arduino.h>
#include <math.h>
#include <Adafruit_BNO08x.h>
#include <AceButton.h>
using namespace ace_button;
#include <Adafruit_ST7789.h>

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
int pinD1 = 1;
AceButton button(pinD1);

enum AxisMode {
  MODE_BOTH, //0
  MODE_X, //1
  MODE_Y, //2
  MODE_RAW, //3
  MODE_COUNT //4
};
AxisMode curMode = MODE_BOTH;

void ChangeMode(AceButton *button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): eventType: "));
  Serial.print(AceButton::eventName(eventType));
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
  if (eventType == (uint8_t)AceButton::kEventDoubleClicked) {
    if (curMode == AxisMode::MODE_RAW) {
      curMode = MODE_BOTH;
    }
    else curMode = (AxisMode)((curMode + 1) % AxisMode::MODE_RAW);
  }
  if (eventType == (uint8_t)AceButton::kEventLongPressed) {
    curMode = MODE_RAW;
  }
}

Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

void setup(void) {
  display.init(135, 240);
  display.setRotation(3);
  canvas.setTextColor(ST77XX_BLACK);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, 1);
  canvas.fillScreen(ST77XX_WHITE);
  canvas.setCursor(0, 20);
  canvas.println("Press Serial Monitor to Start!");
  display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);

  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens
  
  pinMode(pinD1, INPUT_PULLDOWN);
  button.init(pinD1, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(ChangeMode);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  

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

void loop() {
  button.check();
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

  if (curMode == AxisMode::MODE_BOTH || curMode == AxisMode::MODE_X){
    Serial.print(" alpha: ");
    Serial.println(alpha);
    if (curMode == AxisMode::MODE_X) {
    canvas.fillScreen(ST77XX_WHITE);
    canvas.setCursor(0, 20);
    canvas.print(" alpha: ");
    canvas.println(alpha);
    display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
    }
  }

  if (curMode == AxisMode::MODE_BOTH || curMode == AxisMode::MODE_Y){
    Serial.print(" beta: ");
    Serial.println(beta);
    if (curMode == AxisMode::MODE_Y) {
    canvas.fillScreen(ST77XX_WHITE);
    canvas.setCursor(0, 60);
    canvas.print(" beta: ");
    canvas.println(beta);
    display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
    }
  }

  if (curMode == AxisMode::MODE_BOTH) {
    canvas.fillScreen(ST77XX_WHITE);
    canvas.setCursor(0, 20);
    canvas.print(" alpha: ");
    canvas.println(alpha);
    canvas.setCursor(0, 60);
    canvas.print(" beta: ");
    canvas.println(beta);
    display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
  }

  if (curMode == AxisMode::MODE_RAW) {
    Serial.print("Accelerometer - x: ");
    Serial.print(x);
    Serial.print(" y: ");
    Serial.print(y);
    Serial.print(" z: ");
    Serial.println(z);
    canvas.fillScreen(ST77XX_WHITE);
    canvas.setCursor(10, 20);
    canvas.print("Accelerometer - x: ");
    canvas.println(x);
    canvas.setCursor(100, 60);
    canvas.print(" y: ");
    canvas.println(y);
    canvas.setCursor(100, 100);
    canvas.print(" z: ");
    canvas.println(z);
    display.drawRGBBitmap(0,0, canvas.getBuffer(), 240, 135);
  }

  Serial.println();

}

void setReports(void) {
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  } else {
    Serial.println("Set accelerometer report... success!");
  }
}
