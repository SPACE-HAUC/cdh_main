#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE octopOS_driver
#include <boost/test/unit_test.hpp>

#include "../include/octopOS_driver.hpp"
#include "../include/octopos.h"

BOOST_AUTO_TEST_CASE(optional_test) {
    auto o = Optional<int>::Just(5);
    auto n = Optional<int>::None();
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
    pid_t pid = launch("echo", 1);
    BOOST_REQUIRE(pid > 1);
}

BOOST_AUTO_TEST_CASE(launch_listeners_test) {
    octopOS::getInstance();
    BOOST_REQUIRE(launch_listeners());
}

BOOST_AUTO_TEST_CASE(files_in_test) {
    auto olist = files_in("../tests");
    BOOST_REQUIRE(!olist.isEmpty());
    auto list = olist.get();
    BOOST_REQUIRE(list.size() >= 1);
    // list should return full paths - not just filename
    std::string path = "../tests/";
    BOOST_REQUIRE(list.front().compare(0, path.size(), path));
}

BOOST_AUTO_TEST_CASE(module_needs_downgrade_test) {
    Module m1(111, 1, 1); // launched a looooong time ago
    BOOST_REQUIRE(!module_needs_downgrade(m1));
    Module m2(111, 1, time(0)); // launched just now
    BOOST_REQUIRE(module_needs_downgrade(m2));
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

// launch, launch_modules_in, kill_module, downgrade, reboot_module
// all require mocking to test effectively
