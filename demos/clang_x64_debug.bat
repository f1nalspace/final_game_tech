set BUILD_DIR=bin\FPL_Console\x64-Debug
set IGNORED_WARNINGS=-Wno-missing-field-initializers -Wno-sign-conversion -Wno-cast-qual -Wno-unused-parameter -Wno-format-nonliteral
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
clang -g -Weverything %IGNORED_WARNINGS% -O0 -I..\ -lkernel32.lib -o%BUILD_DIR%\FPL_Console.exe FPL_Console\main.cpp