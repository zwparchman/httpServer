OFLAGS = 
CC=gcc
STD=-std=c99
CFLAGS= -g -c -W -Wall -Wextra $(STD) -Wno-missing-field-initializers -Wshadow \
				$(OFLAGS)
LFLAGS= -g $(STD) $(OFLAGS) -lpthread

.PHONY:clean

Objects= server.o utility.o test.o statusCodes.o types.o

all : $(Objects) server tags

tags : *c *h
	ctags *[ch]
server : $(Objects)
	$(CC) $(Std) $(Objects) $(LFLAGS)  -o server 

$(Objects): %.o: %.c 
	$(CC) $(CFLAGS) $<

dbgs: server
	gdb server

dbgc: client
	gdb client

run: server
	./server

time: program
	time ./program

cache: program
	rm c*grind* -f
	valgrind --tool=cachegrind ./program

call: program
	rm c*grind* -f
	valgrind --tool=callgrind ./program

inspect: 
	kcachegrind c*grind\.out\.*

clean:
	rm -f *o 
	rm -f program
	rm -f c*grind\.out\.*
	rm -f file file2 
	rm -f server client
