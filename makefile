CXX = g++

FILE = main
LIBRARIES = -pthread
LINK = -l:libassimp.so.6 -lncurses
LINKDIR = ./src

CPLUS_INCLUDE_PATH = ./include

all: $(FILE).cpp
	$(CXX) -g $(LIBRARIES) $(FILE).cpp -I$(CPLUS_INCLUDE_PATH) -L $(LINKDIR) $(LINK) -o $(FILE)