#ifndef UTILS_H
#define UTILS_H

#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

#include <stdio.h>
#include <algorithm>
#include <stdarg.h>

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
	fplStringFormatArgs(buffer, fplArrayCount(buffer), format, vaList);
	va_end(vaList);
	std::string result = buffer;
	return(result);
}

static uint8_t *LoadFileContent(const char *filename) {
	uint8_t *result = nullptr;
	fplFileHandle handle;
	if (fplFileOpenBinary(filename, &handle)) {
		fplFileSetPosition32(&handle, 0, fplFilePositionMode_End);
		uint32_t fileSize = fplFileGetPosition32(&handle);
		fplFileSetPosition32(&handle, 0, fplFilePositionMode_Beginning);
		result = (uint8_t *)fplMemoryAllocate(fileSize);
		fplFileReadBlock32(&handle, fileSize, result, fileSize);
		fplFileClose(&handle);
	}
	return(result);
}

#endif
