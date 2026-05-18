#ifndef BASE_BUTTON_H
#define BASE_BUTTON_H

#include <cstdint>
#include <map>

class BaseButton {
public:
	virtual ~BaseButton() = default;

	virtual void poll(void);

	virtual bool pressed(void);
};

#endif // BASE_BUTTON_H