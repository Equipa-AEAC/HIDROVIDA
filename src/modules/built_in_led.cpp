#include "modules/built_in_led.h"
#include "hardware/pin_map.h"

#include <Arduino.h>
#include <type_traits>

void BuiltInLedModule::initialize(void)
{
	pinMode(HardwarePinos::PinoLedEmbutido, OUTPUT);
	digitalWrite(HardwarePinos::PinoLedEmbutido, HardwareNiveis::NivelLedDesligado);
}

void BuiltInLedModule::update(void)
{
}

void BuiltInLedModule::setState(uint8_t stateIdx)
{
	Serial.print("state: ");
	Serial.println(stateIdx);
	
	switch (stateIdx) {
		case 0: // OFF
			digitalWrite(HardwarePinos::PinoLedEmbutido, HardwareNiveis::NivelLedDesligado);
			break;
		case 1: // ON
			digitalWrite(HardwarePinos::PinoLedEmbutido, HardwareNiveis::NivelLedLigado);
			break;
		default:
			break;
	}
}

void BuiltInLedModule::triggerAction(uint8_t actionIdx, const Params& params)
{
	switch (actionIdx) {
		case 0: { // blink
			uint32_t duration = 500;

			/*
			if (!params.empty()) {
				std::visit([&duration](const auto& value) {
					using T = std::decay_t<decltype(value)>;
					if constexpr (std::is_same_v<T, int64_t>) {
						if (value > 0) duration = static_cast<uint32_t>(value);
					} else if constexpr (std::is_same_v<T, double>) {
						if (value > 0.0) duration = static_cast<uint32_t>(value);
					}
				}, params[0]);
			}*/

			Serial.print("Duration: ");
			Serial.println(duration);
			digitalWrite(HardwarePinos::PinoLedEmbutido, HardwareNiveis::NivelLedLigado);
			delay(duration);
			Serial.println("Off");
			digitalWrite(HardwarePinos::PinoLedEmbutido, HardwareNiveis::NivelLedDesligado);
			break;
		}
		default:
			break;
	}
}
