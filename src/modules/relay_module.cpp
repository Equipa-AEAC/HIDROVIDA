#include "modules/relay_module.h"

#include <Arduino.h>
#include <type_traits>

RelayModule::RelayModule(const char* moduleName,
                         const char* moduleShortName,
                         uint8_t relayPin,
                         uint8_t relayActiveLevel,
                         uint8_t relayInactiveLevel)
	: BaseModule(moduleName, moduleShortName, {"pulse"}, {"OFF", "ON"}),
	  _relayPin(relayPin),
	  _relayActiveLevel(relayActiveLevel),
	  _relayInactiveLevel(relayInactiveLevel),
	  _relayActive(false),
	  _relayPulseDeadlineMs(0)
{
}

void RelayModule::initialize(void)
{
	pinMode(_relayPin, OUTPUT);
	writeRelay(false);
}

void RelayModule::update(void)
{
	if (!_relayActive || _relayPulseDeadlineMs == 0) {
		return;
	}

	if (millis() >= _relayPulseDeadlineMs) {
		writeRelay(false);
		_relayPulseDeadlineMs = 0;
	}
}

void RelayModule::setState(uint8_t stateIdx)
{
	switch (stateIdx) {
		case 0:
			writeRelay(false);
			_relayPulseDeadlineMs = 0;
			break;
		case 1:
			writeRelay(true);
			_relayPulseDeadlineMs = 0;
			break;
		default:
			break;
	}
}

void RelayModule::triggerAction(uint8_t actionIdx, const Params& params)
{
	switch (actionIdx) {
		case 0: {
			const uint32_t pulseMs = resolvePulseMs(params);
			writeRelay(true);
			_relayPulseDeadlineMs = millis() + pulseMs;
			break;
		}
		default:
			break;
	}
}

bool RelayModule::isActive(void) const
{
	return _relayActive;
}

uint8_t RelayModule::getPin(void) const
{
	return _relayPin;
}

void RelayModule::writeRelay(bool enabled)
{
	_relayActive = enabled;
	digitalWrite(_relayPin, enabled ? _relayActiveLevel : _relayInactiveLevel);
}

uint32_t RelayModule::resolvePulseMs(const Params& params) const
{
	uint32_t pulseMs = 1000;

	if (params.empty()) {
		return pulseMs;
	}

	std::visit([&pulseMs](const auto& value) {
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, int64_t>) {
			if (value > 0) {
				pulseMs = static_cast<uint32_t>(value);
			}
		} else if constexpr (std::is_same_v<T, double>) {
			if (value > 0.0) {
				pulseMs = static_cast<uint32_t>(value);
			}
		}
	}, params[0]);

	return pulseMs;
}
