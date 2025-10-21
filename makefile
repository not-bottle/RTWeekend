CXX = g++

FILE = main
LINK = -l:libassimp.so.6
LINKDIR = ./src

CPLUS_INCLUDE_PATH = ./include

all: $(FILE).cpp
	$(CXX) -g $(FILE).cpp -I$(CPLUS_INCLUDE_PATH) -L $(LINKDIR) $(LINK) -o $(FILE)