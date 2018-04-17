#ifndef _OCTOPOS_DRIVER_H_
#define _OCTOPOS_DRIVER_H_

#include <string>
#include <utility>
#include <ctime>
#include <map>
#include <iostream>
#include <cstdlib>
#include <cstring> // for memset
#include <sys/types.h>
#include <sys/wait.h>
#include <queue>
#include <list>

#include "json.hpp" // TODO(llazarek): Replace with real lib

#include "Optional.hpp"
#include <octopOS/publisher.h>
#include <octopOS/subscriber.h>
#include <octopOS/octopos.h>

/** The absolute path of the octopOS config */
extern const char*  CONFIG_PATH;
/** The topic name for module upgrades */
extern const char*  UPGRADE_TOPIC;
/** The topic name for module downgrades */
extern const char*  DOWNGRADE_TOPIC;
/** The minimum number of seconds of runtime before death for modules.
 *  Modules that die in less than this amount of time will be marked
 *  as having died "suspiciously". This is the *only* criteria for
 *  "suspicious" deaths.
 */
extern const time_t RUNTIME_CUTOFF_DOWNGRADE_S;
/** The maximum number of times in a row that a module can die
 *  "suspiciously" before being downgraded.
 *  See `RUNTIME_CUTOFF_DOWNGRADE_S`.
 */
extern const int    DEATH_COUNT_CUTOFF_DOWNGRADE;
/** Should OctopOS listen for module upgrade requests? */
extern const bool   LISTEN_FOR_MODULE_UPGRADES;


typedef long MemKey;
/** The structure containing all significant information about managed
 *  modules.
 */
struct Module {
    /** The PID of the module process. */
    pid_t pid;
    /** The ID of the tentacle assigned to the module. */
    int tentacle_id;
    /** Whether the module has been intentionally killed */
    bool killed;
    /** Whether the module has been requested to be downgraded */
    bool downgrade_requested;
    /** The time that the module was launched. This is used to
     *  calculate running time when the module dies.
     */
    time_t launch_time;
    /** The number of early/"suspicious" _sequential_ deaths of the module. */
    int early_death_count;
    /**
     * Module constructor.
     * @param _pid
     * @param _tentacle_id
     * @param _launch_time
     * @return A new Module
     */
    Module(pid_t _pid, int _tentacle_id, time_t _launch_time):
        pid(_pid), tentacle_id(_tentacle_id), launch_time(_launch_time),
        killed(false), downgrade_requested(false),
        early_death_count(0) { }
    /**
     * Module default constructor. Just here to be able to put them in std::map.
     * Use the real constructor in your code instead.
     * @return A new Module
     */
    Module() { }
};
typedef std::map<std::string, Module> ModuleInfo;
typedef std::pair<ModuleInfo, MemKey> LaunchInfo;
typedef std::string FilePath;

/**
 * Is `file` accessible?
 * @param file A filepath.
 * @return Is `file` accessible?
 */
bool accessible(FilePath file);

/**
 * Load the JSON file at JSON_FILE.
 * @param json_file The path to the file to load.
 * @return The parsed file, if found.
 */
Optional<json> load(FilePath json_file);

/**
 * Launch the given MODULE with memory key KEY.
 * @param module The path to the module executable.
 * @param key The memory key to provide the module.
 * @return The PID of the launched module.
 */
pid_t launch(FilePath module, MemKey key);

/**
 * @brief Relaunch the given module at the given path.
 *
 * @param module Reference to the module to relaunch, *which will be mutated*.
 * @param path The path of the module executable.
 */
void relaunch(Module &module, FilePath path);

/**
 * @brief Launch all of the modules in the given directory, starting
 * with the given memory key. The memory key will be given to the
 * first module, and every module after will get the previous module's
 * memory key + 1.
 *
 * @param dir The absolute path to the directory containing module
 * executables to launch.
 * @param start_key The starting memory key
 * @return A pair of the `Module`s launched and the next unused memory key.
 */
LaunchInfo launch_modules_in(FilePath dir, MemKey start_key);

/**
 * @brief List the modules in the given directory.
 *
 * @param dir An absolute path to a directory.
 * @return A list of modules in the directory.
 */
std::list<FilePath> modules_in(FilePath dir);


/**
 * @brief List the files in the given directory, if it exists.
 *
 * @param dir An absolute path to a directory.
 * @return A list of files in the directory, if it exists.
 */
Optional< std::list<FilePath> > files_in(FilePath dir);


/**
 * @brief Run the listener/background threads for OctopOS.
 *
 * @return Success status.
 */
bool launch_octopOS_listeners();

/**
 * @brief Launch the OctopOS primary listener thread handling all
 * communication via tentacles.
 *
 * @param tentacle_index The tentacle index on which the listener
 * should operate.
 * @return Success status.
 */
bool launch_octopOS_listener_for_child(int tentacle_index);

/**
 * @brief Find the first `Module` with the given pid in the given set
 * of modules.
 *
 * @param pid
 * @param modules The set of modules to search.
 * @return The `Module` with the given pid, if found.
 */
Optional<std::string> find_module_with(pid_t pid, const ModuleInfo &modules);

/**
 * @brief Reboot the module with the given executable path.
 *
 * @param path The path of the module to reboot.
 * @param modules The set of active modules, which *will be mutated to
 * update with the rebooted module information.*
 * @param downgrade_pub The publisher for downgrade requests.
 */
void reboot_module(std::string path, ModuleInfo *modules,
                   publisher<OctoString> &downgrade_pub);

/**
 * @brief Kill the module with the given executable path.
 *
 * @param path The path of the module to kill.
 * @param modules The set of active modules, which *will be mutated to
 * update the killed `Module`.
 * @return The return of the `kill` system command.
 */
int kill_module(std::string path, ModuleInfo *modules);

/**
 * @brief Does the given module need a downgrade?
 *
 * @param module
 * @return Does the given module need a downgrade?
 */
bool module_needs_downgrade(Module *module);

/**
 * @brief Babysit the given active modules, rebooting and/or
 * downgrading them if they die and handling upgrade requests.
 *
 * @param modules The active set of modules.
 * @param downgrade_pub The publisher for downgrade requests.
 * @param upgrade_sub The subsriber for upgrade requests.
 */
void babysit_forever(ModuleInfo &modules,
                     publisher<OctoString> &downgrade_pub,
                     subscriber<OctoString> &upgrade_sub);

/**
 * @brief Launch OctopOS.
 *
 * @return A reference to the OctopOS instance.
 */
octopOS& launch_octopOS();

/**
 * @brief Reboot all of the active modules that have died (if any). If
 * any modules need to be downgraded, a downgrade will be requested.
 *
 * @param modules The active set of modules.
 * @param downgrade_pub The publisher for downgrade requests.
 */
void reboot_dead_modules(ModuleInfo *modules,
                         publisher<OctoString> *downgrade_pub);

#endif /* _OCTOPOS_DRIVER_H_ */
