#!/bin/bash

env LD_LIBRARY_PATH=./src gdb --args ./main 0 -spp4 2> error.txt
