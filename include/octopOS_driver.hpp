#ifndef _OCTOPOS_DRIVER_H_
#define _OCTOPOS_DRIVER_H_

#include "json.h" // See https://github.com/nlohmann/json
#include <string>
#include <stdexcept>
#include <utility>
#include <ctime>

const char* CONFIG_PATH = "/etc/octopOS/config.json";
const char* REBOOT_TOPIC = "module_reboot";
const time_t RUNTIME_CUTOFF_DOWNGRADE_S = 60;
const bool LISTEN_FOR_MODULE_REBOOTS = true;

using json = nlohmann::json;

typedef const long MemKey;
struct Module {
    pid_t pid;
    MemKey msgkey;
    bool killed;
    time_t launch_time;
    Module(pid_t _pid, MemKey _msgkey, time_t _launch_time):
	pid(_pid), msgkey(_msgkey), launch_time(_launch_time),
	killed(false) { }
};
typedef std::map<std::string, Module> ModuleInfo;
typedef std::pair<ModuleInfo, MemKey> LaunchInfo;
typedef const std::string& FilePath;

class Optional<T> {
public:
    static Optional<T> None() {
	return Optional(true);
    }
    static Optional<T> Just(T value) {
	auto some = Optional(false);
	some.x = value;
	return some;
    }
    bool isEmpty() { return empty; }
    T getDefault(T default_value) {
	return empty ? default_value : x;
    }
    // throws: std::runtime_exception
    T get() {
	if (empty) {
	    throw std::runtime_exception("Get on None");
	}
	return x;
    }
private:
    Optional(bool is_none) : empty(is_none);
    bool empty;
    T x;
};

Optional<json> load(FilePath json_file);
pid_t launch(FilePath module, MemKey key);
ModuleInfo launch_modules_in(FilePath dir, MemKey start_key);
bool launch_listeners();

#endif /* _OCTOPOS_DRIVER_H_ */
