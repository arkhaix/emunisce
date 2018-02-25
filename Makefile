SRCDIRS=Utility/Serialization Utility/HqNx LinuxPlatform Machine BaseApplication/BaseApplication BaseApplication/OpenGLRenderer Gameboy wxApplication
INCDIRS=Utility Platform BaseApplication
LIBDIRS=
IMPORTLIBS=GL
STATICLIBS=
OUTPUT=Emunisce

CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -O2 -Wall -std=c++11 $(foreach srcdir, $(SRCDIRS), -I $(srcdir)) $(foreach incdir, $(INCDIRS), -I $(incdir)) $(shell wx-config --cppflags)
LDFLAGS=-g
LDLIBS=$(foreach libdir, $(LIBDIRS), -L$(libdir)) $(foreach importlib, $(IMPORTLIBS), -l$(importlib)) $(shell wx-config --libs --gl-libs)

SRCS=$(foreach srcdir, $(SRCDIRS), $(wildcard $(srcdir)/*.cpp))
OBJS=$(subst .cpp,.o,$(SRCS))

all: emunisce

emunisce: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) $(OUTPUT)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend