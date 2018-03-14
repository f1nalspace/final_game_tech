set BUILD_DIR=bin\FPL_Console\Windows-x64-Debug
set IGNORED_WARNINGS=-Wno-missing-field-initializers -Wno-sign-conversion -Wno-cast-qual -Wno-unused-parameter -Wno-format-nonliteral -Wno-old-style-cast -Wno-header-hygiene -Wno-c++98-compat -Wno-c++98-compat-pedantic
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
clang -std=c++11 -g -Weverything %IGNORED_WARNINGS% -DFPL_DEBUG -O0 -I..\ -lkernel32.lib -o%BUILD_DIR%\FPL_Console.exe FPL_Console\fpl_console.cpp > %BUILD_DIR%\clang_error.txt 2>&1