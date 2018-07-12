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

typedef struct flogLogState {
	const char *filePath;
	bool isInitialized;
} flogLogState;

extern void flogInit(const char *filePath);
extern void flogWrite(const char *format, ...);

#endif // LOGGING_H

#if defined(FLOG_IMPLEMENTATION) && !defined(FLOG_IMPLEMENTED)
#define FLOG_IMPLEMENTED

static flogLogState flog__globalState = FLOG_INIT_STRUCT;
static char flog_formatBuffer[2048];

extern void flogInit(const char *filePath) {
	flogLogState *state = &flog__globalState;
	state->filePath = filePath;
	state->isInitialized = true;
}

static void flog__WriteString(FILE *file, const char *str) {
	if(*str) {
		const char *p = str;
		while(*p) { ++p; }
		size_t len = p - str;
		fwrite(str, len, 1, file);
	}
}

static char *flog__IntToStr(const uint32_t value, char *buffer, uint32_t leadingZeroCount) {
	buffer[0] = 0;
	uint32_t v = value;
	char *p = buffer;
	uint32_t tmp = v;
	do {
		++p;
		tmp = tmp / 10;
	} while(tmp);
	uint32_t digitCount = (int)(p - buffer);
	if(digitCount < leadingZeroCount) {
		int diff = leadingZeroCount - digitCount;
		p--;
		for(int i = 0; i < diff; ++i) {
			*p = '0';
			++p;
		}
		++p;
	}
	*p = 0;
	const char *digits = "0123456789";
	v = value;
	do {
		*--p = digits[v % 10];
		v /= 10;
	} while(v != 0);
	return (buffer);
}

static void flog__WriteLine(flogLogState *state, const char *line) {
	time_t now = time(NULL);
	struct tm nowTime = *localtime(&now);
	int year = 1900 + nowTime.tm_year;
	int month = nowTime.tm_mon;
	int day = nowTime.tm_mday;
	int hour = nowTime.tm_hour;
	int min = nowTime.tm_min;
	int sec = nowTime.tm_sec;
	FILE *file = fopen(state->filePath, "a+");
	if(file != NULL) {
		char intBuffer[8];
		flog__WriteString(file, "[");
		flog__WriteString(file, flog__IntToStr(year, intBuffer, 4));
		flog__WriteString(file, "-");
		flog__WriteString(file, flog__IntToStr(month, intBuffer, 2));
		flog__WriteString(file, "-");
		flog__WriteString(file, flog__IntToStr(day, intBuffer, 2));
		flog__WriteString(file, " ");
		flog__WriteString(file, flog__IntToStr(hour, intBuffer, 2));
		flog__WriteString(file, ":");
		flog__WriteString(file, flog__IntToStr(min, intBuffer, 2));
		flog__WriteString(file, ":");
		flog__WriteString(file, flog__IntToStr(sec, intBuffer, 2));
		flog__WriteString(file, "] ");
		flog__WriteString(file, line);
		flog__WriteString(file, "\n");
		fflush(file);
		fclose(file);
	}
}

extern void flogWrite(const char *format, ...) {
	if(!flog__globalState.isInitialized) {
		return;
	}
	size_t bufferSize = sizeof(flog_formatBuffer);
	va_list argList;
	va_start(argList, format);
	flog_formatBuffer[0] = 0;
	int count = vsnprintf(flog_formatBuffer, bufferSize, format, argList);
	va_end(argList);
	if(count > 0) {
		flog__WriteLine(&flog__globalState, flog_formatBuffer);
	}
}

#endif // FLOG_IMPLEMENTATION
