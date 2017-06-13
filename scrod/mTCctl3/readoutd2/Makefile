SOURCES =src/udpmtc.cpp src/settings.cpp src/udplistner.cpp src/registerinterface.cpp src/trigger.cpp src/storage.cpp src/monitor.cpp
LIBS = -lboost_system -lboost_filesystem -lboost_thread -lnewt -pthread -lmtc-mpod -lmtc-pip -lmtc-flow -lsnmp -lcurl
OBJECTS = udpmtc.o settings.o udplistner.o registerinterface.o trigger.o storage.o monitor.o
CPPFLAGS = -I. -std=c++11
%.o : src/%.cpp
	g++ -O2 -c $(CPPFLAGS) $< -o $@
all: ${OBJECTS}
	g++ $(OBJECTS)  $(LIBS)  -O2  -o readoutd2
clean:
	rm readoutd2 -f $(OBJECTS) sender shell scrod
sender: test/udpsender.cpp
	g++ test/udpsender.cpp -o sender -lmtc-data -lboost_filesystem -lboost_system
shell: test/udpshell.cpp
	g++ test/udpshell.cpp -I. -o shell -lreadline -lboost_filesystem -lboost_system
scrod: test/udpscrod.cpp
	g++ test/udpscrod.cpp -I. -o scrod -lreadline -lboost_filesystem -lboost_system

install_user:
	mkdir -p /usr/local/include/mtc/readoutd2
	cp lib/readoutProtocol.hpp /usr/local/include/mtc/readoutd2

install: all install_user
	cp readoutd2 /usr/local/bin
	mkdir -p /usr/local/etc/mtc/
	cp -n settings.set /usr/local/etc/mtc/
