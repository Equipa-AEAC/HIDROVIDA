#ifndef BUTTON_CONTROLS_H
#define BUTTON_CONTROLS_H

#include <Arduino.h>
#include "base_controls.h"

class ButtonControls : public BaseControls {
public:
	ButtonControls(int upPin, int downPin, int acceptPin);
	~ButtonControls() override;

	void poll(void) override;

	bool justPressedAccept(void) override;
	bool justPressedUp(void) override;
	bool justPressedDown(void) override;

	bool pressedAccept(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) override;
	bool pressedUp(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) override;
	bool pressedDown(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) override;

private:
	static ButtonControls* activeInstance;

	static void IRAM_ATTR onUpInterrupt();
	static void IRAM_ATTR onDownInterrupt();
	static void IRAM_ATTR onAcceptInterrupt();

	void IRAM_ATTR handleUpInterrupt();
	void IRAM_ATTR handleDownInterrupt();
	void IRAM_ATTR handleAcceptInterrupt();

	int upPin;
	int downPin;
	int acceptPin;
	volatile uint8_t prevUpState;
	volatile uint8_t prevDownState;
	volatile uint8_t prevAcceptState;
	volatile unsigned long lastUpTime;
	volatile unsigned long lastDownTime;
	volatile unsigned long lastAcceptTime;
	unsigned long lastUpTick;
	unsigned long lastDownTick;
	unsigned long lastAcceptTick;
	volatile unsigned long lastUpEdgeMs;
	volatile unsigned long lastDownEdgeMs;
	volatile unsigned long lastAcceptEdgeMs;
	volatile uint32_t upInterruptCount;
	volatile uint32_t downInterruptCount;
	volatile uint32_t acceptInterruptCount;
	unsigned long lastDiagPrintMs;
	static constexpr unsigned long DEBOUNCE_MS = 30;
};

#endif // BUTTON_CONTROLS_H