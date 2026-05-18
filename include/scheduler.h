#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <unordered_map>
#include <vector>
#include <any>
#include <typeinfo>
#include <iostream>
#include <variant>
#include <memory>

#include "modules/base_module.h"

using EventId = uint32_t;

enum class SchedulerMsgType : uint8_t {
    Reload,
	TriggerEvent
};

enum class ScheduledEventType : uint8_t {
	TriggerModule,
	TriggerAction,
	SetState
};

struct TimeKeyBase {
	virtual ~TimeKeyBase() = default;
};

struct AtTime : public TimeKeyBase {
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	uint8_t weekFilter; // Bitmask for days of the week (0=Sunday, 1=Monday, ..., 6=Saturday)

	AtTime(uint8_t h, uint8_t m, uint8_t s, uint8_t wf) : hour(h), minute(m), second(s), weekFilter(wf) {}
};

inline std::shared_ptr<AtTime> ATTIME(uint8_t hour, uint8_t minute, uint8_t second, uint8_t weekFilter) {
    return std::make_shared<AtTime>(hour, minute, second, weekFilter);
}

struct SchedulerMsg {
    SchedulerMsgType type;
    std::shared_ptr<TimeKeyBase> timeKey;
};

struct ScheduledEvent {
	std::shared_ptr<TimeKeyBase> timeKey;
	uint32_t lastTriggeredTime;
	ScheduledEventType eventType;
	uint8_t moduleId{0};
	uint8_t actionId{0};
	uint8_t stateType{0};
	Params parameters;
};

class Scheduler {
public:
	Scheduler();
	~Scheduler();

	void Initialize(void);
	uint8_t registerModule(BaseModule* module);

	EventId addEvent(std::shared_ptr<TimeKeyBase> timeKey, ScheduledEvent event);
	bool removeEvent(EventId eventId);
	bool updateEvent(EventId eventId, std::shared_ptr<TimeKeyBase> timeKey, const ScheduledEvent& event);
	void clearAllEvents();

	uint32_t dispatchDueEvents(void);
	void syncModules(void);

	void dispatchEvent(const ScheduledEvent& event);

	const std::unordered_map<uint8_t, BaseModule*>& getModules() const { return _modules; }
	const std::unordered_map<EventId, std::vector<ScheduledEvent>>& getSchedule() const { return schedule; }
	const std::vector<EventId>& getEventOrder() const { return _eventOrder; }

	BaseModule* getModule(uint8_t id) const {
		auto it = _modules.find(id);
		return (it != _modules.end()) ? it->second : nullptr;
	}

private:
	std::unordered_map<uint8_t, BaseModule*> _modules;
	std::unordered_map<EventId, std::vector<ScheduledEvent>> schedule;
	std::vector<EventId> _eventOrder; // stable insertion order for display
};

#endif // SCHEDULER_H