/*
Name:
	Final Log

Description:
	Simple text file and debug out logging.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_LOG_H
#define FINAL_LOG_H

#include <final_platform_layer.h>

typedef enum {
	LogLevel_Fatal = 0,
	LogLevel_Error,
	LogLevel_Warning,
	LogLevel_Info,
	LogLevel_Debug,
	LogLevel_Min = LogLevel_Fatal,
	LogLevel_Max = LogLevel_Debug,
} LogLevel;

fpl_extern bool LogInit(const char *logFilePath);
fpl_extern void LogShutdown();

fpl_extern void LogWriteRaw(const char *format, ...);
fpl_extern void LogWriteLineBreak();
fpl_extern void LogWrite(const LogLevel level, const char *format, ...);

#endif // FINAL_LOG_H

#if (defined(FINAL_LOG_IMPLEMENTATION) && !defined(FINAL_LOG_IMPLEMENTED)) || (FPL_IS_IDE)

#ifndef FINAL_LOG_IMPLEMENTED
#define FINAL_LOG_IMPLEMENTED
#endif

fpl_internal const char *gLogLevelNames[] = {
	"FATAL",
	"ERROR",
	" WARN",
	" INFO",
	"DEBUG",
};

typedef struct {
	// Handle to the active log file
	fplFileHandle fileHandle;
	// Indicates whether the log system has been initialized
	bool isInitialized;
} Log;

typedef struct {
	// Starting pointer to the string data
	char *data;
	// Length of the string without the zero-terminator
	size_t length;
	// Total length of the allocated string, including the zero-terminator
	size_t allocated;
	// Align to 16 bytes
	int unused;
} LogString;

fpl_internal Log gLog = {0};

static char gLogWriteData[2048] = {0};
static LogString gLogWriteString = {
	gLogWriteData,
	sizeof(gLogWriteData),
	0,
	0,
};

fpl_extern bool LogInit(const char *logFilePath) {
	if (fplGetStringLength(logFilePath) == 0) {
		return false;
	}

	if (gLog.isInitialized) {
		return false;
	}

	fplClearStruct(&gLog);

	bool success = false;
	if (fplFileExists(logFilePath))
		success = fplFileAppendBinary(logFilePath, &gLog.fileHandle);
	else
		success = fplFileCreateBinary(logFilePath, &gLog.fileHandle);

	gLog.isInitialized = success;

	return success;
}

fpl_extern void LogShutdown() {
	if (!gLog.isInitialized) {
		return;
	}
	if (gLog.fileHandle.isValid) {
		fplFileClose(&gLog.fileHandle);
	}
	fplClearStruct(&gLog);
}

fpl_extern void LogWriteRaw(const char *format, ...) {
	if (!gLog.isInitialized) {
		return;
	}

	if (format == NULL || fplGetStringLength(format) == 0) {
		gLogWriteString.length = 1;
		gLogWriteString.data[0] = '\n';
		gLogWriteString.data[1] = '\0';
		fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);
		fplFileFlush(&gLog.fileHandle);
		return;
	}

	va_list argList;
	va_start(argList, format);
	size_t len = fplStringFormatArgs(gLogWriteString.data, gLogWriteString.allocated - 1, format, argList);
	va_end(argList);

	gLogWriteString.length = len + 1;
	gLogWriteString.data[len] = '\n';
	gLogWriteString.data[len + 1] = '\0';

	fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);
	fplFileFlush(&gLog.fileHandle);
}

fpl_extern void LogWriteLineBreak() {
	LogWriteRaw(fpl_null);
}

fpl_extern void LogWrite(const LogLevel level, const char *format, ...) {
	if (!gLog.isInitialized || level < LogLevel_Min || level > LogLevel_Max) {
		return;
	}

	if (fplGetStringLength(format) == 0) {
		gLogWriteString.length = 1;
		gLogWriteString.data[0] = '\n';
		gLogWriteString.data[1] = '\0';
	} else {
		const char *levelName = gLogLevelNames[level];
		fplDateTime utcDate = fplDateTimeQuery(fplDateTimeType_UTC);
		fplDateTimeResult utcDateRes = fplFormatDateTime(utcDate, fplDateTimeType_UTC);

		size_t len;
		char *ptr = gLogWriteString.data;

		gLogWriteString.length = 0;

		len = fplStringFormat(ptr, gLogWriteString.allocated, "%04u-%02u-%02u %02u:%02u:%02u.%03u [%s]: ", utcDateRes.year, utcDateRes.month, utcDateRes.day, utcDateRes.hour, utcDateRes.minute, utcDateRes.second, utcDateRes.millisecond, levelName);
		gLogWriteString.length += len;

		va_list argList;
		va_start(argList, format);
		len = fplStringFormatArgs(gLogWriteString.data + gLogWriteString.length, gLogWriteString.allocated - 1 - gLogWriteString.length, format, argList);
		va_end(argList);

		gLogWriteString.length += len + 1;

		gLogWriteString.data[gLogWriteString.length - 1] = '\n';
		gLogWriteString.data[gLogWriteString.length] = '\0';
	}

	fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);

	fplFileFlush(&gLog.fileHandle);
}

#endif // FINAL_LOG_IMPLEMENTATION