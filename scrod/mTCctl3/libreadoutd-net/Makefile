CORELIB_NAME=libmtc-readoutd-net2.so
CORELIB_FILES=src/scrodnet.cpp 
CORELIB_OBJECTS	= $(CORELIB_FILES:src/%.cpp=%.o)

CORELIB_COMPILE_FLAGS=-D_FILE_OFFSET_BITS=64 -Wall -fPIC -O2  -Ilib/
CORELIB_LIBS=  -lboost_thread -lmtc-data -lboost_system -lboost_filesystem
CORELIB_LINK_FLAGS= -shared -Wl,--no-as-needed,-soname,$(CORELIB_NAME) $(CORELIB_LIBS) -o $(CORELIB_NAME)
CPP=g++
MAKE=make

all: slib

%.o: src/%.cpp
	$(CPP) $(CORELIB_COMPILE_FLAGS) -c -o $@  $<

slib: $(CORELIB_OBJECTS)
	$(CPP) $(CORELIB_LINK_FLAGS) $(CORELIB_OBJECTS)


install: slib
	mkdir -p /usr/local/include/mtc/readoutd2
	cp lib/*.hpp /usr/local/include/mtc/readoutd2
	cp $(CORELIB_NAME) /usr/local/lib/

test: $(CORELIB_OBJECTS)
	 $(CPP) -O2 -o nettest main.cpp $(CORELIB_OBJECTS) $(CORELIB_LIBS)

.PHONY	: clean open

clean:
	rm -rf $(CORELIB_OBJECTS) $(CORELIB_NAME) nettest

open:$(CORELIB_OBJECTS)
	geany  src/*.cpp lib/*.hpp &
