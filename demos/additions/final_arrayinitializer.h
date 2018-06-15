/*
Name:
	Final Array Initializer
Description:
	Class for simulating indexed/named array initializer.

	This file is part of the final_framework.
License:
	MIT License
	Copyright 2018 Torsten Spaete
*/

#ifndef FINAL_ARRAYINITIALIZER_H
#define FINAL_ARRAYINITIALIZER_H

#include <final_platform_layer.h>

// @BAD(final): CPP is such garbage!
// It cannot handle array index initializer such as [index] = value :-(
// So we need this nonsense just to initialize a static array -.-template <typename TIndexType, typename TValueType, size_t size>
template <typename TIndexType, typename TValueType, size_t valueCount>
class ArrayInitializer {
protected:
	TValueType a[valueCount];
public:
	ArrayInitializer() {
		fplMemoryClear(a, sizeof(TValueType) * FPL_ARRAYCOUNT(a));
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

#endif // FINAL_ARRAYINITIALIZER_H