# ♻️ Smart Waste Bin using ESP32

A real-time smart waste monitoring system using ESP32, dual HC-SR04 ultrasonic sensors, an MQ135 gas sensor, and Firebase integration. The project aims to improve waste collection efficiency and monitor harmful gas emissions from dustbins.

---

## 📦 Features

- Real-time waste fill-level monitoring  
- Air quality monitoring inside the bin  
- Dual ultrasonic sensors for accurate detection  
- Gas sensing with MQ135 for CO₂, NH₃, Benzene, etc.  
- Data uploaded to Firebase Realtime Database via HTTP  
- Supports multiple bins using unique Bin IDs  

---

## 🧠 Working Principle

- **Fill-Level Detection:**  
  Two HC-SR04 ultrasonic sensors mounted on the bin measure the distance to the waste. The ESP32 calculates the average of the two readings to determine how full the bin is.

- **Gas Monitoring:**  
  The MQ135 gas sensor detects various harmful gases released from waste. The analog output is read by the ESP32 and sent as part of the data.

- **Data Transmission:**  
  The ESP32 connects to Wi-Fi, creates a JSON object containing:
  - Fill level (%)
  - Gas sensor reading
  - Timestamp
  - Bin ID  
  It then sends this data to Firebase using HTTP PUT or POST requests with the HTTPClient library.

---

## 🔧 Hardware Used

- ESP32 Development Board  
- 2 × HC-SR04 Ultrasonic Sensors  
- 1 × MQ135 Gas Sensor  
- Breadboard and Jumper Wires  
- Power Source (USB or Battery)

---

## 🔌 Connections

| Component        | ESP32 Pin          |
|------------------|--------------------|
| HC-SR04 #1 Trig  | GPIO 12            |
| HC-SR04 #1 Echo  | GPIO 14            |
| HC-SR04 #2 Trig  | GPIO 27            |
| HC-SR04 #2 Echo  | GPIO 26            |
| MQ135 Analog Out | GPIO 34 (ADC)      |
| VCC & GND        | 3.3V / GND         |

> Modify pin numbers in the code as per your connections.

---

## 🧑‍💻 Software Requirements

- Arduino IDE  
- ESP32 Board package installed  
- Required libraries:  
  - WiFi.h  
  - HTTPClient.h  

---

## 🔐 Firebase Setup

1. Go to [Firebase Console](https://console.firebase.google.com/) and create a project.
2. Set up a Realtime Database.
3. In "Rules", enable read and write access (for testing):
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
