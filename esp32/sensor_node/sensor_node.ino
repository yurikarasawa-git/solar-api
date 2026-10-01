#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ── Configuration ────────────────────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* API_URL       = "http://192.168.1.100:8080/api/sensor";
const char* LOCATION      = "1F";  // change per device

#define DHT_PIN  4
#define DHT_TYPE DHT11

const unsigned long POST_INTERVAL_MS = 10000;
const unsigned long DHT_WARMUP_MS    = 2000;  // DHT11 needs ~1s after power-on before first read

// ── Globals ───────────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
unsigned long lastPostTime = 0;

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();
  lastPostTime = millis() + DHT_WARMUP_MS;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 10000) {
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nConnected — IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nWiFi connect timeout, will retry in loop");
  }
}

// ── Loop ──────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();
  if (now - lastPostTime >= POST_INTERVAL_MS) {
    lastPostTime = now;
    if (WiFi.status() == WL_CONNECTED) {
      postSensorData();
    } else {
      Serial.println("WiFi disconnected, reconnecting…");
      WiFi.reconnect();
    }
  }
}

// ── Read & post ───────────────────────────────────────────
void postSensorData() {
  float temperature = NAN, humidity = NAN;
  for (int i = 0; i < 3 && (isnan(temperature) || isnan(humidity)); i++) {
    if (i > 0) delay(500);
    temperature = dht.readTemperature();
    humidity    = dht.readHumidity();
  }

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 read failed after 3 attempts, skipping");
    return;
  }

  JsonDocument doc;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["humidity"]    = round(humidity * 10) / 10.0;
  doc["location"]    = LOCATION;

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);

  if (code == HTTP_CODE_OK) {
    Serial.printf("[%s] Posted — temp: %.1f°C, humidity: %.1f%%\n",
                  LOCATION, temperature, humidity);
  } else {
    Serial.printf("POST failed, HTTP %d\n", code);
  }
  http.end();
}
