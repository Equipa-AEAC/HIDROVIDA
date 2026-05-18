#ifndef GARDEN_H
#define GARDEN_H

#include <display/LiquidCrystal_I2CAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>

#include "hardware/pin_map.h"
#include "control_panel.h"
#include "modules/clock_module.h"
#include "scheduler.h"

// ============================================================================
// Parametros fixos do painel LCD.
// ============================================================================

constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

class Garden {
public:
	static ControlPanel* controlPanel;
	static LiquidCrystal_I2C* lcd;
	static LiquidCrystal_I2CAdapter* lcdAdapter;
	static CharacterDisplayRenderer* renderer;

	static Scheduler* scheduler;
	static ClockModule* clock;

	static void initialize(void);
	static void update(void);
	static void runScheduler(void* pvParameters);
	static void sendToScheduler(SchedulerMsg msg);

	static void i2cCheck(void);

	static char _timestampBuffer[20];
	static char _dateBuffer[11];
	static char _timeBuffer[9];
};

#endif // GARDEN_H
