#define FPL_IMPLEMENTATION
#define FPL_NO_WINDOW
#define FPL_NO_VIDEO
#define FPL_NO_AUDIO
#include <final_platform_layer.hpp>

#include <assert.h>

int main(int argc, char **) {
	fpl::console::ConsoleOut("Hello Linux!\n");
		
	// Memory test
	{		
		fpl::console::ConsoleOut("Allocate memory of 1024\n");
		fpl_size size = 1024;
		void *mem1024 = fpl::memory::MemoryAllocate(size);

		fpl_size storedMemSize = *(fpl_size *)((fpl_u8 *)mem1024 - sizeof(fpl_uintptr) - sizeof(fpl_size));
		fpl::console::ConsoleFormatOut("Stored size: %llu\n", storedMemSize);
		assert(storedMemSize == (size + sizeof(fpl_uintptr) + sizeof(fpl_size)));

		fpl::console::ConsoleOut("Fill memory of 1024\n");
		fpl_u32 *mem1024_32 = (fpl_u32 *)mem1024;
		for (int i = 0; i < (1024 / sizeof(fpl_u32)); ++i) {
			*mem1024_32++ = i * i;
		}
		fpl::console::ConsoleOut("Free memory of 1024\n");
		fpl::memory::MemoryFree(mem1024);
	}

	// Atomics test
	{
		volatile fpl_u32 value = 3;
		fpl_u32 addend = 11;
		fpl::console::ConsoleFormatOut("AtomicAddU32: %llu -> %llu", value, addend);
		fpl_u32 oldValue = fpl::atomics::AtomicAddU32(&value, addend);
		fpl::console::ConsoleFormatOut(" -> %llu, %llu\n", oldValue, value);
		assert(oldValue == 3);
		assert(value == 14);
	}
	{
		volatile fpl_u64 value = 3;
		fpl_u64 exchange = 42;
		fpl_u64 comparand = 3;
		fpl::console::ConsoleFormatOut("AtomicAndCompareExchangeU64: %llu to %llu when %llu", value, exchange, comparand);
		fpl_u64 oldValue = fpl::atomics::AtomicCompareAndExchangeU64(&value, comparand, exchange);
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
		fpl::console::ConsoleFormatOut("Sleep for 3 seconds: %f\n", delta);
	}

	// Library test
	{
		const char* libpthreadFileNames[] = {
        "libpthread.so",
        "libpthread.so.0",
        "libpthread.dylib"
		};
		
		fpl::library::DynamicLibraryHandle pthreadLibraryHandle;
		const char *usedLibraryName = fpl_null;
		for (size_t i = 0; i < FPL_ARRAYCOUNT(libpthreadFileNames); ++i) {
			pthreadLibraryHandle = fpl::library::DynamicLibraryLoad(libpthreadFileNames[i]);
			if (pthreadLibraryHandle.isValid) {
				usedLibraryName = libpthreadFileNames[i];
				break;
			}
		}
		if (pthreadLibraryHandle.isValid) {
			void *createFunc = fpl::library::GetDynamicLibraryProc(pthreadLibraryHandle, "pthread_create");
			assert(createFunc != fpl_null);
			fpl::console::ConsoleFormatOut("Successfully loaded pthread from '%s'\n", usedLibraryName);
			fpl::library::DynamicLibraryUnload(pthreadLibraryHandle);
		} else {
			fpl::console::ConsoleFormatOut("Failed loading pthread library!\n");
		}
	}

	// Platform test
	fpl::console::ConsoleFormatOut("Platform test\n");
	if (fpl::InitPlatform(fpl::InitFlags::None)) {
		fpl::ReleasePlatform();
	}
	
	fpl::console::ConsoleFormatOut("Done\n");
	return 0;
}
