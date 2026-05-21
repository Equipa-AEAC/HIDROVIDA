#include "garden.h"

#include <Wire.h>

ControlPanel* Garden::controlPanel = nullptr;
LiquidCrystal_I2C* Garden::lcd = nullptr;

void Garden::initialize(void) {
  i2cCheck();

  lcd = new LiquidCrystal_I2C(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
  lcd->init();
  lcd->backlight();
  lcd->clear();

  controlPanel = new ControlPanel(lcd);
  controlPanel->initialize();
}

void Garden::update(void) {
  if (controlPanel != nullptr) {
    controlPanel->update();
  }
}

void Garden::i2cCheck(void) {
  Serial.println("[LCD] Verificacao I2C");

  Wire.beginTransmission(LCD_ADDRESS);
  if (Wire.endTransmission() == 0) {
    Serial.println("[LCD] LCD encontrado em 0x27");
  } else {
    Serial.println("[LCD] LCD nao encontrado em 0x27");
  }
}
