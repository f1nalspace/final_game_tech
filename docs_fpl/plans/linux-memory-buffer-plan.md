I wrote a memory mirror buffer `demos/additions/final_buffer.h` that i use reguraly for audio sample processing.
The current implementation only supports the platform "Windows" using memory tricks of (VirtualAlloc3, MapViewOfFile3) that allows a memory block to be mirrored N-times - on OS-Level.
As a fallback, when mirroring is not support it does everything manually.

Now i am on linux and i want the same feature: mirror a memory buffer using OS and memory techniques with Linux equivalents.

I created stubs for the implementation already, please fill this out:
- f_InitMemoryMirrorLinux
- f_ReleaseMemoryMirrorLinux

Add types for linux to the LockFreeRingBuffer right at the very top inside the `#elif defined(FPL_PLATFORM_LINUX)` block.
Read the windows implementation f_InitMemoryMirrorWin32 and f_ReleaseMemoryMirrorWin32 as a reference.

Please ignore LockFreeRingBufferUnitTest() function, as this requires a rewrite and i don't want to do that now.

