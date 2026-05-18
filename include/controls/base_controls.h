#ifndef BASE_CONTROLS_H
#define BASE_CONTROLS_H

#include <cstdint>

#define LEFT_RIGHT_ACCEPT 0x010
#define UP_DOWN_ACCEPT 0x020
#define LEFT_RIGHT_UP_DOWN 0x030
#define LEFT_RIGHT_UP_DOWN_ACCEPT 0x040
#define LEFT_RIGHT_UP_DOWN_BACK_ACCEPT 0x050

#define REPEAT_TIME_DEFAULT 750
#define REPEAT_CYCLES_DEFAULT 75

class BaseControls {
public:
	uint8_t controlMode;
	virtual ~BaseControls() = default;
	
	virtual void poll(void) = 0;

	virtual bool justPressedAccept(void) = 0;
	virtual bool justPressedUp(void) = 0;
	virtual bool justPressedDown(void) = 0;

	virtual bool pressedAccept(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) = 0;
	virtual bool pressedUp(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) = 0;
	virtual bool pressedDown(uint32_t repeatTime = REPEAT_TIME_DEFAULT, uint32_t repeatCycles = REPEAT_CYCLES_DEFAULT) = 0;
};

#endif // BASE_CONTROLS_H