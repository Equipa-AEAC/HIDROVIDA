#include <Arduino.h>
#include <EEPROM.h>
#include <GravityTDS.h>

namespace {
constexpr uint8_t TDS_PIN = 35;
constexpr uint32_t READ_INTERVAL_MS = 1000;
constexpr uint8_t TDS_SAMPLES = 10;  // 10-20 samples recommended for stable output
constexpr float DEFAULT_TEMPERATURE_C = 25.0f;
constexpr uint8_t DEBUG_ADC_SAMPLES = 12;
constexpr uint16_t ADC_LOW_THRESHOLD = 80;

GravityTDS gravityTds;
uint32_t lastReadMs = 0;

float readAveragedTds(float temperatureC, uint8_t samples) {
	float sumPpm = 0.0f;

	for (uint8_t i = 0; i < samples; i++) {
		gravityTds.setTemperature(temperatureC);
		gravityTds.update();
		sumPpm += gravityTds.getTdsValue();
		delay(20);
	}

	return sumPpm / static_cast<float>(samples);
}

uint16_t readAverageRawAdc(uint8_t pin, uint8_t samples) {
	uint32_t sum = 0;
	for (uint8_t i = 0; i < samples; i++) {
		sum += static_cast<uint32_t>(analogRead(pin));
		delay(5);
	}
	return static_cast<uint16_t>(sum / samples);
}
}  // namespace

void setup() {
	Serial.begin(115200);
	delay(500);

	Serial.println();
	Serial.println("[BOOT] ESP32 Gravity TDS test starting...");
	Serial.printf("[BOOT] TDS analog pin: GPIO %u\n", TDS_PIN);

	analogReadResolution(12);
	analogSetPinAttenuation(TDS_PIN, ADC_11db);
	EEPROM.begin(64);

	gravityTds.setPin(TDS_PIN);
	gravityTds.setAref(3.3f);
	gravityTds.setAdcRange(4096.0f);
	gravityTds.begin();

	Serial.println("[INFO] GravityTDS initialized.");
	Serial.println("[INFO] Reading TDS every 1 second.");
}

void loop() {
	if (millis() - lastReadMs < READ_INTERVAL_MS) {
		return;
	}
	lastReadMs = millis();

	// Optional improvement: replace this with DS18B20 measurement for better compensation.
	float waterTemperatureC = DEFAULT_TEMPERATURE_C;
	float tdsPpm = readAveragedTds(waterTemperatureC, TDS_SAMPLES);

	if (!isfinite(tdsPpm) || tdsPpm < 0.0f || tdsPpm > 5000.0f) {
		Serial.printf("[WARN] Invalid TDS reading. Temp=%.1f C\n", waterTemperatureC);
		return;
	}

	if (tdsPpm <= 1.0f) {
		uint16_t rawAvg = readAverageRawAdc(TDS_PIN, DEBUG_ADC_SAMPLES);
		Serial.printf("[WARN] Very low TDS (%.0f ppm). ADC(avg)=%u. Check probe in water, board VCC, and shared GND.\n", tdsPpm, rawAvg);
		if (rawAvg < ADC_LOW_THRESHOLD) {
			Serial.println("[DEBUG] ADC is near zero. Sensor output may be disconnected or no common ground.");
		}
	}

	Serial.printf("TDS: %.0f ppm\n", tdsPpm);
}
