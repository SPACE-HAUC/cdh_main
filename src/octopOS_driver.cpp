// Copyright 2017 Space HAUC Command and Data Handling
// This file is part of Space HAUC which is released under AGPLv3.
// See file LICENSE.txt or go to <http://www.gnu.org/licenses/> for full
// license details.

/*!
 * @file
 *
 * @brief The octopOS driver program. Implements satellite process
 * initialization, management, and error handling.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>
#include <list>
#include <fstream>

#include <octopOS/octopos.h>
#include <octopOS/subscriber.h>

#include "../include/Optional.hpp"
#include "../include/octopOS_driver.hpp"


const char*  CONFIG_PATH = "/etc/octopOS/config.json";
const char*  UPGRADE_TOPIC = "module_upgrade";
const char*  DOWNGRADE_TOPIC = "module_downgrade";
const time_t RUNTIME_CUTOFF_DOWNGRADE_S = 600; // tmp switch back to reasonable
const int    DEATH_COUNT_CUTOFF_DOWNGRADE = 5;
const bool   LISTEN_FOR_MODULE_UPGRADES = true;
const int    OCTOPOS_INTERNAL_TENTACLE_INDEX = 0;

int ChildHandler::reboot_count = 0; // tmp
std::queue<pid_t> ChildHandler::rebootQ;
ModuleInfo *ChildHandler::modules;
publisher<OctoString> *ChildHandler::downgrade_pub;


int memkey_to_tentacle_index(MemKey key) {
    return key - MSGKEY + 1;
}
int tentacle_index_to_memkey(int index) {
    return index + MSGKEY - 1;
}

Optional<json> load(FilePath json_file) {
    if (accessible(json_file)) {
	std::ifstream in(json_file);
	return Just(json::parse(in));
    }
    return None<json>();
}

// Courtesy of:
// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if
// -a-file-exist-using-standard-c-c11-c
bool accessible(FilePath file) {
    struct stat buffer;
    return (stat (file.c_str(), &buffer) == 0);
}

// Returns a list of *complete* (relative or absolute) paths to the
// files in DIRECTORY if directory is accessible.
Optional< std::list<FilePath> > files_in(FilePath directory) {
    DIR *dir;
    struct dirent *ent;

    dir = opendir(directory.c_str());
    if (dir == NULL) {
        return None<std::list<FilePath>>();
    } else {
	std::list<FilePath> files;
	while (ent = readdir(dir)){
	    std::string file = ent->d_name;
	    // Don't add "." and ".."
	    if(file != "." && file != "..") {
		files.push_front(directory + "/" + file);
	    }
	}
	closedir(dir);

	return Just(files);
    }
}

// launches the given module in a new child process
pid_t launch(FilePath module, MemKey key) {
    pid_t pid;
    switch(pid = fork()) {
    case -1:
	perror("Fork failed in attempting to launch module.");
	break;
    case 0: // child
        execl(module.c_str(), std::to_string(key).c_str(), (char*)0);
	exit(0);
    default: // parent
	break;
    }
    return pid;
}

// Modifies MODULE
void relaunch(Module &module, FilePath path) {
    module.killed = false;
    module.downgrade_requested = false;
    module.pid = launch(path, tentacle_index_to_memkey(module.tentacle_id));
    module.launch_time = time(0);
}

bool launch_octopOS_listener_for_child(int tentacle_index) {
    pthread_t thread;
    // tentacle_ID should be a somewhat persistent pointer, because
    // pthread_create casts it to a int* and then derefs it to get the
    // value
    MemKey *idxptr = NULL;
    if((idxptr = (MemKey*)malloc(sizeof(MemKey))) == NULL) {
      return false;
    }
    *idxptr = tentacle_index;
    return !pthread_create(&thread, NULL, octopOS::listen_for_child, idxptr);
}

LaunchInfo launch_modules_in(FilePath dir, MemKey start_key) {
    ModuleInfo modules;
    MemKey current_key = start_key;
    pid_t pid;
    for (FilePath module : modules_in(dir)) {
	pid_t pid = launch(module, current_key);
	// Tentacle IDs for children start at 1 because 0 is for octopOS
	modules[module] = Module(pid, memkey_to_tentacle_index(current_key),
				 time(0));
	launch_octopOS_listener_for_child(modules[module].tentacle_id);
	current_key++;
    }
    return std::make_pair(modules, current_key);
}

bool launch_octopOS_listeners() {
    pthread_t sub_thread;

    // Wait for data doesn't need an argument
    return launch_octopOS_listener_for_child(OCTOPOS_INTERNAL_TENTACLE_INDEX) &&
	   !pthread_create(&sub_thread, NULL,
			   subscriber_manager::wait_for_data, NULL);
}


// Currently just returns all the files in DIR.
//
// In the future we may need to do something more complex than just
// the files in the given directory - for example if a folder
// structure is necessary (for whatever reason).
std::list<FilePath> modules_in(FilePath dir) {
    auto files = files_in(dir);
    if (files.isEmpty()) {
	std::cerr << "Error: Unable to read module path from config: " << dir
		  << std::endl;
    }

    return files.getDefault(std::list<FilePath>());
}

// Modifies MODULES[PATH]
int kill_module(std::string path, ModuleInfo *modules) {
    Module &module = (*modules)[path];
    module.killed = true;
    // Intentional deaths should reset early death counter
    module.early_death_count = 0;
    return kill(module.pid, SIGTERM);
}

// Modifies MODULE to record premature death if necessary
bool module_needs_downgrade(Module *module) {
    int died_quickly =
	(time(0) - (module -> launch_time)) < RUNTIME_CUTOFF_DOWNGRADE_S;
    if (died_quickly) {
	module -> early_death_count += 1;
    } else {
	// "Normal" deaths should reset early death counter
	module -> early_death_count = 0;
    }
    int died_too_many_times =
	(module -> early_death_count) > DEATH_COUNT_CUTOFF_DOWNGRADE;

    return died_quickly && died_too_many_times;
}

void downgrade(FilePath module_name, publisher<OctoString> &downgrade_pub) {
    std::cout << "Publishing module [" << module_name << "] to downgrade pub!"
	      << std::endl; // tmp
    downgrade_pub.publish(module_name);
}

// Modifies MODULES[PATH]
void reboot_module(std::string path, ModuleInfo *modules,
		   publisher<OctoString> &downgrade_pub) {
    Module &module = (*modules)[path];
    if (module.killed || !module_needs_downgrade(&module)) {
	// Death was intentional or unsuspicious
	relaunch(module, path);
    } else {
	// Death warrants downgrade
	module.downgrade_requested = true;
	// Reset death count to give downgraded module a chance
	module.early_death_count = 0;
	downgrade(path, downgrade_pub);
    }
}

Optional<std::string> find_module_with(pid_t pid, const ModuleInfo &modules) {
    auto moduleIt = std::find_if(modules.begin(), modules.end(),
				 [&](const std::pair<std::string, Module> el) {
				     return el.second.pid == pid;
				 });
    if (moduleIt == modules.end()) {
	return None<std::string>();
    } else {
	return Just(moduleIt -> first);
    }
}

// Watch over children, rebooting and upgrading modules
void babysit_forever(ModuleInfo &modules,
		     publisher<OctoString> &downgrade_pub,
		     subscriber<OctoString> &upgrade_sub) {
    std::cout << "> Starting to babysit..." << std::endl;
    std::cout << "> About to register handler..." << std::endl;
    ChildHandler::register_child_handler(&modules, &downgrade_pub);

    std::cout << "> Entering loop..." << std::endl;
    while (1) {
	// reboot any dead modules
	ChildHandler::reboot_dead_modules();

	// check for upgrade data
	if(upgrade_sub.data_available()) {
	    std::string module_path = upgrade_sub.get_data();
	    Module &module = modules[module_path];
	    if (module.downgrade_requested) {
		relaunch(module, module_path);
	    } else {
		kill_module(module_path, &modules);
	    }
	}
	usleep(10000);
    }
}

octopOS& launch_octopOS() {
    octopOS &octopos = octopOS::getInstance();

    if (!launch_octopOS_listeners()) {
	std::cerr << "Critical Error: Unable to spawn OctopOS listener threads."
		  << " Exiting..." << std::endl;
	exit(2);
    }

    return octopos;
}
