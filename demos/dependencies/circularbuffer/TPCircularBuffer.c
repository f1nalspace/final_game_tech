//
//  TPCircularBuffer.c
//  Circular/Ring buffer implementation
//
//  https://github.com/michaeltyson/TPCircularBuffer
//
//  Created by Michael Tyson on 10/12/2011.
//
//  Copyright (C) 2012-2013 A Tasty Pixel
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//

#include "TPCircularBuffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TPMin(a, b) ((a) < (b) ? (a) : (b))
#define TPMax(a, b) ((a) > (b) ? (a) : (b))

#define _isPowerOfTwo(value) (((value) != 0) && (((value) & (~(value) + 1)) == (value)))

static uint32_t _nextPowerOfTwo(const uint32_t input) {
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
static uint32_t _prevPowerOfTwo(const uint32_t input) {
	uint32_t result = _nextPowerOfTwo(input) >> 1;
	return(result);
}

static uint32_t _roundToPowerOfTwo(const uint32_t input) {
	if(_isPowerOfTwo(input))
		return(input);
	uint32_t result = _nextPowerOfTwo(input);
	return(result);
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)


extern bool TPCircularBufferInit(TPCircularBuffer* buffer, uint32_t length, bool allowMirror) {
	assert(length > 0);

	// Get length in multiple of page-sizes
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	uint32_t pageSize = sysInfo.dwPageSize;

	uint8_t* blockAddress = NULL;
	HANDLE fileHandle = NULL;
	uint32_t blockLength = length;

	if(allowMirror) {
		uint32_t roundedSize = (length + pageSize - 1) / pageSize * pageSize;

		// Keep trying until we get our buffer, needed to handle race conditions
		int retries = 10;
		while(retries-- > 0) {
			// Create mapped file with the length of the buffer
			fileHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, roundedSize * 2, NULL);
			if(fileHandle == INVALID_HANDLE_VALUE) {
				// Failed, we cannot continue
				break;
			}

			// Reserve two memory of twice the length of the buffer
			blockAddress = (uint8_t*)VirtualAlloc(NULL, roundedSize * 2, MEM_RESERVE, PAGE_NOACCESS);
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
				blockLength = roundedSize;
				break;
			}

			// Failed cleanup and try again
			CloseHandle(fileHandle);
			fileHandle = NULL;
			blockAddress = NULL;
		}
	}

	ZeroMemory(buffer, sizeof(TPCircularBuffer));

	if(blockAddress != NULL) {
		// Got a mirror block address
		buffer->isMirror = 1;
		buffer->length = blockLength;
		buffer->buffer = (void*)blockAddress;
		buffer->fileHandle = fileHandle;
		return(true);
	}

	// Requires wrapping
	void* wrapMemory = VirtualAlloc(NULL, blockLength, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	buffer->isMirror = 0;
	buffer->length = blockLength;
	buffer->buffer = wrapMemory;

	return(true);
}

extern void TPCircularBufferCleanup(TPCircularBuffer* buffer) {
	if(buffer->isMirror) {
		if(buffer->buffer != NULL) {
			UnmapViewOfFile(buffer->buffer);
			UnmapViewOfFile((uint8_t*)buffer->buffer + buffer->length);
			VirtualFree(buffer->buffer, buffer->length * 2, MEM_RELEASE);
		}
		if(buffer->fileHandle != NULL) {
			CloseHandle(buffer->fileHandle);
		}
	} else {
		if(buffer->buffer != NULL)
			VirtualFree(buffer->buffer, 0, MEM_RELEASE);
	}
	ZeroMemory(buffer, sizeof(TPCircularBuffer));
}

extern TPCircularBufferData TPCircularBufferTail(TPCircularBuffer* buffer, uint32_t* availableBytes) {
	TPCircularBufferData result;
	ZeroMemory(&result, sizeof(result));

	uint32_t fillCount = atomicRead(&buffer->fillCount);

	if(fillCount > 0) {
		if((buffer->tail + fillCount) > buffer->length) {
			result.lengthOfFirst = buffer->length - buffer->tail;
			result.first = (uint8_t*)buffer->buffer + buffer->tail;
			result.lengthOfSecond = fillCount - result.lengthOfFirst;
			result.second = (uint8_t*)buffer->buffer + ((buffer->tail + result.lengthOfFirst) % buffer->length);
			result.isDoubleBuffer = 1;
		} else {
			result.lengthOfFirst = fillCount;
			result.first = (uint8_t*)buffer->buffer + buffer->tail;
			result.isDoubleBuffer = 0;
		}
	}

	*availableBytes = fillCount;
	return(result);
}

extern TPCircularBufferData TPCircularBufferHead(TPCircularBuffer* buffer, uint32_t* availableBytes) {
	TPCircularBufferData result;
	ZeroMemory(&result, sizeof(result));

	uint32_t available = (buffer->length - atomicRead(&buffer->fillCount));

	if(available > 0) {
		result.lengthOfFirst = TPMin(available, buffer->length - buffer->head);
		result.first = (uint8_t*)buffer->buffer + buffer->head;
		if(result.lengthOfFirst < available) {
			result.lengthOfSecond = TPMax(0, available - result.lengthOfFirst);
			result.second = (uint8_t*)buffer->buffer;
			result.isDoubleBuffer = 1;
		}
	}

	*availableBytes = available;
	return(result);
}

inline void TPCircularBufferProduce(TPCircularBuffer* buffer, uint32_t amount) {
	buffer->head = (buffer->head + amount) % buffer->length;
	atomicFetchAdd(&buffer->fillCount, (int)amount);
	assert(buffer->fillCount <= (int32_t)buffer->length);
}


inline void TPCircularBufferConsume(TPCircularBuffer* buffer, uint32_t amount) {
	buffer->tail = (buffer->tail + amount) % buffer->length;
	atomicFetchAdd(&buffer->fillCount, -(int)amount);
	assert(buffer->fillCount >= 0);
}

extern bool TPCircularBufferWrite(TPCircularBuffer* buffer, const void* src, uint32_t len) {
	uint32_t space;
	TPCircularBufferData headData = TPCircularBufferHead(buffer, &space);
	if(space < len) return false;

	if(!headData.isDoubleBuffer) {
		assert(headData.lengthOfFirst >= len);
		memcpy(headData.first, src, len);
	} else {
		uint32_t r = len;
		uint32_t leftCopy = TPMin(r, headData.lengthOfFirst);
		if(leftCopy > 0) {
			memcpy(headData.first, src, leftCopy);
			r -= leftCopy;
		}
		uint32_t rightCopy = TPMin(r, headData.lengthOfSecond);
		if(rightCopy > 0) {
			memcpy(headData.second, (uint8_t*)src + leftCopy, rightCopy);
			r -= rightCopy;
		}
		assert(r == 0);
	}

	TPCircularBufferProduce(buffer, len);

	return(true);
}

extern bool TPCircularBufferRead(TPCircularBuffer* buffer, void* dst, const uint32_t len) {
	uint32_t fillCount;
	TPCircularBufferData tail = TPCircularBufferTail(buffer, &fillCount);
	if(fillCount < len) return(false);

	if(!tail.isDoubleBuffer) {
		memcpy(dst, tail.first, len);
	} else {
		uint32_t x = len;
		uint32_t left = TPMin(x, tail.lengthOfFirst);
		if(left > 0) {
			assert((left % 4) == 0);
			memcpy((uint8_t*)dst, tail.first, left);
			x -= left;
		}
		uint32_t right = TPMin(x, tail.lengthOfSecond);
		if(right > 0) {
			assert((right % 4) == 0);
			memcpy((uint8_t*)dst + left, tail.second, right);
			x -= right;
		}
		assert(x == 0);
	}

	TPCircularBufferConsume(buffer, len);
}

extern void TPCircularBufferClear(TPCircularBuffer* buffer) {
	uint32_t fillCount;
	TPCircularBufferTail(buffer, &fillCount);
	if(fillCount > 0) {
		TPCircularBufferConsume(buffer, fillCount);
	}
}

void assertBytes(const uint8_t* data, const uint8_t test, const uint32_t offset, const uint32_t len) {
	for(uint32_t i = 0; i < len; ++i) {
		uint32_t p = offset + i;
		assert(data[p] == test);
	}
}

extern void TPCircularBufferUnitTest() {
	TPCircularBuffer buffer;
	bool res = TPCircularBufferInit(&buffer, 128, true);
	assert(res);

	// Validate initial buffer
	assert(buffer.length == 128);
	assert(buffer.head == 0);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 0);
	assert(!buffer.isMirror);

	// Validate initial head
	uint32_t writeAvailable = 0;
	TPCircularBufferData head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 128);
	assert(!head.isDoubleBuffer);
	assert(head.first == buffer.buffer);
	assert(head.lengthOfFirst == buffer.length);

	// Validate initial tail
	uint32_t readAvailable = 0;
	TPCircularBufferData tail = TPCircularBufferTail(&buffer, &readAvailable);
	assert(readAvailable == 0);
	assert(!tail.isDoubleBuffer);
	assert(tail.first == NULL);
	assert(tail.lengthOfFirst == 0);

	uint8_t data[1024];

	// Write 32-bytes 0xAA
	memset(data, 0xAA, 1024);
	res = TPCircularBufferWrite(&buffer, &data, 32);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);

	// Validate buffer (0xAA)
	assert(buffer.head == 32);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 32);

	// Validate head (0xAA)
	head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 32 + 64);
	assert(!head.isDoubleBuffer);
	assert(head.first == (uint8_t*)buffer.buffer + 32);
	assert(head.lengthOfFirst == (buffer.length - 32));

	// Write 64-bytes 0xBB
	memset(data, 0xBB, 1024);
	res = TPCircularBufferWrite(&buffer, &data, 64);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);
	assertBytes(buffer.buffer, 0xBB, 32, 64);

	// Validate buffer (0xBB)
	assert(buffer.head == (32 + 64));
	assert(buffer.tail == 0);
	assert(buffer.fillCount == (32 + 64));

	// Validate head (0xBB)
	head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 32);
	assert(!head.isDoubleBuffer);
	assert(head.first == (uint8_t*)buffer.buffer + 32 + 64);
	assert(head.lengthOfFirst == (buffer.length - 32 - 64));

	// Write 16-bytes 0xCC
	memset(data, 0xCC, 1024);
	res = TPCircularBufferWrite(&buffer, &data, 16);
	assert(res);
	assertBytes(buffer.buffer, 0xAA, 0, 32);
	assertBytes(buffer.buffer, 0xBB, 32, 64);
	assertBytes(buffer.buffer, 0xCC, 32 + 64, 16);

	// Validate buffer (0xCC)
	assert(buffer.head == (32 + 64 + 16));
	assert(buffer.tail == 0);
	assert(buffer.fillCount == (32 + 64 + 16));

	// Validate head (0xCC)
	head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 16);
	assert(!head.isDoubleBuffer);
	assert(head.first == (uint8_t*)buffer.buffer + 32 + 64 + 16);
	assert(head.lengthOfFirst == (buffer.length - 32 - 64 - 16));

	// Try to write 32-bytes 0xDD
	memset(data, 0xCC, 1024);
	res = TPCircularBufferWrite(&buffer, &data, 32);
	assert(!res);

	// Validate tail (96 bytes available)
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	assert(readAvailable == 32 + 64 + 16);
	assert(!tail.isDoubleBuffer);
	assert(tail.first == buffer.buffer);
	assert(tail.lengthOfFirst == 32 + 64 + 16);

	// Validate buffer
	assert(buffer.head == 64 + 32 + 16);
	assert(buffer.tail == 0);
	assert(buffer.fillCount == 64 + 32 + 16);

	// Consume 16 bytes
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	TPCircularBufferConsume(&buffer, 16);

	// Validate tail (96 bytes available)
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	assert(readAvailable == 96);
	assert(!tail.isDoubleBuffer);
	assert(tail.first == (uint8_t*)buffer.buffer + 16);
	assert(tail.lengthOfFirst == 96);

	// Write 32-bytes 0xDD
	memset(data, 0xDD, 1024);
	res = TPCircularBufferWrite(&buffer, &data, 32);
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
	head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 0);
	assert(!head.isDoubleBuffer);
	assert(head.first == NULL);
	assert(head.lengthOfFirst == 0);

	// Validate tail (128 bytes available)
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	assert(readAvailable == 128);
	assert(tail.isDoubleBuffer);
	assert(tail.first == (uint8_t*)buffer.buffer + 16);
	assert(tail.lengthOfFirst == 32 + 64 + 16);
	assert(tail.lengthOfSecond == 16);

	// Consume 64 bytes
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	TPCircularBufferConsume(&buffer, 64);

	// Validate head (64 bytes available)
	head = TPCircularBufferHead(&buffer, &writeAvailable);
	assert(writeAvailable == 64);
	assert(!head.isDoubleBuffer);
	assert(head.first == (uint8_t*)buffer.buffer + 16);
	assert(head.lengthOfFirst == 64);

	// Validate tail (64 bytes available)
	tail = TPCircularBufferTail(&buffer, &readAvailable);
	assert(readAvailable == 64);
	assert(tail.isDoubleBuffer);
	assert(tail.first == (uint8_t*)buffer.buffer + 16 + 64);
	assert(tail.lengthOfFirst == 32 + 16);
	assert(tail.lengthOfSecond == 16);

	TPCircularBufferCleanup(&buffer);
}

#elif __APPLE__

#include <mach/mach.h>

#define reportResult(result,operation) (_reportResult((result),(operation),strrchr(__FILE__, '/')+1,__LINE__))
static inline bool _reportResult(kern_return_t result, const char* operation, const char* file, int line) {
	if(result != ERR_SUCCESS) {
		printf("%s:%d: %s: %s\n", file, line, operation, mach_error_string(result));
		return false;
	}
	return true;
}

extern bool _TPCircularBufferInit(TPCircularBuffer* buffer, uint32_t length, size_t structSize) {

	assert(length > 0);

	if(structSize != sizeof(TPCircularBuffer)) {
		fprintf(stderr, "TPCircularBuffer: Header version mismatch. Check for old versions of TPCircularBuffer in your project\n");
		abort();
	}

	// Keep trying until we get our buffer, needed to handle race conditions
	int retries = 3;
	while(true) {

		buffer->length = (uint32_t)round_page(length);    // We need whole page sizes

		// Temporarily allocate twice the length, so we have the contiguous address space to
		// support a second instance of the buffer directly after
		vm_address_t bufferAddress;
		kern_return_t result = vm_allocate(mach_task_self(),
			&bufferAddress,
			buffer->length * 2,
			VM_FLAGS_ANYWHERE); // allocate anywhere it'll fit
		if(result != ERR_SUCCESS) {
			if(retries-- == 0) {
				reportResult(result, "Buffer allocation");
				return false;
			}
			// Try again if we fail
			continue;
		}

		// Now replace the second half of the allocation with a virtual copy of the first half. Deallocate the second half...
		result = vm_deallocate(mach_task_self(),
			bufferAddress + buffer->length,
			buffer->length);
		if(result != ERR_SUCCESS) {
			if(retries-- == 0) {
				reportResult(result, "Buffer deallocation");
				return false;
			}
			// If this fails somehow, deallocate the whole region and try again
			vm_deallocate(mach_task_self(), bufferAddress, buffer->length);
			continue;
		}

		// Re-map the buffer to the address space immediately after the buffer
		vm_address_t virtualAddress = bufferAddress + buffer->length;
		vm_prot_t cur_prot, max_prot;
		result = vm_remap(mach_task_self(),
			&virtualAddress,   // mirror target
			buffer->length,    // size of mirror
			0,                 // auto alignment
			0,                 // force remapping to virtualAddress
			mach_task_self(),  // same task
			bufferAddress,     // mirror source
			0,                 // MAP READ-WRITE, NOT COPY
			&cur_prot,         // unused protection struct
			&max_prot,         // unused protection struct
			VM_INHERIT_DEFAULT);
		if(result != ERR_SUCCESS) {
			if(retries-- == 0) {
				reportResult(result, "Remap buffer memory");
				return false;
			}
			// If this remap failed, we hit a race condition, so deallocate and try again
			vm_deallocate(mach_task_self(), bufferAddress, buffer->length);
			continue;
		}

		if(virtualAddress != bufferAddress + buffer->length) {
			// If the memory is not contiguous, clean up both allocated buffers and try again
			if(retries-- == 0) {
				printf("Couldn't map buffer memory to end of buffer\n");
				return false;
			}

			vm_deallocate(mach_task_self(), virtualAddress, buffer->length);
			vm_deallocate(mach_task_self(), bufferAddress, buffer->length);
			continue;
		}

		buffer->buffer = (void*)bufferAddress;
		buffer->fillCount = 0;
		buffer->head = buffer->tail = 0;
		buffer->atomic = true;

		return true;
	}
	return false;
}

extern void TPCircularBufferCleanup(TPCircularBuffer* buffer) {
	vm_deallocate(mach_task_self(), (vm_address_t)buffer->buffer, buffer->length * 2);
	memset(buffer, 0, sizeof(TPCircularBuffer));
}

#elif __linux__
#elif __unix__
#elif defined(_POSIX_VERSION)
#else
#   error "Unknown compiler"
#endif
