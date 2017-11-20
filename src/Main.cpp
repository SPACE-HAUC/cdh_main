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
#include "../include/octopOS_driver.hpp"
#include <octopOS/subscriber.h>

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
    current_key = launched.second;
    ModuleInfo modules = launched.first;

    if (!launch_listeners()) {
	std::cerr << "Critical Error: Unable to spawn listener threads."
		  << "Exiting..." << std::endl;
	return -1;
    }

    ChildHandler::register_child_death_handler(&modules);

    auto upgrade_module = modules[UPGRADE_TOPIC];
    subscriber<std::string> upgrade_sub(UPGRADE_TOPIC, upgrade_module.msgkey);

    while (LISTEN_FOR_MODULE_UPGRADES) {
	std::string module_path = upgrade_sub.get_data();
	kill_module(module_path, &modules);
    }
}
