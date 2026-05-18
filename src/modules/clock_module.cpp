#include "modules/clock_module.h"

void ClockModule::initialize(void)
{
	if (!_rtc.begin()) {
		Serial.println("[ClockModule::initialize] FAILURE: DS3231 not detected on I2C!");
		_valid = false;
		return;
	}

	if (_rtc.lostPower()) {
		Serial.println("[ClockModule::initialize] Syncing time to compilation time...");
		_rtc.adjust(DateTime(__DATE__, __TIME__));
	}

	_valid = true;

	DateTime now = _rtc.now();
	Serial.printf("[ClockModule::initialize] Time: %04d-%02d-%02d %02d:%02d:%02d\n", 
		now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

void ClockModule::update(void)
{
	_changed = false;
	
	if (!_valid) return;
	unsigned long now = millis();
	if (now - _lastUpdate < 250) return;
	_lastUpdate = now;

	DateTime dt = _rtc.now();

	_changed = _lastUnixTime != dt.unixtime();
	_lastUnixTime = dt.unixtime();

	_second = dt.second();
	_minute = dt.minute();
	_hour   = dt.hour();
	_day    = dt.day();
	_month  = dt.month();
	_year   = dt.year();
	_dayOfTheWeek = dt.dayOfTheWeek();
}
