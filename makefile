#sudo ifconfig can0 down
#sudo ifconfig can0 up &
#gcc -I. -I.. -g -O2 -MT readViaCAN.o -MD -MP -c -o readViaCAN.o readViaCAN.c
#gcc -g -O2 -o readViaCAN readViaCAN.o   -L../canopen ../libcanopen/canopen/.libs/libcanopen.so
#./readViaCAN can0 0x02 werte
#./readViaCAN can0 0x02 bezeichner

#./readViaCAN can0 0x01 werte
#./readViaCAN can0 0x01 bezeichner

#gcc -I. -I.. -g -O2 -MT scancanbus.o -std=gnu99 -MD -MP -c -o scancanbus.o scancanbus.c
#gcc -g -O2 -o scancanbus scancanbus.o   -L../canopen ../libcanopen/canopen/.libs/libcanopen.so

CC=gcc
CPP=g++
CFLAGS=-c -g -O2 -MD -MP 
LDFLAGS=-L /usr/local/lib -L /usr/lib 
CXXLIBS= -lcanopen -lboost_thread -lpthread -lboost_system
CXXFLAGS=-Weffc++
CPPFLAGS=-I/usr/include
INC=-I ./mw
LIBS=-lcanopen
SOURCES=scancanbus.cpp readViaCAN.cpp uvr_can.cpp
OBJECTS=$(SOURCES:.c=.o)


all: $(SOURCES) readViaCAN scancanbus 

.c.o:
	$(CC) $(INC) $(CFLAGS) $< -o $@ $(CXXLIBS)

.cpp.o:
	$(CPP) $(INC) $(CFLAGS) $< -o $@ $(CXXLIBS)

readViaCAN: readViaCAN.o uvr_can.o
	$(CPP) $(INC) $(LDFLAGS) readViaCAN.o uvr_can.o -o $@ $(CXXLIBS) 

scancanbus: scancanbus.o uvr_can.o
	$(CPP) $(INC) $(LDFLAGS) scancanbus.o uvr_can.o -o $@ $(CXXLIBS)

clean:
	$(RM) *.o *~ scancanbus readViaCAN
