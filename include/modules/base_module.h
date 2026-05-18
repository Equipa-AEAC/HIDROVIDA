#ifndef BASE_MODULE_H
#define BASE_MODULE_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>

using Param = std::variant<std::monostate, int64_t, double, std::string, bool>;
using Params = std::vector<Param>;

class BaseModule {
public:
	BaseModule(const char* name = "Unknown", const char* shortName = "?", std::vector<const char*> actions = {}, std::vector<const char*> states = {}) : _name(name), _shortName(shortName), _actions(actions), _states(states) {}

	virtual void initialize(void) = 0;
	virtual void update(void) = 0;

	virtual void setState(uint8_t stateIdx) = 0;
	virtual void triggerAction(uint8_t actionIdx, const Params& params = {}) = 0;

	const char* getName() const { return _name; }
	const char* getShortName() const { return _shortName; }

	std::vector<const char*> getActions() const { return _actions; }
	std::vector<const char*> getStates() const { return _states; }

protected:
	const char* _name;
	const char* _shortName;
	std::vector<const char*> _actions;
	std::vector<const char*> _states;
};

#endif // BASE_MODULE_H