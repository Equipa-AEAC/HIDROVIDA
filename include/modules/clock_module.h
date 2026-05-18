#ifndef CLOCK_MODULE_H
#define CLOCK_MODULE_H

#include <cstdint>
#include <cstdio>
#include <RTClib.h>
#include "base_module.h"

class ClockModule : public BaseModule {
public:
	ClockModule() : BaseModule("Real-time clock", "RTC") {}

	void initialize(void) override;
	void update(void) override;

	void setState(uint8_t stateIdx) override {};
	void triggerAction(uint8_t actionIdx, const Params& params = {}) override {};

	void adjust(const DateTime &dt) { if (_valid) _rtc.adjust(dt); }
	bool changed(void) { return _changed; }
	
	/** Gets the current timestamp in a formatted string.
	 * @param buf The buffer to store the timestamp.
	 * @return The number of characters written to the buffer (null char not included).
	 */
	int getTimestamp(char* buf) const {
		if (!buf) return 0;
		return snprintf(buf, 20, "%04u-%02u-%02u %02u:%02u:%02u",
						(unsigned)_year, (unsigned)_month, (unsigned)_day,
						(unsigned)_hour, (unsigned)_minute, (unsigned)_second);
	}

	int getTime(char* buf) const {
		if (!buf) return 0;
		return snprintf(buf, 20, "%02u:%02u:%02u",
						(unsigned)_hour, (unsigned)_minute, (unsigned)_second);
	}

	int getDate(char* buf) const {
		if (!buf) return 0;
		return snprintf(buf, 20, "%04u-%02u-%02u",
						(unsigned)_year, (unsigned)_month, (unsigned)_day);
	}

	/**
	 * 32-bit times as seconds since 1970-01-01.
	 */
	uint32_t getUnix() {
		if (!_valid) return 0;
		return _rtc.now().unixtime();
	}
	
	uint16_t getYear()		const { return _year;		}
	uint8_t getMonth()		const { return _month;		}
	uint8_t getDay()		const { return _day;		}
	uint8_t getHour()		const { return _hour;		}
	uint8_t getMinute()		const { return _minute;		}
	uint8_t getSecond()		const { return _second;		}
	uint8_t getDayOfTheWeek() const { return _dayOfTheWeek; }
	
private:
	RTC_DS3231 _rtc;
	uint8_t _hour = 0, _minute = 0, _second = 0, _day = 0, _month = 0;
	uint16_t _year = 0;
	uint8_t _dayOfTheWeek = 0;
	bool _valid  = false;
	bool _changed = false;
	uint32_t _lastUnixTime = 0;
	unsigned long _lastUpdate = 0;
};

#endif // CLOCK_MODULE_H