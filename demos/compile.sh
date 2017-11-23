#!/bin/sh
mkdir -p bin/linux_x64
g++ FPL_Linux/main.cpp -std=c++11 -I../ -o bin/linux_x64/fpl_linux 2> compile_log.txt
cat compile_log.txt
