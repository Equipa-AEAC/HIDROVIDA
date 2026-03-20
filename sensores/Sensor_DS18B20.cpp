#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

namespace {
constexpr uint8_t ONE_WIRE_BUS_PIN = 4;
constexpr uint32_t READ_INTERVAL_MS = 1000;

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress deviceAddress;

uint8_t sensorCount = 0;
uint32_t lastReadMs = 0;

void printAddress(const DeviceAddress address) {
	for (uint8_t i = 0; i < 8; i++) {
		if (address[i] < 16) {
			Serial.print("0");
		}
		Serial.print(address[i], HEX);
	}
}
}  // namespace

void setup() {
	Serial.begin(115200);
	delay(500);

	Serial.println();
	Serial.println("[BOOT] ESP32 + DS18B20 test starting...");
	Serial.printf("[BOOT] OneWire data pin: GPIO %u\n", ONE_WIRE_BUS_PIN);

	sensors.begin();
	sensorCount = sensors.getDeviceCount();

	Serial.printf("[INFO] Sensors found on OneWire bus: %u\n", sensorCount);

	if (sensorCount == 0) {
		Serial.println("[WARN] No DS18B20 detected. Check wiring, pull-up resistor (4.7k), and power.");
		return;
	}

	for (uint8_t i = 0; i < sensorCount; i++) {
		if (sensors.getAddress(deviceAddress, i)) {
			Serial.printf("[INFO] Sensor %u address: ", i);
			printAddress(deviceAddress);
			Serial.println();
		} else {
			Serial.printf("[WARN] Could not read address for sensor index %u\n", i);
		}
	}
}

void loop() {
	if (millis() - lastReadMs < READ_INTERVAL_MS) {
		return;
	}
	lastReadMs = millis();

	if (sensorCount == 0) {
		Serial.println("[WARN] Still no sensors detected.");
		return;
	}

	sensors.requestTemperatures();

	for (uint8_t i = 0; i < sensorCount; i++) {
		float tempC = sensors.getTempCByIndex(i);

		if (tempC == DEVICE_DISCONNECTED_C) {
			Serial.printf("[ERROR] Sensor %u disconnected or read error.\n", i);
			continue;
		}

		Serial.printf("[TEMP] Sensor %u: %.2f C\n", i, tempC);
	}
}