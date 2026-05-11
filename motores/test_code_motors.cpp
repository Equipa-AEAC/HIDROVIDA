// ESP32-S3 sequential test for 4 nutrient pumps.
// Pump mapping used by the project:
// Nutrient 1 -> GPIO37
// Nutrient 2 -> GPIO38
// Nutrient 3 -> GPIO39
// Nutrient 4 -> GPIO40

#include <Arduino.h>

constexpr uint8_t PUMP_1_PIN = 10;
constexpr uint8_t PUMP_2_PIN = 11;
constexpr uint8_t PUMP_3_PIN = 12;
constexpr uint8_t PUMP_4_PIN = 13;

constexpr uint8_t PUMP_ON = LOW;
constexpr uint8_t PUMP_OFF = HIGH;

constexpr uint32_t PUMP_ON_TIME_MS = 2000;
constexpr uint32_t PUMP_PAUSE_MS = 1000;

struct PumpChannel {
	uint8_t pin;
	const char* name;
};

PumpChannel pumps[] = {
		{PUMP_1_PIN, "Nutrient 1"},
		{PUMP_2_PIN, "Nutrient 2"},
		{PUMP_3_PIN, "Nutrient 3"},
		{PUMP_4_PIN, "Nutrient 4"},
};

void allPumpsOff() {
	for (const auto& pump : pumps) {
		digitalWrite(pump.pin, PUMP_OFF);
	}
}

void testPump(const PumpChannel& pump) {
	Serial.print("ON  ");
	Serial.println(pump.name);
	digitalWrite(pump.pin, PUMP_ON);
	delay(PUMP_ON_TIME_MS);

	Serial.print("OFF ");
	Serial.println(pump.name);
	digitalWrite(pump.pin, PUMP_OFF);
	delay(PUMP_PAUSE_MS);
}

void setup() {
	Serial.begin(115200);
	delay(500);

	for (const auto& pump : pumps) {
		pinMode(pump.pin, OUTPUT);
	}

	allPumpsOff();

	Serial.println();
	Serial.println("4-pump nutrient test started");
	Serial.println("Active-low outputs: LOW = ON, HIGH = OFF");
}

void loop() {
	for (const auto& pump : pumps) {
		testPump(pump);
	}

	Serial.println("Cycle complete, restarting in 5 seconds");
	allPumpsOff();
	delay(5000);
}
