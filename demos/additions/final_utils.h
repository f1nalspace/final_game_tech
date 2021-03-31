/*
Name:
	Final Utils

Description:
	Contains utility functions.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2021 Torsten Spaete
*/

#ifndef FINAL_UTILS_H
#define FINAL_UTILS_H

#include <final_platform_layer.h>

#if defined(FPL_IS_CPP)
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

template <typename T>
inline void Swap(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

template <typename T, size_t N>
inline size_t ArrayCount(T(&arr)[N]) {
	size_t result = sizeof(arr) / sizeof(arr[0]);
	return(result);
}

// @BAD(final): CPP is such garbage!
// It cannot handle array index initializer such as [index] = value :-(
// So we need this nonsense just to initialize a static array -.-template <typename TIndexType, typename TValueType, size_t size>
template <typename TIndexType, typename TValueType, size_t valueCount>
class ArrayInitializer {
protected:
	TValueType a[valueCount];
public:
	ArrayInitializer() {
		fplMemoryClear(a, sizeof(TValueType) * ArrayCount(a));
	}
	const TValueType &operator [] (TIndexType eindex) const {
		return a[(int)eindex];
	}
	TValueType &operator [] (TIndexType eindex) {
		return a[(int)eindex];
	}
	void Set(TIndexType e, const TValueType &value) {
		a[(int)e] = value;
	}
};

#endif // FPL_IS_CPP

fpl_internal uint32_t NextPowerOfTwo(const uint32_t input) {
	uint32_t x = input;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return(x);
}
fpl_internal uint32_t PrevPowerOfTwo(const uint32_t input) {
	uint32_t result = NextPowerOfTwo(input) >> 1;
	return(result);
}

fpl_internal uint32_t RoundToPowerOfTwo(const uint32_t input) {
	if(fplIsPowerOfTwo(input))
		return(input);
	uint32_t result = NextPowerOfTwo(input);
	return(result);
}

#endif // FINAL_UTILS_H