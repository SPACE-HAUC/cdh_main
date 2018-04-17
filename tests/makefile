all: octopos_driver_test babysit_test reboot_module_test
	echo "Done."

octopos_driver_test: octopOS_driver_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	../include/*.h*
	g++ -g -rdynamic -std=c++11 octopOS_driver_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	-o octopos_driver_test -lboost_unit_test_framework -lpthread

babysit_test: babysit_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	../include/*.h*
	g++ -g -rdynamic -std=c++11 babysit_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	-o babysit_test -lboost_unit_test_framework -lpthread


reboot_module_test: reboot_module_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	../include/*.h*
	g++ -g -rdynamic -std=c++11 reboot_module_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	-o reboot_module_test -lboost_unit_test_framework -lpthread

run: runtest
	printf "Done."

runtest: reboot_module_test babysit_test octopos_driver_test
	./run_tests.sh

clean:
	rm -f ./octopos_driver_test ./babysit_test ./reboot_module_test
