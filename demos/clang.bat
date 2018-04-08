@echo off

if [%1]==[] (
	echo Missing application name argument!
	GOTO :usage
)
set APP_NAME=%1
shift

if [%1]==[] (
	echo Missing build type argument!
	GOTO :usage
)
set "IS_VALID_BUILD_TYPE="
if %1 == Debug set IS_VALID_BUILD_TYPE=1
if %1 == Release set IS_VALID_BUILD_TYPE=1
if not defined IS_VALID_BUILD_TYPE (
	echo Unsupported build type '%1'!
	GOTO :usage
)
set BUILD_TYPE=%1
shift

if [%1]==[] (
	echo Missing additional libs argument!
	GOTO :usage
)
set ADDITIONAL_LIBS=%1
shift

if [%1]==[] (
	echo Missing additional include paths argument!
	GOTO :usage
)
set ADDITIONAL_INCLUDE_PATHS=%1
shift

if [%1]==[] (
	echo Missing translation units argument!
	GOTO :usage
)
set TRANSLATION_UNITS=%1
shift

set PLATFORM_NAME=Windows
set ARCH_NAME=x64
set BUILD_DIR=..\bin\%APP_NAME%\%PLATFORM_NAME%-%ARCH_NAME%-%BUILD_TYPE%

if %BUILD_TYPE%=="Debug" (
	set BUILD_TYPE_DEFINE=FPL_DEBUG
) else (
	set BUILD_TYPE_DEFINE=FPL_RELEASE
)

set STD_STANDARD=c++11
set IGNORED_WARNINGS=-Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-reserved-id-macro -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -Wno-unused-variable -Wno-unused-function -Wno-covered-switch-default
rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
echo Building %APP_NAME%/%PLATFORM_NAME%-%ARCH_NAME%-%BUILD_TYPE%
clang -std=%STD_STANDARD% -g -Weverything %IGNORED_WARNINGS% -D%BUILD_TYPE_DEFINE% -O0 -I..\..\ %ADDITIONAL_INCLUDE_PATHS% -lkernel32.lib %ADDITIONAL_LIBS% -o%BUILD_DIR%\%APP_NAME%.exe %TRANSLATION_UNITS% > %BUILD_DIR%\clang_error.txt 2>&1
goto done

:usage
echo Usage:
echo clang.bat [AppName] [Debug/Release]

:done:
rem Done!