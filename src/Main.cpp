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
    octopOS &octopos = launch_octopOS();
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

    publisher<OctoString> downgrade_pub(DOWNGRADE_TOPIC, current_key++);
    subscriber<OctoString> upgrade_sub(UPGRADE_TOPIC, current_key - 1);
    babysit_forever(modules, downgrade_pub, upgrade_sub);
}
