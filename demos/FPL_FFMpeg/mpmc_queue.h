#pragma once

#include <stdint.h>
#include <limits.h>

//
// Lock-Free Multiple Procuder Multiple Consumer Queue
// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
//
constexpr size_t CACHE_LINE_SIZE = 64;
typedef char CacheLinePad[CACHE_LINE_SIZE];

template <typename T>
struct MPMCBoundedQueueCell {
	volatile uint64_t sequence;
	T data;
};

template <typename T>
struct MPMCBoundedQueue {
	CacheLinePad pad0;
	MPMCBoundedQueueCell<T> *buffer;
	size_t bufferMask;
	CacheLinePad pad1;
	volatile uint64_t enqueuePos;
	CacheLinePad pad2;
	volatile uint64_t dequeuePos;
	CacheLinePad pad3;

	static MPMCBoundedQueue<T> Create(size_t capacity) {
		size_t bufferCount = capacity;
		MPMCBoundedQueue<T> result = {};
		result.buffer = (MPMCBoundedQueueCell<T> *)memory::MemoryAlignedAllocate(sizeof(MPMCBoundedQueueCell<T>) * bufferCount, CACHE_LINE_SIZE);
		result.bufferMask = bufferCount - 1;
		assert((bufferCount >= 2) && ((bufferCount & (bufferCount - 1)) == 0));
		for (size_t i = 0; i < bufferCount; i += 1) {
			result.buffer[i].sequence = i;
		}
		result.enqueuePos = 0;
		result.dequeuePos = 0;
		return(result);
	}

	static void Destroy(MPMCBoundedQueue<T> &queue) {
		memory::MemoryAlignedFree(queue.buffer);
		queue = {};
	}
};

template <typename T>
static bool IsEmpty(MPMCBoundedQueue<T> &queue) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.dequeuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
		if (dif == 0) {
			break;
		} else if (dif < 0)
			return true;
		else
			pos = AtomicLoadU64(&queue.dequeuePos);
	}
	return false;
}

template <typename T>
static bool Enqueue(MPMCBoundedQueue<T> &queue, const T &data) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.enqueuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)pos;
		if (dif == 0) {
			if (IsAtomicCompareAndExchangeU64(&queue.enqueuePos, pos, pos + 1)) {
				break;
			}
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.enqueuePos);
	}
	cell->data = data;
	AtomicStoreU64(&cell->sequence, pos + 1);
	return true;
}

template <typename T>
static bool Dequeue(MPMCBoundedQueue<T> &queue, T &data) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.dequeuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
		if (dif == 0) {
			if (IsAtomicCompareAndExchangeU64(&queue.dequeuePos, pos, pos + 1)) {
				break;
			}
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.dequeuePos);
	}
	data = cell->data;
	AtomicStoreU64(&cell->sequence, pos + queue.bufferMask + 1);
	return true;
}

template <typename T>
static T &PeekReadable(MPMCBoundedQueue<T> &queue, int32_t offset = 1) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.dequeuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)(pos + offset);
		if (dif == 0) {
			break;
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.dequeuePos);
	}
	data = cell->data;
	return true;
}

template <typename T>
static T &PeekWritable(MPMCBoundedQueue<T> &queue, int32_t offset = 1) {
	MPMCBoundedQueueCell<T> *cell;
	uint64_t pos = AtomicLoadU64(&queue.enqueuePos);
	for (;;) {
		cell = &queue.buffer[pos & queue.bufferMask];
		uint64_t seq = AtomicLoadU64(&cell->sequence);
		intptr_t dif = (intptr_t)seq - (intptr_t)(pos + offset);
		if (dif == 0) {
			break;
		} else if (dif < 0)
			return false;
		else
			pos = AtomicLoadU64(&queue.enqueuePos);
	}
	data = cell->data;
	return true;
}

template <typename T>
static T &PeekPrev(MPMCBoundedQueue<T> &queue) {
	T &result = Peek(queue, data, -1);
	return(result);
}

template <typename T>
static T &PeekNext(MPMCBoundedQueue<T> &queue) {
	T &result = Peek(queue, data, 1);
	return(result);
}