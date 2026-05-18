#ifndef RELAY_MODULE_H
#define RELAY_MODULE_H

#include "base_module.h"

// ============================================================================
// Modulo generico para relays digitais.
// Permite ao scheduler ligar/desligar cargas e executar pulsos temporizados.
// ============================================================================

class RelayModule : public BaseModule {
public:
	RelayModule(const char* moduleName,
	            const char* moduleShortName,
	            uint8_t relayPin,
	            uint8_t relayActiveLevel,
	            uint8_t relayInactiveLevel);

	void initialize(void) override;
	void update(void) override;

	void setState(uint8_t stateIdx) override;
	void triggerAction(uint8_t actionIdx, const Params& params = {}) override;

	bool isActive(void) const;
	uint8_t getPin(void) const;

private:
	void writeRelay(bool enabled);
	uint32_t resolvePulseMs(const Params& params) const;

	uint8_t _relayPin;
	uint8_t _relayActiveLevel;
	uint8_t _relayInactiveLevel;
	bool _relayActive;
	uint32_t _relayPulseDeadlineMs;
};

#endif // RELAY_MODULE_H
