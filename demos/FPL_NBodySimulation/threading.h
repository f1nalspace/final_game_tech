#ifndef THREADING_H
#define THREADING_H

#define FPL_NO_PLATFORM_INCLUDES
#include <final_platform_layer.h>

#include <assert.h>
#include <functional>
#include <deque> // @TODO(final): Replace std::deque

// @TODO(final): Allow non-lambda functions as well
typedef std::function<void(const size_t startIndex, const size_t endIndex, const float deltaTime)> thread_pool_task_function;

struct ThreadPoolTask {
	size_t startIndex;
	size_t endIndex;
	float deltaTime;
	uint8_t padding0[4];
	thread_pool_task_function func;
};

constexpr size_t MAX_THREADPOOL_THREAD_COUNT = 128;
struct ThreadPoolState {
	fplThreadHandle *threads[MAX_THREADPOOL_THREAD_COUNT];
	size_t threadCount;
	fplMutexHandle queueMutex;
	fplConditionVariable queueCondition;
	std::deque<ThreadPoolTask> queue;
	volatile uint64_t pendingCount;
	volatile uint64_t queuedCount;
	volatile int stopped;
};

inline void ThreadPoolWorkerThreadProc(const fplThreadHandle *thread, void *data) {
	ThreadPoolState *state = static_cast<ThreadPoolState *>(data);
	ThreadPoolTask task;
	while(true) {
		fplMutexLock(&state->queueMutex);

		while(fplAtomicLoadU64(&state->queuedCount) == 0 && !state->stopped) {
			fplConditionWait(&state->queueCondition, &state->queueMutex, FPL_TIMEOUT_INFINITE);
		}

		if(state->stopped) {
			fplMutexUnlock(&state->queueMutex);
			break;
		}

		task = state->queue.front();
		state->queue.pop_front();
		fplAtomicFetchAndAddU64(&state->queuedCount, -1);
		fplMutexUnlock(&state->queueMutex);

		task.func(task.startIndex, task.endIndex, task.deltaTime);
		fplAtomicFetchAndAddU64(&state->pendingCount, -1);
	}
}

class ThreadPool {
private:
	ThreadPoolState _state;
public:
	ThreadPool(const size_t threadCount) {
		_state = {};
		_state.threadCount = fplMax(fplMin(threadCount, MAX_THREADPOOL_THREAD_COUNT), 1);
		fplMutexInit(&_state.queueMutex);
		fplConditionInit(&_state.queueCondition);
		for(size_t workerIndex = 0; workerIndex < _state.threadCount; ++workerIndex) {
			_state.threads[workerIndex] = fplThreadCreate(ThreadPoolWorkerThreadProc, &_state);
		}
	}
	ThreadPool():
		ThreadPool(ThreadPool::GetConcurrencyThreadCount()) {
	}
	~ThreadPool() {
		fplMutexLock(&_state.queueMutex);
		_state.stopped = 1;
		_state.pendingCount = 0;
		_state.queuedCount = 0;
		_state.queue.clear();
		fplMutexUnlock(&_state.queueMutex);
		fplConditionBroadcast(&_state.queueCondition);

		fplThreadWaitForAll(_state.threads, _state.threadCount, 0, FPL_TIMEOUT_INFINITE);

		fplConditionDestroy(&_state.queueCondition);
		fplMutexDestroy(&_state.queueMutex);
		_state = {};
	}

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool &operator=(const ThreadPool &) = delete;
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool &operator=(ThreadPool &&) = delete;

	inline void AddTask(const ThreadPoolTask &task) {
		_state.queue.push_back(task);
		fplAtomicFetchAndAddU64(&_state.queuedCount, 1);
	}

	inline void WaitUntilDone() {
		fplAssert(_state.queueMutex.isValid);
		fplMutexLock(&_state.queueMutex);
		fplConditionBroadcast(&_state.queueCondition);
		fplMutexUnlock(&_state.queueMutex);
		while(fplAtomicLoadU64(&_state.pendingCount) > 0) {
			fplThreadYield();
		}
	}

	inline void CreateTasks(const size_t itemCount, const thread_pool_task_function &func, const float deltaTime) {
		if(itemCount == 0) return;

		const size_t threads_size = _state.threadCount;
		const size_t itemsPerTask = fplMax((size_t)1, itemCount / threads_size);

		fplMutexLock(&_state.queueMutex);
		size_t tasks_added = 0;
		for(size_t itemIndex = 0; itemIndex < itemCount; itemIndex += itemsPerTask, ++tasks_added) {
			ThreadPoolTask task = {};
			task.func = func;
			task.deltaTime = deltaTime;
			task.startIndex = itemIndex;
			task.endIndex = std::min(itemIndex + itemsPerTask - 1, itemCount - 1);
			AddTask(task);
		}
		fplAtomicFetchAndAddU64(&_state.pendingCount, tasks_added);
		fplMutexUnlock(&_state.queueMutex);
	}

	inline size_t GetThreadCount() {
		return _state.threadCount;
	}

	static size_t GetConcurrencyThreadCount() {
		size_t count = fplCPUGetCoreCount();
		return fplMax(count, 1);
	}
};

#endif