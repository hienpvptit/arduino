#define BUZZ 4

int box_mode = 1;
int old_box_mode = 1;

/*-- Heart Sensor -- */
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

PulseOximeter pox;

void Pox_Begin()
{
  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    //Serial.println("FAILED");
    for (;;);
  } else {
    //Serial.println("SUCCESS");
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
    //Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
  mpu.setIntFreeFallEnabled(true);            // Turn on freefall interrupt
  mpu.setIntZeroMotionEnabled(false);         // Turn off zero motion interrupt
  mpu.setIntMotionEnabled(false);             // Turn off motion interrupt
  mpu.setDHPFMode(MPU6050_DHPF_5HZ);
  mpu.setFreeFallDetectionThreshold(25);      // Setup threshold of free fall detection
  mpu.setFreeFallDetectionDuration(2);        // Setup duration of free fall detection
  attachInterrupt(1, FALL_INT, RISING);
}

void FALL_INT()   // Free fall interrupt program
{
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
    //Serial.println("SSD1306 allocation failed");
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
  0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x0F, 0x78, 0x19, 0xCC, 0x30, 0x86,
  0x24, 0x02, 0x0A, 0x22, 0xF1, 0x52, 0x80, 0x86, 0x18, 0x0C, 0x0C, 0x18,
  0x06, 0x30, 0x03, 0x60, 0x01, 0xC0, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM temp_bmp [] = {
  0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xC0,
  0xE7, 0xE0, 0x06, 0x60, 0xEC, 0x30, 0x0C, 0x30, 0xCC, 0x30, 0x1C, 0x38,
  0x38, 0x1C, 0x70, 0x0E, 0x60, 0x06, 0x60, 0x06, 0x7F, 0xFE, 0x3F, 0xFC
};

/*-----------------------------------------------------------------*/
// SIM800L
#include <SoftwareSerial.h>
SoftwareSerial SIM(10, 11);
String num_addr = "\"+84362912536\"";
String msg_warn = "Warning!!! Fall";
void Send_Message()
{
  SIM.print("AT+CMGF=1\r\n");
  delay(100);
  SIM.print("AT+CMGS=" + num_addr + "\r\n");
  long time_out = millis();
  while (1)
  {
    if (millis() - time_out > 3000)
    {
      //Serial.println("TIMEOUT");
      return;
    }

    if (SIM.available())
    {
      if (SIM.read() == '>')
      {
        SIM.print(msg_warn);
        SIM.write(0x1A);
        return;
      }
    }
  }
}

void setup()
{
  pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW);
  //Serial.begin(115200);
  SIM.begin(9600);
  Wire.begin();

  Oled_Begin();

  Pox_Begin();

  MPU_Begin();

  attachInterrupt(0, BOX_MODE, RISING);
}

void BOX_MODE()
{
  box_mode ++;
  if (box_mode > 3)
    box_mode = 1;
  //Serial.println("Change box_mode to: " + String(box_mode));
}

long last1 = 0;
long last2 = 0;

int heart_rate_tmp[10];
int heart_rate = 0;
int i_h = 0;
int count_fail = 0;
int temperature = 0;

void loop()
{
  pox.update();
  if (millis() - last1 > 1000)
  {
    int tmp = Heart_Read();
    heart_rate_tmp[i_h++] = tmp;
    if (i_h == 10)
    {
      i_h = 0;
      Sort(heart_rate_tmp);
      heart_rate = (heart_rate_tmp[3] + heart_rate_tmp[4] + heart_rate_tmp[5] + heart_rate_tmp[6]) / 4;
      //Serial.println("Heart rate: " + String(heart_rate));
    }

    if (tmp == 0)
    {
      count_fail ++;
      if (count_fail == 3)
      {
        i_h = 0;
        count_fail = 0;
        for(int i=0; i<10; i++)
          heart_rate_tmp[i] = 0;
        heart_rate = 0;
      }
    }
    else
    {
      count_fail = 0;
    }
    last1 = millis();
  }

  if (millis() - last2 > 5000)
  {
    display.clearDisplay();
    display.setCursor(0, -2);
    display.drawBitmap(0, -2, heart_bmp, 16, 16, 1);
    display.setCursor(24, 0);
    display.setTextSize(2);
    display.print("  " + String(heart_rate));
    display.setTextSize(1);
    display.print("bpm");

    display.setCursor(0, 14);
    display.drawBitmap(0, 14, temp_bmp, 16, 16, 1);
    display.setCursor(24, 17);
    display.setTextSize(2);

    temperature = 5.0*analogRead(A3)*100.0/1024.0;
    display.print("  " + String(temperature));
    display.setTextSize(1);
    display.print("o");
    display.setTextSize(2);
    display.print("C");

    display.display();
    last2 = millis();
  }

  if (FreeFall == true)
  {
    //Serial.println("Warning!!! Fall");
    detachInterrupt(1);   // Turn off fall interrupt
    if(box_mode == 1)
      Send_Message();
    while (1)
    {
      digitalWrite(BUZZ, HIGH);
      delay(200);
      digitalWrite(BUZZ, LOW);
      delay(200);
    }
  }

  if (old_box_mode != box_mode)
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Box mode: " + String(box_mode));
    display.display();
    delay(500);
    old_box_mode = box_mode;
  }
}

void Sort(int *A)
{
  int n = 10;
  for (int i = 0; i < n - 1; i++)
  {
    for (int j = n - 1; j > i; j--)
    {
      if (A[j] < A[j - 1])
      {
        int tmp = A[j];
        A[j] = A[j - 1];
        A[j - 1] = tmp;
      }
    }
  }
}
