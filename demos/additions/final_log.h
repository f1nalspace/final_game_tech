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
	// Temporary character buffer, that maps to the memory block
	LogString charBuffer;
	// Handle to the active log file
	fplFileHandle fileHandle;
	// Indicates whether the log system has been initialized
	bool isInitialized;
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


static Log gLog = { 0 };

static char gLogWriteData[256] = { 0 };
static LogString gLogWriteString = {
	gLogWriteData,
	0,
	sizeof(gLogWriteData),
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

	fmemMemoryBlock *mem = &gLog.memory;

	if (!fmemInit(mem, fmemType_Growable, fplMegaBytes(16), 0)) {
		return false;
	}

	bool success = fplFileAppendBinary(logFilePath, &gLog.fileHandle);

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
	fmemFree(&gLog.memory);
	fplClearStruct(&gLog);
}

fpl_extern void LogWriteRaw(const char *format, ...) {
	if (!gLog.isInitialized) {
		return;
	}

	if (format == NULL || fplGetStringLength(format) == 0) {
		fplAssert(gLogWriteString.allocated >= 2);
		gLogWriteString.length = 1;
		gLogWriteString.data[0] = '\n';
		gLogWriteString.data[1] = '\0';
		fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);
		fplFileFlush(&gLog.fileHandle);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	fmemBeginTemporary(&gLog.memory, &temporaryMemory);

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
}

fpl_extern void LogWriteLineBreak() {
	LogWriteRaw(fpl_null);
}

fpl_extern void LogWriteArgs(const LogLevel level, const char *format, va_list argList) {
	if (!gLog.isInitialized || level < LogLevel_Min || level > LogLevel_Max) {
		return;
	}

	if (fplGetStringLength(format) == 0) {
		fplAssert(gLogWriteString.allocated >= 2);
		gLogWriteString.length = 1;
		gLogWriteString.data[0] = '\n';
		gLogWriteString.data[1] = '\0';
		fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);
		fplFileFlush(&gLog.fileHandle);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	fmemBeginTemporary(&gLog.memory, &temporaryMemory);

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
}

fpl_extern void LogWrite(const LogLevel level, const char *format, ...) {
	if (!gLog.isInitialized || level < LogLevel_Min || level > LogLevel_Max) {
		return;
	}

	if (fplGetStringLength(format) == 0) {
		fplAssert(gLogWriteString.allocated >= 2);
		gLogWriteString.length = 1;
		gLogWriteString.data[0] = '\n';
		gLogWriteString.data[1] = '\0';
		fplFileWriteBlock(&gLog.fileHandle, gLogWriteString.data, gLogWriteString.length);
		fplFileFlush(&gLog.fileHandle);
		return;
	}

	fmemMemoryBlock temporaryMemory = fplZeroInit;
	fmemBeginTemporary(&gLog.memory, &temporaryMemory);

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

	va_list argList;
	va_start(argList, format);
	len = fplStringFormatArgs(ptr, remaining, format, argList);
	va_end(argList);

	tempString.length += len + 1;

	tempString.data[tempString.length - 1] = '\n';
	tempString.data[tempString.length] = '\0';

	fplFileWriteBlock(&gLog.fileHandle, tempString.data, tempString.length);

	fplFileFlush(&gLog.fileHandle);

	fmemEndTemporary(&temporaryMemory);
}

#endif // FINAL_LOG_IMPLEMENTATION