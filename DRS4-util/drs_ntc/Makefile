CFLAGS        = -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -Iinclude -I/usr/local/include -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10
LIBS          = -lpthread -lutil -lusb-1.0

CPP_OBJ       = DRS.o averager.o
OBJECTS       = musbstd.o mxml.o strlcpy.o


all: drs_ntc drs_dump_dt_calib

drs_ntc: $(OBJECTS) DRS.o averager.o drs_ntc.o
	$(CXX) $(CFLAGS) $(OBJECTS) DRS.o averager.o drs_ntc.o -o drs_ntc $(LIBS) 

drs_dump_dt_calib: $(OBJECTS) DRS.o drs_dump_dt_calib.o averager.o
	$(CXX) $(CFLAGS) $(OBJECTS) DRS.o averager.o drs_dump_dt_calib.o -o drs_dump_dt_calib $(LIBS) $(WXLIBS)

drs_ntc.o: src/drs_ntc.cpp include/mxml.h include/DRS.h
	$(CXX) $(CFLAGS) -c $<


drs_dump_dt_calib.o: src/drs_dump_dt_calib.cpp include/DRS.h
	$(CXX) $(CFLAGS) -c $<

$(CPP_OBJ): %.o: src/%.cpp include/%.h include/mxml.h include/DRS.h
	$(CXX) $(CFLAGS)  -c $<

$(OBJECTS): %.o: src/%.c include/mxml.h include/DRS.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o drs_ntc drs_dump_dt_calib

install:
	cp drs_ntc /usr/local/bin
