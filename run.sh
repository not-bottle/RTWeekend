#!/bin/bash

env LD_LIBRARY_PATH=./src ./main > test.ppm 2> >(tee error.txt >&2)