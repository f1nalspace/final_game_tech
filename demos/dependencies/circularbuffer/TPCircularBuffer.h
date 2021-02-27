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

#if defined(_MSC_VER)
#	define tp_force_inline __forceinline
#   define atomicFetchAdd(value, addend) InterlockedExchangeAdd((volatile LONG *)(value), (addend))
#   define atomicRead(value) InterlockedCompareExchange((volatile LONG *)(value), 0, 0)
#elif defined(__GNUC__) || defined(__clang__)
#	define tp_force_inline __attribute__((__always_inline__)) inline
#   define atomicFetchAdd(value, addend) __sync_fetch_and_add((volatile uint32_t *)(value), (addend))
#   define atomicRead(value) __sync_add_and_fetch((volatile uint32_t *)(value), 0)
#else
#	error "Unsupported compiler/platform"
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
    void  *buffer;

    uint8_t lengthPadding[60];
    uint32_t length;

    uint8_t tailPadding[60];
    volatile uint32_t tail;

    uint8_t headPadding[60];
    volatile uint32_t head;

    uint8_t fillCountPadding[60];
    volatile int32_t fillCount;
} TPCircularBuffer;

/*!
 * Initialise buffer
 *
 *  Note that the length is advisory only: Because of the way the
 *  memory mirroring technique works, the true buffer length will
 *  be multiples of the device page size (e.g. 4096 bytes)
 *
 *  If you intend to use the AudioBufferList utilities, you should
 *  always allocate a bit more space than you need for pure audio
 *  data, so there's room for the metadata. How much extra is required
 *  depends on how many AudioBufferList structures are used, which is
 *  a function of how many audio frames each buffer holds. A good rule
 *  of thumb is to add 15%, or at least another 2048 bytes or so.
 *
 * @param buffer Circular buffer
 * @param length Length of buffer
 */
#define TPCircularBufferInit(buffer, length) \
    _TPCircularBufferInit(buffer, length, sizeof(*buffer))
extern bool _TPCircularBufferInit(TPCircularBuffer *buffer, uint32_t length, size_t structSize);

/*!
 * Cleanup buffer
 *
 *  Releases buffer resources.
 */
extern void TPCircularBufferCleanup(TPCircularBuffer *buffer);

/*!
 * Clear buffer
 *
 *  Resets buffer to original, empty state.
 *
 *  This is safe for use by consumer while producer is accessing 
 *  buffer.
 */
extern void TPCircularBufferClear(TPCircularBuffer *buffer);
    
// Reading (consuming)

/*!
 * Access end of buffer
 *
 *  This gives you a pointer to the end of the buffer, ready
 *  for reading, and the number of available bytes to read.
 *
 * @param buffer Circular buffer
 * @param availableBytes On output, the number of bytes ready for reading
 * @return Pointer to the first bytes ready for reading, or NULL if buffer is empty
 */
inline void* TPCircularBufferTail(TPCircularBuffer *buffer, uint32_t* availableBytes) {
    *availableBytes = atomicRead(&buffer->fillCount);
    if ( *availableBytes == 0 ) return NULL;
    return (void*)((char*)buffer->buffer + buffer->tail);
}

/*!
 * Consume bytes in buffer
 *
 *  This frees up the just-read bytes, ready for writing again.
 *
 * @param buffer Circular buffer
 * @param amount Number of bytes to consume
 */
inline void TPCircularBufferConsume(TPCircularBuffer *buffer, uint32_t amount) {
    buffer->tail = (buffer->tail + amount) % buffer->length;
    atomicFetchAdd(&buffer->fillCount, -(int)amount);
    assert(buffer->fillCount >= 0);
}

/*!
 * Access front of buffer
 *
 *  This gives you a pointer to the front of the buffer, ready
 *  for writing, and the number of available bytes to write.
 *
 * @param buffer Circular buffer
 * @param availableBytes On output, the number of bytes ready for writing
 * @return Pointer to the first bytes ready for writing, or NULL if buffer is full
 */
inline void* TPCircularBufferHead(TPCircularBuffer *buffer, uint32_t* availableBytes) {
    *availableBytes = (buffer->length - atomicRead(&buffer->fillCount));
    if ( *availableBytes == 0 ) return NULL;
    return (void*)((char*)buffer->buffer + buffer->head);
}
    
// Writing (producing)

/*!
 * Produce bytes in buffer
 *
 *  This marks the given section of the buffer ready for reading.
 *
 * @param buffer Circular buffer
 * @param amount Number of bytes to produce
 */
inline void TPCircularBufferProduce(TPCircularBuffer *buffer, uint32_t amount) {
    buffer->head = (buffer->head + amount) % buffer->length;
    atomicFetchAdd(&buffer->fillCount, (int)amount);
    assert(buffer->fillCount <= (int32_t)buffer->length);
}

/*!
 * Helper routine to copy bytes to buffer
 *
 *  This copies the given bytes to the buffer, and marks them ready for reading.
 *
 * @param buffer Circular buffer
 * @param src Source buffer
 * @param len Number of bytes in source buffer
 * @return true if bytes copied, false if there was insufficient space
 */
inline bool TPCircularBufferProduceBytes(TPCircularBuffer *buffer, const void* src, uint32_t len) {
    uint32_t space;
    void *ptr = TPCircularBufferHead(buffer, &space);
    if ( space < len ) return false;
    memcpy(ptr, src, len);
    TPCircularBufferProduce(buffer, len);
    return true;
}

inline void TPCircularBufferClear(TPCircularBuffer* buffer) {
	uint32_t fillCount;
	if(TPCircularBufferTail(buffer, &fillCount)) {
		TPCircularBufferConsume(buffer, fillCount);
	}
}

#ifdef __cplusplus
}
#endif

#endif
