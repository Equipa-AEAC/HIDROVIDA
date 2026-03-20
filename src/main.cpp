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

// Water level thresholds for interpretation (percentage)
#define LEVEL_LOW_MAX 33
#define LEVEL_MEDIUM_MIN 34
#define LEVEL_MEDIUM_MAX 66
#define LEVEL_HIGH_MIN 67

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
String getWaterLevelInterpretation(int percentage);

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

    // Read sensor data
    int waterPercentage = sensor.readPercentage();
    float waterCM = sensor.readCM();

    // Get interpreted level
    String levelInterpretation = getWaterLevelInterpretation(waterPercentage);

    // Get current time for timestamp
    unsigned long uptime = millis() / 1000; // Convert to seconds

    // ========== OUTPUT TO SERIAL MONITOR ==========
    
    // Header with timestamp
    Serial.print("[");
    Serial.print(uptime);
    Serial.println("s]");

    // Raw sensor values
    Serial.print("  Raw Percentage: ");
    Serial.print(waterPercentage);
    Serial.println("%");

    Serial.print("  Height (CM): ");
    Serial.print(waterCM, 1);  // Print with 1 decimal place
    Serial.println(" cm");

    // Interpreted water level
    Serial.print("  Status: ");
    Serial.println(levelInterpretation);

    // Visual representation (simple bar)
    Serial.print("  Level: [");
    int barLength = 20;  // Visual bar length
    int filledBars = (waterPercentage / 5);  // Each section = 5%
    if (filledBars > barLength) filledBars = barLength;
    
    for (int i = 0; i < barLength; i++) {
      if (i < filledBars) {
        Serial.print("█");
      } else {
        Serial.print("░");
      }
    }
    Serial.println("]");

    // Separator
    Serial.println("----------------------------------------\n");
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
  int testRead = sensor.readPercentage();
  
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
 * @brief Interpret water level percentage as LOW, MEDIUM, or HIGH
 * 
 * @param percentage Water level percentage (0-100)
 * @return String containing interpreted level with emoji indicator
 */
String getWaterLevelInterpretation(int percentage) {
  String interpretation = "";

  if (percentage <= LEVEL_LOW_MAX) {
    interpretation = "LOW (Empty)";
  } 
  else if (percentage >= LEVEL_MEDIUM_MIN && percentage <= LEVEL_MEDIUM_MAX) {
    interpretation = "MEDIUM (Half)";
  } 
  else if (percentage >= LEVEL_HIGH_MIN) {
    interpretation = "HIGH (Full)";
  }
  else {
    // Edge case (shouldn't normally occur)
    interpretation = "MEDIUM (Transition)";
  }

  return interpretation;
}

// ============================================================================
// END OF FILE
// ============================================================================
