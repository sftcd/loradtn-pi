OBJECTS=pbmd.o handlers.o phidgets.o
CFLAGS=-g -O0 -Wall -I../
LIBS= -lphidget21 -lpthread -ldl

all: pbmd

phidgets.o : handlers.h
pbmd.o : phidgets.h

pbmd:$(OBJECTS)

#%: %.c  
	$(CC)  -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	-rm -f *.o pbmd
