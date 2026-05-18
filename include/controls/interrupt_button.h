#ifndef INTERRUPT_BUTTON_H
#define INTERRUPT_BUTTON_H

#include "base_button.h"
#include <cstdint>

class InterruptButton : public BaseButton {
public:
	static constexpr uint8_t MAX_ISR_BUTTONS = 6;
	static InterruptButton* instances[MAX_ISR_BUTTONS];

	InterruptButton(uint8_t pin);
	~InterruptButton() override;

	void poll(void) override;

	bool pressed(void) override;

	void interruptButtonISR(void);

private:
	uint8_t buttonPin;
	uint8_t isrSlot = 0xFF;
	volatile bool rawPressed = false;
	volatile uint32_t lastInterruptTime = 0;

	static const uint32_t DEBOUNCE_MS = 20;
};

#endif // INTERRUPT_BUTTON_H