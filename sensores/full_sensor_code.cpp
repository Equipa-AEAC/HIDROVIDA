#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_PH.h>
#include <GravityTDS.h>
#include <waterlevelsensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <cstring>

namespace {
constexpr uint8_t DS18B20_PIN = 7;
constexpr uint8_t PH_PIN = 4;
constexpr uint8_t TDS_PIN = 5;
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;

constexpr uint16_t DISPLAY_WIDTH = 128;
constexpr uint16_t DISPLAY_HEIGHT = 64;
constexpr uint8_t OLED_RESET_PIN = 255;  // Shared reset pin not connected.
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

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
DFRobot_ESP_PH ph;
GravityTDS tds;
WaterLevelSensor waterLevel;
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, OLED_RESET_PIN);

float g_temperatureC = NAN;
float g_ph = NAN;
float g_tdsPpm = NAN;
int g_waterPercent = -1;

uint32_t g_lastSensorTickMs = 0;
uint32_t g_lastDsRequestMs = 0;
bool g_dsRequested = false;

float readAverageVoltageMv(uint8_t pin, uint8_t samples) {
	uint32_t sum = 0;
	for (uint8_t i = 0; i < samples; i++) {
		sum += static_cast<uint32_t>(analogRead(pin));
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
	uint32_t touchVal = 0;

	for (int i = 0; i < 8; i++) {
		if (lowData[i] > threshold) {
			touchVal |= static_cast<uint32_t>(1) << i;
		}
	}

	for (int i = 0; i < 12; i++) {
		if (highData[i] > threshold) {
			touchVal |= static_cast<uint32_t>(1) << (8 + i);
		}
	}

	int sections = 0;
	while (touchVal & 0x01u) {
		sections++;
		touchVal >>= 1;
	}
	return sections;
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

float readTDS(float temperature) {
	const float temp = isfinite(temperature) ? temperature : DEFAULT_TEMP_C;
	return readAverageTdsPpm(temp, ANALOG_SAMPLES);
}

int readWaterLevel() {
	int percentage = waterLevel.readPercentage();
	if (percentage > 0 && percentage <= 100) {
		return percentage;
	}

	uint8_t lowData[8] = {0};
	uint8_t highData[12] = {0};
	bool ok = readWaterFrame(0x77, 0x78, lowData, highData);
	if (!ok) {
		ok = readWaterFrame(0x3B, 0x3C, lowData, highData);
	}
	if (!ok) {
		return -1;
	}

	const int computed = touchedSectionsFromRaw(lowData, highData) * 5;
	if (computed < 0) {
		return 0;
	}
	if (computed > 100) {
		return 100;
	}
	return computed;
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

void drawLineWithWarn(int16_t y, const String& text, const char* state, bool blinkOn) {
	display.setCursor(0, y);
	display.print(text);

	if (isWarnState(state) && blinkOn) {
		display.print(" WARN ");
		display.print(state);
	}
}

void drawDashboard(bool blinkOn) {
	char line[40];
	const char* tState = rangeState(g_temperatureC, TEMP_LOW_C, TEMP_HIGH_C);
	const char* pState = rangeState(g_ph, PH_LOW, PH_HIGH);
	const char* tdsState = rangeState(g_tdsPpm, TDS_LOW, TDS_HIGH);
	const char* wState = waterState(g_waterPercent);

	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);

	snprintf(line, sizeof(line), "Temp: %sC", isfinite(g_temperatureC) ? String(g_temperatureC, 1).c_str() : "--.-");
	drawLineWithWarn(0, line, tState, blinkOn);

	snprintf(line, sizeof(line), "pH:   %s", isfinite(g_ph) ? String(g_ph, 2).c_str() : "--.--");
	drawLineWithWarn(16, line, pState, blinkOn);

	snprintf(line, sizeof(line), "TDS:  %sppm", isfinite(g_tdsPpm) ? String(g_tdsPpm, 0).c_str() : "----");
	drawLineWithWarn(32, line, tdsState, blinkOn);

	snprintf(line, sizeof(line), "Water: %s%%", (g_waterPercent >= 0) ? String(g_waterPercent).c_str() : "--");
	drawLineWithWarn(48, line, wState, blinkOn);

	display.display();
}

void printSerialStatus() {
	Serial.printf("Temp=%.1fC, pH=%.2f, TDS=%.0fppm, Water=%d%%\n",
								g_temperatureC,
								g_ph,
								g_tdsPpm,
								g_waterPercent);
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

	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
		Serial.println("[ERROR] SSD1306 initialization failed.");
	} else {
		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(SSD1306_WHITE);
		display.setCursor(0, 0);
		display.println("Hydroponics Monitor");
		display.println("Initializing...");
		display.display();
	}

	Serial.println("[INFO] Hydroponics dashboard started.");
}

void loop() {
	const uint32_t now = millis();

	// Non-blocking DS18B20 cycle: request conversion, then read later.
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
		g_waterPercent = readWaterLevel();
		printSerialStatus();
	}

	const bool blinkOn = ((now / 1000u) % 2u) == 0u;
	drawDashboard(blinkOn);
}
