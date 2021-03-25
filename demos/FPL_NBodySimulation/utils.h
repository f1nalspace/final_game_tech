#ifndef UTILS_H
#define UTILS_H

#include <final_platform_layer.h>
#include <assert.h>
#include <stdio.h>
#include <algorithm>
#include <intrin.h>
#include <varargs.h>

#define force_inline fpl_force_inline

template <typename T>
inline T PointerToValue(void *ptr) {
	T result = (T)(uintptr_t)(ptr);
	return(result);
}
template <typename T>
inline void *ValueToPointer(T value) {
	void *result = (T *)(uintptr_t)(value);
	return(result);
}

const float nanosToMilliseconds = 1.0f / 1000000;

template <typename T>
inline void UpdateMin(T &value, T a) {
	value = std::min(value, a);
}

template <typename T>
inline void UpdateMax(T &value, T a) {
	value = std::max(value, a);
}

template <typename T>
inline void Accumulate(T &value, T a) {
	value += a;
}

inline std::string StringFormat(const char *format, ...) {
	char buffer[1024];
	va_list vaList;
	va_start(vaList, format);
	fplFormatStringArgs(buffer, fplArrayCount(buffer), format, vaList);
	va_end(vaList);
	std::string result = buffer;
	return(result);
}

static uint8_t *LoadFileContent(const char *filename) {
	uint8_t *result = nullptr;
	fplFileHandle handle;
	if (fplOpenBinaryFile(filename, &handle)) {
		fplSetFilePosition32(&handle, 0, fplFilePositionMode_End);
		uint32_t fileSize = fplGetFilePosition32(&handle);
		fplSetFilePosition32(&handle, 0, fplFilePositionMode_Beginning);
		result = (uint8_t *)fplMemoryAllocate(fileSize);
		fplReadFileBlock32(&handle, fileSize, result, fileSize);
		fplCloseFile(&handle);
	}
	return(result);
}

#endif
