SRCDIRS=Utility Utility/Serialization Utility/HqNx Platform LinuxPlatform Machine BaseApplication BaseApplication/BaseApplication BaseApplication/OpenGLRenderer Gameboy wxApplication

CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -O2 -Wall -std=c++0x $(foreach srcdir, $(SRCDIRS), -I $(srcdir)) $(shell wx-config --cflags)
LDFLAGS=-g
LDLIBS=-lGL $(shell wx-config --libs --gl-libs)

SRCS=$(foreach srcdir, $(SRCDIRS), $(wildcard $(srcdir)/*.cpp)) 
OBJS=$(subst .cpp,.o,$(SRCS))

all: emunisce

emunisce: $(OBJS)
	g++ $(LDFLAGS) -o Emunisce $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend