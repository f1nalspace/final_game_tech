set BUILD_DIR=build\release\x64
set IGNORED_WARNINGS=-Wno-missing-field-initializers -Wno-sign-conversion -Wno-cast-qual -Wno-unused-parameter -Wno-format-nonliteral
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
clang -Weverything %IGNORED_WARNINGS% -s -O2 -I..\ -lkernel32.lib -o%BUILD_DIR%\FPL_Console.exe FPL_Console\main.cpp