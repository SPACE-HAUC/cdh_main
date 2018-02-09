/*!
 * @file
 * @brief Program driver. Receives commands, and acts upon them.
 */

// Copyright 2016 UMass Lowell Command and Data Handling Team

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include <octopOS/octopos.h>
#include <octopOS/subscriber.h>
#include <octopOS/publisher.h>

#include "../include/Optional.hpp"
#include "../include/octopOS_driver.hpp"

int main(int argc, char const *argv[]) {
    octopOS &octopos = octopOS::getInstance();
    MemKey current_key = MSGKEY;

    Optional<json> maybe_config = load(CONFIG_PATH);
    if (maybe_config.isEmpty()) {
	std::cerr << "Critical Error: Unable to read config at "
		  << CONFIG_PATH
		  << ". Exiting..." << std::endl;
	return 1;
    }
    json config = maybe_config.get();


    LaunchInfo launched = launch_modules_in(config["modules_enabled"],
					    current_key);
    ModuleInfo modules = launched.first;
    // Keep track of the memkeys we've given out so that we can give valid ones
    // when creating our own pub/subs
    current_key = launched.second;

    if (!launch_octopOS_listeners()) {
	std::cerr << "Critical Error: Unable to spawn OctopOS listener threads."
		  << " Exiting..." << std::endl;
	return 2;
    }


    publisher<std::string> downgrade_pub(DOWNGRADE_TOPIC, current_key++);
    ChildHandler::register_child_handler(&modules, &downgrade_pub);

    auto upgrade_module = modules[UPGRADE_TOPIC];
    subscriber<std::string> upgrade_sub(UPGRADE_TOPIC, upgrade_module.msgkey);

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
    }
}
