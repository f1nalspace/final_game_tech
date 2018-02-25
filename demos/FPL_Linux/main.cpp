#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.hpp>

#include <assert.h>

struct ThreadData {
    int num;
    int sleepFor;
};

static void SingleThreadProc(const fpl::threading::ThreadContext &context, void *data) {
    ThreadData *d = (ThreadData *)data;
    fpl::console::ConsoleFormatOut("Sleep in thread %d for %d ms\n", d->num, d->sleepFor);
    fpl::threading::ThreadSleep(d->sleepFor);
}

static void SimpleMultiThreadTest(const uint32_t threadCount) {
    ThreadData threadData[fpl::common::MAX_THREAD_COUNT] = {};
    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        threadData[threadIndex].num = threadIndex + 1;
        threadData[threadIndex].sleepFor = (1 + threadIndex) * 500;
    }
    fpl::threading::ThreadContext *threads[fpl::common::MAX_THREAD_COUNT];
    fpl::console::ConsoleFormatOut("Start %d threads\n", threadCount);
    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        threads[threadIndex] = fpl::threading::ThreadCreate(SingleThreadProc, &threadData[threadIndex]);
    }
    fpl::console::ConsoleFormatOut("Wait all %d threads for exit\n", threadCount);
    fpl::threading::ThreadWaitForAll(threads, threadCount);
    fpl::console::ConsoleFormatOut("All %d threads are done\n", threadCount);
    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        assert(threads[threadIndex]->currentState == fpl::threading::ThreadState::Stopped);                
    }
    fpl::console::ConsoleFormatOut("Destroy %d threads\n", threadCount);
    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        fpl::threading::ThreadDestroy(threads[threadIndex]);
    }
}

struct SharedThreadData {
    fpl::threading::ThreadMutex mutex;
    fpl::threading::ThreadSignal signal;  
};

struct AwaitThreadData {
    ThreadData base;
    SharedThreadData *shared;
};

static void ThreadSlaveProc(const fpl::threading::ThreadContext &context, void *data) {
    AwaitThreadData *d = (AwaitThreadData *)data;

    fpl::console::ConsoleFormatOut("Slave-Thread %d waits for signal\n", d->base.num);
    fpl::threading::SignalWaitForOne(d->shared->mutex, d->shared->signal);
    fpl::console::ConsoleFormatOut("Got signal on Slave-Thread %d\n", d->base.num);
    
    fpl::console::ConsoleFormatOut("Slave-Thread %d is done\n", d->base.num);
}

static void ThreadMasterProc(const fpl::threading::ThreadContext &context, void *data) {
    AwaitThreadData *d = (AwaitThreadData *)data;
    fpl::console::ConsoleFormatOut("Master-Thread %d waits for 5 seconds\n", d->base.num);
    fpl::threading::ThreadSleep(5000);
    
    fpl::console::ConsoleFormatOut("Master-Thread %d sets signal\n", d->base.num);
    fpl::threading::MutexLock(d->shared->mutex);
    fpl::threading::SignalSet(d->shared->signal);
    fpl::threading::MutexUnlock(d->shared->mutex);
    
    fpl::console::ConsoleFormatOut("Master-Thread %d is done\n", d->base.num);
}

static void ConditionThreadTest(const uint32_t threadCount) {
    fpl::console::ConsoleFormatOut("Condition test for %d\n", threadCount);
    
    SharedThreadData shared = {};
    shared.mutex = fpl::threading::MutexCreate();
    shared.signal = fpl::threading::SignalCreate();
    
    AwaitThreadData datas[fpl::common::MAX_THREAD_COUNT] = {};
    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        datas[threadIndex].base.num = 1 + threadIndex;
        datas[threadIndex].shared = &shared;
    }
    
    fpl::threading::ThreadContext *threads[fpl::common::MAX_THREAD_COUNT];
    for (int32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        if (threadIndex == ((int32_t)threadCount - 1)) {
            threads[threadIndex] = fpl::threading::ThreadCreate(ThreadMasterProc, &datas[threadIndex]);
        } else {
            threads[threadIndex] = fpl::threading::ThreadCreate(ThreadSlaveProc, &datas[threadIndex]);
        }
    }
    
    fpl::threading::ThreadWaitForAll(threads, threadCount);
    
    fpl::threading::SignalDestroy(shared.signal);
    fpl::threading::MutexDestroy(shared.mutex);
}

int main(int argc, char **) {
    if (fpl::InitPlatform(fpl::InitFlags::All)) {
        fpl::console::ConsoleOut("Hello Linux!\n");
            
        // Memory test
        {		
            fpl::console::ConsoleOut("Allocate memory of 1024\n");
            size_t size = 1024;
            void *mem1024 = fpl::memory::MemoryAllocate(size);

            size_t storedMemSize = *(size_t *)((uint8_t *)mem1024 - sizeof(uintptr_t) - sizeof(size_t));
            fpl::console::ConsoleFormatOut("Stored size: %llu\n", storedMemSize);
            assert(storedMemSize == (size + sizeof(uintptr_t) + sizeof(size_t)));

            fpl::console::ConsoleOut("Fill memory of 1024\n");
            uint32_t *mem1024_32 = (uint32_t *)mem1024;
            for (int i = 0; i < (1024 / sizeof(uint32_t)); ++i) {
                *mem1024_32++ = i * i;
            }
            fpl::console::ConsoleOut("Free memory of 1024\n");
            fpl::memory::MemoryFree(mem1024);
        }
        
        // Hardware test
        {
            uint32_t cpuCount = fpl::hardware::GetProcessorCoreCount();
            fpl::console::ConsoleFormatOut("CPU core Count: %llu\n", cpuCount);
            assert(cpuCount > 0);
            
            char cpuNameBuffer[2048];
            char *cpuName = fpl::hardware::GetProcessorName(cpuNameBuffer, FPL_ARRAYCOUNT(cpuNameBuffer));
            assert(fpl::strings::GetAnsiStringLength(cpuName) > 0);
            fpl::console::ConsoleFormatOut("CPU name: %s\n", cpuName);
        }

        // Atomics test
        {
            volatile uint32_t value = 3;
            uint32_t addend = 11;
            fpl::console::ConsoleFormatOut("AtomicAddU32: %llu -> %llu", value, addend);
            uint32_t oldValue = fpl::atomics::AtomicAddU32(&value, addend);
            fpl::console::ConsoleFormatOut(" -> %llu, %llu\n", oldValue, value);
            assert(oldValue == 3);
            assert(value == 14);
        }
        {
            volatile uint64_t value = 3;
            uint64_t exchange = 42;
            uint64_t comparand = 3;
            fpl::console::ConsoleFormatOut("AtomicAndCompareExchangeU64: %llu to %llu when %llu", value, exchange, comparand);
            uint64_t oldValue = fpl::atomics::AtomicCompareAndExchangeU64(&value, comparand, exchange);
            fpl::console::ConsoleFormatOut(" -> %llu, %llu\n", oldValue, value);
            assert(oldValue == 3);
            assert(value == 42);
        }

        // Timings test
        {
            double t1 = fpl::timings::GetHighResolutionTimeInSeconds();
            fpl::threading::ThreadSleep(3000);
            double t2 = fpl::timings::GetHighResolutionTimeInSeconds();
            double delta = t2 - t1;
            assert(delta >= 3.0);
            fpl::console::ConsoleFormatOut("Timing for 3000 ms sleep (High res): %f\n", delta);
            
            uint64_t l1 = fpl::timings::GetTimeInMilliseconds();
            fpl::threading::ThreadSleep(1500);
            uint64_t l2 = fpl::timings::GetTimeInMilliseconds();
            int64_t delta2 = l2 - l1;
            assert(delta2 >= 1500);
            fpl::console::ConsoleFormatOut("Timing for 1500 ms sleep: %d\n", delta2);
        }

        // Library test
        {
            const char* libpthreadFileNames[] = {
            "libpthread.so",
            "libpthread.so.0",
            "libpthread.dylib"
            };
            
            fpl::library::DynamicLibraryHandle pthreadLibraryHandle;
            const char *usedLibraryName = nullptr;
            for (size_t i = 0; i < FPL_ARRAYCOUNT(libpthreadFileNames); ++i) {
                pthreadLibraryHandle = fpl::library::DynamicLibraryLoad(libpthreadFileNames[i]);
                if (pthreadLibraryHandle.isValid) {
                    usedLibraryName = libpthreadFileNames[i];
                    break;
                }
            }
            if (pthreadLibraryHandle.isValid) {
                void *createFunc = fpl::library::GetDynamicLibraryProc(pthreadLibraryHandle, "pthread_create");
                assert(createFunc != nullptr);
                fpl::console::ConsoleFormatOut("Successfully loaded pthread from '%s'\n", usedLibraryName);
                fpl::library::DynamicLibraryUnload(pthreadLibraryHandle);
            } else {
                fpl::console::ConsoleFormatOut("Failed loading pthread library!\n");
            }
        }
        
        // Single threading test
        {
            ThreadData threadData = {};
            threadData.num = 1;
            threadData.sleepFor = 3000;
            fpl::console::ConsoleFormatOut("Start thread %d\n", threadData.num);
            fpl::threading::ThreadContext *thread = fpl::threading::ThreadCreate(SingleThreadProc, &threadData);
            fpl::console::ConsoleFormatOut("Wait thread for exit\n");
            fpl::threading::ThreadWaitForOne(thread);
            fpl::console::ConsoleFormatOut("Thread is done\n");
            assert(thread->currentState == fpl::threading::ThreadState::Stopped);
            fpl::threading::ThreadDestroy(thread);
        }
        
        // Multi threads test
        {
            SimpleMultiThreadTest(2);
            SimpleMultiThreadTest(3);
            SimpleMultiThreadTest(4);
            uint32_t coreCount = fpl::hardware::GetProcessorCoreCount();
            int32_t threadCountForCores = coreCount - 1;
            SimpleMultiThreadTest(threadCountForCores);
        }
        
        // Condition and lock tests
        {
            ConditionThreadTest(2);
            ConditionThreadTest(4);
        }

        fpl::ReleasePlatform();
        
        fpl::console::ConsoleFormatOut("Done\n");
        return 0;
    } else {
        return -1;
    }
}
