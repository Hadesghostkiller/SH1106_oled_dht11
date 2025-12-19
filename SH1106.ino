#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <DHT.h>

// --- INCLUDE 2 FILE VIDEO ---
// Lưu ý: Đảm bảo video1.h đã sửa tên biến thành _ALERT như bài trước
#include "video.h"   
#include "video1.h"  

// --- CẤU HÌNH MÀN HÌNH ---
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 9, 8);

// --- CẤU HÌNH DHT11 ---
#define DHTPIN 3      
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// --- CẤU HÌNH NGƯỠNG KÍCH HOẠT (ĐỘ NHẠY) ---
// Thay đổi nhiệt độ +- 1.0 độ là kích hoạt (Cũ là 3.0)
const float THRESH_TEMP = 2; 
// Thay đổi độ ẩm +- 4% là kích hoạt
const float THRESH_HUMID = 9.5;

// Biến lưu giá trị chuẩn (Baseline)
float baseTemp = 0;
float baseHumid = 0;

// --- QUẢN LÝ VIDEO ---
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

  // --- GIAI ĐOẠN 1: LẤY MẪU CHUẨN (3 GIÂY) ---
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(20, 35, "Dang lay mau...");
  u8g2.sendBuffer();

  float sumTemp = 0;
  float sumHumid = 0;
  int count = 0;
  unsigned long startCalib = millis();

  while (millis() - startCalib < 3000) { 
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    // Chỉ cộng dồn nếu đọc thành công
    if (!isnan(t) && !isnan(h)) {
      sumTemp += t;
      sumHumid += h;
      count++;
    }
    delay(200); 
  }

  // Tính trung bình
  if (count > 0) {
    baseTemp = sumTemp / count;
    baseHumid = sumHumid / count;
  } else {
    baseTemp = 25.0; // Giá trị giả định nếu cảm biến hỏng
    baseHumid = 60.0;
  }

  Serial.println(">> DA XONG!");
  Serial.print("Chuan Temp: "); Serial.println(baseTemp);
  Serial.print("Chuan Humid: "); Serial.println(baseHumid);

  // Mặc định chạy Video Bình thường
  currentAnimation = epd_bitmap_allArray; 
  currentLen = epd_bitmap_allArray_LEN;
}

void loop() {
  // --- 1. HIỂN THỊ VIDEO (Đã xóa dòng chữ nhỏ) ---
  if (millis() - lastFrameTime > frameDelay) {
    lastFrameTime = millis();
    
    u8g2.clearBuffer();
    
    // Chỉ vẽ hình, không vẽ chữ rác nữa
    u8g2.drawBitmap(0, 0, 16, 64, currentAnimation[currentFrame]);

    u8g2.sendBuffer();

    currentFrame++;
    if (currentFrame >= currentLen) {
      currentFrame = 0;
    }
  }

  // --- 2. ĐỌC CẢM BIẾN (2 giây/lần) ---
  if (millis() - lastSensorTime > 2000) { 
    lastSensorTime = millis();
    
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (isnan(t) || isnan(h)) {
      Serial.println("Loi Sensor!");
      return;
    }

    // Tính độ lệch
    float diffTemp = abs(t - baseTemp);
    float diffHumid = abs(h - baseHumid);

    Serial.print("T: "); Serial.print(t); Serial.print(" (Lech: "); Serial.print(diffTemp); Serial.print(")");
    Serial.print(" | H: "); Serial.print(h); Serial.print(" (Lech: "); Serial.print(diffHumid); Serial.println(")");

    // --- LOGIC KÍCH HOẠT (OR) ---
    // Chỉ cần 1 trong 2 cái vượt ngưỡng là kích hoạt
    bool isAlert = (diffTemp >= THRESH_TEMP) || (diffHumid >= THRESH_HUMID);

    if (isAlert) {
      // --> BẬT VIDEO CẢNH BÁO
      if (currentAnimation != epd_bitmap_allArray_ALERT) {
        currentAnimation = epd_bitmap_allArray_ALERT; 
        currentLen = epd_bitmap_allArray_LEN_ALERT;   
        currentFrame = 0; 
        Serial.println(">> CANH BAO! (Moi truong thay doi)");
      }
    } else {
      // --> VỀ BÌNH THƯỜNG
      if (currentAnimation != epd_bitmap_allArray) {
        currentAnimation = epd_bitmap_allArray;       
        currentLen = epd_bitmap_allArray_LEN;
        currentFrame = 0;
        Serial.println(">> VE BINH THUONG");
      }
    }
  }
}