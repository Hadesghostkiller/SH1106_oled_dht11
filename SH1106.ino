#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

#include "video.h"   // Video Normal
#include "video1.h"  // Video Alert

// --- Hardware Config ---
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 9, 8);
#define DHTPIN 3      
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// --- Thresholds ---
const float THRESH_TEMP = 2.0; 
const float THRESH_HUMID = 9.5;

// --- Variables ---
float baseTemp = 0;
float baseHumid = 0;

const unsigned char** currentAnimation; 
int currentLen = 0;
int currentFrame = 0;
unsigned long lastFrameTime = 0;
unsigned long lastSensorTime = 0;
int frameDelay = 40; 

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setContrast(255);
  dht.begin();

  // --- Calibration (3s) ---
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 35, "Calibrating...");
  u8g2.sendBuffer();

  float sumTemp = 0, sumHumid = 0;
  int count = 0;
  unsigned long startCalib = millis();

  while (millis() - startCalib < 3000) { 
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      sumTemp += t;
      sumHumid += h;
      count++;
    }
    delay(200); 
  }

  // Set Baseline
  if (count > 0) {
    baseTemp = sumTemp / count;
    baseHumid = sumHumid / count;
  } else {
    baseTemp = 25.0; baseHumid = 60.0; // Fallback
  }

  Serial.printf("Base T: %.1f | Base H: %.1f\n", baseTemp, baseHumid);

  // Init Animation
  currentAnimation = epd_bitmap_allArray; 
  currentLen = epd_bitmap_allArray_LEN;
}

void loop() {
  // --- 1. Animation Loop ---
  if (millis() - lastFrameTime > frameDelay) {
    lastFrameTime = millis();
    
    u8g2.clearBuffer();
    u8g2.drawBitmap(0, 0, 16, 64, currentAnimation[currentFrame]);
    u8g2.sendBuffer();

    currentFrame++;
    if (currentFrame >= currentLen) currentFrame = 0;
  }

  // --- 2. Sensor Logic (2s interval) ---
  if (millis() - lastSensorTime > 2000) { 
    lastSensorTime = millis();
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (isnan(t) || isnan(h)) return;

    // Trigger Logic
    bool isAlert = (abs(t - baseTemp) >= THRESH_TEMP) || (abs(h - baseHumid) >= THRESH_HUMID);

    if (isAlert) {
      // Switch to ALERT
      if (currentAnimation != epd_bitmap_allArray_ALERT) {
        currentAnimation = epd_bitmap_allArray_ALERT; 
        currentLen = epd_bitmap_allArray_LEN_ALERT;   
        currentFrame = 0; 
        Serial.println(">> ALERT Mode");
      }
    } else {
      // Switch to NORMAL
      if (currentAnimation != epd_bitmap_allArray) {
        currentAnimation = epd_bitmap_allArray;       
        currentLen = epd_bitmap_allArray_LEN;
        currentFrame = 0;
        Serial.println(">> NORMAL Mode");
      }
    }
  }
}
