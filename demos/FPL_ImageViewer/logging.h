#ifndef LOGGING_H
#define LOGGING_H

#include <stdbool.h> // bool
#include <stdarg.h> // va_list
#include <stdlib.h> // NULL
#include <stdio.h> // FILE, fopen
#include <stdint.h> // uint32_t
#include <string.h> // strlen
#include <time.h> // time

#if defined(__cplusplus)
#define FLOG_INIT_STRUCT {}
#else
#define FLOG_INIT_STRUCT {0}
#endif

#define FLOG_MAX_FILE_PATH_LENGTH 4096
#define FLOG_MAX_LINE_LENGTH 2048

typedef struct flogLogState {
	char filePath[FLOG_MAX_FILE_PATH_LENGTH];
	bool isInitialized;
} flogLogState;

// Copies the path, so a temporary buffer is fine
extern void flogInit(const char *filePath);
// Thread-safe, each call appends exactly one line
extern void flogWrite(const char *format, ...);

#endif // LOGGING_H

#if defined(FLOG_IMPLEMENTATION) && !defined(FLOG_IMPLEMENTED)
#define FLOG_IMPLEMENTED

static flogLogState flog__globalState = FLOG_INIT_STRUCT;

extern void flogInit(const char *filePath) {
	flogLogState *state = &flog__globalState;
	size_t filePathLength = strlen(filePath);
	if(filePathLength >= sizeof(state->filePath)) {
		state->isInitialized = false;
		return;
	}
	memcpy(state->filePath, filePath, filePathLength + 1);
	state->isInitialized = true;
}

extern void flogWrite(const char *format, ...) {
	const flogLogState *state = &flog__globalState;
	if(!state->isInitialized) {
		return;
	}

	const int tmYearBase = 1900;
	const int tmMonthBase = 1;
	time_t now = time(NULL);
	struct tm nowTime;
#if defined(_WIN32)
	localtime_s(&nowTime, &now);
#else
	localtime_r(&now, &nowTime);
#endif
	int year = tmYearBase + nowTime.tm_year;
	int month = tmMonthBase + nowTime.tm_mon;

	// The whole line is built on the stack, so concurrent callers never share a buffer and each line is a single write
	char line[FLOG_MAX_LINE_LENGTH];
	int prefixLength = snprintf(line, sizeof(line), "[%04d-%02d-%02d %02d:%02d:%02d] ", year, month, nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
	if(prefixLength < 0 || prefixLength >= (int)sizeof(line)) {
		return;
	}

	const size_t newlineLength = 1;
	size_t messageCapacity = sizeof(line) - (size_t)prefixLength - newlineLength;
	va_list argList;
	va_start(argList, format);
	int messageLength = vsnprintf(line + prefixLength, messageCapacity, format, argList);
	va_end(argList);
	if(messageLength <= 0) {
		return;
	}

	// Too long messages are cut off
	size_t storedMessageLength = ((size_t)messageLength < messageCapacity) ? (size_t)messageLength : (messageCapacity - 1);
	size_t lineLength = (size_t)prefixLength + storedMessageLength;
	line[lineLength] = '\n';
	lineLength += newlineLength;

	FILE *file = fopen(state->filePath, "a");
	if(file != NULL) {
		fwrite(line, 1, lineLength, file);
		fclose(file);
	}
}

#endif // FLOG_IMPLEMENTATION
