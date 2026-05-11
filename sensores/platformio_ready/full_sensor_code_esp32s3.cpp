#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_PH.h>
#include <GravityTDS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cstring>

namespace {
constexpr uint8_t DS18B20_PIN = 3;
constexpr uint8_t PH_PIN = 4;
constexpr uint8_t TDS_PIN = 5;
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;

constexpr int8_t TANK_1_DETECT_PIN = -1;
constexpr int8_t TANK_2_DETECT_PIN = -1;

constexpr uint16_t DISPLAY_WIDTH = 128;
constexpr uint16_t DISPLAY_HEIGHT = 64;
constexpr uint8_t OLED_RESET_PIN = 255;
constexpr uint8_t OLED_I2C_ADDR = 0x3C;

constexpr uint32_t SENSOR_INTERVAL_MS = 1000;
constexpr uint32_t DS18B20_CONVERSION_MS = 750;

constexpr uint8_t ANALOG_SAMPLES = 10;
constexpr float ADC_MAX = 4095.0f;
constexpr float ADC_REF_MV = 3300.0f;
constexpr float DEFAULT_TEMP_C = 25.0f;

constexpr float TEMP_LOW_C = 18.0f;
constexpr float TEMP_HIGH_C = 28.0f;
constexpr float PH_LOW = 5.5f;
constexpr float PH_HIGH = 6.5f;
constexpr float TDS_LOW = 500.0f;
constexpr float TDS_HIGH = 1500.0f;
constexpr int WATER_LOW_PERCENT = 20;

constexpr uint8_t WATER_LOW_ADDR_PRIMARY = 0x77;
constexpr uint8_t WATER_HIGH_ADDR_PRIMARY = 0x78;
constexpr uint8_t WATER_LOW_ADDR_ALT = 0x3B;
constexpr uint8_t WATER_HIGH_ADDR_ALT = 0x3C;

// Adjust these to the real marks painted on your Grove 10 cm sensor.
constexpr int TANK_1_MARK_PERCENT = 35;
constexpr int TANK_2_MARK_PERCENT = 70;

enum class DetectorState : uint8_t {
  Disabled,
  Detected,
  NotDetected,
};

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
DFRobot_ESP_PH ph;
GravityTDS tds;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET_PIN);

float g_temperatureC = NAN;
float g_ph = NAN;
float g_tdsPpm = NAN;
int g_waterPercent = -1;
DetectorState g_tank1State = DetectorState::Disabled;
DetectorState g_tank2State = DetectorState::Disabled;

uint32_t g_lastSensorTickMs = 0;
uint32_t g_lastDsRequestMs = 0;
bool g_dsRequested = false;
bool g_displayReady = false;

float readAverageVoltageMv(uint8_t pin, uint8_t samples) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += static_cast<uint32_t>(analogRead(pin));
    delay(10);
  }
  const float avg = static_cast<float>(sum) / static_cast<float>(samples);
  return (avg / ADC_MAX) * ADC_REF_MV;
}

float readAverageTdsPpm(float temperatureC, uint8_t samples) {
  float sum = 0.0f;
  for (uint8_t i = 0; i < samples; i++) {
    tds.setTemperature(temperatureC);
    tds.update();
    sum += tds.getTdsValue();
    delay(20);
  }
  return sum / static_cast<float>(samples);
}

bool readWaterFrame(uint8_t lowAddr, uint8_t highAddr, uint8_t* lowData, uint8_t* highData) {
  const int lowCount = Wire.requestFrom(static_cast<int>(lowAddr), 8);
  if (lowCount != 8) {
    return false;
  }
  for (int i = 0; i < 8; i++) {
    lowData[i] = Wire.read();
  }

  const int highCount = Wire.requestFrom(static_cast<int>(highAddr), 12);
  if (highCount != 12) {
    return false;
  }
  for (int i = 0; i < 12; i++) {
    highData[i] = Wire.read();
  }
  return true;
}

int touchedSectionsFromRaw(const uint8_t* lowData, const uint8_t* highData) {
  constexpr int threshold = 100;
  uint32_t touchMask = 0;

  for (int i = 0; i < 8; i++) {
    if (lowData[i] > threshold) {
      touchMask |= static_cast<uint32_t>(1) << i;
    }
  }

  for (int i = 0; i < 12; i++) {
    if (highData[i] > threshold) {
      touchMask |= static_cast<uint32_t>(1) << (8 + i);
    }
  }

  int sections = 0;
  while (touchMask & 0x01u) {
    sections++;
    touchMask >>= 1;
  }
  return sections;
}

int readWaterLevelPercentage() {
  uint8_t lowData[8] = {0};
  uint8_t highData[12] = {0};

  bool ok = readWaterFrame(WATER_LOW_ADDR_PRIMARY, WATER_HIGH_ADDR_PRIMARY, lowData, highData);
  if (!ok) {
    ok = readWaterFrame(WATER_LOW_ADDR_ALT, WATER_HIGH_ADDR_ALT, lowData, highData);
  }
  if (!ok) {
    return -1;
  }

  int computed = touchedSectionsFromRaw(lowData, highData) * 5;
  if (computed < 0) {
    computed = 0;
  }
  if (computed > 100) {
    computed = 100;
  }
  return computed;
}

float readTemperature() {
  const float tempC = ds18b20.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C) {
    return NAN;
  }
  return tempC;
}

float readPH() {
  const float temp = isfinite(g_temperatureC) ? g_temperatureC : DEFAULT_TEMP_C;
  const float voltageMv = readAverageVoltageMv(PH_PIN, ANALOG_SAMPLES);
  const float value = ph.readPH(voltageMv, temp);
  ph.calibration(voltageMv, temp);
  return value;
}

float readTDS(float temperatureC) {
  const float temp = isfinite(temperatureC) ? temperatureC : DEFAULT_TEMP_C;
  return readAverageTdsPpm(temp, ANALOG_SAMPLES);
}

DetectorState readDetector(int8_t pin) {
  if (pin < 0) {
    return DetectorState::Disabled;
  }
  return (digitalRead(pin) == LOW) ? DetectorState::Detected : DetectorState::NotDetected;
}

const char* detectorStateText(DetectorState state) {
  switch (state) {
    case DetectorState::Disabled:
      return "DISABLED";
    case DetectorState::Detected:
      return "DETECTED";
    case DetectorState::NotDetected:
      return "CLEAR";
    default:
      return "UNKNOWN";
  }
}

const char* tankZoneText(int percentage) {
  if (percentage < 0) {
    return "sensor_error";
  }
  if (percentage < TANK_1_MARK_PERCENT) {
    return "below_tank_1_mark";
  }
  if (percentage < TANK_2_MARK_PERCENT) {
    return "between_tank_1_and_tank_2";
  }
  return "above_tank_2_mark";
}

const char* rangeState(float value, float low, float high) {
  if (!isfinite(value)) {
    return "ERR";
  }
  if (value < low) {
    return "LOW";
  }
  if (value > high) {
    return "HIGH";
  }
  return "OK";
}

const char* waterState(int value) {
  if (value < 0 || value > 100) {
    return "ERR";
  }
  if (value < WATER_LOW_PERCENT) {
    return "LOW";
  }
  return "OK";
}

bool isWarnState(const char* state) {
  return strcmp(state, "OK") != 0;
}

void drawMetricLine(int16_t y, const char* label, float value, uint8_t decimals, const char* unit, const char* state, bool blinkOn) {
  display.setCursor(0, y);
  display.print(label);
  if (isfinite(value)) {
    display.print(value, decimals);
  } else {
    display.print("--");
  }
  display.print(unit);

  if (isWarnState(state) && blinkOn) {
    display.print(" ");
    display.print(state);
  }
}

void drawWaterLine(int16_t y, int percentage, const char* state, bool blinkOn) {
  display.setCursor(0, y);
  display.print("Water:");
  if (percentage >= 0) {
    display.print(percentage);
    display.print("%");
  } else {
    display.print("--");
  }

  if (isWarnState(state) && blinkOn) {
    display.print(" ");
    display.print(state);
  }
}

void drawDashboard(bool blinkOn) {
  if (!g_displayReady) {
    return;
  }

  const char* tState = rangeState(g_temperatureC, TEMP_LOW_C, TEMP_HIGH_C);
  const char* pState = rangeState(g_ph, PH_LOW, PH_HIGH);
  const char* tdsState = rangeState(g_tdsPpm, TDS_LOW, TDS_HIGH);
  const char* wState = waterState(g_waterPercent);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  drawMetricLine(0, "Temp:", g_temperatureC, 1, "C", tState, blinkOn);
  drawMetricLine(16, "pH:", g_ph, 2, "", pState, blinkOn);
  drawMetricLine(32, "TDS:", g_tdsPpm, 0, "ppm", tdsState, blinkOn);
  drawWaterLine(48, g_waterPercent, wState, blinkOn);

  display.display();
}

void scanI2CBus() {
  Serial.println("[INFO] Scanning I2C bus...");
  int found = 0;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("[I2C] Device at 0x");
      if (addr < 16) {
        Serial.print('0');
      }
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("[WARN] No I2C devices found.");
  }
}

void printSerialStatus() {
  Serial.printf("Temp=%.1fC, pH=%.2f, TDS=%.0fppm, Water=%d%%, Zone=%s, Tank1=%s, Tank2=%s\n",
                g_temperatureC,
                g_ph,
                g_tdsPpm,
                g_waterPercent,
                tankZoneText(g_waterPercent),
                detectorStateText(g_tank1State),
                detectorStateText(g_tank2State));
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  analogReadResolution(12);
  analogSetPinAttenuation(PH_PIN, ADC_11db);
  analogSetPinAttenuation(TDS_PIN, ADC_11db);

  EEPROM.begin(64);

  ds18b20.begin();
  ds18b20.setWaitForConversion(false);
  ds18b20.requestTemperatures();
  g_lastDsRequestMs = millis();
  g_dsRequested = true;

  ph.begin();

  tds.setPin(TDS_PIN);
  tds.setAref(3.3f);
  tds.setAdcRange(4096.0f);
  tds.begin();

  if (TANK_1_DETECT_PIN >= 0) {
    pinMode(TANK_1_DETECT_PIN, INPUT_PULLUP);
  }
  if (TANK_2_DETECT_PIN >= 0) {
    pinMode(TANK_2_DETECT_PIN, INPUT_PULLUP);
  }

  scanI2CBus();

  g_displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
  if (!g_displayReady) {
    Serial.println("[WARN] SSD1306 initialization failed.");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Hydroponics monitor");
    display.println("Initializing...");
    display.display();
  }

  Serial.println("[INFO] Hydroponics dashboard started.");
  Serial.printf("[INFO] Tank 1 water mark: %d%%\n", TANK_1_MARK_PERCENT);
  Serial.printf("[INFO] Tank 2 water mark: %d%%\n", TANK_2_MARK_PERCENT);
}

void loop() {
  const uint32_t now = millis();

  if (g_dsRequested && (now - g_lastDsRequestMs >= DS18B20_CONVERSION_MS)) {
    g_temperatureC = readTemperature();
    g_dsRequested = false;
  }

  if (now - g_lastSensorTickMs >= SENSOR_INTERVAL_MS) {
    g_lastSensorTickMs = now;

    if (!g_dsRequested) {
      ds18b20.requestTemperatures();
      g_lastDsRequestMs = now;
      g_dsRequested = true;
    }

    g_ph = readPH();
    g_tdsPpm = readTDS(g_temperatureC);
    g_waterPercent = readWaterLevelPercentage();
    g_tank1State = readDetector(TANK_1_DETECT_PIN);
    g_tank2State = readDetector(TANK_2_DETECT_PIN);
    printSerialStatus();
  }

  const bool blinkOn = ((now / 1000u) % 2u) == 0u;
  drawDashboard(blinkOn);
}
