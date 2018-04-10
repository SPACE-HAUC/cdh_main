test: octopOS_driver_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	../include/*.h*
	g++ -g -rdynamic -std=c++11 octopOS_driver_test.cpp ../src/octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	-o test -lboost_unit_test_framework -lpthread

runtest: test
	./test --catch_system_errors=no

clean:
	rm -f ./test
