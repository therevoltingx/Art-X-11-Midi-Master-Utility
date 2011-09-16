CC=g++
LINKER=g++
OBJS = x-11-mm.o parser.o rtmidi/RtMidi.o
TARGET = x-11-mm
LINKER-FLAGS = -o

#WINDOWS
#CXXFLAGS=-D__WINDOWS_MM__
#LDFLAGS=-lwinmm

#LINUX
CXXFLAGS=-D__LINUX_ALSASEQ__
LDFLAGS=-lpthread -lasound

#MAC OS X
#CXXFLAGS=-D__MACOSX_CORE__ 
#LDFLAGS=-framework CoreMidi -framework CoreAudio -framework CoreFoundation

INSTALL=install
BINDIR=/usr/bin/

#x-11-mm: x-11-mm.o parser.o rtmidi/RtMidi.o getoptp/getopt_pp.o
x-11-mm: $(OBJS)
	$(LINKER) $(LINKER-FLAGS) $(TARGET) $(OBJS) $(LDFLAGS) 

clean:
	rm -rf *.o rtmidi/*.o getopt_pp/*.o

install : x-11-mm 
	$(INSTALL) x-11-mm $(BINDIR)

