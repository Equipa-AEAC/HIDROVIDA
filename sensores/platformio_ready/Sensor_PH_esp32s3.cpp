#include <Arduino.h>
#include <EEPROM.h>
#include <DFRobot_ESP_PH.h>

namespace {
constexpr uint8_t PH_PIN = 4;
constexpr float ESP32_ADC_MAX = 4095.0f;
constexpr float ESP32_VREF_MV = 3300.0f;
constexpr uint8_t ADC_SAMPLES = 10;
constexpr uint32_t READ_INTERVAL_MS = 1000;
constexpr float DEFAULT_TEMP_C = 25.0f;

DFRobot_ESP_PH ph;
uint32_t lastReadMs = 0;

float readAveragedVoltageMv(uint8_t pin, uint8_t samples) {
  uint32_t adcSum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    adcSum += static_cast<uint32_t>(analogRead(pin));
    delay(10);
  }

  const float adcAverage = static_cast<float>(adcSum) / static_cast<float>(samples);
  return (adcAverage / ESP32_ADC_MAX) * ESP32_VREF_MV;
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("[BOOT] ESP32-S3 pH sensor test starting...");
  Serial.printf("[BOOT] pH analog pin: GPIO %u\n", PH_PIN);

  analogReadResolution(12);
  analogSetPinAttenuation(PH_PIN, ADC_11db);

  EEPROM.begin(32);
  ph.begin();

  Serial.println("[INFO] pH library initialized.");
  Serial.println("[INFO] Reading pH every 1 second.");
  Serial.println("[INFO] Serial calibration commands: enterph, calph, exitph");
}

void loop() {
  if (millis() - lastReadMs < READ_INTERVAL_MS) {
    return;
  }
  lastReadMs = millis();

  const float temperatureC = DEFAULT_TEMP_C;
  const float voltageMv = readAveragedVoltageMv(PH_PIN, ADC_SAMPLES);
  const float phValue = ph.readPH(voltageMv, temperatureC);

  if (!isfinite(phValue) || phValue < 0.0f || phValue > 14.0f) {
    Serial.printf("[WARN] Invalid pH reading. V=%.2f mV Temp=%.1f C\n", voltageMv, temperatureC);
  } else {
    Serial.printf("pH: %.2f (%.2f mV)\n", phValue, voltageMv);
  }

  ph.calibration(voltageMv, temperatureC);
}
