#define FPL_IMPLEMENTATION
#include <final_platform_layer.hpp>

#include <assert.h>

int main(int argc, char **) {
	if (fpl::InitPlatform(fpl::InitFlags::None)) {
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
		
		fpl::ReleasePlatform();
	}
	return 0;
}
