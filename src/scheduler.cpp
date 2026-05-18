#include "scheduler.h"

#include <algorithm>

#include "garden.h"

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
}

void Scheduler::Initialize(void)
{
	_modules.clear();
	schedule.clear();
	_eventOrder.clear();
}

uint8_t Scheduler::registerModule(BaseModule *module)
{
	if (module == nullptr) {
		return 0;
	}

	for (const auto &entry : _modules) {
		if (entry.second == module) {
			return entry.first;
		}
	}

	for (uint16_t id = 1; id <= 255; ++id) {
		uint8_t moduleId = static_cast<uint8_t>(id);
		if (_modules.find(moduleId) == _modules.end()) {
			_modules[moduleId] = module;
			return moduleId;
		}
	}

	return 0;
}

EventId Scheduler::addEvent(std::shared_ptr<TimeKeyBase> timeKey, ScheduledEvent event)
{
	static EventId nextEventId = 1;

	if (schedule.size() >= static_cast<size_t>(UINT32_MAX - 1)) {
		return 0;
	}

	event.timeKey = timeKey;

	EventId start = nextEventId;
	do {
		if (nextEventId == 0) {
			nextEventId = 1;
		}

		if (schedule.find(nextEventId) == schedule.end()) {
			EventId assigned = nextEventId;
			schedule[assigned].push_back(event);
			_eventOrder.push_back(assigned);
			++nextEventId;
			return assigned;
		}

		++nextEventId;
		if (nextEventId == 0) {
			nextEventId = 1;
		}
	} while (nextEventId != start);

	return 0;
}

bool Scheduler::removeEvent(EventId eventId)
{
	if (eventId == 0) {
		return false;
	}
	auto it = std::find(_eventOrder.begin(), _eventOrder.end(), eventId);
	if (it != _eventOrder.end()) {
		_eventOrder.erase(it);
	}
	return schedule.erase(eventId) > 0;
}

bool Scheduler::updateEvent(EventId eventId, std::shared_ptr<TimeKeyBase> timeKey, const ScheduledEvent& event)
{
	auto it = schedule.find(eventId);
	if (it == schedule.end()) return false;
	it->second.clear();
	ScheduledEvent updated = event;
	updated.timeKey = timeKey;
	it->second.push_back(updated);
	return true;
}

void Scheduler::clearAllEvents()
{
	schedule.clear();
	_eventOrder.clear();
}

uint32_t Scheduler::dispatchDueEvents(void)
{
	const uint32_t now = Garden::clock->getUnix();
	const uint16_t year = Garden::clock->getYear();
	const uint8_t month = Garden::clock->getMonth();
	const uint8_t day = Garden::clock->getDay();
	const uint8_t hour = Garden::clock->getHour();
	const uint8_t minute = Garden::clock->getMinute();
	const uint8_t second = Garden::clock->getSecond();
	const uint8_t dayofTheWeek = Garden::clock->getDayOfTheWeek();
	uint32_t dispatchedCount = 0;

	for (auto it = schedule.begin(); it != schedule.end();) {
		bool shouldErase = false;

		for (auto &event : it->second) {
			if (event.timeKey == nullptr) {
				continue;
			}
			if (event.lastTriggeredTime >= now - 3) {
				continue;
			}

			if (auto atTimeKey = std::static_pointer_cast<AtTime>(event.timeKey)) {
				// weekFilter bits: 0=Sunday, 1=Monday, ..., 6=Saturday
				// weekFilter == 0 means run every day
				if (dayofTheWeek > 6) {
					continue; // invalid DOW from clock
				}

				if (atTimeKey->weekFilter != 0) {
					const uint8_t dowMask = static_cast<uint8_t>(1u << dayofTheWeek);
					if ((atTimeKey->weekFilter & dowMask) == 0) {
						continue;
					}
				}

				if (atTimeKey->hour == hour && atTimeKey->minute == minute && atTimeKey->second == second) {
					dispatchEvent(event);
					++dispatchedCount;
					event.lastTriggeredTime = now;
				}
			}


			/*
			if (event.timeKey <= now) {
				dispatchEvent(event);
				++dispatchedCount;
				shouldErase = true;
			}
			*/
		}

		if (shouldErase)
			it = schedule.erase(it);
		else
			++it;
	}

	return dispatchedCount;
}

void Scheduler::syncModules(void)
{
	for (const auto &entry : _modules) {
		if (entry.second == nullptr) {
			continue;
		}

		if (entry.second == Garden::clock) {
			continue;
		}

		entry.second->update();
	}
}

void Scheduler::dispatchEvent(const ScheduledEvent &event)
{
	BaseModule *module = nullptr;

	switch (event.eventType) {
		case ScheduledEventType::TriggerModule:
			module = getModule(event.moduleId);
			if (module != nullptr)
				module->triggerAction(event.actionId, event.parameters);
			// 1) Find module by event.moduleId
			// 2) Trigger module-level action
			break;

		case ScheduledEventType::TriggerAction:
			// 1) Find module by event.moduleId
			// 2) Invoke action event.actionId with event.parameters
			break;

		case ScheduledEventType::SetState:
			// 1) Find module by event.moduleId
			// 2) Update state event.stateType using event.parameters
			module = getModule(event.moduleId);
			if (module != nullptr)
				module->setState(event.stateType);
			break;

		default:
			break;
	}
}
