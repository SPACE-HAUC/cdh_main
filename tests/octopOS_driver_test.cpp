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

// BOOST_AUTO_TEST_CASE(launch_octopOS_test) {
    // launch_octopOS();
// }

// struct F {
//     F() : octopos(launch_octopOS()) {
//       std::cout << "Creating octopos" << std::endl;
//     }
//     ~F() {
//       std::cout << "Destroying octopos" << std::endl;
//     }

//     octopOS &octopos;
// };
// BOOST_GLOBAL_FIXTURE(F);

// BOOST_AUTO_TEST_CASE(optional_test) {
//     auto o = Just(5);
//     auto n = None<int>();
//     BOOST_REQUIRE(!o.isEmpty());
//     BOOST_REQUIRE(n.isEmpty());
//     BOOST_REQUIRE_NO_THROW(o.get());
//     BOOST_REQUIRE_THROW(n.get(), std::runtime_error);
//     BOOST_REQUIRE(o.get() == 5);
//     BOOST_REQUIRE(o.getDefault(-1) == 5);
//     BOOST_REQUIRE(n.getDefault(-1) == -1);
// }

// BOOST_AUTO_TEST_CASE(accessible_test) {
//     BOOST_REQUIRE(accessible("./test.json"));
//     BOOST_REQUIRE(!accessible("./thisfiledoesntexist!.88"));
// }

// BOOST_AUTO_TEST_CASE(load_test) {
//     Optional<json> oj = load("./thisfiledoesntexist!.88");
//     BOOST_REQUIRE(oj.isEmpty());

//     Optional<json> oj2 = load("./test.json");
//     BOOST_REQUIRE(!oj2.isEmpty());
//     json j = oj2.get();
//     BOOST_REQUIRE(j["a"] == "something");
//     BOOST_REQUIRE(j["b"] == "else");
// }

// BOOST_AUTO_TEST_CASE(launch_test) {
//     std::cout << "Beginning of launch_test" << std::endl;
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     pid_t pid = launch("./modules/test_module", 0);
//     BOOST_REQUIRE(pid > 1);
//     sleep(1);
//     BOOST_REQUIRE(kill(pid, SIGTERM) == 0);
//     std::cout << "End of launch_test" << std::endl;
// }

// BOOST_AUTO_TEST_CASE(launch_octopOS_listeners_test) {
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     BOOST_REQUIRE(launch_octopOS_listeners());
// }

// BOOST_AUTO_TEST_CASE(files_in_test) {
//     auto olist = files_in("../tests");
//     BOOST_REQUIRE(!olist.isEmpty());
//     auto list = olist.get();
//     BOOST_REQUIRE(list.size() >= 1);
//     // list should return full paths - not just filename
//     std::string path = "../tests/";
//     for(FilePath f: list) {
//       BOOST_REQUIRE(f.compare(0, path.size(), path) == 0);
//     }
// }

// BOOST_AUTO_TEST_CASE(module_needs_downgrade_test) {
//     Module m1(111, 1, 1); // launched a looooong time ago
//     BOOST_REQUIRE(!module_needs_downgrade(&m1));
//     Module m2(111, 1, time(0)); // launched just now
//     m2.early_death_count = 10; // and died early 10 times in a row
//     BOOST_REQUIRE(module_needs_downgrade(&m2));
// }

// BOOST_AUTO_TEST_CASE(find_module_with_test) {
//     ModuleInfo modules = {
// 	{"a", Module(111, 1, 1)},
// 	{"b", Module(222, 2, 1)},
// 	{"c", Module(333, 3, 1)}
//     };
//     auto empty = find_module_with(5, modules);
//     BOOST_REQUIRE(empty.isEmpty());
//     auto oname = find_module_with(222, modules);
//     BOOST_REQUIRE(!oname.isEmpty());
//     BOOST_REQUIRE(oname.get() == "b");
//     auto oname2 = find_module_with(111, modules);
//     BOOST_REQUIRE(!oname2.isEmpty());
//     BOOST_REQUIRE(oname2.get() == "a");
// }

// BOOST_AUTO_TEST_CASE(relaunch_test) {
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     Module m(-1, 0, time(0));
//     relaunch(m, "./modules/test_module");
//     BOOST_REQUIRE(m.pid > 1);
//     BOOST_REQUIRE(!m.killed);
//     BOOST_REQUIRE(!m.downgrade_requested);
//     BOOST_REQUIRE(time(0) - m.launch_time < 2000);
//     sleep(1);
//     BOOST_REQUIRE(kill(m.pid, SIGTERM) == 0);
// }

// // Problem: boost catches sigchld and makes it fail the test case
// // apparently no way to disable without editing source of boost?
// BOOST_AUTO_TEST_CASE(kill_module_test) {
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     const FilePath path = "./modules/test_module";
//     pid_t pid = launch(path, 0);
//     Module m = Module(pid, 0, time(0));
//     ModuleInfo modules = {{path, m}};

//     BOOST_REQUIRE(pid > 1);
//     sleep(1);
//     BOOST_REQUIRE(kill_module(path, &modules) != -1);
//     BOOST_REQUIRE(modules[path].killed);
//     BOOST_REQUIRE(modules[path].early_death_count == 0);
//     sleep(1);
//     // Check that child actually died
//     int status;
//     pid_t result = waitpid(pid, &status, WNOHANG);
//     BOOST_REQUIRE(result > 1);
// }

// BOOST_AUTO_TEST_CASE(launch_modules_in_test) {
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     printf("\n\nStarting launch_in\n\n");
//     const FilePath path = "./modules";
//     auto olist = files_in(path);
//     BOOST_REQUIRE(!olist.isEmpty());
//     auto module_files = olist.get();
//     LaunchInfo info = launch_modules_in(path, MSGKEY);
//     ModuleInfo modules = info.first;
//     BOOST_REQUIRE(module_files.size() == modules.size());
//     sleep(1);
//     for(std::pair<std::string, Module> m: modules) {
// 	auto file = std::find(module_files.begin(),
// 			      module_files.end(),
// 			      m.first);
// 	BOOST_REQUIRE(file != module_files.end());
// 	BOOST_REQUIRE(m.second.pid > 1);
// 	std::cout << "Killing module with pid " << m.second.pid << std::endl;
// 	BOOST_REQUIRE(kill(m.second.pid, SIGTERM) == 0);
//     }
//     printf("\n\nFinished launch_in\n\n");
// }

// BOOST_AUTO_TEST_CASE(reboot_module_test) {
//     printf("\n\nStarting reboot\n\n");
//     // octopOS &octopos = launch_octopOS();
//     // spin up octopos

//     // ----- HERE BE DRAGONS! DO NOT TOUCH! -----
//     BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
//     MemKey current_key = MSGKEY;

//     pthread_t subscriber_manager_thread;
//     BOOST_REQUIRE(pthread_create(&subscriber_manager_thread, NULL,
// 				 subscriber_manager::wait_for_data, NULL) == 0);
//     BOOST_REQUIRE(launch_octopOS_listener_for_child(0));
//     // try  making these publish string ptrs
//     publisher<OctoString> downgrade_pub(DOWNGRADE_TOPIC, current_key++);
//     printf("\n\nAfter\n\n");
//     subscriber<OctoString> downgrade_sub(DOWNGRADE_TOPIC, current_key - 1);
//     // ----------

//     const FilePath path = "./modules";
//     LaunchInfo info = launch_modules_in(path, current_key);
//     ModuleInfo modules = info.first;

//     // first reboot on a module that shouldn't be downgraded because
//     // it hasn't died enough times
//     const FilePath module1 = path + "/test_module";
//     Module &m1 = modules[module1];
//     pid_t oldpid = m1.pid;
//     sleep(1);
//     std::cout << "Killing pid " << m1.pid << std::endl;
//     BOOST_REQUIRE(kill(m1.pid, SIGTERM) == 0);
//     reboot_module(module1, &modules, downgrade_pub);
//     m1 = modules[module1]; // the Module has changed
//     std::cout << "Rebooted pid is " << m1.pid << std::endl;
//     BOOST_REQUIRE(m1.pid != oldpid);
//     BOOST_REQUIRE(m1.pid > 1);
//     BOOST_REQUIRE(!m1.killed);
//     BOOST_REQUIRE(!m1.downgrade_requested);
//     sleep(1);
//     BOOST_REQUIRE(!downgrade_sub.data_available());
//     BOOST_REQUIRE(m1.early_death_count == 1);
//     std::cout << "Past first" << std::endl;

//     sleep(1);
//     // now reboot on a module that SHOULD be downgraded because it has
//     // died too many times
//     oldpid = m1.pid;
//     m1.early_death_count = 200;
//     std::cout << "Killing pid " << m1.pid << std::endl;
//     BOOST_REQUIRE(kill(m1.pid, SIGTERM) != -1);
//     sleep(1);
//     reboot_module(module1, &modules, downgrade_pub);
//     m1 = modules[module1]; // the Module has changed
//     std::cout << "Rebooted pid is " << m1.pid << std::endl;
//     // pid shouldn't have changed because we shouldn't have relaunched it
//     BOOST_REQUIRE(m1.pid == oldpid);
//     BOOST_REQUIRE(!m1.killed);
//     BOOST_REQUIRE(m1.downgrade_requested);
//     sleep(1);
//     BOOST_REQUIRE(downgrade_sub.data_available());
//     BOOST_REQUIRE(downgrade_sub.get_data().get() == module1);
//     BOOST_REQUIRE(m1.early_death_count == 0);

//     sleep(1);
//     // now reboot on a module that shouldn't be downgraded because it
//     // was killed intentionally
//     const FilePath module2 = path + "/test_module2";
//     Module &m2 = modules[module2];
//     oldpid = m2.pid;
//     std::cout << "Killing pid " << m2.pid << std::endl;
//     BOOST_REQUIRE(kill_module(module2, &modules) != -1);
//     BOOST_REQUIRE(m2.killed);
//     reboot_module(module2, &modules, downgrade_pub);
//     m2 = modules[module2]; // the Module has changed
//     std::cout << "Rebooted pid is " << m2.pid << std::endl;
//     BOOST_REQUIRE(m2.pid != oldpid);
//     BOOST_REQUIRE(m2.pid > 1);
//     BOOST_REQUIRE(!m2.downgrade_requested);
//     BOOST_REQUIRE(!downgrade_sub.data_available());
//     BOOST_REQUIRE(m2.early_death_count == 0);
//     sleep(2); // wait long enough for the child to run a bit
//     std::cout << "Killing pid " << m2.pid << std::endl;
//     BOOST_REQUIRE(kill(m2.pid, SIGTERM) != -1);
//     std::cout << "Done" << std::endl;
// }

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
    printf("\n\nAfter\n\n");
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

// BOOST_AUTO_TEST_SUITE_END()
