
CFLAGS=-I/sw/include 
LDFLAGS=-L/sw/lib -lgsl -lgslcblas -lm -ggdb 

CFLAGS+= -Wno-long-double -Wall -W -Wmissing-prototypes -Wconversion 
CFLAGS+= -Wunreachable-code 
#CFLAGS+= -pedantic


all: gpc_test

gpc_test: gpc.o

gpc.c: gpc.h


clean: 
	-rm gpc.o gpc_test
	
test: gpc_test
	./gpc_test