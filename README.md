# Emotional Environmental Monitor (ESP32 C3 + OLED)

## 1. Overview
This project is an interactive environmental monitor system built on the **ESP32 C3 Super Mini**. It displays animated facial expressions on an **OLED screen** that change based on real-time temperature and humidity readings from a **DHT11 sensor**.

The system establishes a "baseline" environment upon startup. If the current environment deviates significantly from this baseline (e.g., it gets hotter or more humid), the character on the screen switches from a "Normal" state to an "Alert/Worried" state.

## 2. Hardware Requirements
* **Microcontroller:** ESP32 C3 Super Mini (or compatible ESP32 C3 board).
* **Display:** 1.3" OLED Display SH1106 (128x64 resolution, I2C interface).
* **Sensor:** DHT11 Temperature & Humidity Sensor.
* **Connections:** Breadboard and Jumper wires.

## 3. Wiring / Pinout
The connections below are configured for the ESP32 C3 Super Mini.

### A. OLED Display (SH1106)
| OLED Pin | ESP32 C3 Pin | Note |
| :--- | :--- | :--- |
| **GND** | GND | Ground |
| **VCC** | 3.3V | Power Supply |
| **SCL** | **GPIO 9** | I2C Clock |
| **SDA** | **GPIO 8** | I2C Data |

### B. DHT11 Sensor
| DHT11 Pin | ESP32 C3 Pin | Note |
| :--- | :--- | :--- |
| **VCC** | 3.3V or 5V | Power Supply |
| **GND** | GND | Ground |
| **DATA** | **GPIO 3** | Data Signal |

## 4. Required Libraries
Install the following libraries via the Arduino IDE Library Manager (*Sketch -> Include Library -> Manage Libraries*):

1.  **U8g2** by *olikraus*
    * Used for controlling the SH1106 OLED display and rendering bitmaps.
2.  **DHT sensor library** by *Adafruit*
    * Used for reading the sensor data (Requires "Adafruit Unified Sensor" dependency).

## 5. Project Structure
The project consists of three main files:

* **`[ProjectName].ino`**: The main firmware file containing logic, sensor reading, and display control.
* **`video.h`**: Header file containing the bitmap array for the **NORMAL** animation.
* **`video1.h`**: Header file containing the bitmap array for the **ALERT** animation.

## 6. How It Works (Logic)

### A. Calibration Phase (Setup)
* Upon power-up, the system enters a **3-second calibration mode**.
* It samples the current temperature and humidity to calculate a **Baseline Average**.
* The screen displays "Dang lay mau..." (Sampling...) during this process.

### B. Monitoring Phase (Loop)
* **Animation:** The OLED renders the video at ~25 FPS (40ms per frame).
* **Sensing:** The system reads the DHT11 sensor every 2 seconds.
* **Trigger Logic:** The system calculates the deviation (`diff`) between the current reading and the baseline.

    **Thresholds:**
    * Temperature Deviation: **± 1.0°C**
    * Humidity Deviation: **± 4.0%**

    **Behavior:**
    * **IF** (Temp Diff ≥ 1.0) **OR** (Humid Diff ≥ 4.0) $\rightarrow$ Play **Alert Video** (`video1.h`).
    * **ELSE** $\rightarrow$ Play **Normal Video** (`video.h`).

## 7. Important Notes for Customization
If you generate new video files using tools like *Image2Cpp*, both header files will default to the same variable names (`epd_bitmap_allArray`). This causes a "Redefinition Error".

**You must rename the variables in `video1.h` manually:**

1.  Open `video1.h`.
2.  Use "Find and Replace" (Ctrl+H) to rename variables:
    * Prefix `epd_bitmap_ezgif_frame_` $\rightarrow$ `frame_alert_`
    * Array `epd_bitmap_allArray` $\rightarrow$ `epd_bitmap_allArray_ALERT`
    * Length `epd_bitmap_allArray_LEN` $\rightarrow$ `epd_bitmap_allArray_LEN_ALERT`
