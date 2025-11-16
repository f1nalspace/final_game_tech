/*
Name:
	Final Utils

Description:
	Contains utility functions.

	This file is part of the final_framework.

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete
*/

#ifndef FINAL_UTILS_H
#define FINAL_UTILS_H

#include <final_platform_layer.h>

#if defined(FPL_IS_CPP)
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

fpl_internal void FormatSize(const size_t value, const size_t maxCount, char *buffer) {
    char *p = buffer;
    if (value < 0) {
        p++;
    }

    size_t tmp = value;
    do {
        p++;
        tmp = tmp / 10;
    } while (tmp);

    // Count thousands
    size_t thousandDotCount = 0;
    tmp = value;
    while (tmp >= 1000) {
        p++;
        ++thousandDotCount;
        tmp = tmp / 1000;
    }

    size_t charCount = p - buffer;

    fplAssert(charCount + 1 <= maxCount);

    *p = 0;
    const char *digits = "0123456789";
    size_t v = value;
    size_t c = 0;
    size_t t = thousandDotCount;
    do {
        if (t > 0) {
            if (c == 3) {
                c = 0;
                --t;
                *--p = '.';
            }
        }
        *--p = digits[v % 10];
        v /= 10;
        c++;
    } while (v != 0);
}

#endif // FINAL_UTILS_H