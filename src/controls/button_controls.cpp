#include "controls/button_controls.h"

ButtonControls* ButtonControls::activeInstance = nullptr;

ButtonControls::ButtonControls(int upPin, int downPin, int acceptPin) {
	this->upPin = upPin;
	this->downPin = downPin;
	this->acceptPin = acceptPin;
	activeInstance = this;

	if (upPin >= 0) pinMode(upPin, INPUT_PULLUP);
	if (downPin >= 0) pinMode(downPin, INPUT_PULLUP);
	if (acceptPin >= 0) pinMode(acceptPin, INPUT_PULLUP);

	if (upPin >= 0) {
		attachInterrupt(digitalPinToInterrupt(upPin), ButtonControls::onUpInterrupt, CHANGE);
	}
	if (downPin >= 0) {
		attachInterrupt(digitalPinToInterrupt(downPin), ButtonControls::onDownInterrupt, CHANGE);
	}
	if (acceptPin >= 0) {
		attachInterrupt(digitalPinToInterrupt(acceptPin), ButtonControls::onAcceptInterrupt, CHANGE);
	}
	
	prevUpState = 0;
	prevDownState = 0;
	prevAcceptState = 0;
	lastUpTime = 0;
	lastDownTime = 0;
	lastAcceptTime = 0;
	lastUpTick = 0;
	lastDownTick = 0;
	lastAcceptTick = 0;
	lastUpEdgeMs = 0;
	lastDownEdgeMs = 0;
	lastAcceptEdgeMs = 0;
	upInterruptCount = 0;
	downInterruptCount = 0;
	acceptInterruptCount = 0;
	lastDiagPrintMs = 0;
}

ButtonControls::~ButtonControls() {
	if (upPin >= 0) {
		detachInterrupt(digitalPinToInterrupt(upPin));
	}
	if (downPin >= 0) {
		detachInterrupt(digitalPinToInterrupt(downPin));
	}
	if (acceptPin >= 0) {
		detachInterrupt(digitalPinToInterrupt(acceptPin));
	}

	if (activeInstance == this) {
		activeInstance = nullptr;
	}
}

void ButtonControls::poll(void) {}

void IRAM_ATTR ButtonControls::onUpInterrupt() {
	if (activeInstance != nullptr) {
		activeInstance->handleUpInterrupt();
	}
}

void IRAM_ATTR ButtonControls::onDownInterrupt() {
	if (activeInstance != nullptr) {
		activeInstance->handleDownInterrupt();
	}
}

void IRAM_ATTR ButtonControls::onAcceptInterrupt() {
	if (activeInstance != nullptr) {
		activeInstance->handleAcceptInterrupt();
	}
}

void IRAM_ATTR ButtonControls::handleUpInterrupt() {
	upInterruptCount++;
	unsigned long now = millis();
	if (now - lastUpEdgeMs < DEBOUNCE_MS) {
		return;
	}
	lastUpEdgeMs = now;

	bool pressed = (upPin >= 0 && digitalRead(upPin) == LOW);
	if (pressed) {
		if (prevUpState == 0) {
			prevUpState = 1;
			lastUpTime = now;
			lastUpTick = 0;
		}
	} else {
		lastUpTick = 0;
		prevUpState = 0;
	}
}

void IRAM_ATTR ButtonControls::handleDownInterrupt() {
	downInterruptCount++;
	unsigned long now = millis();
	if (now - lastDownEdgeMs < DEBOUNCE_MS) {
		return;
	}
	lastDownEdgeMs = now;

	bool pressed = (downPin >= 0 && digitalRead(downPin) == LOW);
	if (pressed) {
		if (prevDownState == 0) {
			prevDownState = 1;
			lastDownTime = now;
			lastDownTick = 0;
		}
	} else {
		lastDownTick = 0;
		prevDownState = 0;
	}
}

void IRAM_ATTR ButtonControls::handleAcceptInterrupt() {
	acceptInterruptCount++;
	unsigned long now = millis();
	if (now - lastAcceptEdgeMs < DEBOUNCE_MS) {
		return;
	}
	lastAcceptEdgeMs = now;

	bool pressed = (acceptPin >= 0 && digitalRead(acceptPin) == LOW);
	if (pressed) {
		if (prevAcceptState == 0) {
			prevAcceptState = 1;
			lastAcceptTime = now;
			lastAcceptTick = 0;
		}
	} else {
		lastAcceptTick = 0;
		prevAcceptState = 0;
	}
}

bool ButtonControls::justPressedAccept(void) {
	return prevAcceptState == 1;
}

bool ButtonControls::justPressedUp(void) {
	return prevUpState == 1;
}

bool ButtonControls::justPressedDown(void) {
	return prevDownState == 1;
}


bool ButtonControls::pressedAccept(uint32_t repeatTime, uint32_t repeatCycles) {
	noInterrupts();
	if (prevAcceptState == 1) {
		prevAcceptState = 2;
		interrupts();
		return true;
	}
	interrupts();

	if (prevAcceptState != 2)
		return false;

	if (millis() - lastAcceptTime < repeatTime)
		return false;
	
	if (lastAcceptTick == 0 || millis() - lastAcceptTick >= repeatCycles) {
		lastAcceptTick = millis();
		return true;
	}

	return false;
}
bool ButtonControls::pressedUp(uint32_t repeatTime, uint32_t repeatCycles) {
	noInterrupts();
	if (prevUpState == 1) {
		prevUpState = 2;
		interrupts();
		return true;
	}
	interrupts();

	if (prevUpState != 2)
		return false;

	if (millis() - lastUpTime < repeatTime)
		return false;
	
	if (lastUpTick == 0 || millis() - lastUpTick >= repeatCycles) {
		lastUpTick = millis();
		return true;
	}

	return false;
}
bool ButtonControls::pressedDown(uint32_t repeatTime, uint32_t repeatCycles) {
	noInterrupts();
	if (prevDownState == 1) {
		prevDownState = 2;
		interrupts();
		return true;
	}
	interrupts();

	if (prevDownState != 2)
		return false;

	if (millis() - lastDownTime < repeatTime)
		return false;
	
	if (lastDownTick == 0 || millis() - lastDownTick >= repeatCycles) {
		lastDownTick = millis();
		return true;
	}

	return false;
}