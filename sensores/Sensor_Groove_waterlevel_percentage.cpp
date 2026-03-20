/*
 * ESP32 Grove Water Level Sensor (10cm) - I2C Reader
 * 
 * Description:
 * Reads water level from Grove Water Level Sensor via I2C communication
 * on custom pins (SDA=21, SCL=22) and outputs data to Serial Monitor
 * 
 * Components:
 * - ESP32-WROOM microcontroller
 * - Grove Water Level Sensor 10cm (I2C address: 0x77 and 0x78)
 * - Wire library for I2C communication
 * 
 * Update interval: 1 second
 * Serial baud rate: 115200
 */

#include <Arduino.h>
#include <Wire.h>
#include <waterlevelsensor.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// I2C Pin Configuration for ESP32
#define I2C_SDA 21  // SDA pin
#define I2C_SCL 22  // SCL pin

// Serial Configuration
#define SERIAL_BAUD 115200

// Update interval in milliseconds
#define UPDATE_INTERVAL 1000

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WaterLevelSensor sensor;
unsigned long lastUpdateTime = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void initializeI2C();
void initializeSensor();
void scanI2CBus();
int readPercentageRobust();
bool readWaterFrame(uint8_t lowAddr, uint8_t highAddr, uint8_t *lowData, uint8_t *highData);
int touchedSectionsFromRaw(const uint8_t *lowData, const uint8_t *highData);

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD);
  delay(100);

  // Print startup message
  Serial.println("\n\n========================================");
  Serial.println("  ESP32 Water Level Sensor System");
  Serial.println("========================================\n");

  // Initialize I2C with custom pins
  initializeI2C();

  // Print detected I2C devices to quickly verify sensor communication
  scanI2CBus();

  // Initialize the water level sensor
  initializeSensor();

  Serial.println("✓ System initialized successfully!\n");
  Serial.println("Starting water level monitoring...\n");
  Serial.println("----------------------------------------\n");

  // Initialize timing
  lastUpdateTime = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Check if it's time to update (1 second interval)
  unsigned long currentTime = millis();
  
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = currentTime;

    // Read and print only percentage (robust against address variant differences)
    int waterPercentage = readPercentageRobust();
    Serial.println(waterPercentage);
  }

  // Small delay to avoid overwhelming the processor
  delay(10);
}

// ============================================================================
// FUNCTION IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Initialize I2C communication with custom pins
 * 
 * Configures the Wire library to use SDA on pin 21 and SCL on pin 22
 * These are custom I2C pins for the ESP32 (not the default hardware I2C pins)
 */
void initializeI2C() {
  Serial.print("Initializing I2C on SDA=");
  Serial.print(I2C_SDA);
  Serial.print(", SCL=");
  Serial.print(I2C_SCL);
  Serial.println("...");

  // Initialize I2C with custom pins for ESP32
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Set I2C speed to 100kHz (standard mode)
  Wire.setClock(100000);

  Serial.println("✓ I2C initialized successfully");
}

/**
 * @brief Scan and print all I2C devices found on the bus
 */
void scanI2CBus() {
  Serial.println("Scanning I2C bus...");
  int found = 0;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  I2C device found at 0x");
      if (addr < 16) {
        Serial.print('0');
      }
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("  No I2C devices found");
  }
}

/**
 * @brief Initialize the Grove Water Level Sensor
 * 
 * Creates sensor object and verifies it's accessible on I2C bus
 * The WaterLevelSensor constructor calls Wire.begin() internally
 */
void initializeSensor() {
  Serial.println("Initializing Water Level Sensor...");

  // The sensor object is created globally, but we verify I2C communication
  // by attempting to read sensor values
  delay(100);  // Give sensor time to initialize

  // Attempt first read to verify sensor is accessible
  int testRead = readPercentageRobust();
  
  if (testRead >= 0 && testRead <= 100) {
    Serial.print("✓ Sensor detected on I2C bus");
    Serial.print(" (Initial reading: ");
    Serial.print(testRead);
    Serial.println("%)");
  } else {
    Serial.println("⚠ Warning: Unusual sensor reading detected");
    Serial.print("  Reading: ");
    Serial.println(testRead);
  }
}

/**
 * @brief Robust percentage read with fallback for common Grove address variants.
 *
 * First uses library readPercentage(). If result is 0, tries direct raw reads using
 * both address pairs seen in Grove examples: 0x77/0x78 and 0x3B/0x3C.
 */
int readPercentageRobust() {
  int percentage = sensor.readPercentage();
  if (percentage > 0) {
    return percentage;
  }

  uint8_t lowData[8] = {0};
  uint8_t highData[12] = {0};

  bool ok = readWaterFrame(0x77, 0x78, lowData, highData);
  if (!ok) {
    ok = readWaterFrame(0x3B, 0x3C, lowData, highData);
  }

  if (!ok) {
    return 0;
  }

  return touchedSectionsFromRaw(lowData, highData) * 5;
}

/**
 * @brief Read low and high channel frames from sensor over I2C.
 */
bool readWaterFrame(uint8_t lowAddr, uint8_t highAddr, uint8_t *lowData, uint8_t *highData) {
  int lowCount = Wire.requestFrom((int)lowAddr, 8);
  if (lowCount != 8) {
    return false;
  }
  for (int i = 0; i < 8; i++) {
    lowData[i] = Wire.read();
  }

  int highCount = Wire.requestFrom((int)highAddr, 12);
  if (highCount != 12) {
    return false;
  }
  for (int i = 0; i < 12; i++) {
    highData[i] = Wire.read();
  }

  return true;
}

/**
 * @brief Convert raw channel data to touched section count using library threshold.
 */
int touchedSectionsFromRaw(const uint8_t *lowData, const uint8_t *highData) {
  const int threshold = 100;
  uint32_t touchVal = 0;

  for (int i = 0; i < 8; i++) {
    if (lowData[i] > threshold) {
      touchVal |= (uint32_t)1 << i;
    }
  }
  for (int i = 0; i < 12; i++) {
    if (highData[i] > threshold) {
      touchVal |= (uint32_t)1 << (8 + i);
    }
  }

  int sections = 0;
  while (touchVal & 0x01) {
    sections++;
    touchVal >>= 1;
  }

  return sections;
}

// ============================================================================
// END OF FILE
// ============================================================================
