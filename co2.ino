#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <SensirionI2cScd4x.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

// Config
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";
String ML_SERVER_URL = "";
bool ML_ENABLED = false;

#define FAST_TEST_MODE false

// Relay pins (active LOW)
#define RELAY_SOLENOID 27
#define RELAY_PUMP1 26
#define RELAY_PUMP2 33
#define RELAY_AIRPUMP 25
#define BUILTIN_LED 2

// I2C
#define LCD_SDA 18
#define LCD_SCL 19
#define SCD40_SDA 21
#define SCD40_SCL 22

#define RELAY_ON(pin) digitalWrite(pin, LOW)
#define RELAY_OFF(pin) digitalWrite(pin, HIGH)

// Timing
#if FAST_TEST_MODE
#define PHASE1_TIME 10000UL
#define PHASE2_TIME 30000UL
#define PHASE3_TIME 10000UL
#define SETTLING_TIME 15000UL
#define PHASE4_TIME 5000UL
#else
#define PHASE1_TIME 60000UL
#define PHASE2_TIME 900000UL
#define PHASE3_TIME 300000UL
#define SETTLING_TIME 600000UL
#define PHASE4_TIME 180000UL
#endif

#define SENSOR_INTERVAL 6000UL
#define LCD_INTERVAL 1000UL
#define MAX_AIR_TIME 60000UL

// Phases
enum Phase {
  AIR_IN = 1,
  ABSORPTION,
  TRANSPORT,
  SETTLING,
  FORM
};

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
SensirionI2cScd4x scd4x;
WebServer server(80);

// State
Phase currentPhase = AIR_IN;
unsigned long phaseStart = 0;
unsigned long phaseDuration = PHASE1_TIME;
unsigned long mlAirTime = PHASE1_TIME;

uint16_t co2 = 0;
float tempC = 0;
float hum = 0;

unsigned long lastSensor = 0;
unsigned long lastLCD = 0;

bool mlDone = false;

// Helpers
void allOff() {
  RELAY_OFF(RELAY_SOLENOID);
  RELAY_OFF(RELAY_PUMP1);
  RELAY_OFF(RELAY_PUMP2);
  RELAY_OFF(RELAY_AIRPUMP);
}

// Setup
void setup() {
  Serial.begin(115200);

  // Prevent relay glitch at boot
  digitalWrite(RELAY_SOLENOID, HIGH);
  digitalWrite(RELAY_PUMP1, HIGH);
  digitalWrite(RELAY_PUMP2, HIGH);
  digitalWrite(RELAY_AIRPUMP, HIGH);

  pinMode(RELAY_SOLENOID, OUTPUT);
  pinMode(RELAY_PUMP1, OUTPUT);
  pinMode(RELAY_PUMP2, OUTPUT);
  pinMode(RELAY_AIRPUMP, OUTPUT);

  allOff();

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  // LCD
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.print("Air-CCS");

  // Sensor
  Wire1.begin(SCD40_SDA, SCD40_SCL);
  scd4x.begin(Wire1, SCD40_I2C_ADDR_62);
  scd4x.stopPeriodicMeasurement();
  delay(500);
  scd4x.startPeriodicMeasurement();

  // WiFi (optional)
  if (WIFI_SSID[0] != '\0') {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  // Start system
  currentPhase = AIR_IN;
  phaseStart = millis();
  phaseDuration = mlAirTime;

  RELAY_ON(RELAY_SOLENOID);
}

// Loop
void loop() {
  unsigned long now = millis();

  server.handleClient();

  // Sensor
  if (now - lastSensor >= SENSOR_INTERVAL) {
    lastSensor = now;

    bool ready = false;
    scd4x.getDataReadyStatus(ready);

    if (ready) {
      uint16_t rawCO2;
      float t, h;

      if (!scd4x.readMeasurement(rawCO2, t, h)) {
        if (rawCO2 > 0 && rawCO2 < 5000) {
          co2 = rawCO2;
        }
        tempC = t;
        hum = h;
      }
    }
  }

  // LCD (no flicker)
  if (now - lastLCD >= LCD_INTERVAL) {
    lastLCD = now;

    lcd.setCursor(0, 0);
    lcd.print("CO2:");
    lcd.print(co2);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(currentPhase);
    lcd.print("   ");
  }

  // FSM
  if (now - phaseStart >= phaseDuration) {
    allOff();

    switch (currentPhase) {

      case AIR_IN:
        currentPhase = ABSORPTION;
        phaseDuration = PHASE2_TIME;
        RELAY_ON(RELAY_AIRPUMP);
        break;

      case ABSORPTION:
        currentPhase = TRANSPORT;
        phaseDuration = PHASE3_TIME;
        RELAY_ON(RELAY_PUMP1);
        break;

      case TRANSPORT:
        currentPhase = SETTLING;
        phaseDuration = SETTLING_TIME;
        break;

      case SETTLING:
        currentPhase = FORM;
        phaseDuration = PHASE4_TIME;
        RELAY_ON(RELAY_PUMP2);
        break;

      case FORM:
        currentPhase = AIR_IN;
        phaseDuration = mlAirTime;
        RELAY_ON(RELAY_SOLENOID);
        mlDone = false; // allow ML update next cycle
        break;
    }

    phaseStart = millis();
  }

  // ML only updates Phase 1 timing
  if (!mlDone && ML_ENABLED && ML_SERVER_URL.length() > 0) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(ML_SERVER_URL + "/api/parameters");

      int code = http.GET();
      if (code == 200) {
        String res = http.getString();

        StaticJsonDocument<256> doc;
        if (!deserializeJson(doc, res)) {
          if (doc.containsKey("air_in_time")) {
            unsigned long val = doc["air_in_time"];
            mlAirTime = constrain(val * 1000UL, 5000UL, MAX_AIR_TIME);
          }
        }
      }

      http.end();
      mlDone = true;
    }
  }

  // Debug (optional but useful)
  Serial.print("CO2: ");
  Serial.println(co2);
}