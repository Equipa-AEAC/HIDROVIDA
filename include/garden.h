#ifndef GARDEN_H
#define GARDEN_H

#include <LiquidCrystal_I2C.h>

#include "control_panel.h"

constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

class Garden {
public:
  static ControlPanel* controlPanel;
  static LiquidCrystal_I2C* lcd;

  static void initialize(void);
  static void update(void);
  static void i2cCheck(void);
};

#endif  // GARDEN_H
