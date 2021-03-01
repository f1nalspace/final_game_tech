//
//  TPCircularBuffer.h
//  Circular/Ring buffer implementation
//
//  https://github.com/michaeltyson/TPCircularBuffer
//
//  Created by Michael Tyson on 10/12/2011.
//
//
//  This implementation makes use of a virtual memory mapping technique that inserts a virtual copy
//  of the buffer memory directly after the buffer's end, negating the need for any buffer wrap-around
//  logic. Clients can simply use the returned memory address as if it were contiguous space.
//  
//  The implementation is thread-safe in the case of a single producer and single consumer.
//
//  Virtual memory technique originally proposed by Philip Howard (http://vrb.slashusr.org/), and
//  adapted to Darwin by Kurt Revis (http://www.snoize.com,
//  http://www.snoize.com/Code/PlayBufferedSoundFile.tar.gz)
//
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

#ifndef TPCircularBuffer_h
#define TPCircularBuffer_h

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#if defined(_WIN32)

#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif
struct IUnknown;
#include <windows.h>

#endif



#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
#if defined(_WIN32)
		uint8_t filePadding[56];
		void* fileHandle;
#endif

		uint8_t bufferPadding[56];
		void* buffer;

		uint8_t lengthPadding[60];
		uint32_t length;

		uint8_t tailPadding[60];
		uint32_t tail;

		uint8_t headPadding[60];
		uint32_t head;

		uint8_t fillCountPadding[60];
		volatile int32_t fillCount;

		uint8_t flagsPadding[60];
		int32_t isMirror;
	} TPCircularBuffer;

	typedef struct {
		uint8_t* first;
		uint8_t* second;
		uint32_t lengthOfFirst;
		uint32_t lengthOfSecond;
		int32_t isDoubleBuffer;
		int32_t padding;
	} TPCircularBufferData;
	
	extern bool TPCircularBufferInit(TPCircularBuffer* buffer, uint32_t length, bool allowMirror);

	extern void TPCircularBufferCleanup(TPCircularBuffer* buffer);

	extern void TPCircularBufferClear(TPCircularBuffer* buffer);

	extern bool TPCircularBufferCanRead(TPCircularBuffer* buffer, uint32_t* availableBytes);

	extern bool TPCircularBufferCanWrite(TPCircularBuffer* buffer, uint32_t* availableBytes);

	extern bool TPCircularBufferWrite(TPCircularBuffer* buffer, const void* src, uint32_t len);

	extern bool TPCircularBufferRead(TPCircularBuffer* buffer, void* dst, const uint32_t len);

	extern void TPCircularBufferClear(TPCircularBuffer* buffer);

	extern void TPCircularBufferUnitTest();

#ifdef __cplusplus
}
#endif

#endif
