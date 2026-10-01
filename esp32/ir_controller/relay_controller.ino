#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define DHT_PIN  4
#define DHT_TYPE DHT11
#define IR_PIN   2

// ── Configuration ────────────────────────────────────────
const char* WIFI_SSID      = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";
const char* IR_API_URL     = "http://192.168.1.100:8080/api/ir";
const char* SENSOR_API_URL = "http://192.168.1.100:8080/api/sensor";
const char* LOCATION       = "IR";

const unsigned long POLL_INTERVAL_MS = 2000;

DHT    dht(DHT_PIN, DHT_TYPE);
IRsend irsend(IR_PIN);

// ── State ─────────────────────────────────────────────────
// Track last known mode per device to only transmit on change
// Up to 8 devices; index matches device id - 1
const int MAX_DEVICES = 8;
String currentMode[MAX_DEVICES];
unsigned long lastPollTime = 0;

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();
  irsend.begin();

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
  if (now - lastPollTime >= POLL_INTERVAL_MS) {
    lastPollTime = now;
    if (WiFi.status() == WL_CONNECTED) {
      postSensorData();
      fetchAndTransmitIr();
    } else {
      Serial.println("WiFi disconnected, reconnecting…");
      WiFi.reconnect();
    }
  }
}

// ── Read & post DHT11 ─────────────────────────────────────
void postSensorData() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 read failed, skipping");
    return;
  }

  JsonDocument doc;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["humidity"]    = round(humidity * 10) / 10.0;
  doc["location"]    = LOCATION;

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(SENSOR_API_URL);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);

  if (code == HTTP_CODE_OK) {
    Serial.printf("[%s] Posted — temp: %.1f°C, humidity: %.1f%%\n",
                  LOCATION, temperature, humidity);
  } else {
    Serial.printf("Sensor POST failed, HTTP %d\n", code);
  }
  http.end();
}

// ── Fetch IR states & transmit on change ─────────────────
void fetchAndTransmitIr() {
  HTTPClient http;
  http.begin(IR_API_URL);
  int code = http.GET();

  if (code != HTTP_CODE_OK) {
    Serial.printf("GET IR failed, HTTP %d\n", code);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  // [{"id":1,"name":"AC","mode":"MODE2","rawCode":"0000 006D ..."},...]
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.printf("JSON parse error: %s\n", err.c_str());
    return;
  }

  for (JsonObject device : doc.as<JsonArray>()) {
    int    id      = device["id"].as<int>();
    String name    = device["name"].as<String>();
    String mode    = device["mode"].as<String>();
    String rawCode = device["rawCode"].as<String>();

    if (id < 1 || id > MAX_DEVICES) continue;
    int idx = id - 1;

    // Only transmit if mode changed
    if (mode == currentMode[idx]) continue;
    currentMode[idx] = mode;

    if (mode == "OFF" || rawCode.isEmpty() || rawCode == "null") {
      Serial.printf("[%s] Mode → OFF (no transmission)\n", name.c_str());
      continue;
    }

    // Parse space-separated hex values into uint16_t array
    uint16_t buf[512];
    int      len = 0;
    char     tmp[rawCode.length() + 1];
    rawCode.toCharArray(tmp, sizeof(tmp));
    char* token = strtok(tmp, " ");
    while (token != nullptr && len < 512) {
      buf[len++] = (uint16_t)strtol(token, nullptr, 16);
      token = strtok(nullptr, " ");
    }

    if (len > 0) {
      irsend.sendRaw(buf, len, 38); // 38 kHz carrier
      Serial.printf("[%s] Transmitted mode %s (%d pulses)\n",
                    name.c_str(), mode.c_str(), len);
    } else {
      Serial.printf("[%s] rawCode parse failed\n", name.c_str());
    }
  }
}
