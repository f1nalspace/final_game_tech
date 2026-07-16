/*
Name:
	Final Log

Description:
	Simple text file and debug out logging.

	This file is part of the final_framework.

Changelog:
	## 2026-07-16
	- Fixed severity filter dropping every line when the severity is LogLevel_All
	- Fixed level name lookup being off by one after the LogLevel_All shift, reading past the array for LogLevel_Trace
	- Changed mutex to an atomic spinlock, so the log is thread-safe without requiring an initialized platform
	- Added a category column to LogWrite/LogWriteArgs, written into the line prefix
	- Changed init signature to pass in the log level severity + LogSetSeverity/LogGetSeverity
	- Added LogLevel_All with value of zero, moving LogLevel_Fatal to 1

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
	LogLevel_All = 0,
	LogLevel_Fatal,
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
	// Max log level severity
	LogLevel severity;
	// Atomic spinlock guarding writes, so no mutex (and thus no initialized platform) is required for thread safety
	volatile uint32_t lock;
	// Indicates whether the log system has been initialized
	volatile uint32_t isInitialized;
} Log;

fpl_extern bool LogInit(const char *logFilePath, const LogLevel severity);
fpl_extern void LogShutdown();

fpl_extern LogLevel LogGetSeverity(void);
fpl_extern void LogSetSeverity(const LogLevel severity);

fpl_extern void LogWriteRaw(const char *format, ...);
fpl_extern void LogWriteLineBreak();
fpl_extern void LogWrite(const LogLevel level, const char *category, const char *format, ...);
fpl_extern void LogWriteArgs(const LogLevel level, const char *category, const char *format, va_list argList);

#endif // FINAL_LOG_H

#if (defined(FINAL_LOG_IMPLEMENTATION) && !defined(FINAL_LOG_IMPLEMENTED)) || (FPL_IS_IDE)

#ifndef FINAL_LOG_IMPLEMENTED
#define FINAL_LOG_IMPLEMENTED
#endif

// Indexed by LogLevel directly, so index 0 is a placeholder for LogLevel_All, which is never written as a line level
fpl_internal const char *gLogLevelNames[] = {
	"    ALL",
	"  FATAL",
	"  ERROR",
	"   WARN",
	"   INFO",
	"VERBOSE",
	"  DEBUG",
	"  TRACE",
};

// The level and category are right-aligned and both padded and truncated to these widths, so the message column always lines up.
enum { LogLevelColumnWidth = 7 };
enum { LogCategoryColumnWidth = 16 };

// Stand-in category for a line logged with none, so the column is never blank.
fpl_internal const char *gLogUnknownCategory = "Unknown";

static Log gLog = fplZeroInit;

// Simple atomic spinlock guarding the log. It needs no init or destroy and works even before the platform is initialized, unlike a mutex.
fpl_internal void LogAcquireLock(Log *log) {
	uint32_t previous = fplAtomicCompareAndSwapU32(&log->lock, 0, 1);
	while (previous != 0) {
		previous = fplAtomicCompareAndSwapU32(&log->lock, 0, 1);
	}
}

fpl_internal void LogReleaseLock(Log *log) {
	fplAtomicStoreU32(&log->lock, 0);
}

fpl_extern LogLevel LogGetSeverity(void) {
	if (!gLog.isInitialized) {
		return LogLevel_All;
	}
	return gLog.severity;
}

fpl_extern void LogSetSeverity(const LogLevel severity) {
	if (!gLog.isInitialized) {
		return;
	}
	gLog.severity = severity;
}

fpl_extern bool LogInit(const char *logFilePath, const LogLevel severity) {
	if (logFilePath == fpl_null || fplGetStringLength(logFilePath) == 0) {
		return false;
	}

	if (gLog.isInitialized) {
		return false;
	}

	fplClearStruct(&gLog);
	gLog.severity = severity;

	fmemMemoryBlock *mem = &gLog.memory;

	if (!fmemInit(mem, fmemType_Growable, fplMegaBytes(16), 0)) {
		return false;
	}

	bool success = fplFileAppendBinary(logFilePath, &gLog.fileHandle);
	if (!success) {
		fmemFree(mem);
		return false;
	}

	// Publish last, with a full barrier, so all setup writes above are visible before any thread sees the log as initialized
	fplAtomicStoreU32(&gLog.isInitialized, 1);

	return true;
}

fpl_extern void LogShutdown() {
	if (!gLog.isInitialized) {
		return;
	}

	LogAcquireLock(&gLog);

	// Mark as uninitialized under the lock, so writers that pass the guard afterwards no longer touch the file or memory
	fplAtomicStoreU32(&gLog.isInitialized, 0);

	if (gLog.fileHandle.isValid) {
		fplFileClose(&gLog.fileHandle);
	}
	fmemFree(&gLog.memory);

	LogReleaseLock(&gLog);

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
	if (!fplAtomicLoadU32(&gLog.isInitialized)) {
		return;
	}

	LogAcquireLock(&gLog);

	if (format == fpl_null || fplGetStringLength(format) == 0) {
		LogWriteNewlineUnsafe();
		LogReleaseLock(&gLog);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	if (!fmemBeginTemporary(&gLog.memory, &temporaryMemory)) {
		LogReleaseLock(&gLog);
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

	LogReleaseLock(&gLog);
}

fpl_extern void LogWriteLineBreak() {
	LogWriteRaw(fpl_null);
}

fpl_extern void LogWriteArgs(const LogLevel level, const char *category, const char *format, va_list argList) {
	// A severity of LogLevel_All passes every level, otherwise levels above the severity are filtered out
	if (!fplAtomicLoadU32(&gLog.isInitialized) || level < LogLevel_Min || level > LogLevel_Max || (gLog.severity > LogLevel_All && level > gLog.severity)) {
		return;
	}

	LogAcquireLock(&gLog);

	if (fplGetStringLength(format) == 0) {
		LogWriteNewlineUnsafe();
		LogReleaseLock(&gLog);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	if (!fmemBeginTemporary(&gLog.memory, &temporaryMemory)) {
		LogReleaseLock(&gLog);
		return;
	}

	LogString tempString = fplZeroInit;
	tempString.allocated = temporaryMemory.size;
	tempString.length = 0;
	tempString.data = (char *)temporaryMemory.base;

	const char *levelName = gLogLevelNames[level];
	bool hasCategory = category != fpl_null && fplGetStringLength(category) > 0;
	const char *categoryName = hasCategory ? category : gLogUnknownCategory;
	fplDateTime utcDate = fplDateTimeQuery(fplDateTimeType_UTC);
	fplDateTimeResult utcDateRes = fplFormatDateTime(utcDate, fplDateTimeType_UTC);

	size_t len;
	char *ptr = tempString.data;
	*ptr = 0;

	tempString.length = 0;

	len = fplStringFormat(ptr, tempString.allocated, "%04u-%02u-%02u %02u:%02u:%02u.%03u [%*.*s] [%*.*s]: ", utcDateRes.year, utcDateRes.month, utcDateRes.day, utcDateRes.hour, utcDateRes.minute, utcDateRes.second, utcDateRes.millisecond, LogLevelColumnWidth, LogLevelColumnWidth, levelName, LogCategoryColumnWidth, LogCategoryColumnWidth, categoryName);
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

	LogReleaseLock(&gLog);
}

fpl_extern void LogWrite(const LogLevel level, const char *category, const char *format, ...) {
	// Same filter as LogWriteArgs, checked early to avoid the va_list setup for filtered lines
	if (!fplAtomicLoadU32(&gLog.isInitialized) || level < LogLevel_Min || level > LogLevel_Max || (gLog.severity > LogLevel_All && level > gLog.severity)) {
		return;
	}

	va_list argList;
	va_start(argList, format);
	LogWriteArgs(level, category, format, argList);
	va_end(argList);
}

#endif // FINAL_LOG_IMPLEMENTATION