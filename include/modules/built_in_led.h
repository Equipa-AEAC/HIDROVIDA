#ifndef BUILT_IN_LED_MODULE_H
#define BUILT_IN_LED_MODULE_H

#include "base_module.h"

class BuiltInLedModule : public BaseModule {
public:
	BuiltInLedModule() : BaseModule("LED embutido", "LED", {"piscar"}, {"OFF", "ON"}) {}

	void initialize(void) override;
	void update(void) override;

	void setState(uint8_t stateIdx) override;
	void triggerAction(uint8_t actionIdx, const Params& params = {}) override;
	
private:
};

#endif // BUILT_IN_LED_MODULE_H
