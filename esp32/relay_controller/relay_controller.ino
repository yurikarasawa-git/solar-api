#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ── Configuration ────────────────────────────────────────
const char* WIFI_SSID      = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";
const char* RELAY_API_URL  = "http://192.168.1.100:8080/api/relay?location=2F";
const char* SENSOR_API_URL = "http://192.168.1.100:8080/api/sensor";
const char* LOCATION       = "2F";

#define DHT_PIN  4
#define DHT_TYPE DHT11

// Relay pins for 4-ch module (active-LOW: LOW = ON, HIGH = OFF)
// relayNumber 1→pin 26, 2→27, 3→14, 4→12
const int RELAY_PINS[4] = { 26, 27, 14, 12 };

const unsigned long POLL_INTERVAL_MS = 2000;
const unsigned long DHT_WARMUP_MS    = 2000;

// ── Globals ───────────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
bool currentState[4]      = { false, false, false, false };
unsigned long lastPollTime = 0;

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();
  lastPollTime = millis() + DHT_WARMUP_MS;

  for (int i = 0; i < 4; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH); // active-LOW: start all OFF
  }

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
      fetchAndApplyRelayStates();
    } else {
      Serial.println("WiFi disconnected, reconnecting…");
      WiFi.reconnect();
    }
  }
}

// ── Read & post DHT11 ─────────────────────────────────────
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

// ── Fetch & apply relay states ────────────────────────────
void fetchAndApplyRelayStates() {
  HTTPClient http;
  http.begin(RELAY_API_URL);
  int code = http.GET();

  if (code != HTTP_CODE_OK) {
    Serial.printf("GET relay failed, HTTP %d\n", code);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  // [{"relayNumber":1,"state":false,"location":"2F",...}, ...]
  JsonDocument doc;
  if (deserializeJson(doc, payload)) return;

  for (JsonObject relay : doc.as<JsonArray>()) {
    int  num   = relay["relayNumber"].as<int>();
    bool state = relay["state"].as<bool>();

    // relayNumbers 1–4 map to index 0–3
    if (num < 1 || num > 4) continue;
    int idx = num - 1;

    if (state != currentState[idx]) {
      currentState[idx] = state;
      digitalWrite(RELAY_PINS[idx], state ? LOW : HIGH); // active-LOW
      Serial.printf("Relay %d → %s\n", num, state ? "ON" : "OFF");
    }
  }
}
