#include <Wire.h>

#define BUZZ 4

// MAX30100
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS 1000
PulseOximeter max30100;

void Heart_Init()
{
	Serial.print("Initializing pulse oximeter..");
	if (!max30100.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    max30100.setIRLedCurrent(MAX30100_LED_CURR_24MA); 
}

int Heart_Read()
{
	max30100.update();
	return (int)max30100.getHeartRate();
}

// MPU6050
#include <MPU6050.h>
MPU6050 mpu;
boolean freefallDetected = false;
void MPU_Init()
{
	Serial.println("Initialize MPU6050");
	while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_16G))
	{
		Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
		delay(500);
	}
	mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
	mpu.setIntFreeFallEnabled(true);
	mpu.setIntZeroMotionEnabled(false);
	mpu.setIntMotionEnabled(false);
	mpu.setDHPFMode(MPU6050_DHPF_5HZ);
	mpu.setFreeFallDetectionThreshold(10);
	mpu.setFreeFallDetectionDuration(2);  
	MPU_checkSettings();
	attachInterrupt(1, Fall_INT, RISING);
}

void Fall_INT()
{
	freefallDetected = true;
	Serial.println("Fall");
 digitalWrite(BUZZ, HIGH);
}

void MPU_checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:                ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Motion Interrupt:     ");
  Serial.println(mpu.getIntMotionEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Zero Motion Interrupt:     ");
  Serial.println(mpu.getIntZeroMotionEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Free Fall Interrupt:       ");
  Serial.println(mpu.getIntFreeFallEnabled() ? "Enabled" : "Disabled");

  Serial.print(" * Free Fal Threshold:          ");
  Serial.println(mpu.getFreeFallDetectionThreshold());

  Serial.print(" * Free FallDuration:           ");
  Serial.println(mpu.getFreeFallDetectionDuration());
  
  Serial.print(" * Clock Source:              ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:             ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets:     ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());

  Serial.print(" * Accelerometer power delay: ");
  switch(mpu.getAccelPowerOnDelay())
  {
    case MPU6050_DELAY_3MS:            Serial.println("3ms"); break;
    case MPU6050_DELAY_2MS:            Serial.println("2ms"); break;
    case MPU6050_DELAY_1MS:            Serial.println("1ms"); break;
    case MPU6050_NO_DELAY:             Serial.println("0ms"); break;
  }  
  
  Serial.println();
}

void MPU_Run()
{
	Vector rawAccel = mpu.readRawAccel();
	Activites act = mpu.readActivites();
}

// Oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void Oled_Init()
{
	if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) // Address 0x3C for 128x32
	{ 
		Serial.println(F("SSD1306 allocation failed"));
		for(;;); // Don't proceed, loop forever
	}
	display.display();
	delay(2000); // Pause for 2 seconds
	display.clearDisplay();
}


//---------------------------
void setup()
{
	Serial.begin(115200);
	pinMode(BUZZ, OUTPUT);
  digitalWrite(BUZZ, LOW);
	//Oled_Init();
	
	Heart_Init();
	
	MPU_Init();
}

void loop()
{
	
}
