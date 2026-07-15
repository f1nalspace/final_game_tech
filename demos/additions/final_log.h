/*
Name:
	Final Log

Description:
	Simple text file and debug out logging.

	This file is part of the final_framework.

Changelog:
	## 2026-07-15
	- Added mutex to Log to make it thread-safe and removed global storages

License:
	MIT License
	Copyright 2017-2026 Torsten Spaete
*/

#ifndef FINAL_LOG_H
#define FINAL_LOG_H

#include <final_platform_layer.h>

#include <final_memory.h>

typedef enum LogLevel {
	LogLevel_Fatal = 0,
	LogLevel_Error,
	LogLevel_Warning,
	LogLevel_Info,
	LogLevel_Verbose,
	LogLevel_Debug,
	LogLevel_Trace,
	LogLevel_Min = LogLevel_Fatal,
	LogLevel_Max = LogLevel_Trace,
} LogLevel;

typedef struct LogString {
	// Starting pointer to the string data
	char *data;
	// Length of the string without the zero-terminator
	size_t length;
	// Total length of the allocated string, including the zero-terminator
	size_t allocated;
	// Align to 16 bytes
	int unused;
} LogString;

typedef struct Log {
	// Memory block for string buffer
	fmemMemoryBlock memory;
	// Handle to the active log file
	fplFileHandle fileHandle;
	// Synchronization primitive for thread safety
	fplMutexHandle mutex;
	// Indicates whether the log system has been initialized
	volatile bool isInitialized;
} Log;

fpl_extern bool LogInit(const char *logFilePath);
fpl_extern void LogShutdown();

fpl_extern void LogWriteRaw(const char *format, ...);
fpl_extern void LogWriteLineBreak();
fpl_extern void LogWrite(const LogLevel level, const char *format, ...);
fpl_extern void LogWriteArgs(const LogLevel level, const char *format, va_list argList);

#endif // FINAL_LOG_H

#if (defined(FINAL_LOG_IMPLEMENTATION) && !defined(FINAL_LOG_IMPLEMENTED)) || (FPL_IS_IDE)

#ifndef FINAL_LOG_IMPLEMENTED
#define FINAL_LOG_IMPLEMENTED
#endif

fpl_internal const char *gLogLevelNames[] = {
	"  FATAL",
	"  ERROR",
	"   WARN",
	"   INFO",
	"VERBOSE",
	"  DEBUG",
	"  TRACE",
};

static Log gLog = fplZeroInit;

fpl_extern bool LogInit(const char *logFilePath) {
	if (logFilePath == fpl_null || fplGetStringLength(logFilePath) == 0) {
		return false;
	}

	if (!fplIsPlatformInitialized()) {
		return false;
	}

	if (gLog.isInitialized) {
		return false;
	}

	fplClearStruct(&gLog);

	if (!fplMutexInit(&gLog.mutex)) {
		return false;
	}

	fmemMemoryBlock *mem = &gLog.memory;

	if (!fmemInit(mem, fmemType_Growable, fplMegaBytes(16), 0)) {
		fplMutexDestroy(&gLog.mutex);
		return false;
	}

	bool success = fplFileAppendBinary(logFilePath, &gLog.fileHandle);
	if (!success) {
		fmemFree(mem);
		fplMutexDestroy(&gLog.mutex);
		return false;
	}

	gLog.isInitialized = true;

	return true;
}

fpl_extern void LogShutdown() {
	if (!gLog.isInitialized) {
		return;
	}

	fplMutexLock(&gLog.mutex);

	if (gLog.fileHandle.isValid) {
		fplFileClose(&gLog.fileHandle);
	}
	fmemFree(&gLog.memory);

	fplMutexUnlock(&gLog.mutex);
	fplMutexDestroy(&gLog.mutex);

	fplClearStruct(&gLog);
}

static void LogWriteNewlineUnsafe(void) {
	char buf[2];
	buf[0] = '\n';
	buf[1] = '\0';
	fplFileWriteBlock(&gLog.fileHandle, buf, 1);
	fplFileFlush(&gLog.fileHandle);
}

fpl_extern void LogWriteRaw(const char *format, ...) {
	if (!gLog.isInitialized) {
		return;
	}

	fplMutexLock(&gLog.mutex);

	if (format == fpl_null || fplGetStringLength(format) == 0) {
		LogWriteNewlineUnsafe();
		fplMutexUnlock(&gLog.mutex);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	if (!fmemBeginTemporary(&gLog.memory, &temporaryMemory)) {
		fplMutexUnlock(&gLog.mutex);
		return;
	}

	LogString tempString = fplZeroInit;
	tempString.allocated = temporaryMemory.size;
	tempString.length = 0;
	tempString.data = (char *)temporaryMemory.base;

	va_list argList;
	va_start(argList, format);
	size_t len = fplStringFormatArgs(tempString.data, tempString.allocated - 1, format, argList);
	va_end(argList);

	tempString.length = len + 1;
	tempString.data[len] = '\n';
	tempString.data[len + 1] = '\0';

	fplFileWriteBlock(&gLog.fileHandle, tempString.data, tempString.length);
	fplFileFlush(&gLog.fileHandle);

	fmemEndTemporary(&temporaryMemory);

	fplMutexUnlock(&gLog.mutex);
}

fpl_extern void LogWriteLineBreak() {
	LogWriteRaw(fpl_null);
}

fpl_extern void LogWriteArgs(const LogLevel level, const char *format, va_list argList) {
	if (!gLog.isInitialized || level < LogLevel_Min || level > LogLevel_Max) {
		return;
	}

	fplMutexLock(&gLog.mutex);

	if (fplGetStringLength(format) == 0) {
		LogWriteNewlineUnsafe();
		fplMutexUnlock(&gLog.mutex);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	if (!fmemBeginTemporary(&gLog.memory, &temporaryMemory)) {
		fplMutexUnlock(&gLog.mutex);
		return;
	}

	LogString tempString = fplZeroInit;
	tempString.allocated = temporaryMemory.size;
	tempString.length = 0;
	tempString.data = (char *)temporaryMemory.base;

	const char *levelName = gLogLevelNames[level];
	fplDateTime utcDate = fplDateTimeQuery(fplDateTimeType_UTC);
	fplDateTimeResult utcDateRes = fplFormatDateTime(utcDate, fplDateTimeType_UTC);

	size_t len;
	char *ptr = tempString.data;
	*ptr = 0;

	tempString.length = 0;

	len = fplStringFormat(ptr, tempString.allocated, "%04u-%02u-%02u %02u:%02u:%02u.%03u [%s]: ", utcDateRes.year, utcDateRes.month, utcDateRes.day, utcDateRes.hour, utcDateRes.minute, utcDateRes.second, utcDateRes.millisecond, levelName);
	tempString.length += len;

	ptr += tempString.length;

	size_t remaining = tempString.allocated - tempString.length - 1;

	va_list tempArgList;
	va_copy(tempArgList, argList);
	len = fplStringFormatArgs(ptr, remaining, format, tempArgList);
	va_end(tempArgList);

	tempString.length += len + 1;

	tempString.data[tempString.length - 1] = '\n';
	tempString.data[tempString.length] = '\0';

	fplFileWriteBlock(&gLog.fileHandle, tempString.data, tempString.length);

	fplFileFlush(&gLog.fileHandle);

	fmemEndTemporary(&temporaryMemory);

	fplMutexUnlock(&gLog.mutex);
}

fpl_extern void LogWrite(const LogLevel level, const char *format, ...) {
	if (!gLog.isInitialized || level < LogLevel_Min || level > LogLevel_Max) {
		return;
	}

	va_list argList;
	va_start(argList, format);
	LogWriteArgs(level, format, argList);
	va_end(argList);
}

#endif // FINAL_LOG_IMPLEMENTATION