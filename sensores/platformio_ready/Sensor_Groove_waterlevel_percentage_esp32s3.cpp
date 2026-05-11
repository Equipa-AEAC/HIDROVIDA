#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;
constexpr uint8_t WATER_LOW_ADDR_PRIMARY = 0x77;
constexpr uint8_t WATER_HIGH_ADDR_PRIMARY = 0x78;
constexpr uint8_t WATER_LOW_ADDR_ALT = 0x3B;
constexpr uint8_t WATER_HIGH_ADDR_ALT = 0x3C;
constexpr uint32_t UPDATE_INTERVAL_MS = 1000;

// Adjust these after checking where your real tank marks land on the sensor.
constexpr int TANK_1_MARK_PERCENT = 35;
constexpr int TANK_2_MARK_PERCENT = 70;

uint32_t lastUpdateMs = 0;

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

  int percentage = touchedSectionsFromRaw(lowData, highData) * 5;
  if (percentage < 0) {
    percentage = 0;
  }
  if (percentage > 100) {
    percentage = 100;
  }
  return percentage;
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
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println("[BOOT] ESP32-S3 Grove water level test starting...");
  Serial.printf("[BOOT] I2C SDA=%u SCL=%u\n", I2C_SDA_PIN, I2C_SCL_PIN);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  scanI2CBus();

  Serial.printf("[INFO] Tank 1 marker: %d%%\n", TANK_1_MARK_PERCENT);
  Serial.printf("[INFO] Tank 2 marker: %d%%\n", TANK_2_MARK_PERCENT);
}

void loop() {
  if (millis() - lastUpdateMs < UPDATE_INTERVAL_MS) {
    return;
  }
  lastUpdateMs = millis();

  const int percentage = readWaterLevelPercentage();
  if (percentage < 0) {
    Serial.println("[ERROR] Water level sensor read failed.");
    return;
  }

  Serial.printf("Water: %d%% (%s)\n", percentage, tankZoneText(percentage));
}
