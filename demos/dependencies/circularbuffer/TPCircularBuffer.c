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


extern bool _TPCircularBufferInit(TPCircularBuffer* buffer, uint32_t length, size_t structSize) {
	assert(length > 0);

	if(structSize != sizeof(TPCircularBuffer)) {
		fprintf(stderr, "TPCircularBuffer: Header version mismatch. Check for old versions of TPCircularBuffer in your project\n");
		abort();
	}

	// Get length in multiple of page-sizes
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	uint32_t roundedSize = (length + sysInfo.dwPageSize - 1) / sysInfo.dwPageSize * sysInfo.dwPageSize;

	uint8_t* blockAddress;
	HANDLE fileHandle;

	// Keep trying until we get our buffer, needed to handle race conditions
	int retries = 3;
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
		if((MapViewOfFileEx(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, roundedSize, blockAddress) == blockAddress) ||
		   (MapViewOfFileEx(fileHandle, FILE_MAP_ALL_ACCESS, 0, 0, roundedSize, (blockAddress + roundedSize))) == (blockAddress + roundedSize)) {
			// Success, we can use the blockAddress as our base-ptr
			break;
		}

		// Failed cleanup and try again
		CloseHandle(fileHandle);
		fileHandle = NULL;
		blockAddress = NULL;
	}

	if(blockAddress != NULL) {
		ZeroMemory(buffer, sizeof(TPCircularBuffer));
		buffer->length = roundedSize;
		buffer->buffer = (void*)blockAddress;
		buffer->fillCount = 0;
		buffer->head = buffer->tail = 0;
		buffer->fileHandle = fileHandle;
		return(true);
	} else {
		return(false);
	}
}

extern void TPCircularBufferCleanup(TPCircularBuffer* buffer) {
	if(buffer->buffer != NULL) {
		UnmapViewOfFile(buffer->buffer);
		UnmapViewOfFile((uint8_t *)buffer->buffer + buffer->length);
		VirtualFree(buffer->buffer, buffer->length * 2, MEM_RELEASE);
	}
	if(buffer->fileHandle != NULL) {
		CloseHandle(buffer->fileHandle);
	}
	ZeroMemory(buffer, sizeof(TPCircularBuffer));
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