#include "controls/interrupt_button.h"
#include <Arduino.h>

InterruptButton* InterruptButton::instances[InterruptButton::MAX_ISR_BUTTONS] = {nullptr};

static void ARDUINO_ISR_ATTR dispatchInterruptButtonISR(uint8_t slot) {
	if (slot < InterruptButton::MAX_ISR_BUTTONS && InterruptButton::instances[slot] != nullptr) {
		InterruptButton::instances[slot]->interruptButtonISR();
	}
}

void ARDUINO_ISR_ATTR h0() { dispatchInterruptButtonISR(0); }
void ARDUINO_ISR_ATTR h1() { dispatchInterruptButtonISR(1); }
void ARDUINO_ISR_ATTR h2() { dispatchInterruptButtonISR(2); }
void ARDUINO_ISR_ATTR h3() { dispatchInterruptButtonISR(3); }
void ARDUINO_ISR_ATTR h4() { dispatchInterruptButtonISR(4); }
void ARDUINO_ISR_ATTR h5() { dispatchInterruptButtonISR(5); }

static uint8_t btnCounter = 0;
InterruptButton::InterruptButton(uint8_t pin)
{
	this->buttonPin = pin;
	int interruptNum = digitalPinToInterrupt(pin);
	
	pinMode(pin, INPUT_PULLUP);
	if (btnCounter >= InterruptButton::MAX_ISR_BUTTONS || interruptNum < 0) {
		return;
	}

	isrSlot = btnCounter;
	instances[isrSlot] = this;
	void (*fn)(void) = nullptr;
	if (isrSlot == 0) {
		fn = (void(*)(void))h0;
	} else if (isrSlot == 1) {
		fn = (void(*)(void))h1;
	} else if (isrSlot == 2) {
		fn = (void(*)(void))h2;
	} else if (isrSlot == 3) {
		fn = (void(*)(void))h3;
	} else if (isrSlot == 4) {
		fn = (void(*)(void))h4;
	} else if (isrSlot == 5) {
		fn = (void(*)(void))h5;
	}
	attachInterrupt(interruptNum, fn, HIGH);
	btnCounter++;
}

InterruptButton::~InterruptButton()
{
	if (isrSlot == 0xFF) {
		return;
	}

	int interruptNum = digitalPinToInterrupt(buttonPin);
	if (interruptNum >= 0) {
		detachInterrupt(interruptNum);
	}

	instances[isrSlot] = nullptr;
}

void ARDUINO_ISR_ATTR InterruptButton::interruptButtonISR(void)
{
	uint32_t now = millis();
	if (now - lastInterruptTime > InterruptButton::DEBOUNCE_MS) {
		rawPressed = true;
	}
	lastInterruptTime = now;
}

void InterruptButton::poll(void)
{
	this->rawPressed = false;
}

bool InterruptButton::pressed(void)
{
	return rawPressed;
}