#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE octopOS_driver
// Child deaths are not an error
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE
#define BOOST_TEST_IGNORE_SIGCHLD
#include <boost/test/unit_test.hpp>

#include "../include/Optional.hpp"
#include "../include/octopOS_driver.hpp"
#include "../include/octopos.h"
#include "../include/subscriber.h"
#include "../include/publisher.h"

BOOST_AUTO_TEST_CASE(reboot_module_test) {
    // ----- HERE BE DRAGONS! DO NOT TOUCH! -----
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    MemKey current_key = MSGKEY;

    pthread_t subscriber_manager_thread;
    BOOST_REQUIRE(pthread_create(&subscriber_manager_thread, NULL,
				 subscriber_manager::wait_for_data, NULL) == 0);
    BOOST_REQUIRE(launch_octopOS_listener_for_child(0));
    publisher<OctoString> downgrade_pub(DOWNGRADE_TOPIC, current_key++);
    subscriber<OctoString> downgrade_sub(DOWNGRADE_TOPIC, current_key - 1);
    // ----------

    const FilePath path = "./modules";
    LaunchInfo info = launch_modules_in(path, current_key);
    ModuleInfo modules = info.first;

    // first reboot on a module that shouldn't be downgraded because
    // it hasn't died enough times
    const FilePath module1 = path + "/test_module";
    Module &m1 = modules[module1];
    pid_t oldpid = m1.pid;
    sleep(1);
    BOOST_REQUIRE(kill(m1.pid, SIGTERM) == 0);
    reboot_module(module1, &modules, downgrade_pub);
    m1 = modules[module1]; // the Module has changed
    BOOST_REQUIRE(m1.pid != oldpid);
    BOOST_REQUIRE(m1.pid > 1);
    BOOST_REQUIRE(!m1.killed);
    BOOST_REQUIRE(!m1.downgrade_requested);
    sleep(1);
    BOOST_REQUIRE(!downgrade_sub.data_available());
    BOOST_REQUIRE(m1.early_death_count == 1);

    sleep(1);
    // now reboot on a module that SHOULD be downgraded because it has
    // died too many times
    oldpid = m1.pid;
    m1.early_death_count = 200;
    BOOST_REQUIRE(kill(m1.pid, SIGTERM) != -1);
    sleep(1);
    reboot_module(module1, &modules, downgrade_pub);
    m1 = modules[module1]; // the Module has changed
    // pid shouldn't have changed because we shouldn't have relaunched it
    BOOST_REQUIRE(m1.pid == oldpid);
    BOOST_REQUIRE(!m1.killed);
    BOOST_REQUIRE(m1.downgrade_requested);
    sleep(1);
    BOOST_REQUIRE(downgrade_sub.data_available());
    BOOST_REQUIRE(downgrade_sub.get_data().get() == module1);
    BOOST_REQUIRE(m1.early_death_count == 0);

    sleep(1);
    // now reboot on a module that shouldn't be downgraded because it
    // was killed intentionally
    const FilePath module2 = path + "/test_module2";
    Module &m2 = modules[module2];
    oldpid = m2.pid;
    BOOST_REQUIRE(kill_module(module2, &modules) != -1);
    BOOST_REQUIRE(m2.killed);
    reboot_module(module2, &modules, downgrade_pub);
    m2 = modules[module2]; // the Module has changed
    BOOST_REQUIRE(m2.pid != oldpid);
    BOOST_REQUIRE(m2.pid > 1);
    BOOST_REQUIRE(!m2.downgrade_requested);
    BOOST_REQUIRE(!downgrade_sub.data_available());
    BOOST_REQUIRE(m2.early_death_count == 0);
    sleep(2); // wait long enough for the child to run a bit
    BOOST_REQUIRE(kill(m2.pid, SIGTERM) != -1);
}
