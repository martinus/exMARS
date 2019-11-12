# default flags
CCFLAGS = -O3 -fomit-frame-pointer

# my own highly tuned settings
#CC      = gcc-3.3.1 
#CCFLAGS = -O3 -fomit-frame-pointer -fforce-addr -funroll-all-loops -fstrict-aliasing -malign-double -fprefetch-loop-arrays -maccumulate-outgoing-args -minline-all-stringops -finline-functions -finline-limit=800 -ffast-math -mno-align-stringops
#CCFLAGS += -mfpmath=sse,387 -mmmx -msse -march=pentium3

# develomental flags
#CCFLAGS += -fprofile-arcs -ftest-coverage
#CCFLAGS = -O0 -ansi -pedantic -Wall -W -Wno-div-by-zero -Wsystem-headers -Wfloat-equal -Wundef -Wendif-labels -Wshadow -Wbad-function-cast -Wcast-qual -Wsign-compare -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Winline -Wlong-long

exmars:
	$(CC) $(CCFLAGS) -c pmars.c 
	$(CC) $(CCFLAGS) -c pspace.c 
	$(CC) $(CCFLAGS) -c rubyinterface.c 
	$(CC) $(CCFLAGS) -c sim.c 
	$(CC) $(CCFLAGS) *.o -o exmars

clean:
	rm -f exmars Mars.so *.o *~ 

distclean: clean
	rm -f *.da