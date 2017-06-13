NAMES=readoutd-net
FILES= src/commandline.cpp src/main.cpp src/storage.cpp
OBJECTS	= $(FILES:src/%.cpp=%.o)

COMPILE_FLAGS=-D_FILE_OFFSET_BITS=64  -O2 
LIBS= -lmtc-readoutd-net2 -lboost_thread -lboost_date_time -lreadline -lboost_filesystem -lmtc-data  -lnewt -lboost_system
LINK_FLAGS= $(CORELIB_LIBS)
CPP=g++
MAKE=make

all: $(NAMES)

%.o: src/%.cpp
	$(CPP) $(COMPILE_FLAGS) -c -o $@  $< 


readoutd-net:  $(OBJECTS)
	g++ $(LINK_FLAGS) $(COMPILE_FLAGS) $(OBJECTS) $(LIBS) -o readoutd-net

install: $(NAMES)
	cp $(NAMES) /usr/local/bin

.PHONY	: clean open

clean:
	rm -rf $(OBJECTS) $(NAMES)
