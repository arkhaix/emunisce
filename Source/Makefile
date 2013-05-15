SRCDIRS=Utility/Serialization Utility/HqNx LinuxPlatform Machine BaseApplication/BaseApplication BaseApplication/OpenGLRenderer Gameboy wxApplication
INCDIRS=Utility Platform BaseApplication ../Import/freetype/Export/x86/include
LIBDIRS=
IMPORTLIBS=GL
STATICLIBS=../Import/freetype/Export/x86/lib/freetype.a

CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -O2 -Wall -std=c++0x $(foreach srcdir, $(SRCDIRS), -I $(srcdir)) $(foreach incdir, $(INCDIRS), -I $(incdir)) $(shell wx-config --cflags)
LDFLAGS=-g
LDLIBS=$(foreach libdir, $(LIBDIRS), -L$(libdir)) $(foreach importlib, $(IMPORTLIBS), -l$(importlib)) $(shell wx-config --libs --gl-libs)

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