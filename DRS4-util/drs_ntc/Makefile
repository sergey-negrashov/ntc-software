CFLAGS        = -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -Iinclude -I/usr/local/include -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10
LIBS          = -lpthread -lutil -lusb-1.0

CPP_OBJ       = DRS.o averager.o
OBJECTS       = musbstd.o mxml.o strlcpy.o



drs_ntc: $(OBJECTS) DRS.o averager.o drs_ntc.o
	$(CXX) $(CFLAGS) $(OBJECTS) DRS.o averager.o drs_ntc.o -o drs_ntc $(LIBS) 


drs_ntc.o: src/drs_ntc.cpp include/mxml.h include/DRS.h
	$(CXX) $(CFLAGS) -c $<


$(CPP_OBJ): %.o: src/%.cpp include/%.h include/mxml.h include/DRS.h
	$(CXX) $(CFLAGS)  -c $<

$(OBJECTS): %.o: src/%.c include/mxml.h include/DRS.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o drs_ntc

install:
	cp drs_ntc /usr/local/bin
