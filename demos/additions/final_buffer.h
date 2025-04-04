/*
Name:
	Final Buffer

Description:
	- Mirror Buffer (Circular Buffer)
	- Lock Free Ring Buffer

	This file is part of the final_framework.

Todo:
	- mmap implementation for mirror buffer

License:
	MIT License
	Copyright 2017-2025 Torsten Spaete

Changelog:

	## 2024-01-01
	- Use VirtualAlloc3 and MapViewOfFile3 in addition of old "try-repeat-loop" way of creating the virtual mapped buffers
*/

#ifndef FINAL_BUFFER_H
#define FINAL_BUFFER_H

#include <final_platform_layer.h>

typedef struct MemoryMirror {
#if defined(FPL_PLATFORM_WINDOWS)
	HANDLE *fileHandle; // Memory mapped file handle/base
#endif
	void *buffer;
	size_t length; // Length of one mirror
	size_t count; // How many mirrors
	fpl_b32 isValid;
	uint8_t padding[4];
} MirroredMemory;

// MemoryMirror

extern bool InitMemoryMirror(MirroredMemory *mem, const size_t length, const size_t count);

extern void ReleaseMemoryMirror(MirroredMemory *mem);

// LockFreeRingBuffer

typedef struct LockFreeRingBuffer {
#if defined(FPL_PLATFORM_WINDOWS)
	uint8_t filePadding[64 - UINTPTR_MAX];
	HANDLE *fileHandle; // Memory mapped file
#endif

	uint8_t bufferPadding[64 - UINTPTR_MAX];
	void *buffer;

	uint8_t lengthPadding[64 - 8];
	uint64_t length;

	uint8_t tailPadding[64 - 8];
	uint64_t tail;

	uint8_t headPadding[64 - 8];
	uint64_t head;

	uint8_t fillCountPadding[64 - 8];
	volatile int64_t fillCount;

	uint8_t flagsPadding[64 - 4];
	fpl_b32 isMirror;
} LockFreeRingBuffer;

extern bool LockFreeRingBufferInit(LockFreeRingBuffer *buffer, const size_t length, const bool allowMirror);
extern void LockFreeRingBufferRelease(LockFreeRingBuffer *buffer);

extern bool LockFreeRingBufferCanRead(LockFreeRingBuffer *buffer, size_t *availableBytes);
extern bool LockFreeRingBufferRead(LockFreeRingBuffer *buffer, void *dst, const size_t len);
extern bool LockFreeRingBufferSkip(LockFreeRingBuffer *buffer, const size_t length);

extern bool LockFreeRingBufferCanWrite(LockFreeRingBuffer *buffer, size_t *availableBytes);
extern bool LockFreeRingBufferWrite(LockFreeRingBuffer *buffer, const void *src, const size_t len);

extern void LockFreeRingBufferClear(LockFreeRingBuffer *buffer);

extern void LockFreeRingBufferUnitTest();

#endif // FINAL_BUFFER_H

#define FINAL_BUFFER_IMPLEMENTATION

#if defined(FINAL_BUFFER_IMPLEMENTATION) && !defined(FINAL_BUFFER_IMPLEMENTED)
#define FINAL_BUFFER_IMPLEMENTED

#if defined(FPL_PLATFORM_WINDOWS)

//
// Required definitions for older windows SDK's
//
#ifndef MEM_REPLACE_PLACEHOLDER

#define MEM_REPLACE_PLACEHOLDER 0x4000
#define MEM_RESERVE_PLACEHOLDER 0x40000
#define MEM_PRESERVE_PLACEHOLDER 0x2

typedef enum MEM_EXTENDED_PARAMETER_TYPE {
  MemExtendedParameterInvalidType = 0,
  MemExtendedParameterAddressRequirements,
  MemExtendedParameterNumaNode,
  MemExtendedParameterPartitionHandle,
  MemExtendedParameterUserPhysicalHandle,
  MemExtendedParameterAttributeFlags,
  MemExtendedParameterImageMachine,
  MemExtendedParameterMax
}  *PMEM_EXTENDED_PARAMETER_TYPE;

#define MEM_EXTENDED_PARAMETER_TYPE_BITS 8

typedef struct MEM_EXTENDED_PARAMETER {
  struct {
    DWORD64 type : MEM_EXTENDED_PARAMETER_TYPE_BITS;
    DWORD64 reserved : 64 - MEM_EXTENDED_PARAMETER_TYPE_BITS;
  } DUMMYSTRUCTNAME;
  union {
    DWORD64 ulong64;
    PVOID   pointer;
    SIZE_T  size;
    HANDLE  handle;
    DWORD   ulong;
  } DUMMYUNIONNAME;
} MEM_EXTENDED_PARAMETER, *PMEM_EXTENDED_PARAMETER;

#endif

#define WIN32_KERNEL_DLL "kernelbase.dll"

#define WIN32_FUNC_VirtualAlloc2(name) PVOID name(HANDLE process, PVOID baseAddress, SIZE_T size, ULONG allocationType, ULONG pageProtection, MEM_EXTENDED_PARAMETER *extendedParameters, ULONG parameterCount)
typedef WIN32_FUNC_VirtualAlloc2(win32_func_virtualalloc2);

#define WIN32_FUNC_MapViewOfFile3(name) PVOID name(HANDLE fileMapping, HANDLE process, PVOID baseAddress, ULONG64 offset, SIZE_T viewSize, ULONG allocationType, ULONG pageProtection, MEM_EXTENDED_PARAMETER *extendedParameters, ULONG parameterCount)
typedef WIN32_FUNC_MapViewOfFile3(win32_func_mapviewoffile3);

static void f_ReleaseMemoryMirrorWin32(MirroredMemory *mem) {
	fplAssert(mem != fpl_null && mem->buffer != fpl_null);
	if(mem->buffer != NULL) {
		for (size_t mirrorIndex = 0; mirrorIndex < mem->count; ++mirrorIndex) {
			UnmapViewOfFile((uint8_t *)mem->buffer + mirrorIndex * mem->length);
		}	
		VirtualFree(mem->buffer, mem->length * mem->count, MEM_RELEASE);
	}
	if(mem->fileHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(mem->fileHandle);
	}
}

static size_t f_RoundToPow2Size(const size_t minimumSize, const size_t blockSize) {
	size_t result = (minimumSize + blockSize - 1) / blockSize * blockSize;
	return result;
}

static bool f_InitMemoryMirrorWin32(MirroredMemory *mem, const size_t length, const size_t count) {
	fplAssert(mem != fpl_null && length > 0 && count > 1);

	// Get length in multiple of page-sizes
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	size_t pageSize = sysInfo.dwAllocationGranularity;

	size_t roundedSize = f_RoundToPow2Size(length, pageSize);

	size_t totalSize = roundedSize * count;

	LARGE_INTEGER largeSize;
	largeSize.QuadPart = totalSize;

	uint8_t *blockAddress = NULL;
	HANDLE fileHandle = NULL;

	// Load VirtualAlloc2 and MapViewOfView3
	win32_func_virtualalloc2 *virtualAlloc2 = fpl_null;
	win32_func_mapviewoffile3 *mapViewOfFile3 = fpl_null;
	HMODULE kernelHandle = LoadLibraryA(WIN32_KERNEL_DLL);
	if (kernelHandle != NULL) {
		virtualAlloc2 = (win32_func_virtualalloc2 *)GetProcAddress(kernelHandle, "VirtualAlloc2");
		mapViewOfFile3 = (win32_func_mapviewoffile3 *)GetProcAddress(kernelHandle, "MapViewOfFile3");
		FreeLibrary(kernelHandle);
	}

	//
	// Try modern way of memory mapping
	//
	if (virtualAlloc2 != fpl_null && mapViewOfFile3 != fpl_null) {
		do {
			// Create mapped file with the length of the buffer
			fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, largeSize.HighPart, largeSize.LowPart, NULL);
			if(fileHandle == INVALID_HANDLE_VALUE) {
				// Failed, we cannot continue
				break;
			}

			// Reserve memory of the entire mirror buffer (two or more)
			blockAddress = virtualAlloc2(0, 0, totalSize, MEM_RESERVE | MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, 0, 0);
			if (blockAddress == NULL) {
				CloseHandle(fileHandle);
				fileHandle = NULL;
			}

			bool mapped = true;
			for (size_t mirrorIndex = 0; mirrorIndex < count; ++mirrorIndex) {
				VirtualFree(blockAddress + mirrorIndex * roundedSize, roundedSize, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER);
				if (!mapViewOfFile3(fileHandle, 0, blockAddress + mirrorIndex * roundedSize, 0, roundedSize, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0)) {
					mapped = false;
					break;
				}
			}

			if (mapped) {
				// Success, we mapped all mirrors
				goto returnResult;
			}

			// Failed cleanup and try again
			CloseHandle(fileHandle);
			fileHandle = NULL;
			blockAddress = NULL;
		} while (false);
	}

	//
	// Second way if modern memory mapping does not work
	// Keep trying a few times until we get our buffer
	//
	int retries = 10;
	while(retries-- > 0) {
		// Create mapped file with the length of the buffer
		fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, largeSize.HighPart, largeSize.LowPart, NULL);
		if(fileHandle == INVALID_HANDLE_VALUE) {
			// Failed, we cannot continue
			break;
		}

		// Reserve memory of the entire mirror buffer (two or more)
		blockAddress = (uint8_t *)VirtualAlloc(NULL, roundedSize * count, MEM_RESERVE, PAGE_NOACCESS);
		if(blockAddress == NULL) {
			// Failed, try again
			CloseHandle(fileHandle);
			continue;
		}

		// Release the full range immediately, but retain the address for the re-mapping
		VirtualFree(blockAddress, 0, MEM_FREE);

		// Re-map all mirror buffers (these may fail, when the OS already used our memory elsewhere)
		size_t mappedCount = 0;
		for (size_t mirrorIndex = 0; mirrorIndex < count; ++mirrorIndex) {
			LPVOID mapAddress = MapViewOfFileEx(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, roundedSize, blockAddress + mirrorIndex * roundedSize);
			if (mapAddress == blockAddress + mirrorIndex * roundedSize) {
				++mappedCount;
			}
		}

		if (mappedCount == count) {
			// Success, we mapped all mirrors
			goto returnResult;
		}

		// Failed cleanup and try again
		CloseHandle(fileHandle);
		fileHandle = NULL;
		blockAddress = NULL;
	}

returnResult:
	if(blockAddress != fpl_null) {
		fplClearStruct(mem);
		mem->fileHandle = fileHandle;
		mem->length = roundedSize;
		mem->buffer = blockAddress;
		mem->count = count;
		mem->isValid = true;
		return(true);
	}

	return(false);
}
#endif

extern bool InitMemoryMirror(MirroredMemory *mem, const size_t length, const size_t count) {
	if(mem == fpl_null || length == 0 || count < 2) return(false);

#if defined(FPL_PLATFORM_WINDOWS)
	return f_InitMemoryMirrorWin32(mem, length, count);
#else
	return(false);
#endif
}

extern void ReleaseMemoryMirror(MirroredMemory *mem) {
	if(mem != fpl_null && mem->buffer != fpl_null) {
#if defined(FPL_PLATFORM_WINDOWS)
		f_ReleaseMemoryMirrorWin32(mem);
#endif
	}
}

extern bool LockFreeRingBufferInit(LockFreeRingBuffer *buffer, const size_t length, const bool allowMirror) {
	if(buffer == fpl_null || length == 0) return(false);

	if(allowMirror) {
		MirroredMemory mirror;
		if(InitMemoryMirror(&mirror, length, 2)) {
			fplClearStruct(buffer);
			buffer->length = mirror.length;
			buffer->buffer = mirror.buffer;
			buffer->isMirror = true;
#if defined(FPL_PLATFORM_WINDOWS)
			buffer->fileHandle = mirror.fileHandle;
#endif
			return(true);
		}
	}

	void *wrapMemory = fplMemoryAllocate(length);
	if(wrapMemory != fpl_null) {
		fplClearStruct(buffer);
		buffer->isMirror = 0;
		buffer->length = length;
		buffer->buffer = wrapMemory;
		return(true);
	}

	return(false);
}

extern void LockFreeRingBufferRelease(LockFreeRingBuffer *buffer) {
	if(buffer == fpl_null) return;
	if(buffer->isMirror && buffer->buffer != fpl_null) {
		MirroredMemory mirror;
		mirror.buffer = buffer->buffer;
		mirror.length = (size_t)buffer->length;
		mirror.count = 2;
#if defined(FPL_PLATFORM_WINDOWS)
		mirror.fileHandle = buffer->fileHandle;
#endif
		mirror.isValid = true;
		ReleaseMemoryMirror(&mirror);
	} else {
		if(buffer->buffer != NULL) {
			fplMemoryFree(buffer->buffer);
		}
	}
	fplClearStruct(buffer);
}

fpl_force_inline void f_LockFreeRingBufferProduce(LockFreeRingBuffer *buffer, uint64_t amount) {
	fplAssert(buffer != fpl_null);
	buffer->head = (buffer->head + amount) % buffer->length;
	fplAtomicFetchAndAddS64(&buffer->fillCount, (int64_t)amount);
	assert(buffer->fillCount <= (int64_t)buffer->length);
}

fpl_force_inline void f_LockFreeRingBufferConsume(LockFreeRingBuffer *buffer, uint64_t amount) {
	fplAssert(buffer != fpl_null);
	buffer->tail = (buffer->tail + amount) % buffer->length;
	fplAtomicFetchAndAddS64(&buffer->fillCount, -(int64_t)amount);
	assert(buffer->fillCount >= 0);
}

extern bool LockFreeRingBufferCanRead(LockFreeRingBuffer *buffer, size_t *availableBytes) {
	if(buffer == fpl_null) return(false);
	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);
	*availableBytes = (size_t)fillCount;
	if(fillCount > 0) {
		return(true);
	}
	return(false);
}

extern bool LockFreeRingBufferCanWrite(LockFreeRingBuffer *buffer, size_t *availableBytes) {
	if(buffer == fpl_null) return(false);
	uint64_t available = (buffer->length - fplAtomicLoadS64(&buffer->fillCount));
	*availableBytes = (size_t)available;
	if(available > 0) {
		return(true);
	}
	return(false);
}

extern bool LockFreeRingBufferWrite(LockFreeRingBuffer *buffer, const void *src, const size_t len) {
	if(buffer == fpl_null) return(false);
	uint64_t available = buffer->length - fplAtomicLoadS64(&buffer->fillCount);
	if(available < len) return false;

	uint8_t *srcAddr = (uint8_t *)src;
	uint8_t *dstAddr = (uint8_t *)buffer->buffer;
	if(buffer->isMirror || (buffer->head + len) <= buffer->length) {
		memcpy(dstAddr + buffer->head, srcAddr, len);
	} else {
		uint64_t bytesLeft = fplMin(fplMin(len, available), buffer->length - buffer->head);
		memcpy(dstAddr + buffer->head, srcAddr, (size_t)bytesLeft);

		uint64_t bytesRight = len - bytesLeft;
		memcpy(dstAddr, srcAddr + bytesLeft, (size_t)bytesRight);
	}
	f_LockFreeRingBufferProduce(buffer, len);
	return(true);
}

extern bool LockFreeRingBufferRead(LockFreeRingBuffer *buffer, void *dst, const size_t len) {
	if(buffer == fpl_null) return(false);
	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);
	if(len > fillCount) return(false);
	uint8_t *srcAddr = (uint8_t *)buffer->buffer;
	uint8_t *dstAddr = (uint8_t *)dst;
	if(dst != fpl_null) {
		if(buffer->isMirror || (buffer->tail + len) <= buffer->length) {
			memcpy(dstAddr, srcAddr + buffer->tail, len);
		} else {
			uint64_t bytesLeft = fplMin(fplMin(len, fillCount), buffer->length - buffer->tail);
			memcpy(dstAddr, srcAddr + buffer->tail, (size_t)bytesLeft);

			uint64_t bytesRight = len - bytesLeft;
			memcpy(dstAddr + bytesLeft, srcAddr, (size_t)bytesRight);
		}
	}
	f_LockFreeRingBufferConsume(buffer, len);
	return(true);
}

extern bool LockFreeRingBufferPeek(LockFreeRingBuffer *buffer, void *dst, const size_t offset, const size_t len) {
	if(buffer == fpl_null) return(false);
	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);
	if((offset + len) > fillCount) return(false);
	uint8_t *srcAddr = (uint8_t *)buffer->buffer;
	uint8_t *dstAddr = (uint8_t *)dst;
	if(dst != fpl_null) {
		if(buffer->isMirror || (buffer->tail + offset + len) <= buffer->length) {
			memcpy(dstAddr, srcAddr + buffer->tail + offset, len);
		} else {
			uint64_t tail = (buffer->tail + offset) % buffer->length;

			uint64_t bytesLeft = fplMin(fplMin(len, fillCount - offset), buffer->length - tail);
			memcpy(dstAddr, srcAddr + tail, (size_t)bytesLeft);

			uint64_t bytesRight = len - bytesLeft;
			memcpy(dstAddr + bytesLeft, srcAddr, (size_t)bytesRight);
		}
	}
	return(true);
}

extern void LockFreeRingBufferClear(LockFreeRingBuffer *buffer) {
	if(buffer == fpl_null) return;
	size_t fillCount = 0;
	LockFreeRingBufferCanRead(buffer, &fillCount);
	if(fillCount > 0) {
		f_LockFreeRingBufferConsume(buffer, fillCount);
	}
	buffer->tail = buffer->head = 0;
	fplAtomicExchangeS64(&buffer->fillCount, 0);
}

extern bool LockFreeRingBufferSkip(LockFreeRingBuffer *buffer, const size_t length) {
	if(buffer == fpl_null)
		return(false);

	size_t fillCount = 0;
	if(!LockFreeRingBufferCanRead(buffer, &fillCount))
		return(false);

	if(fillCount < length)
		return(false);

	f_LockFreeRingBufferConsume(buffer, length);

	return(true);
}

void assertBytes(const uint8_t *data, const uint8_t test, const size_t offset, const size_t len) {
	for(size_t i = 0; i < len; ++i) {
		size_t p = offset + i;
		assert(data[p] == test);
	}
}

extern void LockFreeRingBufferUnitTest() {
	// @FIXME(tspaete): This is totally broken right now and needs to be fixed, due to change from page to allocation granularity size
#if 0
	LockFreeRingBuffer buffer;
	bool res = LockFreeRingBufferInit(&buffer, 128, true);
	assert(res);

	// Validate initial buffer
	assert(buffer.length == 128);
	assert(buffer.head == 0);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 0);
	assert(!buffer.isMirror);

	bool canWrite, canRead;
	size_t writeAvailable, readAvailable;

	// Validate initial head
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(canWrite);
	assert(writeAvailable == 128);

	// Validate initial tail
	canRead = LockFreeRingBufferCanRead(&buffer, &readAvailable);
	assert(!canRead);
	assert(readAvailable == 0);

	uint8_t data[1024];

	// Write 32-bytes 0xAA
	memset(data, 0xAA, 1024);
	res = LockFreeRingBufferWrite(&buffer, &data, 32);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);

	// Validate buffer (0xAA)
	assert(buffer.head == 32);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 32);

	// Validate head (0xAA)
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(canWrite);
	assert(writeAvailable == 32 + 64);

	// Write 64-bytes 0xBB
	memset(data, 0xBB, 1024);
	res = LockFreeRingBufferWrite(&buffer, &data, 64);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);
	assertBytes(buffer.buffer, 0xBB, 32, 64);

	// Validate buffer (0xBB)
	assert(buffer.head == (32 + 64));
	assert(buffer.tail == 0);
	assert(buffer.fillCount == (32 + 64));

	// Validate head (0xBB)
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(canWrite);
	assert(writeAvailable == 32);

	// Write 16-bytes 0xCC
	memset(data, 0xCC, 1024);
	res = LockFreeRingBufferWrite(&buffer, &data, 16);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);
	assertBytes(buffer.buffer, 0xBB, 32, 64);
	assertBytes(buffer.buffer, 0xCC, 32 + 64, 16);

	// Validate buffer (0xCC)
	assert(buffer.head == (32 + 64 + 16));
	assert(buffer.tail == 0);
	assert(buffer.fillCount == (32 + 64 + 16));

	// Validate head (0xCC)
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(canWrite);
	assert(writeAvailable == 16);

	// Try to write 32-bytes 0xDD
	memset(data, 0xCC, 1024);
	res = LockFreeRingBufferWrite(&buffer, &data, 32);
	assert(!res);

	// Validate tail (96 bytes available)
	canRead = LockFreeRingBufferCanRead(&buffer, &readAvailable);
	assert(canRead);
	assert(readAvailable == 32 + 64 + 16);

	// Validate buffer
	assert(buffer.head == 64 + 32 + 16);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 64 + 32 + 16);

	// Consume 16 bytes
	res = LockFreeRingBufferRead(&buffer, fpl_null, 16);
	assert(res);

	// Validate tail (96 bytes available)
	canRead = LockFreeRingBufferCanRead(&buffer, &readAvailable);
	assert(canRead);
	assert(readAvailable == 96);

	// Write 32-bytes 0xDD
	memset(data, 0xDD, 1024);
	res = LockFreeRingBufferWrite(&buffer, &data, 32);
	assert(res);
	assertBytes(buffer.buffer, 0xDD, 0, 16); // Data is overidden
	assertBytes(buffer.buffer, 0xAA, 16, 16);
	assertBytes(buffer.buffer, 0xBB, 32, 64);
	assertBytes(buffer.buffer, 0xCC, 32 + 64, 16);
	assertBytes(buffer.buffer, 0xDD, 32 + 64 + 16, 16); // Written to the very end

	// Validate buffer
	assert(buffer.head == 16); // Head is wrapped
	assert(buffer.tail == 16);
	assert(buffer.fillCount == 64 + 32 + 32);

	// Validate head (0xDD)
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(!canWrite);
	assert(writeAvailable == 0);

	// Validate tail (128 bytes available)
	canRead = LockFreeRingBufferCanRead(&buffer, &readAvailable);
	assert(canRead);
	assert(readAvailable == 128);

	// Consume 64 bytes
	res = LockFreeRingBufferRead(&buffer, fpl_null, 64);
	assert(res);

	// Validate head (64 bytes available)
	canWrite = LockFreeRingBufferCanWrite(&buffer, &writeAvailable);
	assert(canWrite);
	assert(writeAvailable == 64);

	// Validate tail (64 bytes available)
	canRead = LockFreeRingBufferCanRead(&buffer, &readAvailable);
	assert(canRead);
	assert(readAvailable == 64);

	LockFreeRingBufferRelease(&buffer);
#endif
}

#endif // FINAL_BUFFER_IMPLEMENTATION