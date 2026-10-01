#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <INA226.h>  // Rob Tillaart's INA226 library

// ── Configuration ────────────────────────────────────────
const char* WIFI_SSID      = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD  = "YOUR_WIFI_PASSWORD";
const char* SENSOR_API_URL = "http://192.168.1.100:8080/api/sensor";
const char* RELAY_API_URL  = "http://192.168.1.100:8080/api/relay?location=1F";
const char* LOCATION       = "1F";

// GPIO wired to the inverter relay/transistor
#define INVERTER_PIN  5

#define DHT_PIN   4
#define DHT_TYPE  DHT11

// INA226: I2C address 0x40 (A0+A1 both GND)
#define INA226_ADDR  0x40
// 50A/75mV shunt = 0.0015 Ω
#define SHUNT_OHMS   0.0015

const unsigned long POLL_INTERVAL_MS = 10000;
const unsigned long DHT_WARMUP_MS    = 2000;

// ── Globals ───────────────────────────────────────────────
DHT    dht(DHT_PIN, DHT_TYPE);
INA226 ina226(INA226_ADDR);
unsigned long lastPollTime = 0;
bool inaOk                 = false;
bool inverterState         = false;

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(INVERTER_PIN, OUTPUT);
  digitalWrite(INVERTER_PIN, LOW); // inverter OFF on boot

  dht.begin();
  lastPollTime = millis() + DHT_WARMUP_MS;

  Wire.begin();
  ina226.begin();
  if (ina226.isConnected()) {
    ina226.setMaxCurrentShunt(50, SHUNT_OHMS);
    inaOk = true;
    Serial.println("INA226 ready");
  } else {
    Serial.println("INA226 not found — check wiring");
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
      fetchAndApplyRelays();
      postSensorData();
    } else {
      Serial.println("WiFi disconnected, reconnecting…");
      WiFi.reconnect();
    }
  }
}

// ── Fetch relay states & apply inverter pin ───────────────
void fetchAndApplyRelays() {
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

  // [{"relayNumber":5,"state":true,"isInverter":true,...}, ...]
  JsonDocument doc;
  if (deserializeJson(doc, payload)) return;

  for (JsonObject r : doc.as<JsonArray>()) {
    if (!r["isInverter"].as<bool>()) continue; // only care about the inverter relay

    bool newState = r["state"].as<bool>();
    if (newState != inverterState) {
      inverterState = newState;
      digitalWrite(INVERTER_PIN, inverterState ? HIGH : LOW);
      Serial.printf("Inverter → %s\n", inverterState ? "ON" : "OFF");
    }
    break;
  }
}

// ── Read & post sensor data ───────────────────────────────
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

  float voltage = 0, current = 0;
  if (inaOk) {
    voltage = ina226.getBusVoltage();
    current = ina226.getCurrent();
  }

  JsonDocument doc;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["humidity"]    = round(humidity * 10) / 10.0;
  doc["location"]    = LOCATION;
  if (inaOk) {
    doc["batteryVoltage"]  = round(voltage * 100) / 100.0;
    doc["chargingCurrent"] = round(current * 100) / 100.0;
  }

  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(SENSOR_API_URL);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);

  if (code == HTTP_CODE_OK) {
    Serial.printf("[%s] Posted — %.1f°C  %.1f%%  %.2fV  %.2fA\n",
                  LOCATION, temperature, humidity, voltage, current);
  } else {
    Serial.printf("Sensor POST failed, HTTP %d\n", code);
  }
  http.end();
}
