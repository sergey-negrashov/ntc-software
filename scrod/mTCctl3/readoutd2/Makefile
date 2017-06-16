SOURCES =src/udpmtc.cpp src/settings.cpp src/udplistner.cpp src/registerinterface.cpp src/trigger.cpp src/storage.cpp src/monitor.cpp src/motor.cpp
LIBS = -lboost_system -lboost_filesystem -lboost_thread -lnewt -pthread 
OBJECTS = udpmtc.o settings.o udplistner.o registerinterface.o trigger.o storage.o monitor.o motor.o
CPPFLAGS = -I. -std=c++11
%.o : src/%.cpp
	g++ -O2 -c $(CPPFLAGS) $< -o $@
all: ${OBJECTS}
	g++ $(OBJECTS)  $(LIBS)  -O2  -o readoutd2
clean:
	rm readoutd2 -f $(OBJECTS) sender shell scrod

install_user:
	mkdir -p /usr/local/include/mtc/readoutd2
	cp lib/readoutProtocol.hpp /usr/local/include/mtc/readoutd2

install: all install_user
	cp readoutd2 /usr/local/bin
	mkdir -p /usr/local/etc/mtc/
	cp -n settings.set /usr/local/etc/mtc/
