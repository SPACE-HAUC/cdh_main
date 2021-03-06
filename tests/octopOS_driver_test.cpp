// Copyright 2017 Space HAUC Command and Data Handling
// This file is part of Space HAUC which is released under AGPLv3.
// See file LICENSE.txt or go to <http://www.gnu.org/licenses/> for full
// license details.

/*!
 * @file
 *
 * @brief Test for OctopOS driver
 * These tests are in seperate files to avoid strange boost scoping.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE octopOS_driver
// Child deaths are not an error
#define BOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE
#define BOOST_TEST_IGNORE_SIGCHLD
#include <boost/test/unit_test.hpp>
#include <utility>
#include <string>

#include "../include/Optional.hpp"
#include "../include/octopOS_driver.hpp"
#include "../include/octopos.h"
#include "../include/subscriber.h"
#include "../include/publisher.h"

BOOST_AUTO_TEST_CASE(optional_test) {
    auto o = Just(5);
    auto n = None<int>();
    BOOST_REQUIRE(!o.isEmpty());
    BOOST_REQUIRE(n.isEmpty());
    BOOST_REQUIRE_NO_THROW(o.get());
    BOOST_REQUIRE_THROW(n.get(), std::runtime_error);
    BOOST_REQUIRE(o.get() == 5);
    BOOST_REQUIRE(o.getDefault(-1) == 5);
    BOOST_REQUIRE(n.getDefault(-1) == -1);
}

BOOST_AUTO_TEST_CASE(accessible_test) {
    BOOST_REQUIRE(accessible("./test.json"));
    BOOST_REQUIRE(!accessible("./thisfiledoesntexist!.88"));
}

BOOST_AUTO_TEST_CASE(load_test) {
    Optional<json> oj = load("./thisfiledoesntexist!.88");
    BOOST_REQUIRE(oj.isEmpty());

    Optional<json> oj2 = load("./test.json");
    BOOST_REQUIRE(!oj2.isEmpty());
    json j = oj2.get();
    BOOST_REQUIRE(j["a"] == "something");
    BOOST_REQUIRE(j["b"] == "else");
}

BOOST_AUTO_TEST_CASE(launch_test) {
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    pid_t pid = launch("./modules/test_module", 0);
    BOOST_REQUIRE(pid > 1);
    sleep(1);
    BOOST_REQUIRE(kill(pid, SIGTERM) == 0);
}

BOOST_AUTO_TEST_CASE(launch_octopOS_listeners_test) {
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    BOOST_REQUIRE(launch_octopOS_listeners());
}

BOOST_AUTO_TEST_CASE(files_in_test) {
    auto olist = files_in("../tests");
    BOOST_REQUIRE(!olist.isEmpty());
    auto list = olist.get();
    BOOST_REQUIRE(list.size() >= 1);
    // list should return full paths - not just filename
    std::string path = "../tests/";
    for (FilePath f : list) {
      BOOST_REQUIRE(f.compare(0, path.size(), path) == 0);
    }
}

BOOST_AUTO_TEST_CASE(module_needs_downgrade_test) {
    Module m1(111, 1, 1);  // launched a looooong time ago
    BOOST_REQUIRE(!module_needs_downgrade(&m1));
    Module m2(111, 1, time(0));  // launched just now
    m2.early_death_count = 10;  // and died early 10 times in a row
    BOOST_REQUIRE(module_needs_downgrade(&m2));
}

BOOST_AUTO_TEST_CASE(find_module_with_test) {
    ModuleInfo modules = {
        {"a", Module(111, 1, 1)},
        {"b", Module(222, 2, 1)},
        {"c", Module(333, 3, 1)}
    };
    auto empty = find_module_with(5, modules);
    BOOST_REQUIRE(empty.isEmpty());
    auto oname = find_module_with(222, modules);
    BOOST_REQUIRE(!oname.isEmpty());
    BOOST_REQUIRE(oname.get() == "b");
    auto oname2 = find_module_with(111, modules);
    BOOST_REQUIRE(!oname2.isEmpty());
    BOOST_REQUIRE(oname2.get() == "a");
}

BOOST_AUTO_TEST_CASE(relaunch_test) {
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    Module m(-1, 0, time(0));
    relaunch(&m, "./modules/test_module");
    BOOST_REQUIRE(m.pid > 1);
    BOOST_REQUIRE(!m.killed);
    BOOST_REQUIRE(!m.downgrade_requested);
    BOOST_REQUIRE(time(0) - m.launch_time < 2000);
    sleep(1);
    BOOST_REQUIRE(kill(m.pid, SIGTERM) == 0);
}

// Problem: boost catches sigchld and makes it fail the test case
// apparently no way to disable without editing source of boost?
BOOST_AUTO_TEST_CASE(kill_module_test) {
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    const FilePath path = "./modules/test_module";
    pid_t pid = launch(path, 0);
    Module m = Module(pid, 0, time(0));
    ModuleInfo modules = {{path, m}};

    BOOST_REQUIRE(pid > 1);
    sleep(1);
    BOOST_REQUIRE(kill_module(path, &modules) != -1);
    BOOST_REQUIRE(modules[path].killed);
    BOOST_REQUIRE(modules[path].early_death_count == 0);
    sleep(1);
    // Check that child actually died
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    BOOST_REQUIRE(result > 1);
}

BOOST_AUTO_TEST_CASE(launch_modules_in_test) {
    BOOST_REQUIRE_NO_THROW(octopOS::getInstance());
    const FilePath path = "./modules";
    auto olist = files_in(path);
    BOOST_REQUIRE(!olist.isEmpty());
    auto module_files = olist.get();
    LaunchInfo info = launch_modules_in(path, MSGKEY);
    ModuleInfo modules = info.first;
    BOOST_REQUIRE(module_files.size() == modules.size());
    sleep(1);
    for (std::pair<std::string, Module> m : modules) {
        auto file = std::find(module_files.begin(),
                              module_files.end(),
                              m.first);
        BOOST_REQUIRE(file != module_files.end());
        BOOST_REQUIRE(m.second.pid > 1);
        BOOST_REQUIRE(kill(m.second.pid, SIGTERM) == 0);
    }
}
