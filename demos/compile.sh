#!/bin/sh
mkdir -p bin/linux_x64
g++ FPL_Linux/main.cpp -std=c++11 -I../ -o bin/linux_x64/fpl_linux 2> logcompile.txt
g++ FPL_Console/main.cpp -std=c++11 -I../ -o bin/linux_x64/fpl_console 2> logcompile.txt
cat logcompile.txt
