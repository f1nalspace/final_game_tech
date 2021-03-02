#ifndef FINAL_BUFFER_H
#define FINAL_BUFFER_H

#include <final_platform_layer.h>

typedef struct MemoryMirror {
#if defined(FPL_PLATFORM_WINDOWS)
	HWND *fileHandle; // Memory mapped file
#endif
	void *buffer;
	size_t length;
	fpl_b32 isValid;
	uint8_t padding[4];
} MirroredMemory;

// MemoryMirror

extern bool InitMemoryMirror(MirroredMemory *mem, const size_t length);

extern void ReleaseMemoryMirror(MirroredMemory *mem);

// LockFreeRingBuffer

typedef struct LockFreeRingBuffer {
#if defined(FPL_PLATFORM_WINDOWS)
	uint8_t filePadding[64 - UINTPTR_MAX];
	HWND *fileHandle; // Memory mapped file
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

extern bool LockFreeRingBufferCanWrite(LockFreeRingBuffer *buffer, size_t *availableBytes);
extern bool LockFreeRingBufferWrite(LockFreeRingBuffer *buffer, const void *src, const size_t len);

extern void LockFreeRingBufferClear(LockFreeRingBuffer *buffer);

extern void LockFreeRingBufferUnitTest();

#endif // FINAL_BUFFER_H

#if defined(FINAL_BUFFER_IMPLEMENTATION) && !defined(FINAL_BUFFER_IMPLEMENTED)
#define FINAL_BUFFER_IMPLEMENTED

#if defined(FPL_PLATFORM_WINDOWS)
static void f_ReleaseMemoryMirrorWin32(MirroredMemory *mem) {
	fplAssert(mem != fpl_null && mem->buffer != fpl_null);
	if(mem->buffer != NULL) {
		UnmapViewOfFile(mem->buffer);
		UnmapViewOfFile((uint8_t *)mem->buffer + mem->length);
		VirtualFree(mem->buffer, mem->length * 2, MEM_RELEASE);
	}
	if(mem->fileHandle != NULL) {
		CloseHandle(mem->fileHandle);
	}
}

static bool f_InitMemoryMirrorWin32(MirroredMemory *mem, const size_t length) {
	fplAssert(mem != fpl_null && length > 0);

	// Get length in multiple of page-sizes
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	size_t pageSize = sysInfo.dwPageSize;

	size_t roundedSize = (length + pageSize - 1) / pageSize * pageSize;

	uint8_t *blockAddress = NULL;
	HANDLE fileHandle = NULL;

		// Keep trying until we get our buffer, needed to handle race conditions
	int retries = 10;
	while(retries-- > 0) {
		LARGE_INTEGER largeSize;
		largeSize.QuadPart = roundedSize * 2;

		// Create mapped file with the length of the buffer
		fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, largeSize.HighPart, largeSize.LowPart, NULL);
		if(fileHandle == INVALID_HANDLE_VALUE) {
			// Failed, we cannot continue
			break;
		}

		// Reserve two memory of twice the length of the buffer
		blockAddress = (uint8_t *)VirtualAlloc(NULL, roundedSize * 2, MEM_RESERVE, PAGE_NOACCESS);
		if(blockAddress == NULL) {
			// Failed, try again
			CloseHandle(fileHandle);
			continue;
		}

		// Release the full range immediately, but retain the address for the re-mapping
		VirtualFree(blockAddress, 0, MEM_FREE);

		// Re-map both buffers to both buffers (these may fail, when the OS already used our memory elsewhere)
		if((MapViewOfFileEx(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, roundedSize, blockAddress) == blockAddress) &&
			(MapViewOfFileEx(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, roundedSize, blockAddress + roundedSize) == blockAddress + roundedSize)) {
			 // Success, we can use the blockAddress as our base-ptr
			break;
		}

		// Failed cleanup and try again
		CloseHandle(fileHandle);
		fileHandle = NULL;
		blockAddress = NULL;
	}

	if(blockAddress != fpl_null) {
		fplClearStruct(mem);
		mem->fileHandle = fileHandle;
		mem->length = roundedSize;
		mem->buffer = blockAddress;
		mem->isValid = true;
		return(true);
	}

	return(false);
}
#endif

extern bool InitMemoryMirror(MirroredMemory *mem, const size_t length) {
	if(mem == fpl_null || length == 0) return(false);

#if defined(FPL_PLATFORM_WINDOWS)
	return f_InitMemoryMirrorWin32(mem, length);
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
		if(InitMemoryMirror(&mirror, length)) {
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
		mirror.length = buffer->length;
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
	ZeroMemory(buffer, sizeof(LockFreeRingBuffer));
}

extern bool LockFreeRingBufferCanRead(LockFreeRingBuffer *buffer, size_t *availableBytes) {
	if(buffer == fpl_null) return(false);
	uint64_t fillCount = fplAtomicLoadS64(&buffer->fillCount);
	*availableBytes = fillCount;
	if(fillCount > 0) {
		return(true);
	}
	return(false);
}

extern bool LockFreeRingBufferCanWrite(LockFreeRingBuffer *buffer, size_t *availableBytes) {
	if(buffer == fpl_null) return(false);
	uint64_t available = (buffer->length - fplAtomicLoadS64(&buffer->fillCount));
	*availableBytes = available;
	if(available > 0) {
		return(true);
	}
	return(false);
}

inline void f_LockFreeRingBufferProduce(LockFreeRingBuffer *buffer, uint64_t amount) {
	fplAssert(buffer != fpl_null);
	buffer->head = (buffer->head + amount) % buffer->length;
	fplAtomicFetchAndAddS64(&buffer->fillCount, (int64_t)amount);
	assert(buffer->fillCount <= (int64_t)buffer->length);
}

inline void f_LockFreeRingBufferConsume(LockFreeRingBuffer *buffer, uint64_t amount) {
	fplAssert(buffer != fpl_null);
	buffer->tail = (buffer->tail + amount) % buffer->length;
	fplAtomicFetchAndAddS64(&buffer->fillCount, -(int64_t)amount);
	assert(buffer->fillCount >= 0);
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
		memcpy(dstAddr + buffer->head, srcAddr, bytesLeft);

		uint64_t bytesRight = len - bytesLeft;
		memcpy(dstAddr, srcAddr + bytesLeft, bytesRight);
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
			memcpy(dstAddr, srcAddr + buffer->tail, bytesLeft);

			uint64_t bytesRight = len - bytesLeft;
			memcpy(dstAddr + bytesLeft, srcAddr, bytesRight);
		}
	}
	f_LockFreeRingBufferConsume(buffer, len);
	return(true);
}

extern void LockFreeRingBufferClear(LockFreeRingBuffer *buffer) {
	if(buffer == fpl_null) return;
	uint64_t fillCount;
	LockFreeRingBufferCanRead(buffer, &fillCount);
	if(fillCount > 0) {
		f_LockFreeRingBufferConsume(buffer, fillCount);
	}
}

void assertBytes(const uint8_t *data, const uint8_t test, const size_t offset, const size_t len) {
	for(size_t i = 0; i < len; ++i) {
		size_t p = offset + i;
		assert(data[p] == test);
	}
}

extern void LockFreeRingBufferUnitTest() {
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
	uint64_t writeAvailable, readAvailable;

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
}

#endif // FINAL_BUFFER_IMPLEMENTATION