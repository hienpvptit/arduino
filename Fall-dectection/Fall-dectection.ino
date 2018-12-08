/*-- Heart Sensor -- */
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

PulseOximeter pox;

void Pox_Begin()
{
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);    // Setup current of sensor
}

int Heart_Read()
{
  return pox.getHeartRate();    // Return value of heart rate
}
/*--------------------------------------------*/
// MPU
#include <MPU6050.h>
MPU6050 mpu;
bool FreeFall = false;

void MPU_Begin()
{
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  mpu.setIntFreeFallEnabled(true);
  mpu.setIntZeroMotionEnabled(false);
  mpu.setIntMotionEnabled(false);
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);
  mpu.setFreeFallDetectionThreshold(50);
  mpu.setFreeFallDetectionDuration(2);
  attachInterrupt(1, FALL_INT, RISING);
}

void FALL_INT()
{
  Serial.println("Fall");
  FreeFall = true;
}

/*--------------------------------------------*/
// Oled
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 32);

void Oled_Begin()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println("SSD1306 allocation failed");
    while (1) {};
  }
  display.display();
  delay(1000);
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.clearDisplay();
  display.display();
  delay(1000);
}

static const unsigned char PROGMEM heart_bmp [] = {
  0x10, 0x00, 0x10, 0x00,
  0x00, 0x00, 0x0F, 0x78, 0x19, 0xCC, 0x30, 0x86, 0x24, 0x02, 0x0A, 0x22, 0xF1, 0x52, 0x80, 0x86,
  0x18, 0x0C, 0x0C, 0x18, 0x06, 0x30, 0x03, 0x60, 0x01, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM temp_bmp [] = {
  0x10, 0x00, 0x10, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xC0, 0xE7, 0xE0, 0x06, 0x60, 0xEC, 0x30, 0x0C, 0x30,
  0xCC, 0x30, 0x1C, 0x38, 0x38, 0x1C, 0x70, 0x0E, 0x60, 0x06, 0x60, 0x06, 0x7F, 0xFE, 0x3F, 0xFC
};

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  Oled_Begin();

  Pox_Begin();

  MPU_Begin();

}


long last = 0;
void loop()
{
  pox.update();
  if (millis() - last > 1000)
  {
    display.clearDisplay();
    display.setCursor(0, -2);
    display.drawBitmap(0, -2, heart_bmp, 16, 16, 1);
    display.setCursor(24, 0);
    display.setTextSize(2);
    display.print(String(Heart_Read()));
    display.setTextSize(1);
    display.print("bpm");
    
    display.setCursor(0, 14);
    display.drawBitmap(0, 14, temp_bmp, 16, 16, 1);
    display.setCursor(24, 17);
    display.setTextSize(2);
    display.print(String((int)(analogRead(A3)/1024.0 * 100)));
    display.setTextSize(1);
    display.print("o");
    display.setTextSize(2);
    display.print("C");

    display.display();
    last = millis();
  }

  if(FreeFall==true)
  {
    Serial.println("Stop");
    while(1){};  
  }

}
