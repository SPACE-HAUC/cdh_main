main: Main.cpp octopOS_driver.cpp ../include/*.h* \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp
	g++ -g -rdynamic -std=c++11 Main.cpp octopOS_driver.cpp \
	../../OctopOS/src/octopos.cpp ../../OctopOS/src/subscriber.cpp \
	../../OctopOS/src/tentacle.cpp ../../OctopOS/src/utility.cpp \
	-o octopOS_main -lpthread

clean:
	rm -f ./octopOS_main
