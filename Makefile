LIBS=-lpython21
LDFLAGS= -s --entry _DllMain@12 --target=i386-mingw32
PYTHONLIB=/Python/DLLs 
CFLAGS= -fnative-struct

DLLWRAP=dllwrap
DLLTOOL=dlltool


.SUFFIXES: .o

all:  _WConio.pyd 


# Begin _WConio.pyd 

_WConio.pyd: WConiomodule.o _WConio.def
	$(DLLWRAP) --dllname $(notdir $@) --driver-name gcc --def _WConio.def -o $@ $(filter %.o,$^) $(LDFLAGS)  -L/mingw32/lib $(LIBS)

WConiomodule.o: WConiomodule.c
	gcc -O2     -I/Python/include -c WConiomodule.c -o WConiomodule.o


# End _WConio.pyd
 

install:  _WConio.pyd
	cp -f $^ $(PYTHONLIB)

clean:
	rm -f  _WConio.pyd  WConiomodule.o
