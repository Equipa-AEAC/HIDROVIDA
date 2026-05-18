// Prevent MENU_SCREEN definitions from expanding (avoid multiple definitions at link time)
#define LCD_SCREENS_NO_SCREENS

#include <LcdMenu.h>
#include <ItemCommand.h>
#include <ItemBool.h>
#include <ItemList.h>
#include <ItemRange.h>
#include "garden.h"

extern MenuScreen* mainScreen;
extern MenuScreen* schedulerScreen;
extern MenuScreen* hydroMenuScreen;
extern MenuScreen* hydroQualidadeScreen;
extern MenuScreen* hydroNiveisScreen;
extern MenuScreen* hydroAmbienteScreen;
extern MenuScreen* editEventScreen;
extern MenuScreen* editScheduleScreen;
extern MenuScreen* dayOfWeekScreen;

uint16_t _selYear = 2000;
uint8_t _selMonth = 1;
uint8_t _selDay = 1;
uint8_t _selHour = 0;
uint8_t _selMinute = 0;
uint8_t _selSecond = 0;

EventId _selEventId = 0;
uint8_t _selEventType = 0;
uint8_t _selModule = 0;
uint8_t _selAction = 0;
uint8_t _selState = 0;
uint8_t _selTimeKeyType = 0;
uint8_t _selDaysOfWeek = 0;

MenuScreen* editEventScreen = new MenuScreen();
MenuScreen* editScheduleScreen = new MenuScreen();
MenuScreen* dayOfWeekScreen = new MenuScreen();

static void onHourChanged(const Ref<uint8_t> value) { _selHour = value; }
static void onMinuteChanged(const Ref<uint8_t> value) { _selMinute = value; }
static void onSecondChanged(const Ref<uint8_t> value) { _selSecond = value; }

static void buildEditEventScreen();
static void buildEditScheduleScreen();
static void buildDayOfWeekScreen();
void buildSchedulerScreen();

void onRemoveAllConfirmed(void) {
	Garden::scheduler->clearAllEvents();
	buildSchedulerScreen();
}

static String _daySummary;
static String _daysLabel;
static bool _dowBool[7];
static std::vector<EventId> _schedEvtIds;
static std::vector<String> _schedEvtLabels;
static std::vector<const char*> _stateOptions = {"no-op"};
static std::vector<const char*> _actionOptions = {"no-op"};

static const char* _dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char* _dowAbbr[]   = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

typedef void (*BoolCallback)(const bool);

static String buildDaySummary(uint8_t mask) {
	if (mask == 0) return "Every day";
	String s;
	for (uint8_t i = 0; i < 7; i++) {
		if (mask & (1 << i)) {
			if (s.length() > 0) s += " ";
			s += _dowAbbr[i];
		}
	}
	return s;
}

static std::shared_ptr<TimeKeyBase> makeTimeKeyFromSelection() {
	if (_selTimeKeyType == 1) {
		return std::make_shared<AtTime>(_selHour, _selMinute, _selSecond, _selDaysOfWeek);
	}
	return std::make_shared<AtTime>(0, 0, 0, 0);
}

static void onDow0(const bool v) { if (v) _selDaysOfWeek |= (1 << 0); else _selDaysOfWeek &= ~(1 << 0); }
static void onDow1(const bool v) { if (v) _selDaysOfWeek |= (1 << 1); else _selDaysOfWeek &= ~(1 << 1); }
static void onDow2(const bool v) { if (v) _selDaysOfWeek |= (1 << 2); else _selDaysOfWeek &= ~(1 << 2); }
static void onDow3(const bool v) { if (v) _selDaysOfWeek |= (1 << 3); else _selDaysOfWeek &= ~(1 << 3); }
static void onDow4(const bool v) { if (v) _selDaysOfWeek |= (1 << 4); else _selDaysOfWeek &= ~(1 << 4); }
static void onDow5(const bool v) { if (v) _selDaysOfWeek |= (1 << 5); else _selDaysOfWeek &= ~(1 << 5); }
static void onDow6(const bool v) { if (v) _selDaysOfWeek |= (1 << 6); else _selDaysOfWeek &= ~(1 << 6); }

static BoolCallback _dowCallbacks[] = { onDow0, onDow1, onDow2, onDow3, onDow4, onDow5, onDow6 };

static bool _deferredRebuildEditEvent = false;
static bool _deferredRebuildEditSchedule = false;

void processDeferredScreenRebuilds() {
	if (_deferredRebuildEditEvent) {
		_deferredRebuildEditEvent = false;
		buildEditEventScreen();
	}
	if (_deferredRebuildEditSchedule) {
		_deferredRebuildEditSchedule = false;
		buildEditScheduleScreen();
	}
}

bool isAutoRefreshScreen(MenuScreen* screen) {
	return screen == mainScreen ||
	       screen == hydroMenuScreen ||
	       screen == hydroQualidadeScreen ||
	       screen == hydroNiveisScreen ||
	       screen == hydroAmbienteScreen;
}

static void onEventTypeChanged(const uint8_t pos) {
	_selEventType = pos;
	_deferredRebuildEditEvent = true;
}
static void onActionChanged(const uint8_t pos)    { _selAction = pos; }
static void onStateChanged(const uint8_t pos)     { _selState = pos; }

static String _moduleLabel;

static void onModuleCycled() {
	const auto& modules = Garden::scheduler->getModules();
	if (modules.empty()) return;

	auto it = modules.find(_selModule);
	++it;
	if (it == modules.end()) it = modules.begin();
	_selModule = it->first;

	_moduleLabel = "Mod: " + String(it->second->getShortName());
	_deferredRebuildEditEvent = true;
}

static void buildDayOfWeekScreen() {
	while (dayOfWeekScreen->size() > 0) {
		delete dayOfWeekScreen->getItemAt(dayOfWeekScreen->size() - 1);
		dayOfWeekScreen->removeLastItem();
	}

	for (uint8_t i = 0; i < 7; i++) {
		_dowBool[i] = (_selDaysOfWeek & (1 << i)) != 0;
		dayOfWeekScreen->addItem(
			new ItemBool<bool>(_dayNames[i], _dowBool[i], "ON", "OFF", "%s", 0, _dowCallbacks[i]));
	}

	dayOfWeekScreen->addItem(
		ITEM_COMMAND("< Back", []() {
			Garden::controlPanel->menu->setScreen(editScheduleScreen);
		}));
}

static void onTimeKeyTypeChanged(const uint8_t pos) {
	_selTimeKeyType = pos;
	_deferredRebuildEditSchedule = true;
}

static void onDaysBtnPressed() {
	buildDayOfWeekScreen();
	dayOfWeekScreen->setParent(editScheduleScreen);
	Garden::controlPanel->menu->setScreen(dayOfWeekScreen);
}

static void onScheduleBackPressed() {
	editScheduleScreen->setParent(editEventScreen);
	Garden::controlPanel->menu->setScreen(editEventScreen);
}

static void buildEditScheduleScreen() {
	while (editScheduleScreen->size() > 0) {
		delete editScheduleScreen->getItemAt(editScheduleScreen->size() - 1);
		editScheduleScreen->removeLastItem();
	}

	static const std::vector<String> timeKeyTypes = {"Manual", "At Time"};
	if (_selTimeKeyType >= timeKeyTypes.size()) _selTimeKeyType = timeKeyTypes.size() - 1;

	editScheduleScreen->addItem(
		ITEM_LIST("Type", timeKeyTypes, onTimeKeyTypeChanged, _selTimeKeyType, "%s", 0, true));

	if (_selTimeKeyType == 1) {
		editScheduleScreen->addItem(
			ITEM_RANGE_REF("Hour", _selHour, (uint8_t)1, (uint8_t)0, (uint8_t)23, onHourChanged, "%02u"));
		editScheduleScreen->addItem(
			ITEM_RANGE_REF("Min", _selMinute, (uint8_t)1, (uint8_t)0, (uint8_t)59, onMinuteChanged, "%02u"));
		editScheduleScreen->addItem(
			ITEM_RANGE_REF("Sec", _selSecond, (uint8_t)1, (uint8_t)0, (uint8_t)59, onSecondChanged, "%02u"));

		_daySummary = "Days: " + buildDaySummary(_selDaysOfWeek);
		editScheduleScreen->addItem(
			ITEM_COMMAND(_daySummary.c_str(), onDaysBtnPressed));
	}

	editScheduleScreen->addItem(
		ITEM_COMMAND("< Back", onScheduleBackPressed));
}

static void onEditScheduleBtnPressed() {
	buildEditScheduleScreen();
	editScheduleScreen->setParent(editEventScreen);
	Garden::controlPanel->menu->setScreen(editScheduleScreen);
}

static void onDeleteEventBtnPressed() {
	Garden::scheduler->removeEvent(_selEventId);
	_selEventId = 0;
	buildSchedulerScreen();
	schedulerScreen->setParent(mainScreen);
	Garden::controlPanel->menu->setScreen(schedulerScreen);
}

static void onEditEventBackPressed() {
	auto tk = makeTimeKeyFromSelection();
	ScheduledEvent evt;
	evt.timeKey = tk;
	evt.lastTriggeredTime = 0;
	evt.eventType = static_cast<ScheduledEventType>(_selEventType);
	evt.moduleId = _selModule;
	evt.actionId = _selAction;
	evt.stateType = _selState;

	if (_selEventId != 0) {
		Garden::scheduler->updateEvent(_selEventId, tk, evt);
	} else {
		Garden::scheduler->addEvent(tk, evt);
	}

	buildSchedulerScreen();
	schedulerScreen->setParent(mainScreen);
	Garden::controlPanel->menu->setScreen(schedulerScreen);
}

static void buildEditEventScreen() {
	while (editEventScreen->size() > 0) {
		delete editEventScreen->getItemAt(editEventScreen->size() - 1);
		editEventScreen->removeLastItem();
	}

	BaseModule* mod = Garden::scheduler->getModule(_selModule);

	static const std::vector<const char*> eventTypes = {"Trigger Module", "Trigger Action", "Set State"};
	if (_selEventType >= eventTypes.size()) _selEventType = eventTypes.size() - 1;
	_stateOptions = mod ? mod->getStates() : std::vector<const char*>{"no-op"};
	_actionOptions = mod ? mod->getActions() : std::vector<const char*>{"no-op"};
	if (_stateOptions.empty()) _stateOptions.push_back("no-op");
	if (_actionOptions.empty()) _actionOptions.push_back("no-op");
	if (_selState >= _stateOptions.size()) _selState = _stateOptions.size() - 1;
	if (_selAction >= _actionOptions.size()) _selAction = _actionOptions.size() - 1;

	editEventScreen->addItem(
		ITEM_LIST("Event", eventTypes, onEventTypeChanged, _selEventType, "%s", 0, true));

	// Mod: visible for Trigger Module (0) and Set State (2)
	if (_selEventType == 0 || _selEventType == 2) {
		_moduleLabel = mod ? ("Mod: " + String(mod->getShortName())) : "Mod: None";
		editEventScreen->addItem(
			ITEM_COMMAND(_moduleLabel.c_str(), onModuleCycled));
	}

	// Action: visible for Trigger Module (0) and Trigger Action (1)
	if (_selEventType == 0 || _selEventType == 1) {
		editEventScreen->addItem(
			ITEM_LIST("Action", _actionOptions, onActionChanged, _selAction, "%s", 0, true));
	}

	// State: visible for Set State (2) only
	if (_selEventType == 2) {
		editEventScreen->addItem(
			ITEM_LIST("State", _stateOptions, onStateChanged, _selState, "%s", 0, true));
	}

	editEventScreen->addItem(
		ITEM_COMMAND("Edit Schedule >", onEditScheduleBtnPressed));

	editEventScreen->addItem(
		ITEM_COMMAND("Delete Event", onDeleteEventBtnPressed));

	editEventScreen->addItem(
		ITEM_COMMAND("< Back", onEditEventBackPressed));

	Garden::controlPanel->menu->refresh();
}

static void onSchedulerEventSelected() {
	uint8_t pos = Garden::controlPanel->menu->getCursor();
	uint8_t evtIdx = pos - 1;
	if (evtIdx >= _schedEvtIds.size()) return;

	_selEventId = _schedEvtIds[evtIdx];
	const auto& sched = Garden::scheduler->getSchedule();
	auto it = sched.find(_selEventId);
	if (it != sched.end() && !it->second.empty()) {
		const auto& ev = it->second[0];
		_selEventType = static_cast<uint8_t>(ev.eventType);
		_selModule = ev.moduleId;
		_selAction = ev.actionId;
		_selState = ev.stateType;
		auto at = std::static_pointer_cast<AtTime>(ev.timeKey);
		if (at) {
			_selTimeKeyType = 1;
			_selHour = at->hour;
			_selMinute = at->minute;
			_selSecond = at->second;
			_selDaysOfWeek = at->weekFilter;
		} else {
			_selTimeKeyType = 0;
			_selHour = 0;
			_selMinute = 0;
			_selSecond = 0;
			_selDaysOfWeek = 0;
		}
	}
	buildEditEventScreen();
	editEventScreen->setParent(schedulerScreen);
	Garden::controlPanel->menu->setScreen(editEventScreen);
}

void buildSchedulerScreen() {
	while (schedulerScreen->size() > 5) {
		delete schedulerScreen->getItemAt(1);
		schedulerScreen->removeItemAt(1);
	}

	_schedEvtIds.clear();
	_schedEvtLabels.clear();

	const auto& eventOrder = Garden::scheduler->getEventOrder();
	const auto& schedule = Garden::scheduler->getSchedule();

	for (size_t i = 0; i < eventOrder.size(); i++) {
		EventId eid = eventOrder[i];
		auto it = schedule.find(eid);
		if (it == schedule.end()) continue;

		_schedEvtIds.push_back(eid);

		String label = "E" + String(i + 1);
		if (!it->second.empty()) {
			uint8_t modId = it->second[0].moduleId;
			BaseModule* mod = Garden::scheduler->getModule(modId);
			if (mod) {
				const char* sn = mod->getShortName();
				label += " " + String(sn);
			}
		}
		_schedEvtLabels.push_back(label);
	}

	for (size_t i = 0; i < _schedEvtLabels.size(); i++) {
		schedulerScreen->addItemAt(1,
			ITEM_COMMAND(_schedEvtLabels[i].c_str(), onSchedulerEventSelected));
	}
}

static void onAddEventPressed() {
	_selEventId = 0;
	_selEventType = 0;
	_selAction = 0;
	_selState = 0;
	_selTimeKeyType = 0;
	_selHour = 0;
	_selMinute = 0;
	_selSecond = 0;
	_selDaysOfWeek = 0;

	const auto& modules = Garden::scheduler->getModules();
	_selModule = modules.empty() ? 0 : modules.begin()->first;

	buildEditEventScreen();
	editEventScreen->setParent(schedulerScreen);
	Garden::controlPanel->menu->setScreen(editEventScreen);
}

void setupScreens(void) {
	MenuItem* removeAll = schedulerScreen->getItemAt(1);
	MenuItem* apply    = schedulerScreen->getItemAt(2);
	MenuItem* discard  = schedulerScreen->getItemAt(3);
	MenuItem* backBtn  = schedulerScreen->getItemAt(4);

	delete schedulerScreen->getItemAt(0);
	schedulerScreen->clear();

	schedulerScreen->addItem(ITEM_COMMAND("Add event +", onAddEventPressed));
	schedulerScreen->addItem(removeAll);
	schedulerScreen->addItem(apply);
	schedulerScreen->addItem(discard);
	schedulerScreen->addItem(backBtn);
}
