OBJECTS=pbmd.o
CFLAGS=-g -O0 -Wall -I../
LIBS= -lphidget21 -lpthread -ldl

all: pbmd

pbmd:$(OBJECTS)

#%: %.c  
	$(CC)  -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	-rm -f *.o pbmd
