NAMES=setreg.o getreg.o init.o cardinfo.o trg.o collector.o cal.o motor.o
LIBS= -lmtc-readoutd-net2  -lboost_date_time  -lboost_system -lboost_thread
LINK_FLAGS= $(LIBS)
CPP=g++
MAKE=make
COMPILE_FLAGS=-c -fPIC

all: libreadoutd_python.so

libreadoutd_python.so: $(NAMES)
	g++  -shared -Wl,-soname,libreadoutd_python2.so -o libreadoutd_python2.so  $(NAMES) $(LINK_FLAGS) 

setreg.o: src/setreg.cpp
	g++ $(COMPILE_FLAGS)  src/setreg.cpp -o setreg.o

getreg.o: src/getreg.cpp
	g++ $(COMPILE_FLAGS)  src/getreg.cpp -o getreg.o

init.o: src/init.cpp
	g++ $(COMPILE_FLAGS)  src/init.cpp -o init.o

cardinfo.o: src/cardInfo.cpp
	g++ $(COMPILE_FLAGS)  src/cardInfo.cpp -o cardinfo.o

trg.o: src/trg.cpp
	g++ $(COMPILE_FLAGS) src/trg.cpp -o trg.o

collector.o: src/collector.cpp
	g++ $(COMPILE_FLAGS) src/collector.cpp -o collector.o

cal.o: src/cal.cpp	
	g++ $(COMPILE_FLAGS) src/cal.cpp -o cal.o

motor.o: src/motor.o
	g++ $(COMPILE_FLAGS) src/motor.cpp -o motor.o

install: libreadoutd_python.so
	mkdir -p /usr/local/mtc/python
	cp libreadoutd_python2.so /usr/local/mtc/python

.PHONY	: clean

clean:
	rm -rf $(NAMES)
