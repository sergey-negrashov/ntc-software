CORELIB_NAME=libmtc-data.so
CORELIB_FILES=src/datapacket.cpp src/index.cpp src/util.cpp src/memory.cpp src/dataoperations.cpp src/channelignoremask.cpp src/curve.cpp src/pmtmap.cpp src/scrodmap.cpp src/version.cpp src/pmtfacemap.cpp
CORELIB_OBJECTS	= $(CORELIB_FILES:src/%.cpp=%.o)

CORELIB_COMPILE_FLAGS=-D_FILE_OFFSET_BITS=64 -Wall -fPIC -O2  -Ilib/
CORELIB_LIBS= 
CORELIB_LINK_FLAGS= -shared -Wl,-soname,$(CORELIB_NAME) $(CORELIB_LIBS) -o $(CORELIB_NAME)
CPP=g++
MAKE=make

all: slib

%.o: src/%.cpp
	$(CPP) $(CORELIB_COMPILE_FLAGS) -c -o $@  $<

slib: $(CORELIB_OBJECTS)
	$(CPP) $(CORELIB_LINK_FLAGS) $(CORELIB_OBJECTS)


test: $(CORELIB_OBJECTS)
	$(CPP)  $(CORELIB_OBJECTS) tests.cpp -o dataTest
install: slib
	mkdir -p /usr/local/include/mtc/data
	cp lib/*.hpp /usr/local/include/mtc/data
	chmod a+r /usr/local/include/mtc/data/*
	cp $(CORELIB_NAME) /usr/local/lib/

.PHONY	: clean open cleandata

clean:
	rm -rf $(CORELIB_OBJECTS) $(CORELIB_NAME) dataTest

cleandata:
	rm -rf *.txt subtracted.dat

open:
	geany  src/*.cpp lib/mtc/data/*.hpp &
