#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>

// WiFi Credentials
#define WIFI_SSID "wifi name"
#define WIFI_PASSWORD "password"

// Firebase Database Credentials
#define DATABASE_URL "https://firebase_url"
#define API_KEY "firebase_key"

// Bin Info
#define BIN_ID "bin1"
#define BIN_LOCATION "near snacker"   // <-- Added location here

// Sensor Pins
#define TRIG_PIN_1 5
#define ECHO_PIN_1 18
#define TRIG_PIN_2 19
#define ECHO_PIN_2 21
#define MQ135_PIN 34  // Analog pin for MQ135 (ADC1 channel only)

// Constants
#define BIN_HEIGHT 37.0   // Bin height in cm
#define RL 10000.0        // Load resistance of MQ135 (in ohms)
#define RO 9252.40        // Your calibrated Ro from clean air

// Thresholds for DANGER detection (approximate)
#define CO2_THRESHOLD 1000
#define NH3_THRESHOLD 50
#define CO_THRESHOLD 35
#define ALCOHOL_THRESHOLD 100

// MQ135 gas calculation constants (based on datasheet curves)
#define PARA_CO2 116.6020682
#define PARB_CO2 -2.769034857

#define PARA_NH3 77.255
#define PARB_NH3 -3.18

#define PARA_CO 605.18
#define PARB_CO -3.937

#define PARA_ALCOHOL 4.8387
#define PARB_ALCOHOL -1.4974

float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, BIN_HEIGHT * 58 + 2000); // Timeout slightly above max height

  if (duration == 0) {
    return -1.0;  // Invalid reading (no echo)
  }

  float distance = duration * 0.034 / 2;
  return (distance > BIN_HEIGHT) ? BIN_HEIGHT : distance;
}

float getFillLevel(float d1, float d2) {
  if (d1 < 0 || d2 < 0) return 0.0; // Handle invalid readings
  float avg = (d1 + d2) / 2.0;
  float fill = ((BIN_HEIGHT - avg) / BIN_HEIGHT) * 100;
  return constrain(fill, 0.0, 100.0);
}

float getResistance() {
  int adcValue = analogRead(MQ135_PIN);
  float voltage = adcValue * (3.3 / 4095.0); // 12-bit ADC
  if (voltage == 0) return -1.0; // Prevent division by zero
  float rs = (3.3 - voltage) * RL / voltage;
  return rs;
}

float getGasPPM(float rs, float para, float parb) {
  if (rs <= 0) return 0.0;
  float ratio = rs / RO;
  return para * pow(ratio, parb);
}

void getAllGasReadings(float &co2, float &nh3, float &co, float &alcohol, String &status) {
  float rs = getResistance();

  if (rs < 0) {
    co2 = nh3 = co = alcohol = 0.0;
    status = "Sensor Error";
    return;
  }

  co2 = getGasPPM(rs, PARA_CO2, PARB_CO2);
  nh3 = getGasPPM(rs, PARA_NH3, PARB_NH3);
  co = getGasPPM(rs, PARA_CO, PARB_CO);
  alcohol = getGasPPM(rs, PARA_ALCOHOL, PARB_ALCOHOL);

  status = "Safe";
  if (co2 > CO2_THRESHOLD || nh3 > NH3_THRESHOLD || co > CO_THRESHOLD || alcohol > ALCOHOL_THRESHOLD) {
    status = "Danger";
  }
}

void sendDataToFirebase(float sensor1, float sensor2, float fillLevel,
                        float co2, float nh3, float co, float alcohol, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(DATABASE_URL) + "/bins/" + BIN_ID + ".json?auth=" + API_KEY;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"id\": \"" + String(BIN_ID) +
                     "\", \"location\": \"" + String(BIN_LOCATION) +
                     "\", \"sensor1\": " + String(sensor1) +
                     ", \"sensor2\": " + String(sensor2) +
                     ", \"fillLevel\": " + String(fillLevel) +
                     ", \"co2\": " + String(co2) +
                     ", \"nh3\": " + String(nh3) +
                     ", \"co\": " + String(co) +
                     ", \"alcohol\": " + String(alcohol) +
                     ", \"status\": \"" + status + "\"}";

    int httpResponseCode = http.PUT(payload);
    if (httpResponseCode > 0) {
      Serial.println("Data sent: " + payload);
    } else {
      Serial.print("Error: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected!");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(MQ135_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void loop() {
  float d1 = getDistance(TRIG_PIN_1, ECHO_PIN_1);
  float d2 = getDistance(TRIG_PIN_2, ECHO_PIN_2);
  float fill = getFillLevel(d1, d2);

  float co2, nh3, co, alcohol;
  String status;
  getAllGasReadings(co2, nh3, co, alcohol, status);

  Serial.print("Sensor1: "); Serial.print(d1); Serial.print(" cm | ");
  Serial.print("Sensor2: "); Serial.print(d2); Serial.print(" cm | ");
  Serial.print("Fill Level: "); Serial.print(fill); Serial.println(" %");

  Serial.print("CO2: "); Serial.print(co2); Serial.print(" ppm | ");
  Serial.print("NH3: "); Serial.print(nh3); Serial.print(" ppm | ");
  Serial.print("CO: "); Serial.print(co); Serial.print(" ppm | ");
  Serial.print("Alcohol: "); Serial.print(alcohol); Serial.print(" ppm | ");
  Serial.print("Status: "); Serial.println(status);

  sendDataToFirebase(d1, d2, fill, co2, nh3, co, alcohol, status);

  delay(5000); // wait 5 seconds
}
