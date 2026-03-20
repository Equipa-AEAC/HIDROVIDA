/*
 * Hidroponia Main Application
 * 
 * ESP32 -based hydroponic monitoring system
 * 
 * Sensors:
 * - Grove Water Level Sensor (I2C, pins SDA=21, SCL=22)
 */

#include <Arduino.h>
#include "Sensor_Groove_waterlevel_percentage.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

#define SERIAL_BAUD 115200
#define UPDATE_INTERVAL 1000  // 1 second

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WaterLevelSensorModule waterLevelSensor;
unsigned long lastUpdateTime = 0;

// ============================================================================
// SETUP FUNCTION
// ============================================================================

void setup() {
  // Initialize serial communication
  Serial.begin(SERIAL_BAUD);
  delay(100);

  // Print startup message
  Serial.println("\n\n========================================");
  Serial.println("  Hidroponia System - ESP32");
  Serial.println("========================================\n");

  // Initialize sensors
  waterLevelSensor.initialize();

  Serial.println("\n✓ System initialized successfully!\n");
  Serial.println("Starting monitoring...\n");
  Serial.println("----------------------------------------\n");

  // Initialize timing
  lastUpdateTime = millis();
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long currentTime = millis();
  
  // Update at specified interval
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = currentTime;

    // Read and print water level percentage
    int waterLevel = waterLevelSensor.readPercentage();
    Serial.println(waterLevel);
  }

  // Small delay to avoid overwhelming the processor
  delay(10);
}

// ============================================================================
// END OF FILE
// ============================================================================
