CXXFLAGS = -Wall -std=c++17 -pthread
LDFLAGS += -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects
#CXX = arm-linux-gnueabihf-c++
export PATH

floppy: floppy.o music.o
