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

struct babysitInfo {
    ModuleInfo* modules;
    publisher<OctoString> *downgrade_pub;
    subscriber<OctoString> *upgrade_sub;
};

void* run_babysit_forever(void *modules) {
    struct babysitInfo bsi = *(struct babysitInfo*)modules;
    babysit_forever(*bsi.modules, *bsi.downgrade_pub, *bsi.upgrade_sub);
    return NULL;
}

BOOST_AUTO_TEST_CASE(babysit_forever_test) {
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
    subscriber<OctoString> upgrade_sub(UPGRADE_TOPIC, current_key - 1);


    const FilePath path = "./modules";
    LaunchInfo info = launch_modules_in(path, current_key);
    ModuleInfo modules = info.first;

    struct babysitInfo bsi;
    bsi.modules = &modules;
    bsi.downgrade_pub = &downgrade_pub;
    bsi.upgrade_sub = &upgrade_sub;
    pthread_t babysit_thread;
    BOOST_REQUIRE(!pthread_create(&babysit_thread, NULL,
                                  run_babysit_forever, (void*)(&bsi)));

    // first reboot on a module that shouldn't be downgraded because
    // it hasn't died enough times
    const FilePath module1 = path + "/test_module";
    Module &m1 = modules[module1];
    pid_t oldpid = m1.pid;
    sleep(1); // need multiple sleeps to give the reboot thread a ctx switch
    sleep(1);
    BOOST_REQUIRE(kill(m1.pid, SIGTERM) != -1);
    sleep(1);
    sleep(1);
    BOOST_REQUIRE(m1.pid != oldpid);
    BOOST_REQUIRE(m1.pid > 1);
    BOOST_REQUIRE(!m1.killed);
    BOOST_REQUIRE(!m1.downgrade_requested);
    BOOST_REQUIRE(!downgrade_sub.data_available());
    BOOST_REQUIRE(m1.early_death_count == 1);

    // next module death is too many times so shouldn't be rebooted
    sleep(1);
    sleep(1);
    oldpid = m1.pid;
    m1.early_death_count = 200;
    BOOST_REQUIRE(kill(m1.pid, SIGTERM) != -1);
    sleep(1);
    sleep(1);
    BOOST_REQUIRE(m1.pid == oldpid);
    BOOST_REQUIRE(!m1.killed);
    BOOST_REQUIRE(m1.downgrade_requested);
    BOOST_REQUIRE(downgrade_sub.data_available());
    BOOST_REQUIRE(downgrade_sub.get_data().get() == module1);
    BOOST_REQUIRE(m1.early_death_count == 0);

    sleep(1);
    sleep(1);
    // now reboot on a module that shouldn't be downgraded because it
    // was killed intentionally
    const FilePath module2 = path + "/test_module2";
    Module &m2 = modules[module2];
    oldpid = m2.pid;
    BOOST_REQUIRE(kill_module(module2, &modules) != -1);
    BOOST_REQUIRE(m2.killed);
    sleep(1);
    sleep(1);
    BOOST_REQUIRE(m2.pid != oldpid);
    BOOST_REQUIRE(m2.pid > 1);
    BOOST_REQUIRE(!m2.downgrade_requested);
    BOOST_REQUIRE(!downgrade_sub.data_available());
    BOOST_REQUIRE(m2.early_death_count == 0);

    // All done, kill module2
    kill(m2.pid, SIGTERM);
}
