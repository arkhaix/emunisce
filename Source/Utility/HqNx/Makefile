# Automated Makefile



CC = g++
CFLAGS = -Wall -O2 -std=c++0x -I../../WindowsPlatform/ -I../../Machine/

AR = ar
ARFLAGS = rvs

COMPILE = $(CC) $(CFLAGS) -c

OBJFILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))



all: myprog



myprog: $(OBJFILES)

	#$(CC) -o myprog $(OBJFILES)
	$(AR) $(ARFLAGS) myprog.a $(OBJFILES)



%.o: %.cpp

	$(COMPILE) -o $@ $<
