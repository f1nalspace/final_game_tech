/*

░▒▓████████▓▒░▒▓█▓▒░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░              ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓██████████████▓▒░░▒▓████████▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓██████▓▒░ ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓████████▓▒░▒▓█▓▒░             ░▒▓█▓▒▒▓███▓▒░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓██████▓▒░        ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░  
░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░             ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ 
░▒▓█▓▒░      ░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓████████▓▒░       ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓████████▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░ 

Final Gamebox version 1.0

-------------------------------------------------------------------------------
	About
-------------------------------------------------------------------------------

Final Gamebox is a single-header-file gameboy emulator written in C99.

This was created for learning purposes and supports features such as:
- Emulation of the original gameboy (DMG) SoC including:
	- CPU (Central Processing Unit): Z80/8080 like CPU with the entire (2x256) instruction set
	- PPU (Picture Processing Unit): Special GPU for rendering tiles/pixels, including implementation of the Pixel-FIFO mentioned in the "Ultimate Game Boy Talk".
	- APU (Audio Processing Unit): Generating audio samples for 4 Voices (Two Pulse Voices, One general PCM/Wave Voice, One LSFR Noise Voice)
- MMU (Memory Management Unit): Chip that manages any memory read/write operations
- Boot ROM support for DMG
- GamePak loading with support for ROM and RAM banking (MBC1, MBC2, MBC3, MBC5)
- Battery backed external RAM load/save
- Storing and restoring of full save states for filebased roms
- Joypad support (Keyboard, Gamepad)

It can run most test rom files and games such as Tetris, Super Mario land, Kirby, Zelda, etc.
Some games won't work at all or are totally broken, e.g. Duck Tales :-(

All knowledge used in this source is based on tutorials, documentations and the awesome "Ultimate Game Boy Talk".

Even though there is a lot of documentations out there, this project was very hard and it took me months to complete it.
But it was worth it and i learned a lot of from it ;-)

The only dependencies are built-in operating system libraries and a C99 compliant compiler or a C++ compiler that supports fixed bit fields, designated initializers, anonymous structs/unions.

-------------------------------------------------------------------------------
	Author
-------------------------------------------------------------------------------

- Torsten Spaete (alias Finalspace)
- Professional application developer
- 30+ years of programming experience
- Data visualization, Software-Architecture, Multimedia & Physics & Game development

-------------------------------------------------------------------------------
	Known Issues
-------------------------------------------------------------------------------

- Snapshots are disabled on non 64-bit platforms, due to pointer alignment differences
- Audio timing is not always correct, some games plays sound too fast
- The graphics timing for some games are wrong and it may flicker a lot (e.g. Duck Tales)
- Input handling are sometimes broken (e.g. Duck Tales, Alley Way)
- The tiles are not always correct in the background map (See: Add option to select background tile area)
- For some games, the background map scroll X and Y is reset to zero after h/v blank, so we can't show the scroll area properly

-------------------------------------------------------------------------------
	Todo
-------------------------------------------------------------------------------

	- Rename fgbMonochromeColors to fgbMonochromePalette and remove old fgbMonochromePalette
	- Fix dissassembly listbox resize breaks scroll position
	- Show rom/ram banks in UI
	- Gameboy Color support
	- Implement STOP properly
	- Implement HALT bug correctly (some games actually use this and still work fine)

-------------------------------------------------------------------------------
	Resources
-------------------------------------------------------------------------------

Explanations (Watch this first, if you want to get started):

	The Ultimate Game Boy Talk (33c3):
	https://www.youtube.com/watch?v=HyzD8pNlpwI

Documentations:

	Pan Docs:
	https://gbdev.io/pandocs/
	https://github.com/AntonioND/giibiiadvance/blob/master/docs/other_docs/pandocs.txt
	http://problemkaputt.de/pandocs.htm

	// The Cycle-Accurate Game Boy Docs
	https://raw.githubusercontent.com/geaz/emu-gameboy/master/docs/The%20Cycle-Accurate%20Game%20Boy%20Docs.pdf

	// Gameboy: Complete Technical Reference
	https://gekkio.fi/files/gb-docs/gbctr.pdf

	// Wiki entries
	https://gbdev.gg8.se/wiki/articles/Main_Page
	https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM
	https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware

	// Sound
	https://github.com/AntonioND/giibiiadvance/blob/master/docs/other_docs/GBSOUND.txt

Op-Codes:

	// gbops, A op-code table with the ability to search (best table):
	https://izik1.github.io/gbops/index.html

	// Partraiser table of Gameboy CPU (LR35902) instruction set:
	https://www.pastraiser.com/cpu/gameboy/gameboy_opcodes.html

	// JSON generated table based on partraiser table, including important details such as data-length:
	https://github.com/lmmendes/game-boy-opcodes

	// Short summary:
	http://gameboy.mongenel.com/dmg/opcodes.html

Test-Roms:

	// Blargg's Test Roms:
	https://github.com/retrio/gb-test-roms

	// Awesome list of test roms with links and screenshots for different emulators, so you can compare easiely:
	https://daid.github.io/GBEmulatorShootout/

	// Tons of links to test rom repositories
	https://github.com/c-sp/game-boy-test-roms

Tutorials:

	Lazy Stripes:
	https://blog.tigris.fr/2019/07/09/writing-an-emulator-the-first-steps/

	q00.gb:
	https://emudev.de/gameboy-emulator/overview/

	Cinoop:
	https://cturt.github.io/cinoop.html

	Low Level Devel:
	https://www.youtube.com/watch?v=e87qKixKFME&list=PLVxiWMqQvhg_yk4qy2cSC3457wZJga_e5

	Code Slinger:
	http://www.codeslinger.co.uk/pages/projects/gameboy/beginning.html

Community:
	https://forums.nesdev.org/viewtopic.php?t=16174
	https://gbdev.gg8.se/forums/viewtopic.php?id=771

-------------------------------------------------------------------------------
	License
-------------------------------------------------------------------------------

MIT License
Copyright 2024-2026 Torsten Spaete

-------------------------------------------------------------------------------
	Changelog
-------------------------------------------------------------------------------

## v1.0 Initial version

*/

#ifndef FGB_HEADER
#define FGB_HEADER

// ****************************************************************************
// Standard headers without CRT
// ****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>

// ****************************************************************************
// Configuration
// ****************************************************************************
#if defined(_WIN32) || defined(__CYGWIN__)
#	ifdef __GNUC__
#		define FGB_DLLEXPORT __attribute__ ((dllexport))
#		define FGB_DLLIMPORT __attribute__ ((dllimport))
#	else
#		define FGB_DLLEXPORT __declspec(dllexport)
#		define FGB_DLLIMPORT __declspec(dllimport)
#	endif
#	define FGB_DLLLOCAL
#else
#	if __GNUC__ >= 4
#		define FGB_DLLIMPORT __attribute__((visibility("default")))
#		define FGB_DLLEXPORT __attribute__((visibility("default")))
#		define FGB_DLLLOCAL __attribute__((visibility("hidden")))
#	else
#		define FGB_DLLIMPORT
#		define FGB_DLLEXPORT
#		define FGB_DLLLOCAL
#	endif
#endif

#if defined(FGB_PRIVATE)
#define FGB_API static
#elif defined(FGB_LIBRARY_EXPORT)
#define FGB_API FGB_DLLEXPORT
#elif defined(FGB_LIBRARY_IMPORT)
#define FGB_API FGB_DLLIMPORT
#else
#define FGB_API extern
#endif

// ****************************************************************************
// Architecture
// ****************************************************************************
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#define FGB_X64
#define FGB_LITTLE_ENDIAN
#define FGB_64BIT
#elif defined(__i386__) || defined(_M_IX86) || defined(__X86__) || defined(_X86_)
#define FGB_X86
#define FGB_LITTLE_ENDIAN
#define FGB_32BIT
#elif defined(__aarch64__) || defined(_M_ARM64)
#define FGB_ARM64
#define FGB_BIG_ENDIAN
#define FGB_64BIT
#else
#error "Unsupported Architecture, only X86/X64 or ARM64 is supported"
#endif

// Endianess limitation (for now)
#if !defined(FGB_LITTLE_ENDIAN)
#error "This endianess are not supported yet!"
#endif

// ****************************************************************************
// Constants
// ****************************************************************************

// LCD Width in pixels
#define FGB_DISPLAY_WIDTH 160

// LCD Height in pixels
#define FGB_DISPLAY_HEIGHT 144

// Tick Cycles per Second (Hz)
#define FGB_MAX_CPU_CYCLES 4194304U

// Tick Cycles per Frame (Hz)
#define FGB_CPU_CYCLES_PER_FRAME 70368U

// LCD Refresh Rate (59.7 Hz)
#define FGB_DISPLAY_REFRESH_RATE (FGB_MAX_CPU_CYCLES / (float)FGB_CPU_CYCLES_PER_FRAME)

// ****************************************************************************
// Macros
// ****************************************************************************

// Get count from a fixed sized array
#define FGB_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// Get minumum of two values
#define FGB_MIN(a, b) ((a) < (b) ? (a) : (b))

// Get maximum of two values
#define FGB_MAX(a, b) ((a) > (b) ? (a) : (b))

// Create a 32-bit fourcc value
#define FGB_FOURCC(a, b, c, d) (((a) << 0) | ((b) << 8) | ((c) << 16) | ((d) << 24))

// Gets the total number of enum values.
#define FGB_ENUM_VALUE_COUNT(enumType) (((enumType)_Last - (enumType)_First) + 1)

// ****************************************************************************
// Platform Detection
// 
// Only required due to lock-free audio samples buffering
// ****************************************************************************
#ifndef FGB_DISABLE_PLATFORM_DETECTION

#if defined(_WIN32) || defined(_MSC_VER)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define FGB_PLATFORM_WINDOWS
#endif // _WIN32

#endif // FGB_DISABLE_PLATFORM_DETECTION

// ****************************************************************************
// String/Memory
// ****************************************************************************
#if !defined(FGB_STRLEN)
#include <string.h>
#include <stdio.h>
#define FGB_STRLEN(str) (((str) == NULL) ? 0 : strlen(str))
#define FGB_STRINGFORMAT(buffer, bufferSize, format, ...) vsnprintf(buffer, bufferSize, format, __VA_ARGS__)
#define FGB_MEMSET(ptr, value, size) memset(ptr, value, size)
#define FGB_MEMCOPY(dst, src, size) memcpy(dst, src, size)
#else

#if !defined(FGB_STRINGFORMAT)
#error "FGB_STRINGFORMAT was not implemented"
#endif

#if !defined(FGB_MEMSET)
#error "FGB_MEMSET was not implemented"
#endif

#if !defined(FGB_MEMCOPY)
#error "FGB_MEMCOPY was not implemented"
#endif

#endif

// ****************************************************************************
// Assertions
// ****************************************************************************
#ifndef FGB_ASSERT
#include <assert.h>
#define FGB_ASSERT(exp) assert(exp)
#endif

#ifndef FGB_STATIC_ASSERT
#define FGB__M_STATICASSERT_0(exp, line, counter) int fpl__ct_assert_##line_##counter(int ct_assert_failed[(exp)?1:-1])
#define FGB__M_STATIC_ASSERT(exp) FGB__M_STATICASSERT_0(exp, __LINE__, __COUNTER__)
#define FGB_STATIC_ASSERT(exp) FGB__M_STATIC_ASSERT(exp)
#endif

// ****************************************************************************
// Atomics
// ****************************************************************************
#if !defined(FGB_INTERLOCKED_EXCHANGE_64)

#if defined(FGB_PLATFORM_WINDOWS)
#define FGB_INTERLOCKED_EXCHANGE_64(storage, value) InterlockedExchange64((volatile LONG64 *)(storage), (LONG64)(value))
#define FGB_INTERLOCKED_EXCHANGE_ADD_64(storage, addend) InterlockedExchangeAdd64((volatile LONG64 *)(storage), (LONG64)(addend))
#define FGB_INTERLOCKED_LOAD_64(storage) InterlockedOr64((volatile LONG64 *)(storage), 0LL)
#else
#error "Please implement FGB_INTERLOCKED_EXCHANGE_64/FGB_INTERLOCKED_EXCHANGE_ADD_64/FGB_INTERLOCKED_LOAD_64"
#endif

#else
#if !defined(FGB_INTERLOCKED_EXCHANGE_ADD_64)
#error "FGB_INTERLOCKED_EXCHANGE_ADD_64 was not implemented"
#endif

#if !defined(FGB_INTERLOCKED_LOAD_64)
#error "FGB_INTERLOCKED_LOAD_64 was not implemented"
#endif
#endif

// ****************************************************************************
// Time
// ****************************************************************************
#if !defined(FGB_CURRENT_TICKS)

#if defined(FGB_PLATFORM_WINDOWS)
#define FGB_CURRENT_TICKS() GetTickCount64()
#else
#error "Please implement FGB_CURRENT_TICKS"
#endif

#endif

// ****************************************************************************
// Callbacks API
// ****************************************************************************

// Stores a UTC timestamp and the milliseconds to represent a date and time
typedef struct {
	// UTC Unix Timestamp in seconds
	uint64_t epoch;
	// Milliseconds in range of 0..999
	uint64_t milliseconds;
} fgbDateTime;

// Defines a opaque file handle
typedef void *fgbFileHandle;

// Defines file types used for export/import
typedef enum {
	// None
	fgbFileType_None = 0,
	// External RAM
	fgbFileType_ExternalRAM,
	// Snapshot
	fgbFileType_Snapshot,
} fgbFileType;

// Function prototype for creating a binary file
#define FGB_CREATE_FILE_CALLBACK(name) bool name(const char *filePath, fgbFileHandle *fileHandle)
/**
 * @brief Callback for creating a file at the specified path
 * @param filePath The path of the file to be created
 * @param fileHandle A pointer to the file handle that will be used for the created file
 * @return true if the file was created successfully, false otherwise
 */
typedef FGB_CREATE_FILE_CALLBACK(fgbCreateFileCallback);

// Function prototype for opening a binary file
#define FGB_OPEN_FILE_CALLBACK(name) bool name(const char *filePath, fgbFileHandle *fileHandle)
/**
 * @brief Callback for opening a file at the specified path
 * @param filePath The path of the file to be opened
 * @param fileHandle A pointer to the file handle that will be used for the opened file
 * @return true if the file was opened successfully, false otherwise
 */
typedef FGB_OPEN_FILE_CALLBACK(fgbOpenFileCallback);

// Function prototype for closing a file
#define FGB_CLOSE_FILE_CALLBACK(name) void name(fgbFileHandle *fileHandle)
/**
 * @brief Callback for closing the specified file
 * @param fileHandle A pointer to the file handle that will be closed
 */
typedef FGB_CLOSE_FILE_CALLBACK(fgbCloseFileCallback);

// Function prototype for reading from a file into a buffer
#define FGB_READ_FILE_BUFFER_CALLBACK(name) size_t name(fgbFileHandle *fileHandle, void *outBuffer, const size_t maxBufferLen, const size_t readSize)
/**
 * @brief Callback for reading from a file into a buffer
 * @param fileHandle A pointer to the file handle from which data will be read
 * @param outBuffer A pointer to the buffer where the read data will be stored
 * @param maxBufferLen The maximum length of the buffer
 * @param readSize The number of bytes to read from the file
 * @return The number of bytes actually read
 */
typedef FGB_READ_FILE_BUFFER_CALLBACK(fgbReadFileBufferCallback);

// Function prototype for writing data to a file from a buffer
#define FGB_WRITE_FILE_BUFFER_CALLBACK(name) size_t name(fgbFileHandle *fileHandle, const void *inBuffer, const size_t writeSize)
/**
 * @brief Callback for writing data to a file from a buffer
 * @param fileHandle A pointer to the file handle to which data will be written
 * @param inBuffer A pointer to the buffer containing the data to be written
 * @param writeSize The number of bytes to write to the file
 * @return The number of bytes actually written
 */
typedef FGB_WRITE_FILE_BUFFER_CALLBACK(fgbWriteFileBufferCallback);

// Function prototype for flushing the specified file
#define FGB_FLUSH_FILE_CALLBACK(name) void name(fgbFileHandle *fileHandle)
/**
 * @brief Callback for flushing the specified file
 * @param fileHandle A pointer to the file handle that will be flushed
 */
typedef FGB_FLUSH_FILE_CALLBACK(fgbFlushFileCallback);

// Function prototype for getting the size of a file
#define FGB_GET_FILE_SIZE_CALLBACK(name) size_t name(fgbFileHandle *fileHandle)
/**
 * @brief Callback for getting the size of a file
 * @param fileHandle A pointer to the file handle for which the size will be retrieved
 * @return The size of the file in bytes
 */
typedef FGB_GET_FILE_SIZE_CALLBACK(fgbGetFileSizeCallback);

// Function prototype for building a file path
#define FGB_BUILD_FILEPATH_CALLBACK(name) bool name(const char *filePath, const char *folderPath, const fgbFileType fileType, char *outBuffer, const size_t maxBufferLen, const uint8_t slotIndex)
/**
 * @brief Callback for building a file path
 * @param filePath The base path for the file
 * @param folderPath The folder path to be used
 * @param fileType The type of the file being created
 * @param outBuffer A pointer to the buffer where the constructed file path will be stored
 * @param maxBufferLen The maximum length of the output buffer
 * @param slotIndex The index of the slot for the file
 * @return true if the file path was built successfully, false otherwise
 */
typedef FGB_BUILD_FILEPATH_CALLBACK(fgbBuildFilePathCallback);

// Function prototype for allocating memory
#define FGB_ALLOCATE_MEMORY_CALLBACK(name) void *name(const size_t size, void *userData)
/**
 * @brief Callback for allocating memory
 * @param size The size of the memory block to allocate
 * @param userData A pointer to user-defined data that can be used during allocation
 * @return A pointer to the allocated memory, or NULL if the allocation failed
 */
typedef FGB_ALLOCATE_MEMORY_CALLBACK(fgbAllocateMemoryCallback);

// Function prototype for freeing allocated memory
#define FGB_FREE_MEMORY_CALLBACK(name) void name(void* ptr, void *userData)
/**
 * @brief Callback for freeing allocated memory
 * @param ptr A pointer to the memory block to be freed
 * @param userData A pointer to user-defined data that can be used during deallocation
 */
typedef FGB_FREE_MEMORY_CALLBACK(fgbFreeMemoryCallback);

// Function prototype for querying the current date and time
#define FGB_DATETIME_QUERY(name) fgbDateTime name(void)
/**
 * @brief Callback for querying the current date and time
 * @return The current date and time as an fgbDateTime structure
 */
typedef FGB_DATETIME_QUERY(fgbDateTimeQueryCallback);

// Stores the callbacks for UI or path building that is offloaded to the frontend
typedef struct {
	// Callback for creating a binary file
	fgbCreateFileCallback *createFile;
	// Callback for opening a binary file
	fgbOpenFileCallback *openFile;
	// Callback for closing a file
	fgbCloseFileCallback *closeFile;
	// Callback for reading from a file into a buffer
	fgbReadFileBufferCallback *readFile;
	// Callback for writing to a file from a buffer
	fgbWriteFileBufferCallback *writeFile;
	// Callback for flushing the file buffer
	fgbFlushFileCallback *flushFile;
	// Callback for getting the file size
	fgbGetFileSizeCallback *getFileSize;
	// Callback for building a file path
	fgbBuildFilePathCallback *buildFilePath;
	// Callback for allocating dynamic memory
	fgbAllocateMemoryCallback *allocateMemory;
	// Callback for freeing dynamic memory
	fgbFreeMemoryCallback *freeMemory;
	// Callback for query the current date and time
	fgbDateTimeQueryCallback *dateTimeQuery;
	// User data for the allocateMemory and freeMemory callbacks
	void *memoryAllocationUserData;
} fgbCallbacks;

// ****************************************************************************
// Cacheline Detection
// ****************************************************************************

// Detect cache line sizes based on CPU architecture
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define FGB_CACHELINE_SIZE 64
#elif defined(__ARM_ARCH_2__) || defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__) || defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
    #define FGB_CACHELINE_SIZE 32
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define FGB_CACHELINE_SIZE 64
#elif defined(__powerpc__) || defined(__powerpc64__) || defined(__ppc__) || defined(__ppc64__)
    #define FGB_CACHELINE_SIZE 64
#elif defined(__mips__) || defined(__mips64)
    #define FGB_CACHELINE_SIZE 64
#elif defined(__sparc__) || defined(__sparc64__)
    #define FGB_CACHELINE_SIZE 64
#else
    #define FGB_CACHELINE_SIZE 64 // Default to 64 bytes if architecture is unknown
#endif

// Represents an empty container that fits exactly in one cache line
typedef struct {
	// The unused data
	uint8_t data[FGB_CACHELINE_SIZE];
} fgbCacheline;

// ****************************************************************************
// Common types and structures
// ****************************************************************************

// Stores a memory block with a fixed length
typedef struct {
	// Pointer to the start of the memory
	uint8_t *data;
	// Length of the memory in bytes
	size_t length;
} fgbMemory;
FGB_STATIC_ASSERT(sizeof(fgbMemory) % 4 == 0);

// ****************************************************************************
// Game Pak API
// ****************************************************************************

// ROM size in bytes per bank
#define FGB_ROM_SIZE_PER_BANK 0x4000

// Minumum number of ROM banks
#define FGB_MIN_ROM_BANK_COUNT 2

// Minumum number of ROM banks
#define FGB_MAX_ROM_BANK_COUNT 128

// Minimum size of a game pak ROM
#define FGB_MIN_GAMEPAK_SIZE (FGB_ROM_SIZE_PER_BANK * FGB_MIN_ROM_BANK_COUNT)

// Maximum size of a game pak ROM (2 MB largest ROM)
#define FGB_MAX_GAMEPAK_SIZE (FGB_ROM_SIZE_PER_BANK * FGB_MAX_ROM_BANK_COUNT)

// RAM size in bytes per bank
#define FGB_RAM_SIZE_PER_BANK 0x2000

// Minumum number of RAM banks
#define FGB_MIN_RAM_BANK_COUNT 1

// Maximum number of ROM banks
#define FGB_MAX_RAM_BANK_COUNT 16

// Minimum size of a game pak external RAM
#define FGB_MIN_EXTERNAL_RAM_SIZE (FGB_RAM_SIZE_PER_BANK * FGB_MIN_RAM_BANK_COUNT)

// Maximum size of a game pak external RAM (256 KB largest RAM)
#define FGB_MAX_EXTERNAL_RAM_SIZE (FGB_RAM_SIZE_PER_BANK * FGB_MAX_RAM_BANK_COUNT)

// Stores a fixed amount of ROM data
typedef union {
	// Full data array
	uint8_t m[FGB_MAX_GAMEPAK_SIZE + sizeof(size_t)];
	// Anonymous struct holding the data and the size
	struct {
		// Size of the ROM
		size_t length;
		// The entire ROM of the gamepak
		uint8_t data[FGB_MAX_GAMEPAK_SIZE];
	};
} fgbReadOnlyMemory;

// Represents the external RAM (up to 128 KB) in a game pak
typedef union {
	// Full data array
	uint8_t m[FGB_MAX_EXTERNAL_RAM_SIZE + sizeof(size_t) + sizeof(uint64_t) + sizeof(bool) * 8];
	// Anonymous struct holding the external ram and some states
	struct {
		// Fixed amount of memory + size
		struct {
			// Size of the RAM
			size_t length;
			// The entire RAM of the gamepak
			uint8_t data[FGB_MAX_EXTERNAL_RAM_SIZE];
		} memory;
        // Last saved ticks in milliseconds
        uint64_t lastSaveTime;
        // A flag indicating whether the external RAM was modified or not
        bool isDirty;
        // A flag indicating whether a request to save the external RAM was made or not
        bool requestSave;
		// Padding to align to 8 bytes
		bool padding[6];
	};
} fgbExternalRAM;

// Types of game pak combinations that is read from ROM position 0x147, that get maps into feature flags later.
typedef enum {
	// ROM Only
	fgbGamePakType_ROM = 0x00,
	// MBC1
	fgbGamePakType_MBC1 = 0x01,
	// MBC1 + RAM
	fgbGamePakType_MBC1_RAM = 0x02,
	// MBC1 + RAM + Battery
	fgbGamePakType_MBC1_RAM_BATTERY = 0x03,
	// MBC2
	fgbGamePakType_MBC2 = 0x05,
	// MBC2 + Battery
	fgbGamePakType_MBC2_BATTERY = 0x06,
	// ROM + Battery
	fgbGamePakType_ROM_BATTERY = 0x08,
	// ROM + RAM + Battery
	fgbGamePakType_ROM_RAM_BATTERY = 0x09,
	// MMM01
	fgbGamePakType_MMM01 = 0x0B,
	// MMM01 + RAM
	fgbGamePakType_MMM01_RAM = 0x0C,
	// MMM01 + RAM + Battery
	fgbGamePakType_MMM01_RAM_BATTERY = 0x0D,
	// MBC3 + Timer + Battery
	fgbGamePakType_MBC3_TIMER_BATTERY = 0x0F,
	// MBC3 + Timer + RAM + Battery
	fgbGamePakType_MBC3_TIMER_RAM_BATTERY = 0x10,
	// MBC3
	fgbGamePakType_MBC3 = 0x11,
	// MBC3 + RAM
	fgbGamePakType_MBC3_RAM = 0x12,
	// MBC3 + RAM + Battery
	fgbGamePakType_MBC3_RAM_BATTERY = 0x13,
	// MBC5
	fgbGamePakType_MBC5 = 0x19,
	// MBC5 + RAM
	fgbGamePakType_MBC5_RAM = 0x1A,
	// MBC5 + RAM + Battery
	fgbGamePakType_MBC5_RAM_BATTERY = 0x1B,
	// MBC5 + Rumble
	fgbGamePakType_MBC5_RUMBLE = 0x1C,
	// MBC5 + Rumble + RAM
	fgbGamePakType_MBC5_RUMBLE_RAM = 0x1D,
	// MBC5 + Rumble + RAM + Battery
	fgbGamePakType_MBC5_RUMBLE_RAM_BATTERY = 0x1E,
	// MBC6
	fgbGamePakType_MBC6 = 0x20,
	// MBC7 + Sensor + Rumble + RAM + Battery
	fgbGamePakType_MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
	// Pocket Camera
	fgbGamePakType_POCKET_CAMERA = 0xFC,
	// Bandai TAMA5
	fgbGamePakType_BANDAI_TAMA5 = 0xFD,
	// HUC3
	fgbGamePakType_HUC3 = 0xFE,
	// HUC3 + RAM + Battery
	fgbGamePakType_HUC1_RAM_BATTERY = 0xFF,
} fgbGamePakType;

// Defines the types of memory controllers
typedef enum {
	// Basic ROM controller
	fgbMemoryControllerType_ROM = 0,
	// Memory bank controller 1
	fgbMemoryControllerType_MBC1,
	// Memory bank controller 2
	fgbMemoryControllerType_MBC2,
	// MMM 01 controller
	fgbMemoryControllerType_MMM01,
	// Memory bank controller 3
	fgbMemoryControllerType_MBC3,
	// Memory bank controller 5
	fgbMemoryControllerType_MBC5,
	// Memory bank controller 6
	fgbMemoryControllerType_MBC6,
	// Memory bank controller 7
	fgbMemoryControllerType_MBC7,
	// HUC controller 1
	fgbMemoryControllerType_HUC1,
	// HUC controller 3
	fgbMemoryControllerType_HUC3,

	// First memory controller type
	fgbMemoryControllerType_First = fgbMemoryControllerType_ROM,
	// Last memory controller type
	fgbMemoryControllerType_Last = fgbMemoryControllerType_HUC3,
} fgbMemoryControllerType;

// Defines the ROM size types that is read from 0x148
typedef enum {
	// 2 Banks: 32 KB
	fgbRomSizeType_2_Banks_32KB = 0x00,
	// 4 Banks: 64 KB
	fgbRomSizeType_4_Banks_64KB = 0x01,
	// 8 Banks: 128 KB
	fgbRomSizeType_8_Banks_128KB = 0x02,
	// 16 Banks: 256 KB
	fgbRomSizeType_16_Banks_256KB = 0x03,
	// 32 Banks: 512 KB
	fgbRomSizeType_32_Banks_512KB = 0x04,
	// 64 Banks: 1024 KB
	fgbRomSizeType_64_Banks_1024KB = 0x05,
	// 128 Banks: 2 MB
	fgbRomSizeType_128_Banks_2048KB = 0x06,
	// 256 Banks: 4 MB
	fgbRomSizeType_256_Banks_4098KB = 0x07,
	// 512 Banks: 8 MB
	fgbRomSizeType_512_Banks_8192KB = 0x08,
	// 72 Banks: 1152 KB
	fgbRomSizeType_72_Banks_1152KB = 0x52,
	// 80 Banks: 1280 KB
	fgbRomSizeType_80_Banks_1280KB = 0x53,
	// 96 Banks: 1536 KB
	fgbRomSizeType_96_Banks_1536KB = 0x54,
} fgbRomSizeType;

// Defines the external RAM size types that is read from 0x149
typedef enum {
	// No external RAM
	fgbRamSizeType_NoRam = 0x00,
	// Unused
	fgbRamSizeType_Unused = 0x01,
	// 1 Banks: 8 KB
	fgbRamSizeType_1_Banks_8KB = 0x02,
	// 4 Banks: 32 KB
	fgbRamSizeType_4_Banks_32KB = 0x03,
	// 16 Banks: 128 KB
	fgbRamSizeType_16_Banks_128KB = 0x04,
	// 8 Banks: 64 KB
	fgbRamSizeType_8_Banks_64KB = 0x05,
} fgbRamSizeType;

// Defines the destination code types that is read from 0x14A
typedef enum {
	// Destination is for japanese market
	fgbDestinationCodeType_Japanese = 0x00,
	// Destination is for non-japanese market
	fgbDestinationCodeType_NonJapanese = 0x01,
} fgbDestinationCodeType;

// Defines the core types
typedef enum {
	// Gameboy core
	fgbCoreType_DMG = 0,
	// Gameboy Color core
	fgbCoreType_CGB = 1,
	// Gameboy with color support core
	fgbCoreType_CGB_DMG = 2,
	// Super Gameboy core
	fgbCoreType_SGB = 3,

	// First core
	fgbCoreType_First = fgbCoreType_DMG,
	// Last core
	fgbCoreType_Last = fgbCoreType_SGB,
} fgbCoreType;

// Defines the game pak feature flags.
typedef enum {
	// No features
	fgbGamePakFeature_None = 0,
	// RAM feature
	fgbGamePakFeature_RAM = 1 << 0,
	// Battery feature
	fgbGamePakFeature_BATTERY = 1 << 1,
	// Timer feature
	fgbGamePakFeature_TIMER = 1 << 2,
	// Rumble feature
	fgbGamePakFeature_RUMBLE = 1 << 3,
	// Sensor feature
	fgbGamePakFeature_SENSOR = 1 << 4,
} fgbGamePakFeature;

// Stores the title of a game pak
typedef struct {
	// The title (max 16 characters + zero terminator)
	char text[32];
} fgbTitle;

// Stores the information for a game pak, such as title, ram/rom bank counts, checksums, features, etc.
typedef struct {
	// The title of the game
	fgbTitle title;
	// The core type
	fgbCoreType coreType;
	// The type of the game pak
	fgbGamePakType gamePakType;
	// The type of the rom size
	fgbRomSizeType romSizeType;
	// The type of the ram size
	fgbRamSizeType ramSizeType;
	// The memory controller type
	fgbMemoryControllerType mbcType;
	// The features of the game pak
	fgbGamePakFeature features;
	// The 16-bit checksum of the game pak header to compare the actual header against it
	uint16_t headerChecksum;
	// The 16-bit checksum of the rom data to compare the actual data against it
	uint16_t romChecksum;
	// The total number of ROM banks
	uint16_t romBankCount;
	// The total number of RAM banks
	uint16_t ramBankCount;
} fgbGamePakInfo;

// Represents an entire game pak, including the information, the file path, the ROM and the RAM
typedef struct {
	// The full ROM
	fgbReadOnlyMemory rom;
	// The external ram
	fgbExternalRAM ram;
	// The full path to the rom file
	char filePath[1024];
	// The game pak info
	fgbGamePakInfo info;
	// A value indicating whether the rom data checksum has been passed or not
	bool hasROMChecksumPassed;
	// A value indicating whether the license has been passed or not
	bool hasLicensePassed;
	// A value indicating whether the game pak is valid or not
	bool isValid;
	// Padding to align to 8-bytes
	bool padding[5];
} fgbGamePak;

// Defines the gampak load result types
typedef enum {
	// Successful load
	fgbGamePakLoadResultType_Success = 0,
	// Invalid arguments were provided
	fgbGamePakLoadResultType_InvalidArguments,
	// GamePak file not found
	fgbGamePakLoadResultType_FileNotFound,
	// File error occurred
	fgbGamePakLoadResultType_FileError,
	// Not enough data available
	fgbGamePakLoadResultType_NotEnoughData,
	// Memory error related to the file
	fgbGamePakLoadResultType_MemoryErrorFile,
	// Memory error related to the ROM
	fgbGamePakLoadResultType_MemoryErrorROM,
	// Memory error related to the RAM
	fgbGamePakLoadResultType_MemoryErrorRAM,
	// Mismatch in the license logo
	fgbGamePakLoadResultType_MismatchLicenseLogo,
	// Invalid header detected
	fgbGamePakLoadResultType_InvalidHeader,
	// Not supported format detected
	fgbGamePakLoadResultType_UnsupportedFormat,
} fgbGamePakLoadResultType;

/**
  * @brief Loads a game pak from the specified file path
  * @param callbacks Reference to the callbacks structure
  * @param filePath Full path string
  * @param outGamePak Reference to output game pak structure
  * @return The resulting enum value, indicating a success or a specific failure
  */
FGB_API fgbGamePakLoadResultType fgbGamePakLoadFromFile(const fgbCallbacks *callbacks, const char *filePath, fgbGamePak *outGamePak);

/**
  * @brief Loads the specified data with its size into a game pak structure
  * @param data Pointer to the memory block containing the data of the entire game pak
  * @param size Size of the memory block in bytes
  * @param outGamePak Reference to the output game pak structure
  * @return The resulting enum value, indicating a success or a specific failure
  */
FGB_API fgbGamePakLoadResultType fgbGamePakLoadFromMemory(const uint8_t *data, const size_t size, fgbGamePak *outGamePak);

/**
  * @brief Unloads the specified game pak
  * @param gamePak Reference to the gamepak structure
  */
FGB_API void fgbGamePakUnload(fgbGamePak *gamePak);

/**
  * @brief Gets the name of the specified core type
  * @param type The core type
  * @return The resulting null-terminated string
  */
FGB_API const char *fgbGetCoreTypeName(const fgbCoreType type);

/**
  * @brief Gets the name of the specified game pak type
  * @param type The game pak type
  * @return The resulting null-terminated string
  */
FGB_API const char *fgbGetGamePakTypeName(const fgbGamePakType type);

/**
  * @brief Gets the name of the specified gamepak load result type
  * @param type The game pak load result type
  * @return The resulting null-terminated string
  */
FGB_API const char *fgbGetGamePakLoadResultLabel(const fgbGamePakLoadResultType type);

/**
  * @brief Gets the name of the specified memory controller type
  * @param type The memory controller type
  * @return The resulting null-terminated string
  */
FGB_API const char *fgbGetMemoryControllerTypeName(const fgbMemoryControllerType type);

// Function prototype for reading a byte from a address by the current MBC
#define FGB_MBC_READ_FUNC(name) uint8_t name(struct fgbSystem *system, struct fgbMemoryBankController *mbc, const uint16_t address)
/**
  * @brief Callback for reading a byte from a address by the current MBC
  * @param system The reference to the FGB system structure
  * @param mbc The reference to the memory bank controller structure
  * @param address The 16-bit address
  * @return The read byte or 0xFF
  */
typedef FGB_MBC_READ_FUNC(fgb_mbc_read_func);

// Function prototype for writing a byte to a address by the current MBC
#define FGB_MBC_WRITE_FUNC(name) void name(struct fgbSystem *system, struct fgbMemoryBankController *mbc, const uint16_t address, const uint8_t value)
/**
  * @brief Callback for writing a byte to a address by the current MBC
  * @param system The reference to the FGB system structure
  * @param mbc The reference to the memory bank controller structure
  * @param address The 16-bit address
  * @param value The 8-bit value to by written
  */
typedef FGB_MBC_WRITE_FUNC(fgb_mbc_write_func);

// Represents the state for a MBC1
typedef struct {
	// Second ROM Bank in range of 1-127
	uint8_t romBank;
	// RAM Bank in range of 0-3
	uint8_t ramBank;
	// MODE (ROM / RAM)
	uint8_t mode;
	// Is RAM enabled
	bool isRAMEnabled;
} fgbMBC1;

// Represents the state for a MBC2
typedef struct {
	// Second ROM Bank in range of 1-16
	uint8_t romBank;
	// Only one RAM Bank, so a enabled flag is sufficient
	bool isRAMEnabled;
	// Padding to align to 4 bytes
	uint8_t padding[2];
} fgbMBC2;

// Represents the state for a MBC3
typedef struct {
	// Second ROM Bank in range of 1-127
	uint8_t romBank;
	// RAM Bank in range of 0-3 OR $08-$0C for RTC
	uint8_t ramBankOrRTCRegister;
	// RAM + RTC Register enabled
	bool isRAMAndRTCEnabled;
	// Padding to align to 4 bytes
	uint8_t padding;
} fgbMBC3;

// Represents the state for a MBC5
typedef struct {
	// Second ROM Bank in range of 0-480
	uint16_t romBank;
	// RAM Bank in range of 0-15
	uint8_t ramBank;
	// Is RAM enabled
	bool isRAMEnabled;
} fgbMBC5;

// Stores the current memory bank controller data
typedef union {
	// State for MBC1
	fgbMBC1 mbc1;
	// State for MBC2
	fgbMBC2 mbc2;
	// State for MBC3
	fgbMBC3 mbc3;
	// State for MBC5
	fgbMBC5 mbc5;
} fgbMBCData;

// Represents the full state of the current memory bank controller
typedef struct fgbMemoryBankController {
	// Callback to read from the MBC
	fgb_mbc_read_func *read;
	// Callback to write to the MBC
	fgb_mbc_write_func *write;
	// Data/State
	fgbMBCData data;
} fgbMemoryBankController;

// Defines the versions of the external RAM for serialization
typedef enum {
	// None
	fgbExternalRAMStateVersion_None = 0,
	// Initial version
	fgbExternalRAMStateVersion_Initial = 1,

	// First version
	fgbExternalRAMStateVersion_First = fgbExternalRAMStateVersion_Initial,
	// Latest version
	fgbExternalRAMStateVersion_Latest = fgbExternalRAMStateVersion_Initial,
} fgbExternalRAMStateVersion;

// Total count of external ram state versions
#define FGB_EXTERNAL_RAMSTATE_VERSION_COUNT (fgbExternalRAMStateVersion_Latest - fgbExternalRAMStateVersion_First) + 1

// External RAM State Magic Key
#define FGB_EXTERNAL_RAMSTATE_MAGIC_KEY (uint32_t)FGB_FOURCC('F', 'G', 'B', 'E')

#pragma pack(push,1)
// Represents the header for a external RAM State file
typedef struct {
	// 4-byte FourCC Magic code
	uint32_t magic;
	// Version
	fgbExternalRAMStateVersion version;
	// Memory bank controller type
	fgbMemoryControllerType controller;
	// Total number of RAM banks
	uint32_t bankCount;
} fgbExternalRAMStateHeader;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbExternalRAMStateHeader) == 16);

#pragma pack(push,1)
// NR10(FF10): Frequency Sweep Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0-2: Individual Step / Shift (3 BITS, Range of 0-7)
		uint8_t shift : 3;
		// BIT 3: Direction (0 = Increase, 1 = Decrease)
		bool isDecrease : 1;
		// BIT 4-6: Period (3 BITS, Range of 0-7)
		uint8_t period : 3;
		// BIT 7: Unused
		uint8_t unused : 1;
	};
} fgbFrequencySweepRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbFrequencySweepRegister) == 1);

#pragma pack(push,1)
// NR11/NR21/NR41: Sound Lenght Timer & Duty Cycle Register (Channel 4 has no wave duty!)
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-5: Length timer (Large = Short Time, Small = Long Time)
		uint8_t length : 6;
		// BITS 6-7: Wave Duty (0b00 = 12.5%, 0b01 = 25%, 0b10 = 50%, 0b11 = 75%)
		uint8_t waveDuty : 2;
	};
} fgbSoundLengthTimerWaveDutyRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbSoundLengthTimerWaveDutyRegister) == 1);

#pragma pack(push,1)
// NR12/NR22/NR42: Volume Envelope Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0-2: Period in range of (0-7)
		uint8_t period : 3;
		// BIT 3: Envelope direction (0 = Decrease, 1 = Increase)
		bool isInc : 1;
		// BIT 4-7: Initial volume
		uint8_t initialVolume : 4;
	};
} fgbVolumeEnvelopeRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbVolumeEnvelopeRegister) == 1);

#pragma pack(push,1)
// NR13/NR23/NR33: Period Low Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0-7: Period Low (Total 11 Bits, Range: 0 - 2047)
		uint8_t periodLow;
	};
} fgbPeriodLowRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbPeriodLowRegister) == 1);

#pragma pack(push,1)
// NR14/NR24/NR34: Period High & Control Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0-2: Period High (Total 11 Bits, Range: 0 - 2047)
		uint8_t periodHigh : 3;
		// BIT 3-5: Unused
		uint8_t unused : 3;
		// BIT 6: Is length enabled (0 = No length, 1 = Armed)
		bool isLengthEnabled : 1;
		// BIT 7: Trigger voice (0 = Not triggered, 1 = Triggered)
		bool isTriggered : 1;
	};
} fgbPeriodHighAndControlRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbPeriodHighAndControlRegister) == 1);

#pragma pack(push,1)
// NR30(FF1A): Channel 3 DAC Enable Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-6: Unused
		uint8_t unused : 7;
		// BIT 7 (0 = Turned off, 1 = Powered on)
		bool isPowered : 1;
	};
} fgbWavePowerRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbWavePowerRegister) == 1);

#pragma pack(push,1)
// NR31(FF1B): Channel 3 Length Timer Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-7: Length in range of 0-255
		uint8_t length : 8;
	};
} fgbWaveLengthTimerRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbWaveLengthTimerRegister) == 1);

#pragma pack(push,1)
// NR32(FF1C): Channel 3 Output Level Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-4: Unused
		uint8_t unused04 : 5;
		// BITS 5-6: Output Level (0b00 = Mute, 0b01 = 100%, 0b10 = 50%, 0b11 = 25%)
		uint8_t outputLevel : 2;
		// BIT 7: Unused
		uint8_t unused7 : 1;
	};
} fgbWaveOutputLevelRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbWaveOutputLevelRegister) == 1);

#pragma pack(push,1)
// NR41(FF1B): Channel 4 Length Timer Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-5: Length in range of 0-63
		uint8_t length : 6;
		// BITS 6-7: Unused
		uint8_t unused : 2;
	};
} fgbNoiseLengthTimerRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbNoiseLengthTimerRegister) == 1);

#pragma pack(push,1)
// NR43(FF22): Channel 4 Frequency & Randomness Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-2: Clock Divider
		uint8_t clockDivider : 3;
		// BITS 3: LFSR Width (0 = 15-bit, 1 = 7-Bit)
		bool lfsrWidth : 1;
		// BITS 4-7: Clock Shift
		uint8_t clockShift : 4;
	};
} fgbNoiseFrequencyRandomnessRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbNoiseFrequencyRandomnessRegister) == 1);

#pragma pack(push,1)
// NR44(FF23): Channel 4 Control Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-5: Unused
		uint8_t unused : 6;
		// BITS 6: Length Enable (0 = Disabled, 1 = Enabled)
		bool isLengthEnabled : 1;
		// BITS 7: Trigger (0 = Not trigger, 1 = Triggered)
		bool isTriggered : 1;
	};
} fgbNoiseControlRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbNoiseControlRegister) == 1);

#pragma pack(push,1)
// NR50(FF24): Master Volume & VIN Panning Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0-2: Right volume (3 BITS, Range of 0-7)
		uint8_t rightVolume : 3;
		// BIT 3: VIN right
		bool hasRight : 1;
		// BIT 4-6: Left volume (3 BITS, Range of 0-7)
		uint8_t leftVolume : 3;
		// BIT 7: VIN left
		bool hasLeft : 1;
	};
} fgbAudioMasterVolumeVINRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbAudioMasterVolumeVINRegister) == 1);

#pragma pack(push,1)
// NR51(FF25): Sound Panning Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0: Channel-1 right
		bool sweepRight : 1;
		// BIT 1: Channel-2 right
		bool toneRight : 1;
		// BIT 2: Channel-3 right
		bool waveRight : 1;
		// BIT 3: Channel-4 right
		bool noiseRight : 1;
		// BIT 4: Channel-1 left
		bool sweepLeft : 1;
		// BIT 5: Channel-2 left
		bool toneLeft : 1;
		// BIT 6: Channel-3 left
		bool waveLeft : 1;
		// BIT 7: Channel-4 left
		bool noiseLeft : 1;
	};
} fgbSoundPanningRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbSoundPanningRegister) == 1);

#pragma pack(push,1)
// NR52(FF26): Audio Master Control Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BIT 0: Channel-1 on/off
		bool onVoice1 : 1;
		// BIT 1: Channel-2 on/off
		bool onVoice2 : 1;
		// BIT 2: Channel-3 on/off
		bool onVoice3 : 1;
		// BIT 3: Channel-4 on/off
		bool onVoice4 : 1;
		// BIT 4-6: Unused (Must always be set to 0b111)
		uint8_t unused : 3;
		// BIT 7: Audio on/off
		bool audioOnOff : 1;
	};
} fgbAudioMasterControlRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbAudioMasterControlRegister) == 1);

#pragma pack(push,1)
// Wave Pattern Entry (2 Samples, Each 4 BITS long)
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-3: Lower Nipple Sample
		uint8_t lower : 4;
		// BITS 4-7: Higher Nipple Sample
		uint8_t upper : 4;
	};
} fgbWavePatternEntry;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbWavePatternEntry) == 1);

// Total number of wave pattern entries
#define FGB_WAVE_PATTERN_RAM_SIZE 16

#pragma pack(push,1)
// Wave Pattern RAM union (16 Bytes, FF30-FF3F)
typedef union {
	// 16 bytes memory
	uint8_t m[FGB_WAVE_PATTERN_RAM_SIZE];
	// 16 wave pattern entries
	fgbWavePatternEntry entries[FGB_WAVE_PATTERN_RAM_SIZE];
} fgbWavePatternRAM;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbWavePatternRAM) == FGB_WAVE_PATTERN_RAM_SIZE);

#pragma pack(push,1)
// Sound Register (48 bytes, FF10-FF3F)
typedef union {
	// Memory of 48 bytes
	uint8_t m[48];
	// Anonymous struct storing all sound registers
	struct {
		fgbFrequencySweepRegister nr10;						// FF10: NR10, Channel 1 Sweep Register (R/W)
		fgbSoundLengthTimerWaveDutyRegister nr11;			// FF11: NR11, Channel 1 Sound Length Timer / Duty Cycle (R/W)
		fgbVolumeEnvelopeRegister nr12;						// FF12: NR12, Channel 1 Volume Envelope (R/W)
		fgbPeriodLowRegister nr13;							// FF13: NR13, Channel 1 Period Low (W)
		fgbPeriodHighAndControlRegister nr14;				// FF14: NR14, Channel 1 Period High & Control (R/W)

		uint8_t ff15_unused;								// FF15: Unused
		fgbSoundLengthTimerWaveDutyRegister nr21;			// FF16: NR21, Channel 2 Sound Length Timer / Duty Cycle (R/W)
		fgbVolumeEnvelopeRegister nr22;						// FF17: NR22, Channel 2 Volume Envelope (R/W)
		fgbPeriodLowRegister nr23;							// FF18: NR23, Channel 2 Period Low (W)
		fgbPeriodHighAndControlRegister nr24;				// FF19: NR24, Channel 2 Period High & Control (R/W)

		fgbWavePowerRegister nr30;							// FF1A: NR30, Channel 3 Sound on/off (R/W)
		fgbWaveLengthTimerRegister nr31;					// FF1B: NR31, Channel 3 Sound Length Timer (W)
		fgbWaveOutputLevelRegister nr32;					// FF1C: NR32, Channel 3 Select Output Level (R/W)
		fgbPeriodLowRegister nr33;							// FF1D: NR33, Channel 3 Period Low (W)
		fgbPeriodHighAndControlRegister nr34;				// FF1E: NR34, Channel 3 Period High & Control (R/W)

		uint8_t ff1f_unsued;								// FF1F: Unused
		fgbNoiseLengthTimerRegister nr41;					// FF20: NR41, Channel 4 Sound Length Timer (W)
		fgbVolumeEnvelopeRegister nr42;						// FF21: NR42, Channel 4 Volume Envelope (R/W)
		fgbNoiseFrequencyRandomnessRegister nr43;			// FF22: NR43, Channel 4 Frequency & Randomness (R/W)
		fgbNoiseControlRegister nr44;						// FF23: NR44, Channel 4 Control (R/W)

		fgbAudioMasterVolumeVINRegister masterVolumeVIN;	// FF24: NR50, Master volume & VIN panning (R/W)
		fgbSoundPanningRegister soundPanning;				// FF25: NR51, Sound Panning (R/W)
		fgbAudioMasterControlRegister audioMasterControl;	// FF26: NR52, Audio master control (R/W)
		uint8_t ff27_ff2f_unused[9];						// FF27-FF2F: Unused 9 bytes
		fgbWavePatternRAM wavePatternRAM;					// FF30-FF3F: Wave Pattern RAM (16 Bytes)
	};
} fgbSoundRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbSoundRegister) == 48);

// Two Stereo Channels for the APU
#define FGB_APU_CHANNEL_COUNT 2

// Size of one sample in bytes
#define FGB_APU_SAMPLE_SIZE sizeof(uint8_t)

// Total number of audio frames
#define FGB_APU_MAX_FRAME_COUNT 16384

// Size of one audio frame in bytes
#define FGB_APU_FRAME_SIZE (FGB_APU_SAMPLE_SIZE * FGB_APU_CHANNEL_COUNT)

// Total number of audio samples
#define FGB_APU_MAX_SAMPLE_COUNT (FGB_APU_MAX_FRAME_COUNT * FGB_APU_CHANNEL_COUNT)

// Stores audio samples in a lock-free style that enforces cacheline alignments
typedef struct {
	// Cacheline alignment for samples (To prevent false sharing)
	fgbCacheline cachelineAlignmentSamples;
	// The interleaved stereo samples
	uint8_t samples[FGB_APU_MAX_SAMPLE_COUNT];
	// Cacheline alignment for tail (To prevent false sharing)
	fgbCacheline cachelineAlignmentTail;
	// The sample cursor for storing the read position, which will wrap around
	volatile int64_t tailPosition;
	// Cacheline alignment for head (To prevent false sharing)
	fgbCacheline cachelineAlignmentHead;
	// The sample cursor for storing the write position, which will wrap around
	volatile int64_t headPosition;
	// Cacheline alignment for the fill count (To prevent false sharing)
	fgbCacheline cachelineAlignmentFillCount;
	// The current frame count
	volatile int64_t fillCount;
} fgbAudioSampleBuffer;

// Represents the audio buffers for the APU
typedef struct {
	// Cacheline alignment for buffers (To prevent false sharing)
	fgbCacheline cachelineAlignmentBuffers;
	// Actual audio buffers
	fgbAudioSampleBuffer buffers[2];
	// Cacheline alignment for activeBufferIndex (To prevent false sharing)
	fgbCacheline cachelineAlignmentActiveBufferIndex;
	// Active buffer index, either 0 or 1
	volatile int64_t activeBufferIndex;
	// Cacheline alignment for fillCount (To prevent false sharing)
	fgbCacheline cachelineAlignmentFillCount;
	// The current fill count as number of frames
	volatile int64_t fillCount;
} fgbAudioBuffer;

// Defines the sound length states
typedef enum {
	// Disabled
	fgbSoundLengthState_Disabled = 0,
	// Continue
	fgbSoundLengthState_Continue,
	// End is reached
	fgbSoundLengthState_EndReached,
} fgbSoundLengthState;

// Stores the data for a sound length timer
typedef struct {
	// Timer that counts down until it reaches zero, than disables the voice
	uint16_t timer;
	// Max length that is either 64 or 256
	uint16_t maxLength;
	// Length in range of 0-63 or 0-255 (The cycle timer is set by maxLength - length, because the higher the length, the shorter the time of the voice)
	uint16_t length;
	// Is Sound length timer enabled
	bool isEnabled;
	// Padding to align to 8 bytes
	uint8_t padding;
} fgbSoundLengthTimer;
FGB_STATIC_ASSERT(sizeof(fgbSoundLengthTimer) == 8);

// Stores the data for a volume envelope
typedef struct {
	// Timer that counts down to zero, then it resets back to the period
	int32_t timer;
	// Initial volume in range of 0-15
	uint8_t initialVolume;
	// Current volume in range of 0-15
	uint8_t currentVolume;
	// Period in range of 0-7
	uint8_t period;
	// Zombie volume step (Hack: Zombie Volume Decrease)
	uint8_t zombieStep;
	// Is volume increasing
	bool isIncreasing;
	// Is Volume envelope enabled
	bool isEnabled;
	// Padding to align to 4 bytes
	uint8_t padding[6];
} fgbVolumeEnvelope;
FGB_STATIC_ASSERT(sizeof(fgbVolumeEnvelope) == 16);

#pragma pack(push,1)
// Stores the sound period/frequency (11 Bits, the remaining 5 bits are zero)
typedef union {
	// Period in 16 BITS
	uint16_t period;
	// Anonymous struct storing all 8-bits
	struct {
		// BITS 0-7: Low part
		uint8_t low : 8;
		// BITS 8-10: High part
		uint8_t high : 3;
		// BITS 11-15: Unused
		uint8_t unused : 5;
	};
} fgbSoundFrequency;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbSoundFrequency) == 2);

// Stores the data for a frequency sweep
typedef struct {
	// Period timer that counts down to zero and then it resets back to the period
	int32_t timer;
	// Copy of the current frequency, so the sweep can be updated - even when a new frequency was set
	uint16_t shadow;
	// Current period in range of 0-7
	uint8_t period;
	// Current shift in range of 0-7
	uint8_t shift;
	// Is frequency sweep enabled
	bool isEnabled;
	// Is frequency decreasing or increasing
	bool isDecrease;
	// Was last calculation of the frequency sweep a decrease or not
	bool wasLastCalculationDecrease;
	// Padding to align 4 bytes
	uint8_t padding[5];
} fgbFrequencySweep;
FGB_STATIC_ASSERT(sizeof(fgbFrequencySweep) == 16);

// Stores the states for a square wave
typedef struct {
	// A timer that counts down until it reaches zero and than starts with (2048 - period) * 4 again
	int32_t timer;
	// Current period in range of 0-2047 (11 Bit), only used for the timer
	uint16_t period;
	// Current sequencer step in range of 0-7 (Column-Index in the duty table)
	uint8_t step;
	// Current duty cycle in range of 0-3 (Row-Index in the duty table)
	uint8_t dutyCycle;
} fgbSquareWave;
FGB_STATIC_ASSERT(sizeof(fgbSquareWave) == 8);

// Represents the state of a frame sequencer
typedef struct {
	// Last timer divider value
	uint16_t lastDivider;
	// The current step in range of 0-7 (Increments/Modulates every 8192 cycles - 512 Hz)
	uint8_t step;
	// Padding to align to 4 bytes
	uint8_t padding;
} fgbFrameSequencer;
FGB_STATIC_ASSERT(sizeof(fgbFrameSequencer) == 4);

// Represents the state of a sound voice
typedef struct {
	// Is speaker on the left/right enabled
	bool isSpeakerEnabled[2];
	// DAC Power / Enable playback
	bool isPowered;
	// Enable flag turned on by trigger bit and gets reset by length timer / frequency sweep / volume envelope
	bool isEnabled;
	// Mute flag set by the outside
	bool isMuted;
	// Padding to align to 8 bytes
	uint8_t padding[3];
} fgbVoice;
FGB_STATIC_ASSERT(sizeof(fgbVoice) == 8);

// Represents the full tone-voice
typedef struct {
	// Base voice
	fgbVoice base;

	// Volume Envelope
	fgbVolumeEnvelope envelope;
	// Square Wave
	fgbSquareWave squareWave;
	// Sound Length Timer
	fgbSoundLengthTimer length;
	// Frame Sequencer
	fgbFrameSequencer frameSequencer;
	// Sound Frequency
	fgbSoundFrequency freq;

	// Padding to align to 64 bytes
	uint8_t padding[18];
} fgbToneVoice;
FGB_STATIC_ASSERT(sizeof(fgbToneVoice) == 64);

// Represents the full sweep-voice
typedef struct {
	// Base voice
	fgbVoice base;

	// Frequency Sweep
	fgbFrequencySweep sweep;
	// Volume Envelope
	fgbVolumeEnvelope envelope;
	// Square Wave
	fgbSquareWave squareWave;
	// Sound Length Timer
	fgbSoundLengthTimer length;
	// Sound Frequency
	fgbSoundFrequency freq;

	// Padding to align to 64 bytes
	uint8_t padding[4];
} fgbSweepVoice;
FGB_STATIC_ASSERT(sizeof(fgbSweepVoice) == 64);

// Represents the full wave-voice
typedef struct {
	// Base voice
	fgbVoice base;

	// Wave pattern RAM (16 Byte, Contains 32 samples, each 4-bit long)
	fgbWavePatternRAM patternRAM;
	// Sound Length Timer
	fgbSoundLengthTimer length;

	// Timer that counts down until it reaches zero, than it increments the wave position and resets back to its period 2 * (2048 - frequency)
	int32_t timer;

	// The period the timer gets reset to: 2 * (2048 - frequency)
	uint16_t period;
	// Sound Frequency
	fgbSoundFrequency freq;

	// Current position in the pattern RAM in range of 0-31
	uint8_t wavePosition;
	// Current output sample in range of 0-15
	uint8_t outputSample;
	// Output level in  range 0-3 (Volume shift: 0 = Muted, 1 = 100%, 2 = 50%, 3 = 25%)
	uint8_t outputLevel;
	// Volume shift to convert the output level into a volume of range 0-15
	uint8_t volumeShift;

	// Is the channel actually playing
	bool isPlaying;
	// Padding to align to 4 bytes
	uint8_t padding0[3];

	// Padding to align to 64 bytes
	uint8_t padding1[16];
} fgbWaveVoice;
FGB_STATIC_ASSERT(sizeof(fgbWaveVoice) == 64);

// Represents the full noise-voice
typedef struct {
	// Base voice
	fgbVoice base;

	// Volume Envelope
	fgbVolumeEnvelope envelope;
	// Sound Length Timer
	fgbSoundLengthTimer length;

	// Timer that counts down to zero and then gets reset
	int32_t timer;
	// Timer period that gets applied to the timer
	uint16_t period;
	// Linear Feedback Shift Register
	uint16_t lfsr;
	// Timer shift
	uint8_t clockShift;
	// Timer divisor
	uint8_t divisorIndex;
	// Current 4-bit sample
	uint8_t sample;
	// Is 7-bit or 15-bit
	bool is7Bit;

	// Padding to align to 64 bytes
	uint8_t padding[20];
} fgbNoiseVoice;
FGB_STATIC_ASSERT(sizeof(fgbNoiseVoice) == 64);

// Defines the sound register types
typedef enum {
	// NRx0 register type
	fgbSoundRegType_NRx0 = 0,
	// NRx1 register type
	fgbSoundRegType_NRx1 = 1,
	// NRx2 register type
	fgbSoundRegType_NRx2 = 2,
	// NRx3 register type
	fgbSoundRegType_NRx3 = 3,
	// NRx4 register type
	fgbSoundRegType_NRx4 = 4,
} fgbSoundRegType;

// Stores the state of a highpass filter
typedef struct {
	// Charge factor in range of 0.0 to 1.0
	float chargeFactor;
	// Current capacitor value
	float capacitor;
} fgbHighPassFilter;
FGB_STATIC_ASSERT(sizeof(fgbHighPassFilter) == 8);

#pragma pack(push,4)
// Stores the states of the audio processing unit
typedef struct {
	// High Pass Filter
	fgbHighPassFilter highPassFilter;
	// Frame Sequencer
	fgbFrameSequencer frameSequencer;
	// The target sample rate in Hz
	uint32_t sampleRate;
	// The number of CPU cycles required to produce two samples (4 MHz >> Timer Frequency / Sample Rate)
	uint32_t cyclesPerSample;
	// The current sample cycles timer, that starts ats zero and counts up until the the number of clocks per sample/frame is reached
	uint32_t sampleCycleTimer;
	// Volume for left/right stero channel in range of 0.0 to 1.0
	float stereoVolume[2];
	// Left/Right enabled flag
	bool stereoEnabled[2];
	// Power is on or off
	bool isPowerOn;
	// Padding to align to 64 bytes
	uint8_t padding[29];
} fgbAPUState;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbAPUState) == 64);

// Stores all 4 voices
typedef struct {
	// Voice 1: Sweep/Tone
	fgbSweepVoice sweep;
	// Voice 2: Tone
	fgbToneVoice tone;
	// Voice 3: Wave
	fgbWaveVoice wave;
	// Voice 4: Noise
	fgbNoiseVoice noise;
} fgbVoices;

// APU RAM (256 KB)
typedef uint8_t fgbAudioRAM[0x100];

// Stores the state and data of the audio processing unit
typedef struct {
	// Audio buffer
	fgbAudioBuffer buffer;
	// The tiny 256 bytes of RAM for the APU
	fgbAudioRAM ram;
	// All 4 voices
	fgbVoices voices;
	// State
	fgbAPUState state;
} fgbAPU;

#pragma pack(push,1)
// FF40: LCDC register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		bool backgroundEnabled : 1;					// Bit 0: Background enabled Flag -> 0 = Background on, 1 = Background off
		bool objEnable : 1;							// Bit 1: Sprites enabled Flag -> 0 = Sprites are on, 1 = Sprites are off
		bool objSize : 1;							// Bit 2: Object Block Compisition Selection Flag -> 0 = 8x8 pixels, 1 = 8x16 pixels
		bool backgroundDataAreaSelect : 1;			// Bit 3: Background Code Area Selection Flag -> 0 = 9800h-9BFFh, 1 = 9C00h-9FFFh
		bool backgroundWindowTilesAreaSelect : 1;	// Bit 4: Background/Window Tiles Selection Flag -> 0 = 8800h-97FFh, 1 = 8000h-8FFFh
		bool windowEnable : 1;						// Bit 5: Window enabled Flag -> 0 = Window on, 1 = Window off
		bool windowDataAreaSelect: 1;				// Bit 6: Window Code Area Selection Flag -> 0 = 9800h-9BFFh, 1 = 9C00h-9FFFh
		bool lcdEnabled : 1;						// Bit 7: LCD Controller Operation Stop Flag -> 0 = LCD on, 1 = LCD off
	};
} fgbLCDControl;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbLCDControl) == 1);

#pragma pack(push,1)
// FF41: LCD Status Register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		uint8_t lcdMode : 2;		// Bit 0-1: LCDMode (R)
		bool lycEqualsLY : 1;		// Bit 2: LYC == LY (R)
		bool mode0IntSelect : 1;	// Bit 3: HBlank (R/W)
		bool mode1IntSelect : 1;	// Bit 4: VBlank (R/W)
		bool mode2IntSelect : 1;	// Bit 5: OAM (R/W)
		bool lycIntSelect : 1;		// Bit 6: LYC Compare (R/W)
		bool unused : 1;			// Bit 7 (unused)
	};
} fgbLCDStatus;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbLCDStatus) == 1);

#pragma pack(push,1)
// Represents the monochrome palette register
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		uint8_t id0 : 2;	// Bit 0-1
		uint8_t id1 : 2;	// Bit 2-3
		uint8_t id2 : 2;	// Bit 4-5
		uint8_t id3 : 2;	// Bit 6-7
	};
} fgbPaletteData;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbPaletteData) == 1);

#pragma pack(push, 1)
// Represents the full LCD Register (12 bytes)
typedef union {
	// Full data for all registers
	uint8_t m[12];
	// Anonymous struct storing all registers
	struct {
		fgbLCDControl lcdc;			// FF40: LCD Control (R/W)			
		fgbLCDStatus stat;			// FF41: LCDC Status (R/W)
		uint8_t scy;				// FF42: Y Scroll (R/W)
		uint8_t scx;				// FF43: X Scroll (R/W)
		uint8_t ly;					// FF44: LCDC Y coordinate (R)
		uint8_t lyc;				// FF45: LY Compare (R/W)
		uint8_t dma;				// FF46: DMA transfer and start address (W)
		fgbPaletteData bgp;			// FF47: Background Palette Data (Non-CGB only, R/W)
		fgbPaletteData obp0;		// FF48: Object Palette Data 0 (Non-CGB only, R/W)
		fgbPaletteData obp1;		// FF49: Object Palette Data 1 (Non-CGB only, R/W)
		uint8_t wy;					// FF4A: Window Y position (R/W)
		uint8_t wx;					// FF4B: Window X position minus 7 (R/W)
	};
} fgbLCDRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbLCDRegister) == 12);

#pragma pack(push, 1)
// Joypad Register (1 Byte)
typedef union fgbJoypadRegister {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		bool dpadRight_Or_ButtonA : 1;	// Bit 0: P10 (0 = Pressed, 1 = Not Pressed)
		bool dpadLeft_Or_ButtonB : 1;	// Bit 1: P11 (0 = Pressed, 1 = Not Pressed)
		bool dpadUp_Or_Select : 1;		// Bit 2: P12 (0 = Pressed, 1 = Not Pressed)
		bool dpadDown_Or_Start : 1;		// Bit 3: P13 (0 = Pressed, 1 = Not Pressed)
		bool directionSelection : 1;	// Bit 4: P14 (0 = Selected, 1 = Not Selected)
		bool buttonSelection : 1;		// Bit 5: P15 (0 = Selected, 1 = Not Selected)
		uint8_t unused : 2;				// Bit 6-7 (Unused, is always 0b11)
	};
} fgbJoypadRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbJoypadRegister) == 1);

// Defines the interrupt Types
typedef enum {
	// None Interrupt
	fgbInterruptType_None = 0,
	// Vertical Blank Interrupt
	fgbInterruptType_VerticalBlank = 1 << 0,
	// LCD Status Interrupt
	fgbInterruptType_LCDStatus = 1 << 1,
	// Timer Interrupt
	fgbInterruptType_Timer = 1 << 2,
	// Serial Interrupt
	fgbInterruptType_Serial = 1 << 3,
	// Joypad Interrupt
	fgbInterruptType_Joypad = 1 << 4,
} fgbInterruptType;

#pragma pack(push, 1)
// Represents the interrupt flags (1 Byte)
typedef union {
	// U8 value
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		bool vblank : 1;	// Bit 0: Vertical Blank
		bool stat : 1;		// Bit 1: LCDC (Stat Referenced)
		bool timer : 1;		// Bit 2: Timer Overflow
		bool serial : 1;	// Bit 3: Serial I/O Transfer Completion
		bool joypad : 1;	// Bit 4: Joypad, P10-P13 Terminal Negative Edge
		uint8_t unused : 3;	// Bit 5-7: Unused
	};
} fgbInterruptsFlags;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbInterruptsFlags) == 1);

#pragma pack(push, 1)
// Represents the serial data transfer registers (2 Bytes)
typedef struct {
	// Anonymous union storing either the bits for the SC register or the byte value
	union {
		// Anonymous struct storing all 8 bits
		struct {
			bool sio_clk : 1;	// Bit 0: SCK Terminal I/O Selection, 0 = Use external clock, 1 = Use internal clock
			bool sio_fast : 1;	// Bit 1: CGB, Clock Switching Flag, 0 = 8 KHz (16 KHz), 1 = 256 Khz (512 KHz)
			uint8_t empty : 5;		// Bit 2-6: Unused
			bool sio_en : 1;	// Bit 7: Serial transfer start flag, 0 = No Serial Transfer, 1 = Start Serial Transfer
		};
		uint8_t sc;				// FF01: Serial Transfer Control Register
	};
	uint8_t sb;					// FF02: Serial Transfer Data
} fgbSerialTransferRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbSerialTransferRegister) == 2);

// Defines the timer clock types
typedef enum {
	// Timer Clock of 1024 Hz (BITS 00)
	fgbTimerClockType_00_1024 = 0b00,
	// Timer Clock of 16 Hz (BITS 01)
	fgbTimerClockType_01_16 = 0b01,
	// Timer Clock of 64 Hz (BITS 10)
	fgbTimerClockType_10_64 = 0b10,
	// Timer Clock of 256 Hz (BITS 11)
	fgbTimerClockType_11_256 = 0b11,
} fgbTimerClockType;

#pragma pack(push, 1)
// FF07: Timer control register
typedef union {
	// U8 value.
	uint8_t u8;
	// Anonymous struct storing all 8 bits
	struct {
		// Bit 0-1: Clock Select (2 BITS, 0b00 = 1024 Hz, 0b01 = 16 Hz, 0b10 = 64 Hz, 0b11 = 256 Hz)
		uint8_t clock : 2;
		// Bit 2: Timer Enable (0 = Stop, 1 = Start)
		bool isEnabled : 1;
		// Bit 3-7: Unused
		uint8_t unused : 5;
	};
} fgbTimerControlRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTimerControlRegister) == 1);

#pragma pack(push, 1)
// Represents the full timer Registers (4 bytes)
typedef struct {
	// FF04: Divider Register (DIV, R/W)
	uint8_t divider;
	// FF05: Timer Counter (TIMA, R/W)
	uint8_t counter;
	// FF06: Timer Modulo (TMA, R/W)
	uint8_t modulo;
	// FF07: Timer Control (TAC, R/W)
	fgbTimerControlRegister tac;
} fgbTimerRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTimerRegister) == 4); 

#pragma pack(push, 1)
// Represents the full timer state.
typedef struct {
	// Timer register
	fgbTimerRegister reg;
	// Cycles until a timer is reloaded (Range: 0 - 4)
	uint32_t reloadCycles;
	// Cycles until a timer interrupt will be fired (Range: 0 - 1)
	uint32_t interruptCycles;
	// Divider counter
	uint16_t divider;
	// Timer is reloading
	bool isReloading;
	// Padding to align to 16 bytes
	uint8_t padding;
} fgbTimer;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTimer) == 16); 

#pragma pack(push, 1)
// Represents the full IO register (128 bytes)
typedef union {
	// Full data
	uint8_t m[128];
	// Anonymous struct that contains all individual registers
	struct {
		fgbJoypadRegister joypad;				// FF00: Joypad (1 byte)
		fgbSerialTransferRegister serial;		// FF01-FF02: Serial transfer (2 bytes)
		uint8_t unused_ff03;					// FF03: Unused (1 byte)
		fgbTimerRegister timer;					// FF04-FF07: Timer (4 bytes)
		uint8_t unused_ff08_ff0e[7];			// FF08-FF0E: Unused (7 bytes)
		fgbInterruptsFlags interruptRequest;	// FF0F: Interrupt Flags (1 byte)
		fgbSoundRegister sound;					// FF10-FF3F: Sound (24 bytes)
		fgbLCDRegister lcd;						// FF40-FF4B: LCD register (12 bytes)
		uint8_t unused_ff4c_ff4f[4];			// FF4C-FF4F: Unused (CGB???, 4 bytes)
		uint8_t bootFlag;						// FF50: Boot flag (1 byte)
		uint8_t hdma1;							// FF51: HDMA-1 (CGB, 1 byte)
		uint8_t hdma2;							// FF52: HDMA-2 (CGB, 1 byte)
		uint8_t hdma3;							// FF53: HDMA-3 (CGB, 1 byte)
		uint8_t hdma4;							// FF54: HDMA-4 (CGB, 1 byte)
		uint8_t hdma5;							// FF55: HDMA-5 (CGB, 1 byte)
		uint8_t rp;								// FF56: RP (1 byte)
		uint8_t unused_ff57_ff67[10];			// FF57-FF67: Unused (10 bytes)
		uint8_t bcps;							// FF68: BCPS (CGB, 1 byte)
		uint8_t bcpd;							// FF69: BCPD (CGB, 1 byte)
		uint8_t ocps;							// FF6A: OCPS (CGB, 1 byte)
		uint8_t ocpd;							// FF6B: OCPD (CGB, 1 byte)
		uint8_t unused_ff6c_ff7f[26];			// FF6C-FF7F: Unused (26 bytes)
		fgbInterruptsFlags interruptEnable;		// FFFF: Interrupt Enable (1 byte)
	};
} fgbIORegisters;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbIORegisters) == 128);

#pragma pack(push, 1)
// Represents the CPU flags register
typedef union {
	// Full 8-bit flags
	uint8_t flags;
	// Anonymous struct storing all 8 bits
	struct {
		uint8_t empty : 4;		// empty (Bit 0-3)
		bool fullCarryFlag : 1; // C (Bit 4: Full Carry Flag)
		bool halfCarryFlag : 1; // H (Bit 5: Half Carry Flag)
		bool negativeFlag : 1;  // N (Bit 6: Negative Flag)
		bool zeroFlag : 1;		// Z (Bit 7: Zero Flag)
	};
} fgbFlagsRegister;
FGB_STATIC_ASSERT(sizeof(fgbFlagsRegister) == 1);
#pragma pack(pop)

#pragma pack(push, 1)
// Represents the CPU registers
typedef struct {
	// Anonymous union storing the AF register
	union {
		uint16_t af;			// Accumulator & Flags register (16-bit)
		// Anonymous struct storing the F and the A register (Little Endian)
		struct {
			fgbFlagsRegister f;	// Flags register (8-Bit)
			uint8_t a;			// A register (Math accumulator, 8-bit)
		};
	};
	// Anonymous union storing the BC register
	union {
		uint16_t bc;			// BC register (16-bit)
		// Anonymous struct storing the C and the B register (Little Endian)
		struct {
			uint8_t c;			// C register (8-Bit)
			uint8_t b;			// B register (8-Bit)
		};
	};
	// Anonymous union storing the DE register
	union {
		uint16_t de;			// DE register (16-bit)
		// Anonymous struct storing the D and the E register (Little Endian)
		struct {
			uint8_t e;			// E register (8-Bit)
			uint8_t d;			// D register (8-Bit)
		};
	};
	// Anonymous union storing the HL register
	union {
		uint16_t hl;			// HL register (16-bit)
		// Anonymous struct storing the H and the L register (Little Endian)
		struct {
			uint8_t l;			// L register (8-Bit)
			uint8_t h;			// H register (8-Bit)
		};
	};
	uint16_t sp;				// 16-bit stack pointer
	uint16_t pc;				// 16-bit program counter
	uint32_t padding;			// Padding to align to 16-bytes
} fgbCPURegisters;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbCPURegisters) == 16);

// Defines the register types
typedef enum {
	// None register
	fgbRegisterType_None = 0,
	// A register
	fgbRegisterType_A,
	// F register
	fgbRegisterType_F,
	// B register
	fgbRegisterType_B,
	// C register
	fgbRegisterType_C,
	// D register
	fgbRegisterType_D,
	// E register
	fgbRegisterType_E,
	// H register
	fgbRegisterType_H,
	// L register
	fgbRegisterType_L,
	// AF register
	fgbRegisterType_AF,
	// BC register
	fgbRegisterType_BC,
	// DE register
	fgbRegisterType_DE,
	// HL register
	fgbRegisterType_HL,
	// SP register
	fgbRegisterType_SP,
	// PC register
	fgbRegisterType_PC,

	// First register type
	fgbRegisterType_First = fgbRegisterType_None,
	// Last register type
	fgbRegisterType_Last = fgbRegisterType_PC,
} fgbRegisterType;

// Total count of register types
#define FGB_REGISTER_TYPE_COUNT (fgbRegisterType_Last - fgbRegisterType_First) + 1

#pragma pack(push, 1)
// Value Storage (1-4 byte)
typedef struct {
	// Anonymous union storing a 16-bit value in various ways
	union {
		// U16 value
		uint16_t u16;
		// S16 value
		int16_t s16;
		// Anonymous struct storing the low and high U8 value
		struct {
			uint8_t ulow;
			uint8_t uhigh;
		};
		// Anonymous struct storing the low and high S8 value
		struct {
			int8_t slow;
			int8_t shigh;
		};
	};
	// Flag indicating whether the value is 8-bit (false) or 16-bit (true)
	bool isWide;
	// Flag indicating whether the value is signed or not
	bool isSign;
} fgbValue;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbValue) == 4);

// Defines the instruction types
typedef enum {
	// None instruction
	fgbInstructionType_None = 0,
	// NOP instruction
	fgbInstructionType_NOP,
	// ADC instruction
	fgbInstructionType_ADC,
	// ADD instruction
	fgbInstructionType_ADD,
	// AND instruction
	fgbInstructionType_AND,
	// BIT instruction
	fgbInstructionType_BIT,
	// CALL instruction
	fgbInstructionType_CALL,
	// CCF instruction
	fgbInstructionType_CCF,
	// CP instruction
	fgbInstructionType_CP,
	// CPL instruction
	fgbInstructionType_CPL,
	// DAA instruction
	fgbInstructionType_DAA,
	// DEC instruction
	fgbInstructionType_DEC,
	// DI instruction (Disable Interrupt)
	fgbInstructionType_DI,
	// EI instruction (Enable Interrupt)
	fgbInstructionType_EI,
	// HALT instruction
	fgbInstructionType_HALT,
	// INC instruction
	fgbInstructionType_INC,
	// JP instruction (Jump Absolute)
	fgbInstructionType_JP,
	// JR instruction (Jump Relative)
	fgbInstructionType_JR,
	// LD instruction (Load)
	fgbInstructionType_LD,
	// OR instruction
	fgbInstructionType_OR,
	// POP instruction
	fgbInstructionType_POP,
	// PREFIX instruction
	fgbInstructionType_PREFIX,
	// PUSH instruction
	fgbInstructionType_PUSH,
	// RES instruction
	fgbInstructionType_RES,
	// RET instruction (Return)
	fgbInstructionType_RET,
	// RETI instruction (Return and Enable IME)
	fgbInstructionType_RETI,
	// RL instruction
	fgbInstructionType_RL,
	// RLA instruction
	fgbInstructionType_RLA,
	// RLC instruction
	fgbInstructionType_RLC,
	// RLCA instruction
	fgbInstructionType_RLCA,
	// RR instruction
	fgbInstructionType_RR,
	// RRA instruction
	fgbInstructionType_RRA,
	// RRC instruction
	fgbInstructionType_RRC,
	// RRCA instruction
	fgbInstructionType_RRCA,
	// RST instruction
	fgbInstructionType_RST,
	// SBC instruction
	fgbInstructionType_SBC,
	// SCF instruction
	fgbInstructionType_SCF,
	// SET instruction
	fgbInstructionType_SET,
	// SLA instruction
	fgbInstructionType_SLA,
	// SRA instruction
	fgbInstructionType_SRA,
	// SRL instruction
	fgbInstructionType_SRL,
	// STOP instruction
	fgbInstructionType_STOP,
	// SUB instruction
	fgbInstructionType_SUB,
	// SWAP instruction
	fgbInstructionType_SWAP,
	// XOR instruction
	fgbInstructionType_XOR,

	// First instruction type
	fgbInstructionType_First = fgbInstructionType_None,
	// Last instruction type
	fgbInstructionType_Last = fgbInstructionType_XOR,
} fgbInstructionType;

// Total count of instruction types
#define FGB_INSTRUCTION_TYPE_COUNT (fgbInstructionType_Last - fgbInstructionType_First) + 1

// Defines the condition types
typedef enum {
	// Always
	fgbConditionType_Always = 0,
	// Not Zero
	fgbConditionType_NotZero,
	// Zero
	fgbConditionType_Zero,
	// Not Carry
	fgbConditionType_NotCarry,
	// Carry
	fgbConditionType_Carry,
} fgbConditionType;

// Defines the addressing modes
typedef enum {
	// Implied
	fgbAddressingMode_Implied = 0,
	// Constant (No Data, Value Constant)
	fgbAddressingMode_Constant,
	// Constant with memory register (Target Address, Data from Bus, Constant)
	fgbAddressingMode_Constant_MemReg,
	// Constant with register (Target Register, Data from Register, Constant)
	fgbAddressingMode_Constant_Reg,
	// Immediate signed 8-bit value
	fgbAddressingMode_I8,
	// Register into Bus with 16-bit immediate address
	fgbAddressingMode_MemA16_Reg,
	// Register into Bus with 8-bit immediate address + constant
	fgbAddressingMode_MemConstantOffsetA8_Reg,
	// Register into Bus with address from register and constant offset
	fgbAddressingMode_MemConstantOffsetReg_Reg,
	// Bus with address from register
	fgbAddressingMode_MemReg,
	// Register into Bus from address from register
	fgbAddressingMode_MemReg_Reg,
	// 8-bit immediate value into Bus with address from register
	fgbAddressingMode_MemReg_U8,
	// 8-bit immediate value into Bus with address from register with decremented result
	fgbAddressingMode_MemRegDec_Reg,
	// 8-bit immediate value into Bus with address from register with incremented result
	fgbAddressingMode_MemRegInc_Reg,
	// Register
	fgbAddressingMode_Reg,
	// 8-bit signed immediate into register
	fgbAddressingMode_Reg_I8,
	// Bus with address from 16-bit immediate address into register
	fgbAddressingMode_Reg_MemA16,
	// Bus with address from 8-bit immediate address + constant into register
	fgbAddressingMode_Reg_MemConstantOffsetA8,
	// Bus with address from register + constant offset into register
	fgbAddressingMode_Reg_MemConstantOffsetReg,
	// Bus with address from register into register
	fgbAddressingMode_Reg_MemReg,
	// Bus with address from register into register with decremented result
	fgbAddressingMode_Reg_MemRegDec,
	// Bus with address from register into register with incremented result
	fgbAddressingMode_Reg_MemRegInc,
	// Register into register
	fgbAddressingMode_Reg_Reg,
	// Register into register with 8-bit signed immediate offset
	fgbAddressingMode_Reg_RegOffsetI8,
	// 16-bit immediate value into register
	fgbAddressingMode_Reg_U16,
	// 8-bit immediate value into register
	fgbAddressingMode_Reg_U8,
	// 16-bit immediate value
	fgbAddressingMode_U16,
} fgbAddressingMode;

#pragma pack(push, 4)
// Represents the definition of a CPU instruction
typedef struct {
	// Instruction type
	fgbInstructionType type;
	// Addressing mode
	fgbAddressingMode mode;
	// Condition type
	fgbConditionType condition;
	// Register type for A
	fgbRegisterType regA;
	// Register type for B
	fgbRegisterType regB;
	// Flags that needs to be modified (4-bytes)
	char flags[4];
	// Constant value or value (4-bytes)
	uint32_t value;
	// Length of the instruction in bytes
	uint8_t length;
	// Number of M-cycles for normal execution
	uint8_t normalCycles;
	// Number of M-cycles for branched execution
	uint8_t branchCycles;
	// Length of the data bits (0 = No data, 8 = 8-bit, 16 = 16-bit)
	uint8_t dataLength;
} fgbInstruction;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbInstruction) == 32);

// Defines the operand types
typedef enum {
	// Register operand
	fgbOperandType_Register = 0,
	// Register with offset operand
	fgbOperandType_RegisterWithOffset,
	// Immediate operand
	fgbOperandType_Immediate,
	// Memory address operand
	fgbOperandType_MemoryAddress,
	// Memory address with offset operand
	fgbOperandType_MemoryAddressWithOffset,
	// Memory address with offset from register operand
	fgbOperandType_MemoryAddressWithRegisterOffset,
	// Memory address from register operand
	fgbOperandType_MemoryFromRegister,
	// Memory address from register operand + resulting increment
	fgbOperandType_MemoryFromRegisterInc,
	// Memory address from register operand + resulting decrement
	fgbOperandType_MemoryFromRegisterDec,
	// Memory address from register with offset operand
	fgbOperandType_MemoryFromRegisterWithOffset,
	// Constant operand
	fgbOperandType_Constant,
} fgbOperandType;

#pragma pack(push, 4)
// Represents a single operand in a decoded instruction
typedef struct {
	// Anonymous union holding either the immediate value or the offset
	union {
		// Immediate value
		fgbValue immediate;
		// Offset
		fgbValue offset;
	};
	// Register type
	fgbRegisterType reg;
	// Operand type
	fgbOperandType type;
	// 16-bit address
	uint16_t address;
	// Constant value
	uint16_t constant;
} fgbOperand;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbOperand) == 16);

#pragma pack(push, 4)
// Represents a full decoded instruction
typedef struct {
	// Array of two operands
	fgbOperand operands[2];
	// Instruction type
	fgbInstructionType type;
	// Addressing mode
	fgbAddressingMode mode;
	// Position in the program
	uint32_t position;
	// Length of the instruction in bytes
	uint32_t length;
	// Padding to align to 16-bytes
	uint32_t paddingU32[3];
	// Number of operands
	uint8_t operandCount;
	// Op-code byte
	uint8_t opCode;
	// Padding to align to 4-byte
	uint8_t paddingU8;
	// Flag indicating whether the instruction is prefix instruction or not
	bool isPrefix;
} fgbDecodedInstruction;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbDecodedInstruction) == 64);

// Represents the reference to the full instruction table with all 256 * 2 instructions
typedef struct fgbInstructionTable {
	// Reference to the first normal instruction (up to 256)
	const fgbInstruction *normal;
	// Reference to the first prefix instruction (up to 256)
	const fgbInstruction *prefix;
} fgbInstructionTable;

/**
  * @brief Gets the full instruction table
  * @return Returns the instruction table structure
  */
FGB_API fgbInstructionTable fgbGetInstructionTable(void);

// Defines the emulation states
typedef enum {
	// Stepping emulation state
	fgbEmulationState_Step = 0,
	// Micro-Stepping emulation state
	fgbEmulationState_MicroStep,
	// Breakpoint emulation state
	fgbEmulationState_Breakpoint,
	// Paused emulation state
	fgbEmulationState_Paused,
	// Running emulation state
	fgbEmulationState_Running,
	// Error emulation state
	fgbEmulationState_Error,
} fgbEmulationState;

// Defines the operation types used in the fetch-decode-execute pipeline
typedef enum {
	// None operation
	fgbOperation_None = 0,
	// Decode operation
	fgbOperation_Decode = 1,
	// Fetch operation
	fgbOperation_Fetch = 2,
	// Execute operation
	fgbOperation_Execute = 3,

	// First operation type
	fgbOperation_First = fgbOperation_None,
	// Last operation type
	fgbOperation_Last = fgbOperation_Execute,
} fgbOperation;

#pragma pack(push, 1)
// Stores the datas of the interrupt handler, containg the register, the IME flag and the ticks
typedef struct {
	// Anonymous struct holding the register the IME (Has no purpose)
	struct {
		// FF0F: Interrupt flags
		fgbInterruptsFlags request;
		// FFFF: Interrupt enable
		fgbInterruptsFlags enable;
		// IME: Interrupt master enable flag
		bool isMasterEnabled;
		// Padding to align to 4 bytes
		bool padding4;
	};
	// Number of remaming ticks to enable IME in a multiple of 4
	uint8_t ticksEnableIME;
	// Number of remaining ticks that prevent certain interrupt types not be pending right away (e.g. VBlank)
	uint8_t ticksRequestInterruptDelay;
	// Padding to align to 8 bytes
	uint8_t padding[2];
} fgbInterrupts;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbInterrupts) == 8);

#pragma pack(push, 4)
// Represents the data register, that stores the value, offset and addresses
typedef struct {
	// Stores a 2-byte value, that is typically one or two fetched operands
	fgbValue value;
	// Padding for U32
	uint32_t paddingU32;
	// Stores the target address, when the destination is memory
	uint16_t targetAddress;
	// Stores the source address, when the source is memory (used for disassembly only)
	uint16_t sourceAddress;
	// Stores an signed offset  (used for disassembly only)
	int8_t offset;
	// Indicates if the destination is memory or not
	bool isMemoryTarget;
	// When set, either a value or/and addresses/offsets was set
	bool hasData;
	// Padding to align to 16-bytes
	bool paddingU8;
} fgbDataRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbDataRegister) == 16);

// Represents the gameboy cpu tick cycles, up to 4.194.304 Hz, one cycle equals one Hz (short T-Cycles)
typedef uint64_t fgbTickCycles;

// Represents the gameboy memory cycles, 1/4 of a tick cycle (short M-Cycles)
typedef uint64_t fgbMemoryCycles;

#pragma pack(push, 8)
// Represents the full instruction register, storing a copy of the register, the data and some required flags and states
typedef struct {
	// A full copy of the instruction definition
	fgbInstruction instruction;
	// Holds the fetched data and addresses
	fgbDataRegister data;
	// The start CPU cycles, used to measure how many cycles it actually was emulated
	fgbTickCycles startTicks;
	// The PC of the first op-code bus read
	uint16_t startPC;
	// The op code byte
	uint8_t opcode;
	// This is set to true, when this is a prefix instruction
	bool isPrefixInstruction;
	// This is set to true, when any jump instruction was setting the PC
	bool wasBranchTaken;
	// Padding to align to 64 bytes
	uint8_t padding[3];
} fgbInstructionRegister;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbInstructionRegister) == 64);

// Defines the CPU states
typedef enum {
	// Normal fetch/decode/execution pipeline
	fgbCPUStateType_Normal = 0,
	// CPU is in halt state, waiting to get to wake-up by an interrupt
	fgbCPUStateType_Halt,
	// CPU is in stop mode, that executes a NOP on DMG
	fgbCPUStateType_Stop,
} fgbCPUStateType;

#pragma pack(push, 8)
// Represents the state of the CPU
typedef struct {
	// The total tick cycles emulated
	fgbTickCycles totalTickCycles;
	// The current memory cycles emulated
	fgbMemoryCycles currentMemoryCycles;
	// Type of the CPU state
	fgbCPUStateType type;
	// The last saved PC, so we can detect infinite loops
	uint16_t lastPC;
	// The last saved SP, so we can detect infinite loops
	uint16_t lastSP;
	// Padding to align to 28-bytes
	uint32_t paddingU32;
	// Number of remaining cycles to exit HALT mode
	uint8_t ticksLeaveHaltMode;
	// Flag that indicates if the PC is skipped (HALT bug)
	bool skipPC;
	// Padding to align to 32-bytes
	bool paddingU8[2];
} fgbCPUState;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbCPUState) == 32);

#pragma pack(push, 8)
// Represents the full CPU with all registers and the state
typedef struct {
	// The instruction register, such as start PC, op-code and a copy of the instruction definition
	fgbInstructionRegister instructionRegister;
	// State
	fgbCPUState state;
	// CPU registers
	fgbCPURegisters registers;
	// Padding to align to 128 bytes
	uint64_t padding[2];
} fgbCPU;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbCPU) == 128);

#pragma pack(push, 1)
// Represents the OAM Entry (4-Byte)
typedef struct {
	// Y-Position
	uint8_t y;
	// X-Position
	uint8_t x;
	// Tile ID
	uint8_t tileID;
	// Anonymous union storing either each 8-bits individually or the full U8 attributes
	union {
		// Anonymous struct storing all 8 bits
		struct {
			uint8_t cgbPaletteNumber : 3;		// Bit 0-2: CGB Color Palette
			bool cgbTileVRAMBank : 1;		// Bit 3: CGB VRAM Bank
			bool paletteNumber : 1;			// Bit 4: DMG/MGB Palette (0 = OBJ0, 1 = OBH1)
			bool horizontalFlip : 1;		// Bit 5: Horizontal Flip (0 = Normal, 1 = Horizontally flipped)
			bool verticalFlip : 1;			// Bit 6: Vertical Flip (0 = Normal, 1 = Vertically flipped)
			bool backgroundPriority : 1;	// Bit 7: Background/Window over Sprite Priority (0 = Sprite have priority, 1 = Background/Window are drawn over sprite)
		};
		// Attributes value
		uint8_t attributes;
	};
} fgbOAMEntry;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbOAMEntry) == 4);

// Size of an OAM entry
#define FGB_OAM_ENTRY_SIZE sizeof(fgbOAMEntry)

// Total number of OAM entries in the table
#define FGB_MAX_OAM_ENTRY_COUNT 40

// Total size of the OAM table
#define FGB_OAM_TABLE_SIZE FGB_OAM_ENTRY_SIZE * FGB_MAX_OAM_ENTRY_COUNT

// Represents the full OAM Table (160 Bytes or 40 Entries)
typedef union {
	// Full memory (160-bytes)
	uint8_t m[FGB_MAX_OAM_ENTRY_COUNT * FGB_OAM_ENTRY_SIZE];
	// OAM entries (40 entries)
	fgbOAMEntry entries[FGB_MAX_OAM_ENTRY_COUNT];					
} fgbOAMTable;
FGB_STATIC_ASSERT(sizeof(fgbOAMTable) == FGB_OAM_TABLE_SIZE);

// The total amount of the tilemap in bytes (Block 0-2)
#define FGB_TOTAL_TILEMAP_SIZE 0x1800

// The width of a single tile in pixels
#define FGB_TILE_WIDTH 8

// The height of a single tile in pixels
#define FGB_TILE_HEIGHT 8

// The total size in bytes for one tile
#define FGB_TILE_SIZE (2 * FGB_TILE_HEIGHT)

// The total amount of tiles in a tilemap (192 tiles)
#define FGB_TOTAL_TILE_COUNT (FGB_TOTAL_TILEMAP_SIZE / FGB_TILE_SIZE)

// The total number of horizontal tiles for the tile map
#define FGB_TILEMAP_HORIZONTAL_COUNT 16

// The total number of vertical tiles for the tile map
#define FGB_TILEMAP_VERTICAL_COUNT 24

// The total width for all horizontal tiles in pixels
#define FGB_TILEMAP_WIDTH (FGB_TILEMAP_HORIZONTAL_COUNT * FGB_TILE_WIDTH)

// The total height for all vertical tiles in pixels
#define FGB_TILEMAP_HEIGHT (FGB_TILEMAP_VERTICAL_COUNT * FGB_TILE_HEIGHT)

// The width of one line of the entire tilemap in bytes
#define FGB_TILEMAP_LINE_WIDTH (FGB_TILE_SIZE * FGB_TILEMAP_HORIZONTAL_COUNT)

// The size of the tilemap area in the VRAM
#define FGB_VRAM_TILEMAP_SIZE 0x8000

// The size of the data area in the VRAM
#define FGB_VRAM_DATA_SIZE 0x800

// The number of horizontal tile/character codes in area 9800h-9ffh
#define FGB_TILEDATA_HORIZONTAL_COUNT 32

// The number of vertical tile/character codes in area 9800h-9ffh
#define FGB_TILEDATA_VERTICAL_COUNT 32

// The size for one tile index
#define FGB_TILEMAP_INDEX_SIZE sizeof(uint8_t)

// The total size of tile/character map per bank (1 KB)
#define FGB_TILEMAP_BANK_SIZE (FGB_TILEDATA_HORIZONTAL_COUNT * FGB_TILEDATA_VERTICAL_COUNT * FGB_TILEMAP_INDEX_SIZE)

// The total size of the entire tile/character map in area 9800h - 9ffh(2 banks * 1 KB)
#define FGB_TILEMAP_TOTAL_SIZE (FGB_TILEMAP_BANK_SIZE * 2)

// The width in pixels of the entire background map
#define FGB_BACKGROUND_MAP_WIDTH (FGB_TILEDATA_HORIZONTAL_COUNT * FGB_TILE_WIDTH)

// The height in pixels of the entire background map
#define FGB_BACKGROUND_MAP_HEIGHT (FGB_TILEDATA_VERTICAL_COUNT * FGB_TILE_HEIGHT)

// First color index
#define FGB_COLOR_INDEX_0 0b00
// Second color index
#define FGB_COLOR_INDEX_1 0b01
// Third color index
#define FGB_COLOR_INDEX_2 0b10
// Fourth color index
#define FGB_COLOR_INDEX_3 0b11

// Defines the color indices
typedef enum {
	// Zero color index (0b00)
	fgbColorIndex_Zero = FGB_COLOR_INDEX_0,
	// One color index (0b01)
	fgbColorIndex_One = FGB_COLOR_INDEX_1,
	// Two color index (0b10)
	fgbColorIndex_Two = FGB_COLOR_INDEX_2,
	// Three color index (0b11)
	fgbColorIndex_Three = FGB_COLOR_INDEX_3
} fgbColorIndex;

#pragma pack(push, 1)
// Represents a single 24-bit color
typedef union {
	// Full memory (3 bytes)
	uint8_t m[3];
	// Anonymous struct that holds the RGB values
	struct {
		// Red component (1 byte, Little endian)
		uint8_t r;
		// Green component (1 byte, Little endian)
		uint8_t g;
		// Blue component (1 byte, Little endian)
		uint8_t b;
	};
} fgbColor;
#pragma pack(pop)

// Represents the colors the maps from the palette for BG, OBJ0, OBJ1 and the system
typedef union {
	// All 16 colors
	fgbColor m[16];
	// Anonymous struct that holds all 4*4 colors
	struct {
		// Background colors: 0 = White (0b00), 1 = Light Gray (0b01), 2 = Gray (0b10), 3 = Dark Gray (0b11)
		fgbColor background[4];
		// Sprite-0 colors: 0 = White (0b00), 1 = Light Gray (0b01), 2 = Gray (0b10), 3 = Dark Gray (0b11)
		fgbColor sprite0[4];
		// Sprite-1 colors: 0 = White (0b00), 1 = Light Gray (0b01), 2 = Gray (0b10), 3 = Dark Gray (0b11)
		fgbColor sprite1[4];
		// System Colors: 0 = Turned off, 1 = Turned on, 2 = Unused, 3 = Unused
		fgbColor system[4];
	};
} fgbMonochromeColors;

// Color palette for DMG (https://i.pinimg.com/originals/05/ad/bc/05adbc3f01b2b7015562ba22a60ac375.jpg)
#define FGB_DMG_COLOR_OFF {202, 220, 159}
#define FGB_DMG_COLOR_ON {130, 153, 30}
#define FGB_DMG_COLOR_00 {206, 226, 113}
#define FGB_DMG_COLOR_01 {125, 153, 13}
#define FGB_DMG_COLOR_10 {48, 98, 48}
#define FGB_DMG_COLOR_11 {15, 56, 15}

// Color palette for MGB (https://www.color-hex.com/color-palette/45300) 
#define FGB_MGB_COLOR_OFF {0, 0, 0}
#define FGB_MGB_COLOR_ON {200, 200, 200}
#define FGB_MGB_COLOR_00 {230, 230, 230}
#define FGB_MGB_COLOR_01 {159, 159, 159}
#define FGB_MGB_COLOR_10 {84, 84, 84}
#define FGB_MGB_COLOR_11 {0, 0, 0}

// SGB Color Palette (Row 2)
// https://en.wikipedia.org/wiki/List_of_video_game_console_palettes

#define FGB_SGB_COLOR_OFF {230, 0, 0}
#define FGB_SGB_COLOR_ON {255, 133, 132}

#define FGB_SGB_OBJ0_COLOR_00 {190, 255, 153}
#define FGB_SGB_OBJ0_COLOR_01 {86, 178, 33}
#define FGB_SGB_OBJ0_COLOR_10 {0, 131, 0}
#define FGB_SGB_OBJ0_COLOR_11 {0, 0, 0}

#define FGB_SGB_OBJ1_COLOR_00 {158, 255, 240}
#define FGB_SGB_OBJ1_COLOR_01 {101, 164, 155}
#define FGB_SGB_OBJ1_COLOR_10 {0, 0, 254}
#define FGB_SGB_OBJ1_COLOR_11 {0, 0, 0}

#define FGB_SGB_BG_COLOR_00 {255, 211, 211}
#define FGB_SGB_BG_COLOR_01 {255, 133, 132}
#define FGB_SGB_BG_COLOR_10 {148, 58, 58}
#define FGB_SGB_BG_COLOR_11 {0, 0, 0}

static fgbMonochromeColors FGB_DEFAULT_DMG_COLORS = {
	.background = {FGB_DMG_COLOR_00, FGB_DMG_COLOR_01, FGB_DMG_COLOR_10, FGB_DMG_COLOR_11},
	.sprite0 = {FGB_DMG_COLOR_00, FGB_DMG_COLOR_01, FGB_DMG_COLOR_10, FGB_DMG_COLOR_11},
	.sprite1 = {FGB_DMG_COLOR_00, FGB_DMG_COLOR_01, FGB_DMG_COLOR_10, FGB_DMG_COLOR_11},
	.system = {FGB_DMG_COLOR_OFF, FGB_DMG_COLOR_ON},
};

static fgbMonochromeColors FGB_DEFAULT_MGB_COLORS = {
	.background = {FGB_MGB_COLOR_00, FGB_MGB_COLOR_01, FGB_MGB_COLOR_10, FGB_MGB_COLOR_11},
	.sprite0 = {FGB_MGB_COLOR_00, FGB_MGB_COLOR_01, FGB_MGB_COLOR_10, FGB_MGB_COLOR_11},
	.sprite1 = {FGB_MGB_COLOR_00, FGB_MGB_COLOR_01, FGB_MGB_COLOR_10, FGB_MGB_COLOR_11},
	.system = {FGB_MGB_COLOR_OFF, FGB_MGB_COLOR_ON},
};

static fgbMonochromeColors FGB_DEFAULT_SGB_COLORS = {
	.background = {FGB_SGB_BG_COLOR_00, FGB_SGB_BG_COLOR_01, FGB_SGB_BG_COLOR_10, FGB_SGB_BG_COLOR_11},
	.sprite0 = {FGB_SGB_OBJ0_COLOR_00, FGB_SGB_OBJ0_COLOR_01, FGB_SGB_OBJ0_COLOR_10, FGB_SGB_OBJ0_COLOR_11},
	.sprite1 = {FGB_SGB_OBJ1_COLOR_00, FGB_SGB_OBJ1_COLOR_01, FGB_SGB_OBJ1_COLOR_10, FGB_SGB_OBJ1_COLOR_11},
	.system = {FGB_SGB_COLOR_OFF, FGB_SGB_COLOR_ON},
};

typedef union {
	uint8_t m[4 * 4 * 3];
	fgbColor colors[4 * 4];
	struct {
		fgbColor background[4];
		fgbColor sprite0[4];
		fgbColor sprite1[4];
		fgbColor system[4]; // 0 = Turned off (black), 1 = Turned on (a bit lighter than black), 2 = Unused, 3 = Unused
	};
} fgbMonochromePalette;

// Defines the PPU modes
typedef enum {
	// Horizontal blank mode
	fgbPPUMode_HBlank = 0b00,
	// Vertical blank mode
	fgbPPUMode_VBlank = 0b01,
	// OAM search mode
	fgbPPUMode_OAMSearch = 0b10,
	// Pixel transfer mode
	fgbPPUMode_PixelTransfer = 0b11,
} fgbPPUMode;

#pragma pack(push, 1)
// Stores the 4 color indices for half a tile line, containing 4 pixels
typedef union {
	// Full 8-bit value
	uint8_t value;
	// Anonymous struct that stores all 8-bits of the tile pixel
	struct {
		uint8_t index0 : 2; // BITS 0-1: First index that maps to fgbColorIndex
		uint8_t index1 : 2; // BITS 2-3: Second index that maps to fgbColorIndex
		uint8_t index2 : 2; // BITS 4-5: Third index that maps to fgbColorIndex
		uint8_t index3 : 2; // BITS 6-7: Fourth index that maps to fgbColorIndex
	};
} fgbTilePixel4;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTilePixel4) == 1);

#pragma pack(push, 1)
// Stores the 8 pixels for a single line in a tile
typedef union {
	// Full 16-bit value
	uint8_t m[2];
	// Anonymous struct that stores the lower and upper bytes of the tile line
	struct {
		// Lower line of pixel colors (Little endian)
		fgbTilePixel4 lower;
		// Upper line of pixel colors (Little endian)
		fgbTilePixel4 upper;
	};
} fgbTileLine;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTileLine) == 2);

#pragma pack(push, 1)
// Represents a full tile containing 8 lines (16 bytes)
typedef union {
	// Full memory (16-bytes)
	uint8_t m[16];
	// Anonymous struct that holds all 8 tile lines
	struct {
		fgbTileLine lines[8];
	};
} fgbTile;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTile) == FGB_TILE_SIZE);

#pragma pack(push, 1)
// Stores all tiles for a VRAM bank (6 KB or 384 tiles)
typedef union {
	// Full tiles memory (6 KB)
	uint8_t m[0x1800];
	// Array of tiles (384)
	fgbTile tiles[FGB_TOTAL_TILE_COUNT];
} fgbVRAMTiles;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbVRAMTiles) == 0x1800);

#pragma pack(push, 1)
// Represents the data for a VRAM bank (2 KB)
typedef union {
	// Full memory (2 KB)
	uint8_t m[FGB_TILEMAP_TOTAL_SIZE];
	// Anonymous struct that holds both banks
	struct {
		// The data of bank 0 (9800-9BFF, 1KB)
		uint8_t bank0[FGB_TILEMAP_BANK_SIZE];
		// The data of bank 1 (9C00-9FFF, 1KB)
		uint8_t bank1[FGB_TILEMAP_BANK_SIZE];
	};
} fgbVRAMData;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbVRAMData) == FGB_TILEMAP_TOTAL_SIZE);

// The total size of a VRAM bank in bytes (8 KB)
#define FGB_VRAM_BANK_SIZE 0x2000

#pragma pack(push, 1)
// Represents a full VRAM bank (8 KB)
typedef union {
	// Full Bank: 0x8000 - 0x9FFF (8 KB)
	uint8_t m[FGB_VRAM_BANK_SIZE];
	// Anonymous struct that holds the tiles area and the data area
	struct {
		// 0x8000 - 0x97FF: Tiles area (0x8000, 6 KB)
		fgbVRAMTiles tiles;
		// 0x9800 - 0x9FFF: Data area (0x800, 2 KB)
		fgbVRAMData data;
	};
} fgbVRAMBank;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbVRAMBank) == FGB_VRAM_BANK_SIZE);

// The total VRAM size (8 KB on DMG, 16 KB on CGB)
#define FGB_VRAM_TOTAL_SIZE FGB_VRAM_BANK_SIZE

#pragma pack(push, 1)
// Represents the entire video ram (8 KB)
typedef union {
	// Full VRAM: 0x8000 - 0x9FFF
	uint8_t m[FGB_VRAM_TOTAL_SIZE];
	// Anonymous struct that holds all banks
	struct {
		// The first 8 KB of Video RAM (Bank 0)
		fgbVRAMBank bank0;
	};
} fgbVRAM;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbVRAM) == FGB_VRAM_TOTAL_SIZE);

#pragma pack(push, 1)
// Stores the X and Y position of a tile
typedef struct {
	// The X position of the tile in range of 0-15
	uint8_t x;
	// The Y position of the tile in range of 0-23
	uint8_t y;
} fgbTilePosition;
#pragma pack(pop)
FGB_STATIC_ASSERT(sizeof(fgbTilePosition) == 2);

// Defines the pixel types
typedef enum {
	// No pixel set
	fgbPixelType_None = 0,
	// Background pixel set
	fgbPixelType_Background,
	// Window pixel set
	fgbPixelType_Window,
	// Sprite-0 pixel set
	fgbPixelType_Sprite0,
	// Sprite-1 pixel set
	fgbPixelType_Sprite1,
} fgbPixelType;

// Defines the value and states for a single pixel
typedef struct {
	// Output RGB color
	fgbColor color;
	// Raw 2 BIT pixel value
	uint8_t value;
	// Current pixel type
	uint8_t type; // fgbPixelType
	// Has sprites higher priority over background (Inverse of background priority)
	bool spritePriority;
	// Padding to align to 8 bytes
	uint8_t padding[2];
} fgbPixel;

// Max number of pixels in the FIFO queue
#define FGB_PIXEL_FIFO_LENGTH 16

#pragma pack(push, 8)
// Represents the full pixel FIFO containing a buffer of 16 entries
typedef struct {
	// Pixel buffer of 16 entries
	fgbPixel pixels[FGB_PIXEL_FIFO_LENGTH];
	// Padding to align to 8 bytes
	uint64_t padding0[3];
	// Pop index (0-15, always wrapped to zero)
	uint8_t out;
	// Push index (0-15, always wrapped to zero)
	uint8_t in;
	// Number of pixels in the queue
	uint8_t len;
	// Struct padding
	uint8_t padding1[5];
} fgbPixelFIFO;
#pragma pack(pop)

// Defines the PPU fetch states.
typedef enum {
	// Query tile ID.
	fgbPPUFetchState_Tile = 0,
	// First byte for the tile.
	fgbPPUFetchState_Data0,
	// Second byte for the tile.
	fgbPPUFetchState_Data1,
	// Waiting step.
	fgbPPUFetchState_Waiting,
	// Pushing pixels to the display.
	fgbPPUFetchState_Push,
} fgbPPUFetchState;

// Sprite linked list entry that stores a copy of the OAM entry.
typedef struct fgbLineSpriteEntry {
	// Pointer to the next entry.
	struct fgbLineSpriteEntry *next;
	// Copy of the OAM entry.
	fgbOAMEntry entry;
} fgbLineSpriteEntry;

// Stores the current sprites for the PPU pipeline.
typedef struct {
	// Fixed memory of a linked list with 10 entries.
	fgbLineSpriteEntry buffer[10];
	// Contains the first OAM entry link, up to 10.
	fgbLineSpriteEntry *first;
	// Number of line sprites (max 10).
	size_t count;
} fgbLineSpriteList;

// Stores the states for the PPU pipeline.
typedef struct {
	// The Y offset in the tile (Range 0-7).
	uint8_t offsetY;
	// Current X position on the line in, regardless if it is visible or not (Range 0-159).
	uint8_t lineX;
	// Current X position pixels will be pushed to the display (Range 0-159).
	uint8_t pushX;
	// Current X position in the FIFO (Range 0-159).
	uint8_t fifoX;
} fgbPipelineState;

// Stores the data of the Pixel FIFO fetcher, such as OAM entries, current tile line/id/data and the X position.
typedef struct {
	// Fetched OAM Entries.
	fgbOAMEntry entries[3];
	// Current fetch state.
	fgbPPUFetchState state;
	// Current fetched Tile line.
	fgbTileLine tileLine;
	// 3 Lines of sprites tiles (6 bytes).
	uint8_t entryTileData[6];
	// Number of fetched OAM entries (Range: 0 to 3).
	uint8_t entryCount;
	// Current fetched Tile ID.
	uint8_t tileID;
	// Current X position for the next pixels to fetch (Range 0-159).
	uint8_t currentX;
	// Padding to align to 32 bytes.
	uint8_t padding[5];
} fgbPPUFetchRegister;

// Stores everything required for the PPU pipeline, such as sprites, pixel-FIFO, fetch-register, positions, etc.
typedef struct {
	// Sprite list.
	fgbLineSpriteList sprites;
	// FIFO state.
	fgbPixelFIFO fifo;
	// Pipeline state.
	fgbPipelineState state;
	// Fetch register.
	fgbPPUFetchRegister fetch;
	// The current tile type (Background/Window).
	fgbPixelType tileType;
	// The current tile position in the background/window tile map (Only for visualization).
	fgbTilePosition tilePos;
	// Padding to align to 4 bytes.
	uint16_t padding;
} fgbPPUPipeline;

// Represents the full state of the DMA.
typedef struct {
	// The current byte offset, incremented until 160 (0xA0).
	uint8_t offset;
	// The base address in multiple of 256, 0 = 0, 1 = 0x100, 2 = 0x200, etc.
	uint8_t base;
	// The number of ticks the DMA should be delayed, before it is going to run.
	uint8_t delay;
	// A flag indicating whether DMA is active or not.
	bool isActive;
} fgbDMA;

// Represents a single tile information in the background map (Debugger only!).
typedef struct {
	// Pixel type used for the tile (fgbPixelType).
	uint8_t type;
	// Is background data area selected.
	bool backgroundDataAreaSelect;
	// Is background/window tiles area selected.
	bool backgroundWindowTilesAreaSelect;
	// Padding to align to 4 bytes.
	uint8_t padding;
} fgbBackgroundMapTileInfo;

// Represents the data for the PPU Background map (Debugger only!).
typedef struct {
	// Tile infos, useful for displaying the correct tiles.
	fgbBackgroundMapTileInfo tilesInfos[FGB_BACKGROUND_MAP_WIDTH * FGB_BACKGROUND_MAP_HEIGHT];
	// 32x32 * 8*8 Colors (RGB).
	fgbColor colors[FGB_BACKGROUND_MAP_WIDTH * FGB_BACKGROUND_MAP_HEIGHT];
	// Horizontal scroll position.
	uint8_t scrollX;
	// Vertical scroll position.
	uint8_t scrollY;
	// Align to 4 bytes.
	uint8_t padding[2];
} fgbPPUBackgroundMap;

// Represents the PPU states such as ticks, current line, flags, etc.
typedef struct {
	// The total number of PPU ticks for an entire frame.
	uint32_t frameTicks;

	// The total number of ticks, while the LCD is turned off (No PPU emulation will be executed, until the LCD is turned on).
	uint32_t lcdOffTicks;

	// The total number of frames.
	uint32_t frameCount;

	// The current number of PPU ticks for a line.
	uint16_t lineTicks;

	// The current line of the window.
	uint8_t windowLine;

	// Padding to align to 16 bytes.
	uint8_t padding;

	// A flag indicating whether a frame is finished and should be displayed on the outside.
	bool isFrameFinished;

	// A flag indicating whether the video RAM was updated.
	bool isVRAMUpdated;

	// A flag indicating whether the background map was updated.
	bool isBackgroundMapUpdated;

	// A flag indicating whether the actual pixels of the Display/Tilemap "should" be updated or not (does not prevent the PPU to tick!).
	bool isDisplayEnabled;

	// A flag indicating whether a LCD STAT interrupt has been requested.
	bool hasLCDStatusInterruptRequested;

	// A flag indicating whether background rendering are enabled.
	bool isBackgroundEnabled;

	// A flag indicating whether window rendering are enabled.
	bool isWindowEnabled;

	// A flag indicating whether sprites rendering are enabled.
	bool isSpritesEnabled;

	// Padding to align to 32 bytes.
	uint8_t padding1[8];
} fgbPPUState;
FGB_STATIC_ASSERT(sizeof(fgbPPUState) == 32);

// Stores the entire data for the PPU (Picture Processing Unit), such as pixels, video ram, pipeline, oam entries, palette, registers, etc.
typedef struct {
	// The background map.
	fgbPPUBackgroundMap backgroundMap;
	// Tile Map Pixels Memory (RGB).
	fgbColor tilemap[FGB_TILEMAP_WIDTH * FGB_TILEMAP_HEIGHT];
	// Pixels Memory (RGB).
	fgbColor display[FGB_DISPLAY_WIDTH * FGB_DISPLAY_HEIGHT];
	// 8K Video RAM (CGB +8 KB as Bank-1).
	fgbVRAM vram;
	// Pixel FIFO pipeline.
	fgbPPUPipeline pipeline;
	// Sprite Attribute Memory (140 bytes or 40 entries).
	fgbOAMTable oam;
	// Current monochrome palettes, mapping each type and its id to an RGBA color.
	fgbMonochromePalette currentMonochromeColors;
	// State.
	fgbPPUState state;
	// LCD Register.
	fgbLCDRegister lcd;
	// DMA (OAM Automatic Data Trasfer).
	fgbDMA dma;
} fgbPPU;

// Defines the log levels
typedef enum {
	// Fatal failure, emulation can't continue
	fgbLogLevel_Fatal = 0,
	// Error that may continue, but will mostly stop emulation
	fgbLogLevel_Error,
	// An issue, but nothing that stops emulation
	fgbLogLevel_Warning,
	// Highlevel operation calls (> 20)
	fgbLogLevel_Info,
	// Medium operation calls (> 100)
	fgbLogLevel_Debug,
	// Tiny operation calls (> 1000)
	fgbLogLevel_Trace,

	// First log level
	fgbLogLevel_First = fgbLogLevel_Fatal,
	// First log level
	fgbLogLevel_Last = fgbLogLevel_Trace,
} fgbLogLevel;

// Function prototype for writing to the log
#define FGB_LOG_CALLBACK(name) void name(void *userData, const fgbLogLevel level, const char *kind, const char *message)

/**
  * @brief Callback for writing to the log
  * @param userData The pointer to the opaque user data
  * @param level The log level
  * @param kind The what-kind-it-is string
  * @param message The message string
  */
typedef FGB_LOG_CALLBACK(fgb_LogCallback);

// Stores the logging configuration, such as callback and user data.
typedef struct {
	// Callback function
	fgb_LogCallback *callback;
	// User data for callback
	void *userData;
	// Are logging enabled
	bool isEnabled;
} fgbLog;

// Minimum boot rom size (256 Bytes)
#define FGB_BOOT_MIN_ROM_SIZE 0x100
// Maximum boot rom size (4 KB)
#define FGB_BOOT_MAX_ROM_SIZE 0x1000

// Contains the data of the boot ROM
typedef struct {
	// Boot rom data
	uint8_t data[FGB_BOOT_MAX_ROM_SIZE];
	// Boot rom length in bytes
	size_t length;
	// Boot rom inserted
	bool isEnabled;
	// Padding to align to 8 bytes
	uint8_t padding[7];
} fgbBootROM;

// Contains the folder path strings for serialization
typedef struct {
	// Full folder path where the external ram files are located
	const char *externalRAMFolderPath;
	// Full folder path where the snapshot files are located
	const char *snapshotFolderPath;
} fgbDirectories;

// Defines the microstep types
typedef enum {
	// CPU tick microstep
	fgbMicroStepType_CPUTick = 0,
	// Hardware tick microstep
	fgbMicroStepType_HardwareTick = 0,
	// Timer microstep
	fgbMicroStepType_Timer,
	// APU microstep
	fgbMicroStepType_APU,
	// PPU microstep
	fgbMicroStepType_PPU,
	// DMA microstep
	fgbMicroStepType_DMA,
	// Bus microstep
	fgbMicroStepType_Bus,

	// First micro step type
	fgbMicroStepType_First = fgbMicroStepType_CPUTick,
	// Last micro step type
	fgbMicroStepType_Last = fgbMicroStepType_Bus,
} fgbMicroStepType;

// Total count of microstep types
#define FGB_MICROSTEP_TYPE_COUNT (fgbMicroStepType_Last - fgbMicroStepType_First) + 1

// Function prototype for a micro step
#define FGB_MICRO_STEP_CALLBACK(name) void name(struct fgbSystem *gb, void *userData, const fgbMicroStepType type, const fgbTickCycles cycles)

/**
  * @brief Callback for a micro step
  * @param gb The reference to the opaque system
  * @param userData The opaque user data
  * @param type The micro step type
  * @param cycles The current number of tick cycles in Hz
  */
typedef FGB_MICRO_STEP_CALLBACK(fgbMicroStepFunc);

// Defines the breakpoint types
typedef enum {
	// LCD Control Power breakpoint
	fgbBreakpointType_LCDControlPower = 0,
	// LCD Control Mode breakpoint
	fgbBreakpointType_LCDControlMode,
	// PPU draw pixel breakpoint
	fgbBreakpointType_PPUDrawPixel,
	// PPU FIFO pop breakpoint
	fgbBreakpointType_PPUFIFOPop,
	// PPU FIFO push breakpoint
	fgbBreakpointType_PPUFIFOPush,
	// PPU begin frame breakpoint
	fgbBreakpointType_PPUFrameBegin,
	// PPU end frame breakpoint
	fgbBreakpointType_PPUFrameEnd,
	// APU write to NR50 breakpoint
	fgbBreakpointType_APUNR50Write,
	// APU write to NR51 breakpoint
	fgbBreakpointType_APUNR51Write,
	// APU write to NR52 breakpoint
	fgbBreakpointType_APUNR52Write,
	// APU write to Voice-1 breakpoint
	fgbBreakpointType_APUVoice1Write,
	// APU write to Voice-2 breakpoint
	fgbBreakpointType_APUVoice2Write,
	// APU write to Voice-3 breakpoint
	fgbBreakpointType_APUVoice3Write,
	// APU write to Voice-4 breakpoint
	fgbBreakpointType_APUVoice4Write,

	// First breakpoint type
	fgbBreakpointType_First = fgbBreakpointType_LCDControlPower,
	// Last breakpoint type
	fgbBreakpointType_Last = fgbBreakpointType_APUVoice4Write,
} fgbBreakpointType;

// Number of breakpoint types
#define FGB_BREAKPOINT_TYPE_COUNT (fgbBreakpointType_Last - fgbBreakpointType_First) + 1

// Function prototype for a breakpoint execution
#define FGB_BREAKPOINT_CALLBACK(name) void name(struct fgbSystem *gb, void *userData, const fgbBreakpointType type)

/**
  * @brief Callback for a breakpoint execution
  * @param gb The reference to the opaque system
  * @param userData The opaque user data
  * @param type The micro step type
  */
typedef FGB_BREAKPOINT_CALLBACK(fgbBreakpointFunc);

// Stores the breakpoints configuration
typedef struct {
	// Breakpoints filter array
	bool filter[FGB_BREAKPOINT_TYPE_COUNT];
	// Callback function
	fgbBreakpointFunc *callback;
	// Userdata for callback
	void *userData;
	// Are breakpoints enabled
	bool isEnabled;
} fgbBreakpoints;

// Stores the micro stepping configuration
typedef struct {
	// Microsteps filter array
	bool filter[FGB_MICROSTEP_TYPE_COUNT];
	// Callback function
	fgbMicroStepFunc *callback;
	// Userdata for callback
	void *userData;
	// Are micro stepping enabled
	bool isEnabled;
} fgbMicroStepping;

// Represents the debug configuration
typedef struct {
	// Micro stepping states
	fgbMicroStepping microStepping;
	// Breakpoint states
	fgbBreakpoints breakpoints;	
} fgbDebug;

// Represents the emulator configuration
typedef struct {
	// The boot rom
	fgbBootROM bootROM;
	// Log configuration
	fgbLog log;
	// The custom colors
	fgbMonochromeColors colors;
	// Debug configuration
	fgbDebug debug;
	// Stored callback functions for e.g. file IO
	fgbCallbacks callbacks;
	// Directories
	fgbDirectories directories;
	// The target sample rate in Hz, if 0 the default of 48000 is used
	uint32_t targetSampleRate;
	// If enabled, the pixels for the screen or tilemap won't be updated
	bool isScreenDisabled;
	// Start the emulation in pause mode, waiting to step or continue it actively
	bool paused;
} fgbConfiguration;

// The size for each Work RAM Bank (4 KB)
#define FGB_WORK_RAM_BANK_SIZE 0x1000

// The total number of Work RAM banks
#define FGB_WORK_RAM_BANK_COUNT 2

// The total size of Work RAM
#define FGB_WORK_RAM_TOTAL_SIZE FGB_WORK_RAM_BANK_SIZE * FGB_WORK_RAM_BANK_COUNT

// Represents a single bank of the work RAM (4 KB)
typedef union {
	// Total of 4 KB
	uint8_t m[FGB_WORK_RAM_BANK_SIZE];
} fgbWorkRAMBank;

// Represents the full work RAM (8 KB or 32 KB)
typedef union {
	// Anonymous struct that holds all banks
	struct {
		// 4 KB Work RAM (Bank 0)
		fgbWorkRAMBank bank0;
		// N*4 KB Work RAM (Bank 1 or N)
		fgbWorkRAMBank bank1_to_N[(FGB_WORK_RAM_BANK_COUNT - 1)];
	};
	// Two work ram banks
	fgbWorkRAMBank banks[FGB_WORK_RAM_BANK_COUNT];
	// 8K or 32K Working RAM (DMG: 2 banks, CGB: 8 Banks)
	uint8_t m[FGB_WORK_RAM_TOTAL_SIZE];
} fgbWorkRAM;

// Represents the high RAM (127 bytes)
typedef struct {
	// 127 bytes High RAM (+1 for padding)
	uint8_t m[0x80];
} fgbHighRAM;

// Represents the work and high RAM (8 KB + 127 Bytes)
typedef struct {
	// Work ram (8 KB)
	fgbWorkRAM work;
	// High ram (127 Bytes)
	fgbHighRAM high;
} fgbRAM;

// Represents the data for the serial data transfer
typedef struct {
	// Serial transfer register
	fgbSerialTransferRegister reg;
	// Align to 4 bytes
	uint8_t padding[2];
} fgbSerialState;

// Represents the full state of the serial data transfer
typedef struct {
	// Current character buffer (~2 KB)
	char data[2032];
	// Number of characters
	size_t count;
	// Serial state
	fgbSerialState state;
} fgbSerial;

// Defines the error types
typedef enum {
	// No error
	fgbErrorType_None = 0,
	// Infinite loop detected
	fgbErrorType_InfiniteLoop,
	// Instruction execution failed
	fgbErrorType_ExecutionError,
} fgbErrorType;

// Stores the current error message and its type
typedef struct {
	// Error message
	char message[508];
	// Error type
	fgbErrorType type;
} fgbError;

// Stores the boot register and its state
typedef struct {
	// Boot register
	uint8_t reg;
	// Is boot rom active
	bool isActive;
	// Padding to align to 4 bytes
	uint8_t padding[2];
} fgbBootState;

// Represents the boot rom and state
typedef struct {
	// Full boot rom
	fgbBootROM rom;
	// Boot state
	fgbBootState state;
} fgbBoot;

// Defines the button states
typedef enum {
	// Button is released
	fgbButtonState_Released = 0,
	// Button is pressed
	fgbButtonState_Pressed = 1,
	// Button is holding down
	fgbButtonState_Hold = 2,
} fgbButtonState;

// Defines the button types
typedef enum {
	// Start button
	fgbButtonType_Start = 0,
	// Select button
	fgbButtonType_Select,
	// A button
	fgbButtonType_A,
	// B button
	fgbButtonType_B,
	// Up button
	fgbButtonType_Up,
	// Down button
	fgbButtonType_Down,
	// Left button
	fgbButtonType_Left,
	// Right button
	fgbButtonType_Right,
} fgbButtonType;

// Represents the state of a controller
typedef union {
	// Anonymous struct holding all button states
	struct {
		// Start button state
		uint8_t start;
		// Select button state
		uint8_t select;
		// A button state
		uint8_t a;
		// B button state
		uint8_t b;
		// Up button state
		uint8_t up;
		// Down button state
		uint8_t down;
		// Left button state
		uint8_t left;
		// Right button state
		uint8_t right;
	};
	// Full memory for all button states
	uint8_t m[8];
} fgbControllerState;

// Stores the registers and states of the joypad
typedef struct {
	// Controller state that was requested by the user
	fgbControllerState requestedState;
	// Current controller state
	fgbControllerState currentState;
	// Current joypad register
	fgbJoypadRegister reg;
	// Last joypad register
	fgbJoypadRegister lastButtonStates;
	// Flag indicating whether the states has been changed
	bool isStateChanged;
	// Padding to align to 8 bytes
	uint8_t padding[5];
} fgbJoypadState;

// Defines the states that are followed after a reset
typedef enum {
	// None
	fgbResetState_None = 0,
	// Pause
	fgbResetState_Pause,
	// Running
	fgbResetState_Running,
} fgbResetState;

// Represents the full emulator backend/system
typedef struct {
	// Fully loaded Game Pak
	fgbGamePak gamePak;
	// Current state of the Picture Processing Unit
	fgbPPU ppu;
	// Current state of the Audio Processing Unit
	fgbAPU apu;
	// Current data of the RAM (Work + High)
	fgbRAM ram;
	// Current boot rom and state
	fgbBoot boot;
	// Current serial state
	fgbSerial serial;
	// Current error state
	fgbError error;
	// Current IO Registers
	fgbIORegisters io;
	// Current CPU state
	fgbCPU cpu;
	// Active callbacks configuration
	fgbCallbacks callbacks;
	// Active debug configuration
	fgbDebug debug;
	// Active monochrome color palette
	fgbMonochromeColors systemMonochromeColors;
	// Joypad
	fgbJoypadState joypad;
	// Active memory bank controller
	fgbMemoryBankController mbc;
	// Active logging configuration
	fgbLog log;
	// Active directories configuration
	fgbDirectories directories;
	// Current timer state
	fgbTimer timer;
	// Current interrupts state
	fgbInterrupts interrupts;
	// Current emulation state
	fgbEmulationState state;
	// Active core type
	fgbCoreType coreType;
	// Current reset state, that is applied after a reset
	fgbResetState resetState;
} fgbSystem;

// Defines the test result types
typedef enum {
	// Test was successful
	fgbTestResultType_Success = 0,
	// Test failed due to invalid argument
	fgbTestResultType_InvalidArguments,
	// Test failed due to invalid game pak
	fgbTestResultType_InvalidGamePak,
	// Test failed due to out-of-memory
	fgbTestResultType_OutOfMemory,
	// Test failed due to emulation failure
	fgbTestResultType_EmulationFailed,
	// Test failed while processing test
	fgbTestResultType_FailedProcessingTest,
	// Test not passed
	fgbTestResultType_NotPassed,
} fgbTestResultType;

// Defines the data for a test result
typedef struct {
	// The output text
	char output[4096];
	// Number of ticks the test took (Filled out by FGB)
	uint64_t tickCycles;
	// Number of CPU tick cycles the test took (Filled out by FGB)
	fgbTickCycles cpuCycles;
	// The length of the output text
	size_t outputLen;
	// First result code, if there is one
	uint8_t codeA;
	// Second result code, if there is one
	uint8_t codeB;
	// If true, the test was finished, regardless of the result
	bool finished;
	// The test succeeded
	bool success;
} fgbTestResultData;

// Function prototype for a test run execution
#define FGB_RUNTEST_FUNC(name) bool name(fgbSystem *system, fgbTestResultData *data)

/**
  * @brief Callback for a test run execution
  * @param gb The reference to the opaque system
  * @param data The reference to the test result data
  * @return Returns a boolean indicating whether the test run was successful or not
  */
typedef FGB_RUNTEST_FUNC(fgbRunTestFunc);

// Defines the initialization result types
typedef enum {
	// Initialization was successful
	fgbInitResult_Success = 0,
	// Initialization failed due to invalid arguments
	fgbInitResult_InvalidArguments,
	// Initialization failed due to invalid game pak
	fgbInitResult_InvalidGamePak,
	// Initialization failed due to a missing boot rom
	fgbInitResult_MissingBootROM,
	// Initialization failed due to a missing game pak
	fgbInitResult_MissingGamePak,
	// Initialization failed due to failure in the core test
	fgbInitResult_CoreTestFailed,
} fgbInitResult;

/**
  * @brief Initializes the specified @ref fgbSystem with the @ref fgbConfiguration and the passed @ref fgbGamePak
  * @param system The reference to the @ref fgbSystem
  * @param config The reference to a @ref fgbConfiguration or null to use a default configuration
  * @param gamePak The reference to a loaded @ref fgbGamePak (please use @ref fgbGamePakLoadFromFile or @fgbGamePakLoadFromMemory to load a valid gamePak)
  * @return The resulting @ref fgbInitResult
  */
FGB_API fgbInitResult fgbInit(fgbSystem *system, const fgbConfiguration *config, const fgbGamePak *gamePak);

/**
  * @brief Shuts down the specified @ref fgbSystem, regardless of its states.
  * @param system The reference to the @ref fgbSystem
  * @note Note that the memory used for the specified @ref fgbSystem is not released!
  */
FGB_API void fgbShutdown(fgbSystem *system);

/**
  * @brief Fetch/Decodes and executes one CPU instruction, when an instruction is executed at least 4 CPU cycles are used
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the emulator tick was executed successfully, false otherwise
  */
FGB_API bool fgbTick(fgbSystem *system);

/**
  * @brief Execute until the next tick
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the emulator was set to step mode, false otherwise
  */
FGB_API bool fgbStep(fgbSystem *system);

/**
  * @brief Activate microstep, so that the emulator waits until the micro-step is skipped or the emulation is resumed
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the emulator was set to micro-step mode, false otherwise
  */
FGB_API bool fgbMicroStep(fgbSystem *system);

/**
  * @brief Resumes the emulation, when paused
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the emulator was resumed, false otherwise
  */
FGB_API bool fgbResume(fgbSystem *system);

/**
  * @brief Pauses the emulation the emulation, when running
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the emulator was paused, false otherwise
  */
FGB_API bool fgbPause(fgbSystem *system);

/**
  * @brief Request an reset to the default settings/start of the rom/boot
  * @param system The reference to the @ref fgbSystem
  * @param paused A value indicating whether the the emulator should be in pause or running state
  * @note The reset is executed in @ref fgbTick() as the very first step
  * @return Returns true when the emulator has a reset requested, false otherwise
  */
FGB_API bool fgbReset(fgbSystem *system, const bool paused);

/**
  * @brief Gets the state of the game boy
  * @param system The reference to the @ref fgbSystem
  * @return Returns the current @ref fgbEmulationState
  */
FGB_API fgbEmulationState fgbGetState(const fgbSystem *system);

/**
  * @brief Gets the name of the specified instruction type
  * @param type The instruction type
  * @return Returns the constant string of the instruction type or NULL
  */
FGB_API const char *fgbGetInstructionName(const fgbInstructionType type);
/**
  * @brief Gets the name of the specified addressing mode
  * @param mode The addressing mode
  * @return Returns the constant string of the addressing mode or NULL
  */
FGB_API const char *fgbGetAddressingModeName(const fgbAddressingMode mode);
/**
  * @brief Gets the name of the specified register type
  * @param reg The register type
  * @return Returns the constant string of the register type or NULL
  */
FGB_API const char *fgbGetRegisterName(const fgbRegisterType reg);

/**
  * @brief Runs a single test
  * @param callbacks Reference to the callbacks structure
  * @param gamePak Reference to the game pak structure
  * @param maxTickCount Maximum number of test ticks the test can run
  * @param func Pointer to the test run function
  * @param data Reference to the output test result data
  * @return Returns the test result type
  */
FGB_API fgbTestResultType fgbRunTest(const fgbCallbacks *callbacks, const fgbGamePak *gamePak, const uint64_t maxTickCount, fgbRunTestFunc *func, fgbTestResultData *data);

/**
  * @brief Decodes a single instruction from the specified ROM memory and position to the output instruction structure
  * @param rom Reference to the ROM memory
  * @param pos Absolute position in the ROM
  * @param outInstruction Reference to the output decoded instruction structure
  * @return Returns a boolean indicating whether the decode was successful or not
  */
FGB_API bool fgbDecodeInstruction(const fgbMemory *rom, const uint32_t pos, fgbDecodedInstruction *outInstruction);

/**
  * @brief Formats the specified instruction into a string buffer
  * @param destBuffer Pointer to the destination buffer
  * @param maxDestBufferLen Total size of the destination buffer
  * @param instruction Reference to the input instruction
  * @return Returns the number of characters required to format the instruction
  */
FGB_API size_t fgbFormatInstruction(char *destBuffer, size_t maxDestBufferLen, const fgbDecodedInstruction *instruction);

/**
  * @brief Reads a single 8-bit value from the bus by the specified address
  * @param system The reference to the @ref fgbSystem
  * @param address The source 16-bit address
  * @return Returns the read 8-bit value or 0xFF
  */
FGB_API uint8_t fgbBusRead8(fgbSystem *system, const uint16_t address);
/**
  * @brief Writes a single 8-bit value to the bus to the specified address
  * @param system The reference to the @ref fgbSystem
  * @param address The target 16-bit address
  * @param value The 8-bit value to write
  */
FGB_API void fgbBusWrite8(fgbSystem *system, const uint16_t address, const uint8_t value);

/**
  * @brief Returns a value indicating whether breakpoints for the specified type are enabled or not
  * @param system The reference to the @ref fgbSystem
  * @param type The breakpoint type
  * @return Returns true when the breakpoints are enabled for the specified type, false otherwise
  */
FGB_API bool fgbIsBreakpointEnabled(fgbSystem *system, const fgbBreakpointType type);
/**
  * @brief Enables or disables a breakpoint by the specified type
  * @param system The reference to the @ref fgbSystem
  * @param type The breakpoint type
  * @param enable A value indicating whether it should enable or disable the breakpoint
  */
FGB_API void fgbBreakpointEnable(fgbSystem *system, const fgbBreakpointType type, const bool enable);
/**
  * @brief Gets the label of the specified breakpoint type
  * @param type The breakpoint type
  * @return Returns the constant label of the breakpoint type or NULL
  */
FGB_API const char *fgbGetBreakpointTypeLabel(const fgbBreakpointType type);

/**
  * @brief Gets the current rom bank index for bank-1
  * @param system The reference to the @ref fgbSystem
  * @return Returns the index of the rom bank-1
  */
FGB_API uint16_t fgbGetROMBank(const fgbSystem *system);

/**
  * @brief Clears the states of all joypad buttons
  * @param system The reference to the @ref fgbSystem
  */
FGB_API void fgbClearButtons(fgbSystem *system);
/**
  * @brief Sets the state of the specified single button and its state
  * @param system The reference to the @ref fgbSystem
  * @param button The button type
  * @param isDown A value indicating whether the button is pressed or not
  */
FGB_API void fgbSetButtonState(fgbSystem *system, const fgbButtonType button, const bool isDown);

/**
  * @brief Sets the current monochrome color palette
  * @param system The reference to the @ref fgbSystem
  * @param colors The reference to the source monochrome color palette
  */
FGB_API void fgbSetColorPalette(fgbSystem *system, const fgbMonochromeColors *colors);

/**
  * @brief Returns a value indicating whether the frame was updated or not
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the frame was updated, false otherwise
  */
FGB_API bool fgbIsFrameUpdated(const fgbSystem *system);
/**
  * @brief Returns a value indicating whether the VRAM was updated or not
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the VRAM was updated, false otherwise
  */
FGB_API bool fgbIsVRAMUpdated(const fgbSystem *system);

// ****************************************************************************
// APU API
// ****************************************************************************

/**
  * @brief Fetches N audio samples for the specified number of audio frames
  * @param system The reference to the @ref fgbSystem
  * @param frameCount The number of audio frames to be fetched
  * @param outSamples A pointer to the output 8-bit sample buffer
  * @return Returns the number of audio frames fetched
  */
FGB_API uint32_t fgbFetchAudioSamples(fgbSystem *system, const uint32_t frameCount, uint8_t *outSamples);

// Defines the voice types
typedef enum {
	// Sweep voice type
	fgbVoiceType_Sweep = 0,
	// Tone voice type
	fgbVoiceType_Tone = 1,
	// Wave voice type
	fgbVoiceType_Wave = 2,
	// Noise voice type
	fgbVoiceType_Noise = 3,
} fgbVoiceType;

// Defines the states for a voice
typedef enum {
	// Voice is powered off
	fgbVoiceState_Off = 0,
	// Powered by not enabled yet
	fgbVoiceState_Powered,
	// Active and powered
	fgbVoiceState_Active,
	// Voice is muted by user
	fgbVoiceState_Muted,

	// First voice state
	fgbVoiceState_First = fgbVoiceState_Off,
	// Last voice state
	fgbVoiceState_Last = fgbVoiceState_Muted,
} fgbVoiceState;

// Total count of voice states
#define FGB_VOICE_STATE_COUNT (fgbVoiceState_Last - fgbVoiceState_First) + 1

// Defines the speaker types
typedef enum {
	// Left speaker
	fgbSpeakerType_Left = 0,
	// Right speaker
	fgbSpeakerType_Right = 1,
} fgbSpeakerType;

/**
  * @brief Gets the voice state for the specified voice type
  * @param system The reference to the @ref fgbSystem
  * @param type The voice type
  * @return The resulting voice state
  */
FGB_API fgbVoiceState fgbGetAudioVoiceState(const fgbSystem *system, const fgbVoiceType type);

/**
  * @brief Gets the current volume for the specified voice type in range of 0.0 to 1.0
  * @param system The reference to the @ref fgbSystem
  * @param type The voice type
  * @return The volume in range of 0.0 (quiet) to 1.0 (loud)
  */
FGB_API float fgbGetAudioVoiceVolume(const fgbSystem *system, const fgbVoiceType type);

/**
  * @brief Gets the current volume for the specified speaker type in range of 0.0 to 1.0
  * @param system The reference to the @ref fgbSystem
  * @param type The speaker type
  * @return The volume in range of 0.0 (quiet) to 1.0 (loud)
  */
FGB_API float fgbGetAudioSpeakerVolume(const fgbSystem *system, const fgbSpeakerType type);

/**
  * @brief Gets a value indicating whether the specified voice is muted or not
  * @param system The reference to the @ref fgbSystem
  * @param type The voice type
  * @return Returns true when the specified voice is muted, false otherwise
  */
FGB_API bool fgbIsAudioVoiceMuted(const fgbSystem *system, const fgbVoiceType type);

/**
  * @brief Mutes or unmutes the specified voice
  * @param system The reference to the @ref fgbSystem
  * @param type The voice type
  * @param mute A value indicating whether the voice should be muted or unmuted
  */
FGB_API void fgbSetAudioVoiceMute(fgbSystem *system, const fgbVoiceType type, const bool mute);

/**
  * @brief Gets a value indicating whether the audio system is powered on or not
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when the audio system is powered on, false otherwise
  */
FGB_API bool fgbIsAudioPowered(const fgbSystem *system);

// ****************************************************************************
//
// Snapshots API
//
// ****************************************************************************

// Defines the allowed snapshot versions
typedef enum {
	// No version
	fgbSnapshotVersion_None = 0,

	// Initial version (1.0)
	fgbSnapshotVersion_Initial,

	// Current version
	fgbSnapshotVersion_Current = fgbSnapshotVersion_Initial,

	// First version
	fgbSnapshotVersion_First = fgbSnapshotVersion_Initial,
	// Last version
	fgbSnapshotVersion_Last = fgbSnapshotVersion_Initial,
} fgbSnapshotVersion;

// Represents a full snapshot of the emulator
typedef struct {
	// PPU state
	fgbPPU ppu;
	// External RAM
	fgbExternalRAM externalRam;
	// APU state
	fgbAPU apu;
	// Internal RAM
	fgbRAM internalRam;
	// Serial transfer state
	fgbSerial serial;
	// IO Registers
	fgbIORegisters io;
	// CPU
	fgbCPU cpu;
	// Game Pak Info
	fgbGamePakInfo gameInfo;
	// Monochrome color palette
	fgbMonochromeColors monochromeColors;
	// Joypad state
	fgbJoypadState joypad;
	// Timer state
	fgbTimer timer;
	// Interrupts state
	fgbInterrupts interrupts;
	// Data of the MBC
	fgbMBCData mbcData;
	// Date time the snapshot was taken
	fgbDateTime dateTime;
	// Version the snapshot was loaded
	fgbSnapshotVersion version;
} fgbSnapshot;

/**
  * @brief Returns value indicating whether the any snapshots are supported or not.
  * @param system The reference to the @ref fgbSystem
  * @return Returns true when snapshots, false otherwise
  */
FGB_API bool fgbAreSnapshotsSupported(const fgbSystem *system);

/**
  * @brief Returns value indicating whether the specified snapshot is valid or not.
  * @param system The reference to the @ref fgbSystem
  * @param snapshot The reference to the @ref fgbSnapshot
  * @return Returns true when the specified snapshot is valid, false otherwise
  */
FGB_API bool fgbIsSnapshotValid(const fgbSystem *system, const fgbSnapshot *snapshot);

/**
  * @brief Exports the given system into the specified snapshot
  * @param system The reference to the @ref fgbSystem
  * @param outSnapshot The reference to the @ref fgbSnapshot
  * @return Returns true when the system was exported into the snapshot, false otherwise
  */
FGB_API bool fgbSnapshotExport(fgbSystem *system, fgbSnapshot *outSnapshot);

/**
  * @brief Saves the specified snapshot into a file from the specified rom file path and slot index
  * @param system The reference to the @ref fgbSystem
  * @param romFilePath The full path to the ROM file
  * @param slotIndex The zero-based slot index in range of 0-5
  * @param snapshot The reference to the source @ref fgbSnapshot
  * @return Returns true when the specified snapshot was saved into a file, false otherwise
  */
FGB_API bool fgbSnapshotSaveToFile(const fgbSystem *system, const char *romFilePath, const uint8_t slotIndex, const fgbSnapshot *snapshot);

/**
  * @brief Loads a snapshot from the specified rom file path and slot index
  * @param system The reference to the @ref fgbSystem
  * @param romFilePath The full path to the ROM file
  * @param slotIndex The zero-based slot index in range of 0-5
  * @param snapshot The reference to the target @ref fgbSnapshot
  * @return Returns true when the specified snapshot was loaded from a file, false otherwise
  */
FGB_API bool fgbSnapshotLoadFromFile(const fgbSystem *system, const char *romFilePath, const uint8_t slotIndex, fgbSnapshot *snapshot);

/**
  * @brief Imports the specified snapshot into the specified system
  * @param system The reference to the @ref fgbSystem
  * @param snapshot The reference to the source @ref fgbSnapshot
  * @return Returns true when the specified snapshot was imported into the system, false otherwise
  */
FGB_API bool fgbSnapshotImport(fgbSystem *system, const fgbSnapshot *snapshot);

#endif // FGB_HEADER

//*******************************************************************************************************************************************
// 
// IMPLEMENTATION / SOURCE
//
//*******************************************************************************************************************************************
#if defined(FGB_IMPLEMENTATION) && !defined(FGB_IMPLEMENTED)
#define FGB_IMPLEMENTED

#include <stdio.h>
#include <math.h>

// Clear the specified reference to the struct to zero
#define fgbClearStruct(ptr) FGB_MEMSET(ptr, 0, sizeof(*(ptr)))

static void fgb__CopyStruct(const void *sourceStruct, void *targetStruct, const size_t sourceSize, const size_t targetSize) {
	FGB_ASSERT(sourceSize == targetSize);
	FGB_ASSERT(sourceStruct != NULL);
	FGB_ASSERT(targetStruct != NULL);
	FGB_MEMCOPY(targetStruct, sourceStruct, sourceSize);
}

// Copy the source struct into the target struct (size and null ptr are validated automatically)
#define fgbCopyStruct(source, target) fgb__CopyStruct(source, target, sizeof(*(source)), sizeof(*(target)))

//
// Disable voices entirely
//
#define FGB_APU_DISABLE_VOICE_SWEEP 0
#define FGB_APU_DISABLE_VOICE_TONE 0
#define FGB_APU_DISABLE_VOICE_WAVE 0
#define FGB_APU_DISABLE_VOICE_NOISE 0

//
// Core/CPU Logging
// 
#define FGB_CPU_INSTRUCTION_TICK_LOGGING 0

#define FGB_BUS_LOGGING 0

#define FGB_INTERRUPT_LOGGING 0

//
// PPU Logging
// 
#define FGB_PPU_TICK_LOGGING 0
#define FGB_PPU_DMA_TRANSFER_LOGGING 0
#define FGB_PPU_VRAM_ACCESS_WARNING_LOGGING 0
#define FGB_PPU_OAM_ACCESS_WARNING_LOGGING 0

//
// APU Logging
// 
#define FGB_APU_VOICE_ENABLE_LOGGING 0
#define FGB_APU_VOICE_TRIGGER_LOGGING 0

#define FGB_APU_WRITE_REGISTER_IO_LOGGING 0
#define FGB_APU_READ_REGISTER_IO_LOGGING 0

#define FGB_APU_NRx4_SET_LOGGING 0

#define FGB_APU_FREQUENCY_SET_LOGGING 0

#define FGB_APU_VOLUME_SET_LOGGING 0
#define FGB_APU_VOLUME_CHANGED_LOGGING 0

#define FGB_APU_LENGTH_CHANGED_LOGGING 0
#define FGB_APU_LENGTH_UPDATED_LOGGING 0

#define FGB_APU_FREQUENCY_SWEEP_CHANGED_LOGGING 0
#define FGB_APU_FREQUENCY_SWEEP_UPDATED_LOGGING 0

//
// MBC Logging
// 
#define FGB_MBC_ROM_BANK_SWITCH_LOGGING 0
#define FGB_MBC_RAM_BANK_SWITCH_LOGGING 0

//
// Forward declarations
//

// Updates the hardware subsystems 4-times, such as audio/video/timer, etc.
static void fgb__HWTick4(fgbSystem *system);

// Read 8-bit from the bus and the specified address
static inline uint8_t fgb__BusRead8(fgbSystem *system, const uint16_t address, const bool tick);

// Read 8-bit directly from the bus and the specified address without any updating any hardware subsystems or ticks/counters
static inline uint8_t fgb__BusRead8_Direct(fgbSystem *system, const uint16_t address);

// Request an interrupt for the specified type
static void fgb__InterruptRequest(fgbSystem *system, const fgbInterruptType type, const char *reason);

// ********************************************************************************************************************
// 
// Utils
// 
// ********************************************************************************************************************

#define FGB__BREAK { *((int *)(uintptr_t)0) = 0x42; }

#define FGB__BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define FGB__BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

static uint16_t fgb__SwapU16(const uint16_t value) {
	uint16_t result = ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00);
	return(result);
}

static size_t fgb__StringFormatArgs(char *destBuffer, const size_t maxDestBufferLen, const char *format, va_list argList) {
	if (format == NULL) {
		return 0;
	}

	va_list listCopy;
	va_copy(listCopy, argList);

	// @NOTE(final): Need to clear the first character, otherwise vsnprintf() does weird things... O_o
	if (destBuffer != NULL) {
		if (maxDestBufferLen == 0) {
			return 0;
		}
		destBuffer[0] = 0;
	}

	int charCount = (int)FGB_STRINGFORMAT(destBuffer, maxDestBufferLen, format, listCopy);
	if (charCount < 0) {
		return 0;
	}
	size_t result = charCount;
	if (destBuffer != NULL) {
		size_t requiredMaxDestBufferLen = charCount + 1;
		if (maxDestBufferLen < requiredMaxDestBufferLen) {
			return 0;
		}
		destBuffer[charCount] = 0;
	}
	va_end(listCopy);
	return(result);
}

static size_t fgb__StringFormat(char *destBuffer, const size_t maxDestBufferLen, const char *format, ...) {
	if (format == NULL) {
		return 0;
	}
	va_list argList;
	va_start(argList, format);
	size_t result = fgb__StringFormatArgs(destBuffer, maxDestBufferLen, format, argList);
	va_end(argList);
	return(result);
}

static size_t fgb__StringCopyLen(const char *source, const size_t sourceLen, char *dest, const size_t maxDestLen) {
	if (source == NULL || dest == NULL) {
		return 0;
	}
	size_t requiredLen = sourceLen;
	if (maxDestLen < requiredLen) {
		return 0;
	}
	FGB_MEMCOPY(dest, source, sourceLen * sizeof(char));
	return sourceLen;
}

static size_t fgb__StringCopy(const char *source, char *dest, const size_t maxDestLen) {
	size_t sourceLen = FGB_STRLEN(source);
	if (sourceLen == 0) {
		return 0;
	}
	size_t result = fgb__StringCopyLen(source, sourceLen, dest, maxDestLen);
	return result;
}

static size_t fgb__StringAppendLen(const char *appended, const size_t appendedLen, char *dest, size_t maxDestLen) {
	if (appended == NULL || appendedLen == 0) {
		return 0;
	}
	size_t curBufferLen = FGB_STRLEN(dest);
	size_t requiredSize = curBufferLen + appendedLen + 1;
	char *start = dest + curBufferLen;
	size_t remainingBufferSize = maxDestLen - (curBufferLen > 0 ? curBufferLen + 1 : 0);
	size_t result = fgb__StringCopyLen(appended, appendedLen, start, remainingBufferSize);
	return result;
}

static size_t fgb__StringAppend(const char *appended, char *buffer, size_t maxBufferLen) {
	size_t appendedLen = FGB_STRLEN(appended);
	size_t result = fgb__StringAppendLen(appended, appendedLen, buffer, maxBufferLen);
	return result;
}

static bool fgb__IsStringEqual(const char *a, const char *b) {
	if (a == NULL || b == NULL) {
		return false;
	}
	size_t lenA = FGB_STRLEN(a);
	size_t lenB = FGB_STRLEN(b);
	if (lenA != lenB) {
		return false;
	}
	for (size_t i = 0; i < lenA; ++i) {
		int r = a[i] - b[i];
		if (r != 0) {
			return false;
		}
	}
	return true;
}

// ********************************************************************************************************************
//
// Interlocked
//
// ********************************************************************************************************************
static int64_t fgb__InterlockedExchange64(volatile int64_t *storage, const int64_t addend) {
	return FGB_INTERLOCKED_EXCHANGE_64(storage, addend);
}

static int64_t fgb__InterlockedExchangeAdd64(volatile int64_t *storage, const int64_t addend) {
	return FGB_INTERLOCKED_EXCHANGE_ADD_64(storage, addend);
}

static int64_t fgb__InterlockedExchangeIncrement64(volatile int64_t *storage) {
	return FGB_INTERLOCKED_EXCHANGE_ADD_64(storage, 1) + 1;
}

static int64_t fgb__InterlockedRead64(volatile int64_t *storage) {
	return FGB_INTERLOCKED_LOAD_64(storage);
}

// ********************************************************************************************************************
//
// File/IO
//
// ********************************************************************************************************************
static bool fgb__FileCreate(const fgbCallbacks *callbacks, const char *filePath, fgbFileHandle *fileHandle) {
	if (fileHandle == NULL || callbacks->createFile == NULL)
		return false;
	return callbacks->createFile(filePath, fileHandle);
}
static bool fgb__FileOpen(const fgbCallbacks *callbacks, const char *filePath, fgbFileHandle *fileHandle) {
	if (fileHandle == NULL || callbacks->openFile == NULL)
		return false;
	return callbacks->openFile(filePath, fileHandle);
}
static void fgb__FileClose(const fgbCallbacks *callbacks, fgbFileHandle *fileHandle) {
	if (fileHandle == NULL || callbacks->closeFile == NULL)
		return;
	callbacks->closeFile(fileHandle);
}
static void fgb__FileFlush(const fgbCallbacks *callbacks, fgbFileHandle fileHandle) {
	if (fileHandle == NULL || callbacks->flushFile == NULL)
		return;
	callbacks->flushFile(fileHandle);
}
static size_t fgb__FileWrite(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const void *buffer, const size_t writeSize) {
	if (fileHandle == NULL || callbacks->writeFile == NULL)
		return 0;
	return callbacks->writeFile(fileHandle, buffer, writeSize);
}
static size_t fgb__FileRead(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, void *buffer, const size_t maxBufferLen, const size_t readSize) {
	if (fileHandle == NULL || callbacks->readFile == NULL)
		return 0;
	return callbacks->readFile(fileHandle, buffer, maxBufferLen, readSize);
}
static size_t fgb__GetFileSize(const fgbCallbacks *callbacks, fgbFileHandle fileHandle) {
	if (fileHandle == NULL || callbacks->getFileSize == NULL)
		return 0;
	return callbacks->getFileSize(fileHandle);
}
static bool fgb__BuildFilePath(const fgbCallbacks *callbacks, const char *filePath, const char *folderPath, const fgbFileType fileType, char *outBuffer, const size_t maxBufferLen, const uint8_t slotIndex) {
	if (callbacks->buildFilePath == NULL || filePath == NULL || fileType  == fgbFileType_None)
		return false;
	return callbacks->buildFilePath(filePath, folderPath, fileType, outBuffer, maxBufferLen, slotIndex);
}
static void *fgb__AllocateMemory(const fgbCallbacks *callbacks, const size_t size) {
	if (callbacks->allocateMemory == NULL || size == 0)
		return NULL;
	return callbacks->allocateMemory(size, callbacks->memoryAllocationUserData);
}
static void fgb__FreeMemory(const fgbCallbacks *callbacks, void *ptr) {
	if (callbacks->freeMemory == NULL || ptr == NULL)
		return;
	callbacks->freeMemory(ptr, callbacks->memoryAllocationUserData);
}

static fgbDateTime fgb__DateTimeQuery(const fgbCallbacks *callbacks) {
	if (callbacks->dateTimeQuery == NULL) {
		fgbDateTime zero = { 0 };
		return zero;
	}
	return callbacks->dateTimeQuery();
}

static void fgb__FileWriteU8(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const uint8_t u8) {
	fgb__FileWrite(callbacks, fileHandle, &u8, sizeof(u8));
}
static bool fgb__FileReadU8(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, uint8_t *outU8) {
	size_t s = sizeof(*outU8);
	size_t read = fgb__FileRead(callbacks, fileHandle, outU8, s, s);
	return read == s;
}
static void fgb__FileWriteS8(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const int8_t s8) {
	fgb__FileWrite(callbacks, fileHandle, &s8, sizeof(s8));
}
static bool fgb__FileReadS8(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, int8_t *outS8) {
	size_t s = sizeof(*outS8);
	size_t read = fgb__FileRead(callbacks, fileHandle, outS8, s, s);
	return read == s;
}
static void fgb__FileWriteU16(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const uint16_t u16) {
	fgb__FileWrite(callbacks, fileHandle, &u16, sizeof(u16));
}
static bool fgb__FileReadU16(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, uint16_t *outU16) {
	size_t s = sizeof(*outU16);
	size_t read = fgb__FileRead(callbacks, fileHandle, outU16, s, s);
	return read == s;
}
static void fgb__FileWriteU32(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const uint32_t u32) {
	fgb__FileWrite(callbacks, fileHandle, &u32, sizeof(u32));
}
static bool fgb__FileReadU32(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, uint32_t *outU32) {
	size_t s = sizeof(*outU32);
	size_t read = fgb__FileRead(callbacks, fileHandle, outU32, s, s);
	return read == s;
}
static void fgb__FileWriteU64(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const uint64_t u64) {
	fgb__FileWrite(callbacks, fileHandle, &u64, sizeof(u64));
}
static bool fgb__FileReadU64(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, uint64_t *outU64) {
	size_t s = sizeof(*outU64);
	size_t read = fgb__FileRead(callbacks, fileHandle, outU64, s, s);
	return read == s;
}
static void fgb__FileWriteSize(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const size_t size) {
	fgb__FileWrite(callbacks, fileHandle, &size, sizeof(size));
}
static bool fgb__FileReadSize(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, size_t *outSize) {
	size_t s = sizeof(*outSize);
	size_t read = fgb__FileRead(callbacks, fileHandle, outSize, s, s);
	return read == s;
}
static void fgb__FileWriteStruct(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const size_t size, const void *inStruct) {
	fgb__FileWriteSize(callbacks, fileHandle, size);
	fgb__FileWrite(callbacks, fileHandle, inStruct, size);
}

#define fgb__FileWriteAutoStruct(callbacks, fileHandle, inStruct) fgb__FileWriteStruct(callbacks, fileHandle, sizeof(*(inStruct)), inStruct)

static bool fgb__FileReadStruct(const fgbCallbacks *callbacks, fgbFileHandle fileHandle, const size_t size, void *outStruct) {
	size_t readSize = 0;
	if (!fgb__FileReadSize(callbacks, fileHandle, &readSize)) {
		return false;
	}
	if (readSize != size) {
		return false;
	}
	size_t readData = fgb__FileRead(callbacks, fileHandle, outStruct, size, size);
	if (readData != size) {
		return false;
	}
	return true;
}

#define fgb__FileReadAutoStruct(callbacks, fileHandle, outStruct) fgb__FileReadStruct(callbacks, fileHandle, sizeof(*(outStruct)), outStruct)

// ********************************************************************************************************************
//
// Logging
//
// ********************************************************************************************************************
static const char *fgb__logLevelToNameTable[] = {
	[fgbLogLevel_Fatal] = "Fatal",
	[fgbLogLevel_Error] = "Error",
	[fgbLogLevel_Warning] = "Warn ",
	[fgbLogLevel_Info] = "Info ",
	[fgbLogLevel_Debug] = "Debug",
	[fgbLogLevel_Trace] = "Trace",
};

static void fgb__LogArgs(const fgbSystem *system, const fgbLogLevel level, const char *kind, const char *format, va_list args) {
	if (!system->log.isEnabled || system->log.callback == NULL)
		return;

	static char logBuffer[1024];

	va_list localArgs;
	va_copy(localArgs, args);

	fgb__StringFormatArgs(logBuffer, FGB_ARRAYCOUNT(logBuffer), format, localArgs);

	system->log.callback(system->log.userData, level, kind, logBuffer);

	va_end(localArgs);
}

static void fgb__Log(const fgbSystem *system, const fgbLogLevel level, const char *kind, const char *format, ...) {
	if (!system->log.isEnabled)
		return;

	va_list args;
	va_start(args, format);
	fgb__LogArgs(system, level, kind, format, args);
	va_end(args);
}

#define FGB__TRACE(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Trace, kind, format, ## __VA_ARGS__)
#define FGB__DEBUG(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Debug, kind, format, ## __VA_ARGS__)
#define FGB__INFO(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Info, kind, format, ## __VA_ARGS__)
#define FGB__WARN(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Warning, kind, format, ## __VA_ARGS__)
#define FGB__ERROR(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Error, kind, format, ## __VA_ARGS__)
#define FGB__FATAL(system, kind, format, ...) fgb__Log(system, fgbLogLevel_Fatal, kind, format, ## __VA_ARGS__)

static void FGB__Failure(fgbSystem *system, const fgbErrorType type, const char *kind, const char *format, ...) {
	system->state = fgbEmulationState_Error;
	system->error.type = type;

	// Format error message
	{
		va_list args;
		va_start(args, format);
		fgb__StringFormatArgs(system->error.message, FGB_ARRAYCOUNT(system->error.message), format, args);
		va_end(args);
	}

#if 0
	// Hard crash, to stop the debugger here
	FGB__BREAK;
#endif

	// Output to log
	if (system->log.isEnabled) {
		va_list args;
		va_start(args, format);
		fgb__LogArgs(system, fgbLogLevel_Fatal, kind, format, args);
		va_end(args);
	}
}

#define fgb__KindName_Core "Core"
#define fgb__KindName_Interrupts "Interrupts"
#define fgb__KindName_GamePak "GamePak"
#define fgb__KindName_MBC "MBC"
#define fgb__KindName_Bus "Bus"
#define fgb__KindName_CPU "CPU"
#define fgb__KindName_PPU "PPU"
#define fgb__KindName_IO "IO"
#define fgb__KindName_RAM "RAM"
#define fgb__KindName_DMA "DMA"
#define fgb__KindName_APU "APU"

#define FGB__STR_NX(str) #str
#define FGB__STR(str) FGB__STR_NX(str)
#define FGB__CONCAT_NX(a, b) a ## b
#define FGB__CONCAT(a, b) FGB__CONCAT_NX(a, b)

#define FGB__SYSNAME_ADDON(baseName, addonName) FGB__CONCAT(FGB__CONCAT(baseName, "::"), addonName)

// ********************************************************************************************************************
// 
// Breakpoints / Microstepping
// 
// ********************************************************************************************************************

static void fgb__MicroStep(fgbSystem *system, const fgbMicroStepType type) {
	fgbMicroStepping *ms = &system->debug.microStepping;
	if (!ms->isEnabled || !ms->filter[type]) {
		return;
	}
	if (ms->callback != NULL) {
		ms->callback((struct fgbSystem *)system, ms->userData, type, system->cpu.state.totalTickCycles);
	}
}

static bool fgb__Breakpoint(fgbSystem *system, const fgbBreakpointType type) {
	fgbBreakpoints *bp = &system->debug.breakpoints;
	if (!bp->isEnabled || (!bp->filter[type])) {
		return false;
	}

	system->state = fgbEmulationState_Breakpoint;

	system->ppu.state.isFrameFinished = true;

	if (bp->callback != NULL) {
		bp->callback((struct fgbSystem *)system, bp->userData, type);
	}

	return true;
}

FGB_API bool fgbIsBreakpointEnabled(fgbSystem *system, const fgbBreakpointType type) {
	if (system == NULL || type < fgbBreakpointType_First || type > fgbBreakpointType_Last) {
		return false;
	}
	return system->debug.breakpoints.filter[type];
}

FGB_API void fgbBreakpointEnable(fgbSystem *system, const fgbBreakpointType type, const bool enable) {
	if (system == NULL || type < fgbBreakpointType_First || type > fgbBreakpointType_Last) {
		return;
	}
	system->debug.breakpoints.filter[type] = enable;
}

static const char *fgb__breakpointTypeNameMap[] = {
	[fgbBreakpointType_LCDControlPower] = "LCDC Power",
	[fgbBreakpointType_LCDControlMode] = "LCDC Mode",
	[fgbBreakpointType_PPUDrawPixel] = "PPU Pixel",
	[fgbBreakpointType_PPUFIFOPush] = "FIFO Push",
	[fgbBreakpointType_PPUFIFOPop] = "FIFO Pop",
	[fgbBreakpointType_PPUFrameBegin] = "Frame Begin",
	[fgbBreakpointType_PPUFrameEnd] = "Frame End",
	[fgbBreakpointType_APUNR50Write] = "NR50 (Vol)",
	[fgbBreakpointType_APUNR51Write] = "NR51 (Channel)",
	[fgbBreakpointType_APUNR52Write] = "NR52 (Master)",
	[fgbBreakpointType_APUVoice1Write] = "Voice1",
	[fgbBreakpointType_APUVoice2Write] = "Voice2",
	[fgbBreakpointType_APUVoice3Write] = "Voice3",
	[fgbBreakpointType_APUVoice4Write] = "Voice4",
};

FGB_API const char *fgbGetBreakpointTypeLabel(const fgbBreakpointType type) {
	if (type < fgbBreakpointType_First || type > fgbBreakpointType_Last) {
		return NULL;
	}
	return fgb__breakpointTypeNameMap[type];
}

// ********************************************************************************************************************
// 
// Memory Map / Address Ranges
// 
// ********************************************************************************************************************

// 0000 - 00FF: Boot ROM 0
#define FGB__BUS_ADDRESS_BOOT_FROM 0x0000
#define FGB__BUS_ADDRESS_BOOT_TO 0x00FF

// 0000 - 7FFF: Gamepak ROM (Real start is 0100)
#define FGB__BUS_ADDRESS_ROM_FROM 0x0000
#define FGB__BUS_ADDRESS_ROM_TO 0x7FFF

// 8000: VRAM Begin
#define FGB__BUS_ADDRESS_PPU_VRAM_FROM 0x8000

// 8000 - 97FF: PPU Tiles
#define FGB__BUS_ADDRESS_PPU_TILES_FROM 0x8000
#define FGB__BUS_ADDRESS_PPU_TILES_TO 0x97FF

// 9800 - 9FFF: PPU Data
#define FGB__BUS_ADDRESS_PPU_DATA_FROM 0x9800
#define FGB__BUS_ADDRESS_PPU_DATA_TO 0x9FFF

// 9FFF: VRAM End
#define FGB__BUS_ADDRESS_PPU_VRAM_TO 0x9FFF

// A000 - BFFF: External RAM
#define FGB__BUS_ADDRESS_EXTERNAL_RAM_FROM 0xA000
#define FGB__BUS_ADDRESS_EXTERNAL_RAM_TO 0xBFFF

// C000 - CFFF: Work RAM Bank 0
#define FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM 0xC000
#define FGB__BUS_ADDRESS_WORK_RAM_BANK0_TO 0xCFFF	

// D000 - DFFF: Work RAM Bank 1 OR N
#define FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM 0xD000
#define FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_TO 0xDFFF

// E000 - FDFF: Shadow RAM (Mappes directly to C000 to 0xCDFF)
#define FGB__BUS_ADDRESS_SHADOW_RAM_FROM 0xE000
#define FGB__BUS_ADDRESS_SHADOW_RAM_TO 0xFDFF

// FE00 - FE9F: OAM RAM
#define FGB__BUS_ADDRESS_PPU_OAM_FROM 0xFE00
#define FGB__BUS_ADDRESS_PPU_OAM_TO 0xFE9F

// FEA0 - FEFF: Unused / Access Denied
#define FGB__BUS_ADDRESS_PROHIBITED_FROM 0xFEA0
#define FGB__BUS_ADDRESS_PROHIBITED_TO 0xFEFF

// -> FF00: Start of IO-Registers
#define FGB__BUS_ADDRESS_IO_REGISTERS_FROM 0xFF00

// FF01 - FF02: Serial Register
#define FGB__BUS_ADDRESS_SERIAL_REGISTER_FROM 0xFF01
#define FGB__BUS_ADDRESS_SERIAL_REGISTER_TO 0xFF02

// FF04 - FF07: Timer Register
#define FGB__BUS_ADDRESS_TIMER_REGISTER_FROM 0xFF04
#define FGB__BUS_ADDRESS_TIMER_REGISTER_TO 0xFF07

// FF10 - FF3F: Sound Register
#define FGB__BUS_ADDRESS_SOUND_REGISTER_FROM 0xFF10
#define FGB__BUS_ADDRESS_SOUND_REGISTER_TO 0xFF3F

// FF40 - FF4B: LCD Register
#define FGB__BUS_ADDRESS_PPU_LCD_REGISTER_FROM 0xFF40
#define FGB__BUS_ADDRESS_PPU_LCD_REGISTER_TO 0xFF4B

// FF4C - FF7F: End of IO-Registers
#define FGB__BUS_ADDRESS_IO_REGISTERS_TO 0xFF7F

// FF80 - FFFE: High RAM
#define FGB__BUS_ADDRESS_HIGH_RAM_FROM 0xFF80
#define FGB__BUS_ADDRESS_HIGH_RAM_TO 0xFFFE

// FF00: Controller Register (P1)
#define FGB__BUS_ADDRESS_CONTROLLER_REGISTER 0xFF00

// FF0F: Interrupt Request Reigster
#define FGB__BUS_ADDRESS_INTERRUPT_REQUEST_REGISTER 0xFF0F

// FFFF: Interrupt Enable Reigster
#define FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER 0xFFFF

// FF50: Boot ROM Register
#define FGB__BUS_ADDRESS_BOOT_ROM_REGISTER 0xFF50

// ********************************************************************************************************************
// 
// External RAM Save/Load Implementation
// 
// ********************************************************************************************************************

// Only allow to save every 30 seconds
#define FGB__EXTERNAL_RAM_SAVE_TIME_DELTA_MS (1000ULL * 30ULL)

static char fgb__ExternalRAMFilePathBuffer[2048];

static bool fgb__ExternalRAMSave(fgbSystem *system, const fgbGamePak *outGamePak) {
	const fgbCallbacks *cb = &system->callbacks;

	if (!fgb__BuildFilePath(cb, outGamePak->filePath, system->directories.externalRAMFolderPath, fgbFileType_ExternalRAM, fgb__ExternalRAMFilePathBuffer, sizeof(fgb__ExternalRAMFilePathBuffer), 0)) {
		return false;
	}

	const char *filePath = fgb__ExternalRAMFilePathBuffer;
	if (FGB_STRLEN(filePath) == 0)
		return false;

	fgbFileHandle fileHandle;
	if (!fgb__FileCreate(cb, filePath, &fileHandle)) {
        return false;
    }

    fgbExternalRAMStateHeader header = { 0 };
    header.magic = FGB_EXTERNAL_RAMSTATE_MAGIC_KEY;
    header.version = fgbExternalRAMStateVersion_Latest;
    header.bankCount = outGamePak->info.ramBankCount;
    header.controller = outGamePak->info.mbcType;

	fgb__FileWrite(cb, fileHandle, &header, sizeof(header));

    fgb__FileWrite(cb, fileHandle, outGamePak->ram.memory.data, outGamePak->ram.memory.length);

    fgb__FileFlush(cb, fileHandle);

    fgb__FileClose(cb, fileHandle);

	return true;
}

static bool fgb__ExternalRAMLoad(fgbSystem *system, fgbGamePak *outGamePak) {
	const fgbCallbacks *cb = &system->callbacks;

	if (!fgb__BuildFilePath(cb, outGamePak->filePath, system->directories.externalRAMFolderPath, fgbFileType_ExternalRAM, fgb__ExternalRAMFilePathBuffer, sizeof(fgb__ExternalRAMFilePathBuffer), 0)) {
		return false;
	}

	const char *filePath = fgb__ExternalRAMFilePathBuffer;
	if (FGB_STRLEN(filePath) == 0)
		return false;

	bool result = false;

	FGB__INFO(system, fgb__KindName_MBC, "Load External RAM with size '%zu' from rom file '%s'", outGamePak->ram.memory.length, filePath);

	fgbFileHandle fileHandle;

	if (!fgb__FileOpen(cb, filePath, &fileHandle)) {
		return false;
	}

	fgbExternalRAMStateHeader header = { 0 };

	size_t read = fgb__FileRead(cb, fileHandle, &header, sizeof(header), sizeof(header));
	if (read != sizeof(header)) {
		goto done;
	}

	if (header.magic != FGB_EXTERNAL_RAMSTATE_MAGIC_KEY || header.version < fgbExternalRAMStateVersion_First || header.version > fgbExternalRAMStateVersion_Latest) {
		goto done;
	}

	if (header.bankCount != outGamePak->info.ramBankCount) {
		goto done;
	}

	uint32_t ramSize = header.bankCount * FGB_RAM_SIZE_PER_BANK;

	if (ramSize < FGB_MIN_EXTERNAL_RAM_SIZE || ramSize > FGB_MAX_EXTERNAL_RAM_SIZE) {
		goto done;
	}

	if (ramSize != outGamePak->ram.memory.length) {
		goto done;
	}

	read = fgb__FileRead(cb, fileHandle, outGamePak->ram.memory.data, outGamePak->ram.memory.length, outGamePak->ram.memory.length);
	if (read != outGamePak->ram.memory.length) {
		goto done;
	}

	result = true;

done:
	fgb__FileClose(cb, fileHandle);

	return result;
}

// ********************************************************************************************************************
// 
// GamePak Implementation
// 
// ********************************************************************************************************************

// Hash-Function for Nintendo Logo Compare: Fowler–Noll–Vo (FNV)
#define FGB_LOGO_HASH_BASE 2166136261u
#define FGB_LOGO_HASH_FACTOR 16777619u

// Total checksum of original nintendo logo (No need for original bytes)
#define FGB_NINTENDO_LOGO_CHECKSUM 0x016bad3f

static const char *fgb__GamePakTypeToNameTable[256] = {
	[fgbGamePakType_ROM] = "ROM",
	[fgbGamePakType_MBC1] = "MBC1",
	[fgbGamePakType_MBC1_RAM] = "MBC1+RAM",
	[fgbGamePakType_MBC1_RAM_BATTERY] = "MBC1+RAM+BATTERY",
	[fgbGamePakType_MBC2] = "MBC2",
	[fgbGamePakType_MBC2_BATTERY] = "MBC2+BATTERY",
	[fgbGamePakType_ROM_BATTERY] = "ROM+BATTERY",
	[fgbGamePakType_ROM_RAM_BATTERY] = "ROM+RAM+BATTERY",
	[fgbGamePakType_MMM01] = "MMM01",
	[fgbGamePakType_MMM01_RAM] = "MMM01+RAM",
	[fgbGamePakType_MMM01_RAM_BATTERY] = "MMM01+RAM+BATTERY",
	[fgbGamePakType_MBC3_TIMER_BATTERY] = "MBC3+TIMER+BATTERY",
	[fgbGamePakType_MBC3_TIMER_RAM_BATTERY] = "MBC3+TIMER+RAM+BATTERY",
	[fgbGamePakType_MBC3] = "MBC3",
	[fgbGamePakType_MBC3_RAM] = "MBC3+RAM",
	[fgbGamePakType_MBC3_RAM_BATTERY] = "MBC3+RAM+BATTERY",
	[fgbGamePakType_MBC5] = "MBC5",
	[fgbGamePakType_MBC5_RAM] = "MBC5+RAM",
	[fgbGamePakType_MBC5_RAM_BATTERY] = "MBC5+RAM+BATTERY",
	[fgbGamePakType_MBC5_RUMBLE] = "MBC5+RUMBLE",
	[fgbGamePakType_MBC5_RUMBLE_RAM] = "MBC5+RUMBLE+RAM",
	[fgbGamePakType_MBC5_RUMBLE_RAM_BATTERY] = "MBC5+RUMBLE+RAM+BATTERY",
	[fgbGamePakType_MBC6] = "MBC6",
	[fgbGamePakType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
	[fgbGamePakType_POCKET_CAMERA] = "POCKET+CAMERA",
	[fgbGamePakType_BANDAI_TAMA5] = "Bandai TAMA5",
	[fgbGamePakType_HUC3] = "HuC3",
	[fgbGamePakType_HUC1_RAM_BATTERY] = "HuC1+RAM+BATTERY",
};

FGB_API const char *fgbGetGamePakTypeName(const fgbGamePakType type) {
	if (type < FGB_ARRAYCOUNT(fgb__GamePakTypeToNameTable))
		return fgb__GamePakTypeToNameTable[type];
	return NULL;
}

static fgbMemoryControllerType fgb__GamePakTypeToMemoryControllerTypeTable[256] = {
	[fgbGamePakType_ROM] = fgbMemoryControllerType_ROM,
	[fgbGamePakType_MBC1] = fgbMemoryControllerType_MBC1,
	[fgbGamePakType_MBC1_RAM] = fgbMemoryControllerType_MBC1,
	[fgbGamePakType_MBC1_RAM_BATTERY] = fgbMemoryControllerType_MBC1,
	[fgbGamePakType_MBC2] = fgbMemoryControllerType_MBC2,
	[fgbGamePakType_MBC2_BATTERY] = fgbMemoryControllerType_MBC2,
	[fgbGamePakType_ROM_BATTERY] = fgbMemoryControllerType_ROM,
	[fgbGamePakType_ROM_RAM_BATTERY] = fgbMemoryControllerType_ROM,
	[fgbGamePakType_MMM01] = fgbMemoryControllerType_MMM01,
	[fgbGamePakType_MMM01_RAM] = fgbMemoryControllerType_MMM01,
	[fgbGamePakType_MMM01_RAM_BATTERY] = fgbMemoryControllerType_MMM01,
	[fgbGamePakType_MBC3_TIMER_BATTERY] = fgbMemoryControllerType_MBC3,
	[fgbGamePakType_MBC3_TIMER_RAM_BATTERY] = fgbMemoryControllerType_MBC3,
	[fgbGamePakType_MBC3] = fgbMemoryControllerType_MBC3,
	[fgbGamePakType_MBC3_RAM] = fgbMemoryControllerType_MBC3,
	[fgbGamePakType_MBC3_RAM_BATTERY] = fgbMemoryControllerType_MBC3,
	[fgbGamePakType_MBC5] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC5_RAM] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC5_RAM_BATTERY] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC5_RUMBLE] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC5_RUMBLE_RAM] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC5_RUMBLE_RAM_BATTERY] = fgbMemoryControllerType_MBC5,
	[fgbGamePakType_MBC6] = fgbMemoryControllerType_MBC6,
	[fgbGamePakType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = fgbMemoryControllerType_MBC7,
	[fgbGamePakType_POCKET_CAMERA] = fgbMemoryControllerType_ROM, // Unknown
	[fgbGamePakType_BANDAI_TAMA5] = fgbMemoryControllerType_ROM, // Unknown
	[fgbGamePakType_HUC3] = fgbMemoryControllerType_HUC3,
	[fgbGamePakType_HUC1_RAM_BATTERY] = fgbMemoryControllerType_HUC1,
};

typedef struct {
	uint32_t success;
	fgbMemoryControllerType type;
} fgbMemoryControllerTypeResult;

static fgbMemoryControllerTypeResult fgb__LookupMemoryControllerType(const fgbGamePakType type) {
	if (type < FGB_ARRAYCOUNT(fgb__GamePakTypeToMemoryControllerTypeTable)) {
		fgbMemoryControllerType match = fgb__GamePakTypeToMemoryControllerTypeTable[type];
		return (fgbMemoryControllerTypeResult) { true, match };
	}
	return (fgbMemoryControllerTypeResult) {0};
}

static const char *fgb__MemoryControllerTypeToNameTable[256] = {
	[fgbMemoryControllerType_ROM] = "ROM",
	[fgbMemoryControllerType_MBC1] = "MBC1",
	[fgbMemoryControllerType_MBC2] = "MBC2",
	[fgbMemoryControllerType_MMM01] = "MMM01",
	[fgbMemoryControllerType_MBC3] = "MBC3",
	[fgbMemoryControllerType_MBC5] = "MBC5",
	[fgbMemoryControllerType_MBC6] = "MBC6",
	[fgbMemoryControllerType_MBC7] = "MBC7",
	[fgbMemoryControllerType_HUC1] = "HUC1",
	[fgbMemoryControllerType_HUC3] = "HUC3",
};

FGB_API const char *fgbGetMemoryControllerTypeName(const fgbMemoryControllerType type) {
	if (type < FGB_ARRAYCOUNT(fgb__MemoryControllerTypeToNameTable))
		return fgb__MemoryControllerTypeToNameTable[type];
	return NULL;
}

static fgbGamePakFeature fgb__GamePakTypeToFeatureTable[256] = {
	[fgbGamePakType_ROM] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC1] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC1_RAM] = fgbGamePakFeature_RAM,
	[fgbGamePakType_MBC1_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC2] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC2_BATTERY] = fgbGamePakFeature_BATTERY,
	[fgbGamePakType_ROM_BATTERY] = fgbGamePakFeature_BATTERY,
	[fgbGamePakType_ROM_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MMM01] = fgbGamePakFeature_None,
	[fgbGamePakType_MMM01_RAM] = fgbGamePakFeature_RAM,
	[fgbGamePakType_MMM01_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC3_TIMER_BATTERY] = fgbGamePakFeature_TIMER | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC3_TIMER_RAM_BATTERY] = fgbGamePakFeature_TIMER | fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC3] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC3_RAM] = fgbGamePakFeature_RAM,
	[fgbGamePakType_MBC3_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC5] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC5_RAM] = fgbGamePakFeature_RAM,
	[fgbGamePakType_MBC5_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC5_RUMBLE] = fgbGamePakFeature_RUMBLE,
	[fgbGamePakType_MBC5_RUMBLE_RAM] = fgbGamePakFeature_RUMBLE | fgbGamePakFeature_RAM,
	[fgbGamePakType_MBC5_RUMBLE_RAM_BATTERY] = fgbGamePakFeature_RUMBLE | fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_MBC6] = fgbGamePakFeature_None,
	[fgbGamePakType_MBC7_SENSOR_RUMBLE_RAM_BATTERY] = fgbGamePakFeature_SENSOR | fgbGamePakFeature_RUMBLE | fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
	[fgbGamePakType_POCKET_CAMERA] = fgbGamePakFeature_None,
	[fgbGamePakType_BANDAI_TAMA5] = fgbGamePakFeature_None,
	[fgbGamePakType_HUC3] = fgbGamePakFeature_None,
	[fgbGamePakType_HUC1_RAM_BATTERY] = fgbGamePakFeature_RAM | fgbGamePakFeature_BATTERY,
};

typedef struct {
	uint32_t success;
	fgbGamePakFeature features;
} fgbGamePakFeaturesResult;

static fgbGamePakFeaturesResult fgb__LookupGamePakFeatures(const fgbGamePakType type) {
	if (type < FGB_ARRAYCOUNT(fgb__GamePakTypeToFeatureTable)) {
		fgbGamePakFeature match = fgb__GamePakTypeToFeatureTable[type];
		return (fgbGamePakFeaturesResult) { true, match };
	}
	return (fgbGamePakFeaturesResult) {0};
}

static const char *fgb__RomSizeTypeToNameTable[256] = {
	[fgbRomSizeType_2_Banks_32KB] = "2 Banks, 32 KB",
	[fgbRomSizeType_4_Banks_64KB] = "4 Banks, 64 KB",
	[fgbRomSizeType_8_Banks_128KB] = "8 Banks, 128 KB",
	[fgbRomSizeType_16_Banks_256KB] = "16 Banks, 256 KB",
	[fgbRomSizeType_32_Banks_512KB] = "32 Banks, 512 KB",
	[fgbRomSizeType_64_Banks_1024KB] = "64 Banks, 1 MB",
	[fgbRomSizeType_128_Banks_2048KB] = "128 Banks, 2 MB",
	[fgbRomSizeType_256_Banks_4098KB] = "256 Banks, 4 MB",
	[fgbRomSizeType_512_Banks_8192KB] = "512 Banks, 8 MB",
	[fgbRomSizeType_72_Banks_1152KB] = "72 Banks, 1.1 MB",
	[fgbRomSizeType_80_Banks_1280KB] = "80 Banks, 1.2 MB",
	[fgbRomSizeType_96_Banks_1536KB] = "96 Banks, 1.5 MB",
};

static uint16_t fgb__RomSizeTypeToBankCountTable[256] = {
	[fgbRomSizeType_2_Banks_32KB] = 2,
	[fgbRomSizeType_4_Banks_64KB] = 4,
	[fgbRomSizeType_8_Banks_128KB] = 8,
	[fgbRomSizeType_16_Banks_256KB] = 16,
	[fgbRomSizeType_32_Banks_512KB] = 32,
	[fgbRomSizeType_64_Banks_1024KB] = 64,
	[fgbRomSizeType_128_Banks_2048KB] = 128,
	[fgbRomSizeType_256_Banks_4098KB] = 256,
	[fgbRomSizeType_512_Banks_8192KB] = 512,
	[fgbRomSizeType_72_Banks_1152KB] = 72,
	[fgbRomSizeType_80_Banks_1280KB] = 80,
	[fgbRomSizeType_96_Banks_1536KB] = 96,
};

typedef struct {
	uint32_t success;
	uint16_t count;
	uint16_t padding;
} fgbBankCountResult;

static fgbBankCountResult fgb__LookupGamePakROMBankCount(const fgbRomSizeType type) {
	if (type < FGB_ARRAYCOUNT(fgb__RomSizeTypeToBankCountTable)) {
		uint16_t count = fgb__RomSizeTypeToBankCountTable[type];
		return (fgbBankCountResult) { true, count };
	}
	return (fgbBankCountResult) {0};
}

static const char *fgb__RamSizeTypeToNameTable[256] = {
	[fgbRamSizeType_NoRam] = "No RAM",
	[fgbRamSizeType_Unused] = "Unused",
	[fgbRamSizeType_1_Banks_8KB] = "1 Banks, 8 KB",
	[fgbRamSizeType_4_Banks_32KB] = "4 Banks, 32 KB",
	[fgbRamSizeType_16_Banks_128KB] = "16 Banks, 128 KB",
	[fgbRamSizeType_8_Banks_64KB] = "8 Banks, 64 KB",
};

static uint16_t fgb__RamSizeTypeToBankCountTable[256] = {
	[fgbRamSizeType_NoRam] = 0,
	[fgbRamSizeType_Unused] = 0,
	[fgbRamSizeType_1_Banks_8KB] = 1,
	[fgbRamSizeType_4_Banks_32KB] = 4,
	[fgbRamSizeType_16_Banks_128KB] = 16,
	[fgbRamSizeType_8_Banks_64KB] = 8,
};

static fgbBankCountResult fgb__LookupGamePakRAMBankCount(const fgbRamSizeType type) {
	if (type < FGB_ARRAYCOUNT(fgb__RamSizeTypeToBankCountTable)) {
		uint16_t count = fgb__RamSizeTypeToBankCountTable[type];
		return (fgbBankCountResult) { true, count };
	}
	return (fgbBankCountResult) {0};
}

// TODO(final): Fill out old license code table (https://raw.githubusercontent.com/gb-archive/salvage/master/txt-files/gbrom.txt)
static const char *fgb__OldLicenseCodeNameTable[0xFF] = {
	[0x00] = "None",
	[0x01] = "Nintendo",
};

// TODO(final): Fill out new license code table (https://gbdev.io/pandocs/The_GamePak_Header.html#0144-0145---new-licensee-code)
static const char *fgb__NewLicenseCodeNameTable[256] = {
	[0] = "None",
	[1] = "Nintendo",
};

static const char *fgb__CoreTypeToNameTable[] = {
	[fgbCoreType_DMG] = "Gameboy",
	[fgbCoreType_CGB] = "Gameboy Color",
	[fgbCoreType_CGB_DMG] = "Gameboy Color Mode",
	[fgbCoreType_SGB] = "Super Gameboy",
};

FGB_API const char *fgbGetCoreTypeName(const fgbCoreType type) {
	return fgb__CoreTypeToNameTable[type];
}

static const char *fgb__GamePakLoadResultTypeNameTable[0x100] = {
	[fgbGamePakLoadResultType_Success] = "Success",
	[fgbGamePakLoadResultType_InvalidArguments] = "Invalid Arguments",
	[fgbGamePakLoadResultType_FileNotFound] = "File Not Found",
	[fgbGamePakLoadResultType_FileError] = "File IO Error",
	[fgbGamePakLoadResultType_NotEnoughData] = "Not Enough Data",
	[fgbGamePakLoadResultType_MemoryErrorFile] = "Failed allocating file buffer",
	[fgbGamePakLoadResultType_MemoryErrorROM] = "Failed allocating ROM banks",
	[fgbGamePakLoadResultType_MemoryErrorRAM] = "Failed allocating RAM banks",
	[fgbGamePakLoadResultType_MismatchLicenseLogo] = "Mismatch License Logo",
	[fgbGamePakLoadResultType_InvalidHeader] = "Invalid Header",
	[fgbGamePakLoadResultType_UnsupportedFormat] = "Unsupported Format",
};

FGB_API const char *fgbGetGamePakLoadResultLabel(const fgbGamePakLoadResultType type) {
	return fgb__GamePakLoadResultTypeNameTable[type];
}

// Header starts at 0x100
#define FGB__GAMEPAK_HEADER_POSITION 0x0100

#pragma pack(push, 1)
typedef struct {
	// Entry point: 0x100 - 0x103
	uint8_t entryPoint[4];

	// Nintendo logo: 0x104 - 0x133
	uint8_t nintendoLogo[48];

	// Title: 0x134 - 0x143
	char title[11];
	char manufactorCode[4];
	uint8_t cgbFlag;

	// New license code: 0x144 - 0x145
	uint16_t newLicenseCode;

	// SGB flag: 0x146
	uint8_t sgbFlag;
	// GamePak type: 0x147
	uint8_t gamePakType; // fgbGamePakType
	// Rom size: 0x148
	uint8_t romSizeType; // fgbRomSizeType
	// Ram size: 0x149
	uint8_t ramSizeType; // fgbRamSizeType
	// Destination code: 0x14A
	uint8_t destinationCode; // fgbDestinationCodeType
	// Old license code: 0x14B
	uint8_t oldLicenseCode;
	// Old license code: 0x14C
	uint8_t versionNumber;
	// Header checksum: 0x14D
	uint8_t headerChecksum;
	// Global checksum: 0x14E-0x14F
	uint16_t globalChecksum;
} fgb__GamePakHeader;
#pragma pack(pop)

// Fowler–Noll–Vo (FNV)
static uint32_t fgb__fnv32(const uint8_t *data, const uint32_t base, const uint32_t factor, size_t length) {
    uint32_t hash = base;
    for (size_t i = 0; i < length; i++) {
        hash ^= data[i];
        hash *= factor;
    }
    return hash;
}

static uint32_t fgb__ComputeNintendoLogoChecksum(const uint8_t data[48]) {
	if (data == NULL)
		return 0;
	return fgb__fnv32(data, FGB_LOGO_HASH_BASE, FGB_LOGO_HASH_FACTOR, 48);
}

static uint8_t fgb__ComputeGamePakHeaderChecksum(const uint8_t *data, const size_t size) {
	if (data == NULL || size < FGB_MIN_GAMEPAK_SIZE)
		return 0;
	uint32_t sum = 0;
	uint32_t len = 0x014C - 0x134;
	const uint8_t *p = &data[0] + 0x0134;
	for (uint32_t i = 0; i <= len; ++i) {
		sum = sum - p[i] - 1;
	}
	sum &= 0xFF;
	uint8_t result = (uint8_t)sum;
	return(result);
}

static uint16_t fgb__ComputeGamePakGlobalChecksum(const uint8_t *data, const size_t size, const uint16_t bankCount, const uint16_t globalChecksum) {
	if (data == NULL || size < FGB_MIN_GAMEPAK_SIZE || bankCount == 0 || globalChecksum == 0) {
		return 0;
	}

	uint32_t sum = 0;
	uint32_t len = bankCount * FGB_ROM_SIZE_PER_BANK;
	const uint8_t *p = data;
	for (uint32_t i = 0; i < len; ++i) {
		sum = sum + p[i];
	}

	// Remove checksum bytes
	sum -= globalChecksum & 0xFF;
	sum -= (globalChecksum >> 8) & 0xFF;

	// Leave only the 16-bit values and swap low/high bytes
	sum &= 0xFFFF;
	sum = fgb__SwapU16((uint16_t)sum);

	uint16_t result = (uint16_t)sum;

	return result;
}

FGB_API void fgbGamePakUnload(fgbGamePak *gamePak) {
	if (gamePak == NULL) {
		return;
	}
	fgbClearStruct(gamePak);
}

static fgbGamePakLoadResultType fgb__GamePakLoad(const uint8_t *data, const size_t size, fgbGamePak *outGamePak) {
	if (data == NULL || size == 0 || outGamePak == NULL) {
		return fgbGamePakLoadResultType_InvalidArguments;
	}
	if (size < FGB_MIN_GAMEPAK_SIZE) {
		return fgbGamePakLoadResultType_NotEnoughData;
	}

	// Not allowed to pass the gamepak ROM as data
	if (data == outGamePak->rom.data) {
		return fgbGamePakLoadResultType_MemoryErrorROM;
	}

	fgbClearStruct(outGamePak);

	const fgb__GamePakHeader *header = (const fgb__GamePakHeader *)(data + FGB__GAMEPAK_HEADER_POSITION);

	// Build title
	size_t maxTitleLen = FGB_ARRAYCOUNT(outGamePak->info.title.text);
	char *title = outGamePak->info.title.text;
	title[0] = '\0';
	fgb__StringAppendLen(header->title, 11, title, maxTitleLen);
	fgb__StringAppendLen(header->manufactorCode, 4, title, maxTitleLen);
	title[15] = header->cgbFlag;
	title[16] = '\0';

	if (header->oldLicenseCode == 0x33) {
		// Title can only be a length of 11 characters, when new license code replaces old license code
		title[12] = '\0';
	}

	fgbCoreType coreType;
	if (header->sgbFlag == 0x03) {
		coreType = fgbCoreType_SGB;
	} else if (header->cgbFlag & (1 << 7)) {
		switch (header->cgbFlag) {
			case 0xC0:
				coreType = fgbCoreType_CGB;
				break;
			case 0x80:
				coreType = fgbCoreType_CGB_DMG;
				break;
			default:
				coreType = fgbCoreType_CGB_DMG;
				break;
		}
	} else {
		coreType = fgbCoreType_DMG;
	}

	outGamePak->info.coreType = coreType;
	outGamePak->info.gamePakType = header->gamePakType;
	outGamePak->info.romSizeType = header->romSizeType;
	outGamePak->info.ramSizeType = header->ramSizeType;

	fgbBankCountResult romBankCountRes = fgb__LookupGamePakROMBankCount(header->romSizeType);
	if (!romBankCountRes.success) {
		return fgbGamePakLoadResultType_UnsupportedFormat;
	}

	fgbBankCountResult ramBankCountRes = fgb__LookupGamePakRAMBankCount(header->ramSizeType);
	if (!ramBankCountRes.success) {
		return fgbGamePakLoadResultType_UnsupportedFormat;
	}

	fgbMemoryControllerTypeResult mbcRes = fgb__LookupMemoryControllerType(header->gamePakType);
	if (!mbcRes.success) {
		return fgbGamePakLoadResultType_UnsupportedFormat;
	}

	fgbGamePakFeaturesResult featuresRes = fgb__LookupGamePakFeatures(header->gamePakType);
	if (!featuresRes.success) {
		return fgbGamePakLoadResultType_UnsupportedFormat;
	}

	outGamePak->info.romBankCount = romBankCountRes.count;
	outGamePak->info.ramBankCount = ramBankCountRes.count;
	outGamePak->info.mbcType = mbcRes.type;
	outGamePak->info.features = featuresRes.features;

	uint8_t headerChecksum = fgb__ComputeGamePakHeaderChecksum(data, size);

	uint16_t globalChecksum = fgb__ComputeGamePakGlobalChecksum(data, size, outGamePak->info.romBankCount, header->globalChecksum);

	bool headerPassed = headerChecksum == header->headerChecksum;
	if (!headerPassed) {
		return fgbGamePakLoadResultType_InvalidHeader;
	}

	uint32_t logoChecksum = fgb__ComputeNintendoLogoChecksum(header->nintendoLogo);

	bool licensePassed = logoChecksum == FGB_NINTENDO_LOGO_CHECKSUM;

	bool globalPassed = globalChecksum == header->globalChecksum;

	if (size > FGB_ARRAYCOUNT(outGamePak->rom.data)) {
		return fgbGamePakLoadResultType_MemoryErrorROM;
	}
	outGamePak->rom.length = size;
	FGB_MEMCOPY(outGamePak->rom.data, data, size);

	outGamePak->info.romChecksum = globalChecksum;
	outGamePak->info.headerChecksum = headerChecksum;
	outGamePak->hasROMChecksumPassed = globalPassed;
	outGamePak->hasLicensePassed = licensePassed;

	bool hasRAM = (outGamePak->info.features & fgbGamePakFeature_RAM) == fgbGamePakFeature_RAM;

	if (hasRAM && outGamePak->info.ramBankCount > 0) {
		size_t ramSize = outGamePak->info.ramBankCount * FGB_RAM_SIZE_PER_BANK;
		if (ramSize > FGB_ARRAYCOUNT(outGamePak->ram.memory.data)) {
			return fgbGamePakLoadResultType_MemoryErrorRAM;
		}
		outGamePak->ram.memory.length = ramSize;
		fgbClearStruct(outGamePak->ram.memory.data);
	}

	outGamePak->isValid = true;

	return fgbGamePakLoadResultType_Success;
}

FGB_API fgbGamePakLoadResultType fgbGamePakLoadFromFile(const fgbCallbacks *callbacks, const char *filePath, fgbGamePak *outGamePak) {
	if (callbacks == NULL || FGB_STRLEN(filePath) == 0 || outGamePak == NULL) {
		return fgbGamePakLoadResultType_InvalidArguments;
	}

	fgbFileHandle fileHandle = NULL;

	if (!fgb__FileOpen(callbacks, filePath, &fileHandle)) {
		return fgbGamePakLoadResultType_FileNotFound;
	}

	fgbGamePakLoadResultType result;

	uint8_t *data = NULL;

	size_t size = fgb__GetFileSize(callbacks, fileHandle);
	if (size < FGB_MIN_GAMEPAK_SIZE) {
		result = fgbGamePakLoadResultType_NotEnoughData;
		goto done;
	}

	data = fgb__AllocateMemory(callbacks, size);
	if (data == NULL) {
		result = fgbGamePakLoadResultType_MemoryErrorFile;
		goto done;
	}

	size_t read = fgb__FileRead(callbacks, fileHandle, data, size, size);
	if (read != size) {
		result = fgbGamePakLoadResultType_FileError;
		goto done;
	}

	result = fgb__GamePakLoad(data, size, outGamePak);

	if (result == fgbGamePakLoadResultType_Success) {
		fplCopyString(filePath, outGamePak->filePath, fplArrayCount(outGamePak->filePath));
	}

done:
	if (data != NULL) {
		fgb__FreeMemory(callbacks, data);
	}

	if (fileHandle) {
		fgb__FileClose(callbacks, fileHandle);
	}

	return result;
}

FGB_API fgbGamePakLoadResultType fgbGamePakLoadFromMemory(const uint8_t *data, const size_t size, fgbGamePak *outGamePak) {
	if (data == NULL || size == 0 || outGamePak == NULL) {
		return fgbGamePakLoadResultType_InvalidArguments;
	}
	fgbGamePakLoadResultType result = fgb__GamePakLoad(data, size, outGamePak);
	return result;
}

// ********************************************************************************************************************
// Memory Bank Controller Implementation
// ********************************************************************************************************************

#define FGB__MBC_BATTERY_ADDRESS_FROM 0x0146
#define FGB__MBC_BATTERY_ADDRESS_TO 0x0149

static void fgb__MBC_BatteryWriteRequest(fgbSystem *system) {
	// Request a save the external RAM to a file
	fgbGamePak *gamepak = &system->gamePak;
	if (gamepak->info.features & fgbGamePakFeature_BATTERY && gamepak->ram.memory.length > 0) {
		gamepak->ram.requestSave |= true;
	}
}

static void fgb__MBC_RAM_Enabled(fgbSystem *system, const bool isEnabled) {
	if (isEnabled) {
		fgb__MBC_BatteryWriteRequest(system);
	}
}

static void fgb__MBC_ROMBankChanged(fgbSystem *system, const uint16_t oldBank, const uint16_t newBank) {
#if FGB_MBC_ROM_BANK_SWITCH_LOGGING
	FGB__DEBUG(system, fgb__KindName_MBC, "ROM Bank changed from '%u' to '%u'", oldBank, newBank);
#endif
}

static void fgb__MBC_RAMBankChanged(fgbSystem *system, const uint8_t oldBank, const uint8_t newBank) {
#if FGB_MBC_RAM_BANK_SWITCH_LOGGING
	FGB__DEBUG(system, fgb__KindName_MBC, "RAM Bank changed from '%u' to '%u'", oldBank, newBank);
#endif
	
	// NOTE(final): On bank switching, it is a good time to save the external RAM to disk, so we request a write here
	fgb__MBC_BatteryWriteRequest(system);
}

static void fgb__MBC_RAMUpdated(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbGamePak *gamepak = &system->gamePak;
	if (gamepak->info.features & fgbGamePakFeature_BATTERY && gamepak->ram.memory.length > 0) {
		// Mark the RAM as dirty, so it may be saved on request
		gamepak->ram.isDirty |= true;
	}
}

static uint8_t fgb__ROM_Read(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address) {
	fgbSystem *system = (fgbSystem * )gbOpaque;
	fgbGamePak *gamepak = &system->gamePak;
	if (address >= FGB__BUS_ADDRESS_ROM_FROM && address <= FGB__BUS_ADDRESS_ROM_TO) {
		return gamepak->rom.data[address - FGB__BUS_ADDRESS_ROM_FROM];
	} else {
		return 0xFF;
	}
}

static void fgb__ROM_Write(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address, const uint8_t value) {
	// Nothing todo, because writing to ROM without MBC is not allowed
}

static const uint8_t *fgb__GetROMBank(const uint16_t bankIndex, const fgbReadOnlyMemory *rom, const uint16_t romOffset) {
	size_t romSize = rom->length;
	uintptr_t bankPosition = bankIndex * FGB_ROM_SIZE_PER_BANK;
	uintptr_t romPosition = bankPosition + romOffset;
	if (romPosition < romSize) {
		return rom->data + bankPosition;
	}
	return NULL;
}

static uint8_t *fgb__GetRAMBank(const uint16_t bankIndex, fgbExternalRAM *ram, const uint16_t ramOffset) {
	size_t ramSize = ram->memory.length;
	uintptr_t bankPosition = bankIndex * FGB_RAM_SIZE_PER_BANK;
	uintptr_t ramPosition = bankPosition + ramOffset;
	if (ramPosition < ramSize) {
		return ram->memory.data + bankPosition;
	}
	return NULL;
}

static uint8_t fgb__MBC1_Read(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBCData *data = &mbc->data;
	fgbMBC1 *mbc1 = &data->mbc1;

	if (address <= 0x3FFF) {
		// ROM Bank 0
		return gamepak->rom.data[address];
	} else if (address <= 0x7FFF) {
		// ROM Bank 1 - 127
		uint16_t offset = address - 0x4000;
		const uint8_t *romBank = fgb__GetROMBank(mbc1->romBank, &gamepak->rom, offset);
		if (romBank != NULL) {
			return romBank[offset];
		} else {
			return 0xFF; // ROM-Bank out-of-range
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 3
		uint16_t offset = address - 0xA000;
		const uint8_t *ramBank;
		if (mbc1->isRAMEnabled && ((ramBank = fgb__GetRAMBank(mbc1->ramBank, &gamepak->ram, offset)) != NULL)) {
			return ramBank[offset];
		} else {
			return 0xFF; // External RAM not enabled or RAM-Bank out-of-range
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC1 Read from address '$%04X'", address);
		return 0;
	}
}

static void fgb__MBC1_Write(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address, const uint8_t value) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbMBCData *data = &mbc->data;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBC1 *mbc1 = &data->mbc1;

	if (address <= 0x1FFF) {
		// Enable RAM
		mbc1->isRAMEnabled = (value & 0x0A) != 0;
		fgb__MBC_RAM_Enabled(system, mbc1->isRAMEnabled);
	} else if (address <= 0x3FFF) {
		// ROM Bank Number (lower 5 bits)
		uint8_t oldROMBank = mbc1->romBank;
		mbc1->romBank &= ~0x1F;
		mbc1->romBank |= (value & 0x1F);

		// Selecting a bank of $20, $40, $60, $0 needs to be incremented by one
		uint8_t n = mbc1->romBank & 0x1f;
		if (n == 0x20 || n == 0x40 || n == 0x60 || n == 0x00) {
			mbc1->romBank++;
		}

		fgb__MBC_ROMBankChanged(system, oldROMBank, mbc1->romBank);
	} else if (address <= 0x5FFF) {
		// RAM Bank Number OR Upper 2 bits of ROM Bank Number
		if (mbc1->mode == 0x00) {
			uint8_t oldRAMBank = mbc1->ramBank;
			mbc1->ramBank = value & 0x3;
			fgb__MBC_RAMBankChanged(system, oldRAMBank, mbc1->ramBank);
		} else if (mbc1->mode == 0x01) {
			uint8_t oldROMBank = mbc1->romBank;
			mbc1->romBank &= ~0x60;
			mbc1->romBank |= (value & 0x3) << 5;
			fgb__MBC_ROMBankChanged(system, oldROMBank, mbc1->romBank);
		}
	} else if (address <= 0x7FFF) {
		// ROM/RAM Mode Select
		uint8_t mode = value & 0x1;
		if (mode != mbc1->mode) {
			if (mode == 0x00) {
				uint8_t oldRAMBank = mbc1->ramBank;
				uint8_t newRAMBank = (mbc1->romBank & 0x60) >> 5;

				mbc1->romBank &= ~0x60;
				mbc1->ramBank = newRAMBank;

				fgb__MBC_RAMBankChanged(system, oldRAMBank, mbc1->ramBank);
			} else if (mode == 0x01) {
				uint8_t oldRomBank = mbc1->romBank;
				uint8_t oldRamBank = mbc1->ramBank;

				mbc1->ramBank = 0x00;
				fgb__MBC_RAMBankChanged(system, oldRamBank, mbc1->ramBank);

				mbc1->romBank &= ~0x60;
				mbc1->romBank |= (oldRamBank & 0x3) << 5;
				fgb__MBC_ROMBankChanged(system, oldRomBank, mbc1->romBank);
			}
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 3
		uint16_t offset = address - 0xA000;
		uint8_t *ramBank;
		if (mbc1->isRAMEnabled && ((ramBank = fgb__GetRAMBank(mbc1->ramBank, &gamepak->ram, offset)) != NULL)) {
			ramBank[offset] = value;
			fgb__MBC_RAMUpdated(system, address, value);
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC1 Write '$%02X' to address '$%04X'", value, address);
	}
}

static uint8_t fgb__MBC2_Read(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBCData *data = &mbc->data;
	fgbMBC2 *mbc2 = &data->mbc2;

	if (address <= 0x3FFF) {
		// ROM Bank 0
		return gamepak->rom.data[address];
	} else if (address <= 0x7FFF) {
		// ROM Bank 1 - 127
		uint16_t offset = address - 0x4000;
		const uint8_t *romBank = fgb__GetROMBank(mbc2->romBank, &gamepak->rom, offset);
		if (romBank != NULL) {
			return romBank[offset];
		} else {
			return 0xFF; // ROM-Bank out-of-range
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0
		size_t ramSize = gamepak->ram.memory.length;
		uint16_t offset = address - 0xA000;
		if (mbc2->isRAMEnabled && offset < ramSize) {
			uint8_t *ram = gamepak->ram.memory.data;
			return ram[offset];
		} else {
			return 0xFF; // External RAM is not enabled
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC2 Read from address '$%04X'", address);
		return 0;
	}
}

static void fgb__MBC2_Write(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address, const uint8_t value) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbMBCData *data = &mbc->data;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBC2 *mbc2 = &data->mbc2;

	if (address <= 0x1FFF) {
		// Enable RAM (The least significant bit of the upper address byte must be '0' to enable/disable gamepak RAM)
		if ((address & 0x0100) == 0) {
			mbc2->isRAMEnabled = (value & 0x0A) != 0;
			fgb__MBC_RAM_Enabled(system, mbc2->isRAMEnabled);
		}
	} else if (address <= 0x3FFF) {
		// ROM Bank Number (The least significant bit of the upper address byte must be '1' to select a ROM bank)
		if ((address & 0x0100) != 0) {
			// 0 is not allowed, because its ROM 0 so its set 1 in that case
			uint8_t oldROMBank = mbc2->romBank;
			mbc2->romBank = (value & 0b1111);
			if (mbc2->romBank == 0) mbc2->romBank = 1;
			fgb__MBC_ROMBankChanged(system, oldROMBank, mbc2->romBank);
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0
		size_t ramSize = gamepak->ram.memory.length;
		uint16_t offset = address - 0xA000;
		if (mbc2->isRAMEnabled && offset < ramSize) {
			gamepak->ram.memory.data[offset] = value;
			fgb__MBC_RAMUpdated(system, address, value);
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC2 Write '$%02X' to address '$%04X'", value, address);
	}
}

static uint8_t fgb__MBC3_Read(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBCData *data = &mbc->data;
	fgbMBC3 *mbc3 = &data->mbc3;

	if (address <= 0x3FFF) {
		// ROM Bank 0
		return gamepak->rom.data[address];
	} else if (address <= 0x7FFF) {
		// ROM Bank 1 - 127
		uint16_t offset = address - 0x4000;
		const uint8_t *romBank = fgb__GetROMBank(mbc3->romBank, &gamepak->rom, offset);
		if (romBank != NULL) {
			return romBank[offset];
		} else {
			return 0xFF; // ROM-Bank out-of-range
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 3
		uint16_t offset = address - 0xA000;
		const uint8_t *ramBank;
		if (mbc3->isRAMAndRTCEnabled && ((ramBank = fgb__GetRAMBank(mbc3->ramBankOrRTCRegister, &gamepak->ram, offset)) != NULL)) {
			return ramBank[offset];
		} else {
			return 0xFF; // External RAM not enabled or RAM-Bank out-of-range
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC3 Read from address '$%04X'", address);
		return 0;
	}
}

static void fgb__MBC3_Write(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address, const uint8_t value) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbMBCData *data = &mbc->data;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBC3 *mbc3 = &data->mbc3;

	if (address <= 0x1FFF) {
		// RAM Enable
		mbc3->isRAMAndRTCEnabled = (value & 0x0A) != 0;
		fgb__MBC_RAM_Enabled(system, mbc3->isRAMAndRTCEnabled);
	} else if (address <= 0x3FFF) {
		// ROM Bank Number (0 is not allowed, because its ROM 0 so its set 1 in that case)
		uint8_t oldROMBank = mbc3->romBank;
		mbc3->romBank = value;
		if (mbc3->romBank == 0) mbc3->romBank = 1;
		fgb__MBC_ROMBankChanged(system, oldROMBank, mbc3->romBank);
	} else if (address < 0x7FFF) {
		// TODO(final): RAM Bank Switching is not implemented for MBC3!
		// TODO(final): RTC is not implemented for MBC3!
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 3
		uint16_t offset = address - 0xA000;
		uint8_t *ramBank;
		if (mbc3->isRAMAndRTCEnabled && ((ramBank = fgb__GetRAMBank(mbc3->ramBankOrRTCRegister, &gamepak->ram, offset)) != NULL)) {
			ramBank[offset] = value;
			fgb__MBC_RAMUpdated(system, address, value);
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC3 Write '$%02X' to address '$%04X'", value, address);
	}
}

static uint8_t fgb__MBC5_Read(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBCData *data = &mbc->data;
	fgbMBC5 *mbc5 = &data->mbc5;

	if (address <= 0x3FFF) {
		// ROM Bank 0
		return gamepak->rom.data[address];
	} else if (address <= 0x7FFF) {
		// ROM Bank 0 - 480
		uint16_t offset = address - 0x4000;
		const uint8_t *romBank = fgb__GetROMBank(mbc5->romBank, &gamepak->rom, offset);
		if (romBank != NULL) {
			return romBank[offset];
		} else {
			return 0xFF; // ROM-Bank out-of-range
		}
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 15
		uint16_t offset = address - 0xA000;
		const uint8_t *ramBank;
		if (mbc5->isRAMEnabled && ((ramBank = fgb__GetRAMBank(mbc5->ramBank, &gamepak->ram, offset)) != NULL)) {
			return ramBank[offset];
		} else {
			return 0xFF; // External RAM not enabled or RAM-Bank out-of-range
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC5 Read from address '$%04X'", address);
		return 0;
	}
}

static void fgb__MBC5_Write(struct fgbSystem *gbOpaque, struct fgbMemoryBankController *mbcOpaque, const uint16_t address, const uint8_t value) {
	fgbSystem *system = (fgbSystem *)gbOpaque;
	fgbMemoryBankController *mbc = (fgbMemoryBankController *)mbcOpaque;
	fgbMBCData *data = &mbc->data;
	fgbGamePak *gamepak = &system->gamePak;
	fgbMBC5 *mbc5 = &data->mbc5;

	if (address <= 0x1FFF) {
		// RAM Enable
		mbc5->isRAMEnabled = (value & 0x0A) != 0;
		fgb__MBC_RAM_Enabled(system, mbc5->isRAMEnabled);
	} else if (address <= 0x2FFF) {
		// ROM Bank Number — lower 8-bits
		uint16_t oldROMBank = mbc5->romBank;
		mbc5->romBank &= ~0xFF;
		mbc5->romBank |= (uint16_t)value;
		fgb__MBC_ROMBankChanged(system, oldROMBank, mbc5->romBank);
	} else if (address < 0x3FFF) {
		// ROM Bank Number — upper 8-bits
		uint16_t oldROMBank = mbc5->romBank;
		mbc5->romBank &= ~0xFF00;
		mbc5->romBank |= ((uint16_t)value << 8);
		fgb__MBC_ROMBankChanged(system, oldROMBank, mbc5->romBank);
	} else if (address < 0x5FFF) {
		// RAM Bank Number
		uint8_t oldAMBank = mbc5->ramBank;
		mbc5->ramBank = value & 0xF;
		fgb__MBC_RAMBankChanged(system, oldAMBank, mbc5->ramBank);
	} else if (address >= 0xA000 && address <= 0xBFFF) {
		// RAM Bank 0 - 15
		uint16_t offset = address - 0xA000;
		uint8_t *ramBank;
		if (mbc5->isRAMEnabled && ((ramBank = fgb__GetRAMBank(mbc5->ramBank, &gamepak->ram, offset)) != NULL)) {
			ramBank[offset] = value;
			fgb__MBC_RAMUpdated(system, address, value);
		}
	} else {
		FGB__WARN(system, fgb__KindName_MBC, "Unsupported MBC5 Write '$%02X' to address '$%04X'", value, address);
	}
}

static void fgb__MBCInit(fgbSystem *system, const fgbGamePak *gamePak, fgbMemoryBankController *mbc) {
	switch (gamePak->info.mbcType) {
		case fgbMemoryControllerType_MBC1:
			mbc->data.mbc1.romBank = 0x01;
			mbc->data.mbc1.ramBank = 0x00;
			mbc->data.mbc1.isRAMEnabled = false;
			mbc->data.mbc1.mode = 0x00;
			mbc->read = fgb__MBC1_Read;
			mbc->write = fgb__MBC1_Write;
			break;

		case fgbMemoryControllerType_MBC2:
			mbc->data.mbc2.romBank = 0x01;
			mbc->data.mbc2.isRAMEnabled = false;
			mbc->read = fgb__MBC2_Read;
			mbc->write = fgb__MBC2_Write;
			break;

		case fgbMemoryControllerType_MBC3:
			mbc->data.mbc3.romBank = 0x01;
			mbc->data.mbc3.ramBankOrRTCRegister = 0x00;
			mbc->data.mbc3.isRAMAndRTCEnabled = false;
			mbc->read = fgb__MBC3_Read;
			mbc->write = fgb__MBC3_Write;
			break;

		case fgbMemoryControllerType_MBC5:
			mbc->data.mbc5.romBank = 0x01;
			mbc->data.mbc5.ramBank = 0x00;
			mbc->data.mbc5.isRAMEnabled = false;
			mbc->read = fgb__MBC5_Read;
			mbc->write = fgb__MBC5_Write;
			break;

		case fgbMemoryControllerType_ROM:
			mbc->read = fgb__ROM_Read;
			mbc->write = fgb__ROM_Write;
			break;

		default:
			FGB__WARN(system, fgb__KindName_MBC, "MBC Type '%s' not supported", fgb__MemoryControllerTypeToNameTable[gamePak->info.mbcType]);
			break;
	}
}

FGB_API uint16_t fgbGetROMBank(const fgbSystem *system) {
	if (system == NULL || system->mbc.read == NULL) {
		return 0;
	}
	switch (system->gamePak.info.mbcType) {
		case fgbMemoryControllerType_MBC1:
			return system->mbc.data.mbc1.romBank;
		case fgbMemoryControllerType_MBC2:
			return system->mbc.data.mbc2.romBank;
		case fgbMemoryControllerType_MBC3:
			return system->mbc.data.mbc3.romBank;
		case fgbMemoryControllerType_MBC5:
			return system->mbc.data.mbc5.romBank;
		default:
			return 0;
	}
}

// ********************************************************************************************************************
// 
// APU Implementation v3
// 
// This is my third rewrite of the APU, which has a much cleaner architecture and are much easier to understand.
// 
// It is mostly based on: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
// 
// But contains lots of fixes/hacks from other emulators as well.
// 
// - Voices are disabled when the high 5 bits of the volume envelop register are not set
// - A voice is not triggered when the high 5 bits of the volume envelop register are not set
// 
// Bugs:
// 
// - In Super Mario Land 2, collecting a coin by block or by item does not change the frequency at all, which is wrong.
//
// - There may some weird random sound errors, that are hearable - but may be related to the problem above.
// 
// Important:
// 
// - Period/Timers should never be reset when the register changes!
// 
// ********************************************************************************************************************

#define FGB__APU_SPEAKER_LEFT 0
#define FGB__APU_SPEAKER_RIGHT 1

// The number of ticks per APU step
#define FGB__APU_TICKS 2

// We have 4 voices in total
#define FGB__APU_VOICE_COUNT 4

static void fgb__APUWrite(fgbSystem *system, const uint16_t address, const uint8_t value);
static uint8_t fgb__APURead(fgbSystem *system, const uint16_t address);

static const uint8_t fgb__APU_DefaultRegister[] = {
	// FF10 - FF14: Channel 1 (NR10-NR14)
	0x80, 0xBF, 0xF3, 0x00, 0xBF,
	// FF15 - FF19: Channel 2 (NR20-NR24)
	0xFF, 0x3F, 0x00, 0xFF, 0xBF,
	// FF1A - FF1E: Channel 3 (NR30-NR34)
	0x7F, 0xFF, 0x9F, 0xFF, 0xBF,
	// FF1F - FF23: Channel 4 (NR40-NR44)
	0xFF, 0xFF, 0x00, 0x00, 0xBF,

	// FF24 - FF26: Channel control / Sound output terminal selection / Sound on/off (NR50-NR52)
	0x77, 0xF3, 0xF0,

	// FF27 - FF2F: Unused
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,

	// FF30 - FF3F: Wave 00 - 15
	0xac, 0xdd, 0xda, 0x48,
	0x36, 0x02, 0xcf, 0x16,
	0x2c, 0x04, 0xe5, 0x2c,
	0xac, 0xdd, 0xda, 0x48
};
FGB_STATIC_ASSERT(sizeof(fgb__APU_DefaultRegister) == sizeof(fgbSoundRegister));

static uint8_t fgb__APU_ReadORTab[] = {
	0x80, 0x3F, 0x00, 0xFF, 0xBF,							// NR10-NR14
	0xFF, 0x3F, 0x00, 0xFF, 0xBF,							// NR20-NR24
	0x7F, 0xFF, 0x9F, 0xFF, 0xBF,							// NR30-NR34
	0xFF, 0xFF, 0x00, 0x00, 0xBF,							// NR40-NR44
	0x00, 0x00, 0x70,										// NR50-NR52
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	// Unused
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,			// Wave RAM 0-7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,			// Wave RAM 8-15
};
FGB_STATIC_ASSERT(sizeof(fgb__APU_ReadORTab) == sizeof(fgbSoundRegister));

static const uint8_t fgb__squareWaveDutyTable[4][8] = {
	{0, 0, 0, 0, 0, 0, 0, 1}, // 12.5% high, 87.5% low
	{1, 0, 0, 0, 0, 0, 0, 1}, // 25% high, 75% low
	{1, 0, 0, 0, 0, 1, 1, 1}, // 50% high, 50% low
	{0, 1, 1, 1, 1, 1, 1, 0}  // 75% high, 25% low
};

#define FGB__NOISE_DIVISOR_COUNT 8

// There are 8 divisor states for the noise voice
static const uint8_t fgb__noiseDivTable[FGB__NOISE_DIVISOR_COUNT] = { 8, 16, 32, 48, 64, 80, 96, 112 };

static const char *fgb__apuSoundRegisterLabels[48] = {
	[0x00] = "NR10 (Sweep)",
	[0x01] = "NR11 (Length)",
	[0x02] = "NR12 (Env)",
	[0x03] = "NR13 (Freq Low)",
	[0x04] = "NR14 (Freq High)",
	[0x05] = "FF15 (Unused)",
	[0x06] = "NR21 (Length)",
	[0x07] = "NR22 (Env)",
	[0x08] = "NR23 (Freq Low)",
	[0x09] = "NR24 (Freq High)",
	[0x0A] = "NR30 (On/Off)",
	[0x0B] = "NR31 (Length)",
	[0x0C] = "NR32 (Output Level)",
	[0x0D] = "NR33 (Freq Low)",
	[0x0E] = "NR34 (Freq High)",
	[0x0F] = "FF1F (Unused)",
	[0x10] = "NR41 (Length)",
	[0x11] = "NR42 (Env)",
	[0x12] = "NR43 (Poly)",
	[0x13] = "NR44 (Counter)",
	[0x14] = "NR50 (Master Volume)",
	[0x15] = "NR51 (Panning)",
	[0x16] = "NR52 (Master Control)",
	[0x17] = "FF27 (Unused)",
	[0x18] = "FF28 (Unused)",
	[0x19] = "FF29 (Unused)",
	[0x1A] = "FF2A (Unused)",
	[0x1B] = "FF2B (Unused)",
	[0x1C] = "FF2C (Unused)",
	[0x1D] = "FF2D (Unused)",
	[0x1E] = "FF2E (Unused)",
	[0x1F] = "FF2F (Unused)",
	[0x20] = "WAV00",
	[0x21] = "WAV01",
	[0x22] = "WAV02",
	[0x23] = "WAV03",
	[0x24] = "WAV04",
	[0x25] = "WAV05",
	[0x26] = "WAV06",
	[0x27] = "WAV07",
	[0x28] = "WAV08",
	[0x29] = "WAV09",
	[0x2A] = "WAV10",
	[0x2B] = "WAV11",
	[0x2C] = "WAV12",
	[0x2D] = "WAV13",
	[0x2E] = "WAV14",
	[0x2F] = "WAV15",
};

static inline uint8_t fgb__PackVolumeTo3BITS(const float volume) {
	FGB_ASSERT(volume >= 0 && volume <= 1.0f);
	uint8_t result = (uint8_t)(volume * 7) & 0b111;
	return result;
}

static inline float fgb__UnpackVolumeFrom3BITS(const uint8_t bits) {
	FGB_ASSERT(bits <= 0b111);
	float result = bits / 7.0f;
	return result;
}

//
// Square Wave
//
static inline int32_t fgb__GetSquareWaveTimer(const uint16_t frequency) {
	return (2048 - frequency) * 4;
}

static void fgb__InitSquareWave(fgbSquareWave *squareWave) {
	squareWave->step = 0;
	squareWave->dutyCycle = 0;
	squareWave->period = 0;
	squareWave->timer = 0;
}

static void fgb__StartSquareWave(fgbSquareWave *squareWave) {
	squareWave->timer = squareWave->period;
}

static void fgb__SetSquareWavePeriod(fgbSquareWave *squareWave, const uint16_t frequency) {
	squareWave->period = fgb__GetSquareWaveTimer(frequency);
}

static void fgb__StopSquareWave(fgbSquareWave *squareWave) {
	squareWave->step = 0;
	squareWave->dutyCycle = 0;
	squareWave->timer = 0;
}

static uint8_t fgb__GetSquareWaveSample(fgbSquareWave *squareWave, const uint8_t volume) {
	uint8_t step = squareWave->step;

	uint8_t row = squareWave->dutyCycle & 0b11;
	uint8_t column = step & 0b111;

	uint8_t sample;

	uint8_t duty = fgb__squareWaveDutyTable[row][column];
	if (duty == 0) {
		sample = 0;
	} else {
		sample = volume;
	}

	return sample;
}

static bool fgb__TickSquareWave(fgbSquareWave *squareWave) {
	if (--squareWave->timer <= 0) {
		squareWave->timer = squareWave->period;
		if (squareWave->period) {
			uint8_t step = squareWave->step;
			squareWave->step = (step + 1) % 8;
			return true;
		}
	}
	return false;
}

//
// Frame Sequencer
//

// Fixed frequency of the frame sequencer (works for most games, but may be wrong when the timer clock changes)
#define FGB__APU_FRAME_SEQUENCER_FREQUENCY 512

// Timer period of the frame sequencer based on the frequency of the frame sequencer (works for most games, but may be wrong when the timer clock changes)
#define FGB__APU_FRAME_SEQUENCER_PERIOD (FGB_MAX_CPU_CYCLES / FGB__APU_FRAME_SEQUENCER_FREQUENCY)

// Initialize the frame sequencer for the very first time
static void fgb__InitFrameSequencer(const fgbTimer *timer, fgbFrameSequencer *frameSeq) {
	frameSeq->step = 0;
	frameSeq->lastDivider = timer->divider;
}

// Stop the frame sequencer
static void fgb__StopFrameSequencer(fgbFrameSequencer *frameSeq) {
	frameSeq->step = 0;
}

// Start the frame sequencer from step 0
static void fgb__StartFrameSequencer(const fgbTimer *timer, fgbFrameSequencer *frameSeq) {
	frameSeq->step = 0;
	frameSeq->lastDivider = timer->divider;
}

static bool fgb__TickFrameSequencer(const fgbTimer *timer, fgbFrameSequencer *frameSeq, uint8_t *step) {
	uint16_t diff = timer->divider - frameSeq->lastDivider;
	uint16_t currentDivider = timer->divider;
	if (diff >= FGB__APU_FRAME_SEQUENCER_PERIOD) {
		frameSeq->step = (frameSeq->step + 1) % 8;
		//frameSeq->lastDivider += FGB__APU_FRAME_SEQUENCER_PERIOD;
		frameSeq->lastDivider = currentDivider;
		*step = frameSeq->step;
		return true;
	}
	return false;
}

//
// Sound Frequency
// 
// Simply stores the 11-bit frequency in a 16-bit unsigned int and with the power of unions, we can access the high/low part individually
// 
// Used for the Square-Wave period in the Sweep/Tone Voice and for the Wave-Voice period
//
static void fgb__InitSoundFrequency(fgbSoundFrequency *freq) {
	freq->period = 0;
}

static void fgb__SetSoundPeriod(fgbSoundFrequency *freq, const uint16_t period) {
	freq->period = period;
}

static void fgb__SetSoundPeriodLow(fgbSystem *system, fgbSoundFrequency *freq, const uint8_t low, const fgbVoiceType type) {
	freq->low = low;
#if FGB_APU_FREQUENCY_SET_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Set Low Frequency, Low: %u, Period: %u", system->cpu.state.totalTickCycles, (type + 1), freq->low, freq->period);
#endif
}

static void fgb__SetSoundPeriodHigh(fgbSystem *system, fgbSoundFrequency *freq, const uint8_t high, const fgbVoiceType type) {
	freq->high = high & 0b111;
#if FGB_APU_FREQUENCY_SET_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Set High Frequency, High: %u, Period: %u", system->cpu.state.totalTickCycles, (type + 1), freq->high, freq->period);
#endif
}

//
// Sound Length Timer
// 
// Supports disabling a voice when a count-down reaches zero
// 
// The timer is enabled only, when the enabled flag in NRx4 was set
// 
// Based on: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Length_Counter
//

// Initialize the sound length timer with the specified max length of 64 or 256
// Note that Voice 3 has a different max length of 256
// 
// This is called when the APU is powered on
static void fgb__InitSoundLengthTimer(fgbSoundLengthTimer *lengthTimer, const uint16_t maxLength) {
	FGB_ASSERT(maxLength == 64 || maxLength == 256);
	lengthTimer->maxLength = maxLength;
	lengthTimer->length = 0;
	lengthTimer->timer = lengthTimer->maxLength;
	lengthTimer->isEnabled = false;
}

// Resets the length timer to a default period and disables the timer
// 
// This is called when the APU is powered off
static void fgb__StopSoundLengthTimer(fgbSoundLengthTimer *lengthTimer) {
	lengthTimer->timer = lengthTimer->maxLength;
	lengthTimer->isEnabled = false;
}

// Gets the sound length timer in range of 1-64 or 1-256 (Obscure Behavior: 0 is is always 64 or 256, 256 is used for the wave voice only)
static inline uint16_t fgb__GetGetSoundLengthTimer(const uint16_t maxLength, const uint16_t len) {
	FGB_ASSERT(maxLength == 64 || maxLength == 256);
	uint16_t value = len == 0 ? maxLength : maxLength - len;
	return value;
}

// Called when the gameboy wrote to NR14, regardless if the voice is triggered or not
static void fgb__EnableSoundLengthTimer(fgbSoundLengthTimer *lengthTimer, const bool enable) {
	lengthTimer->isEnabled = enable;
}

// Called when the gameboy wrote to NR14 and the voice gets triggered
static void fgb__TriggerSoundLengthTimer(fgbSoundLengthTimer *lengthTimer) {
	lengthTimer->timer = fgb__GetGetSoundLengthTimer(lengthTimer->maxLength, lengthTimer->length);

	// NOTE(final): Do not enable the length timer here, this handled before and regardless of the voice is triggered
}

// Called when the gameboy writes to NR11: Length needs to be applied immediately, so we set the length and reset the timer
static void fgb__ChangeSoundLengthTimer(fgbSoundLengthTimer *lengthTimer, const uint16_t length) {
	lengthTimer->timer = lengthTimer->maxLength - length;
}

// Clocked by the frame sequencer on steps (0, 2, 4, 6)
static fgbSoundLengthState fgb__TickSoundLengthTimer(fgbSoundLengthTimer *lengthTimer) {
	if (!lengthTimer->isEnabled) {
		return fgbSoundLengthState_Disabled; // Length is disabled, nothing todo
	}

	if (lengthTimer->timer > 0 && --lengthTimer->timer == 0) {
		lengthTimer->isEnabled = false;
		return fgbSoundLengthState_EndReached; // Indicates to disable the voice
	}

	return fgbSoundLengthState_Continue; // Continue until the timer is zero
}

//
// Volume Envelope
// 
// Changes the volume over a period of time, until the volume reaches 0 or 15
// 
// The volume is only changed, when the period is set
// 
// If neither the initial volume nor the direction is set (all high 5 bits are not-set), then a voice should be disabled
// 
// Based on: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Volume_Envelope
//

// Maximum period of a volume envelope
#define FGB__MAX_VOLUME_ENVELOPE_PERIOD 8

// Maximum volume of a volume envelope (Range 0-15)
#define FGB__MAX_VOLUME_ENVELOPE_VOL 15

// Gets volume envelope timer in range of 1-8 (Obscure Behavior: 0 is is always 8)
static inline uint8_t fgb__GetVolumeEnvelopTimer(const uint8_t period) {
	return period == 0 ? FGB__MAX_VOLUME_ENVELOPE_PERIOD : period;
}

static inline bool fgb__IsVolumeEnvelopValid(const fgbVolumeEnvelope *env) {
	return env->initialVolume || env->isIncreasing;
}

static void fgb__InitVolumeEnvelope(fgbVolumeEnvelope *env) {
	env->isEnabled = false;
	env->initialVolume = 0;
	env->isIncreasing = false;
	env->period = 0;
	env->timer = 0;
}

static void fgb__StopVolumeEnvelope(fgbVolumeEnvelope *env) {
	env->timer = 0;
	env->isEnabled = false;
}

static uint8_t fgb__GetVolumeEnvelopeRegister(const fgbVolumeEnvelope *env) {
	fgbVolumeEnvelopeRegister reg = { 
		.period = env->period,
		.isInc = env->isIncreasing,
		.initialVolume = env->initialVolume
	};
	return reg.u8;
}

static void fgb__ApplyVolumeEnvelope(fgbSystem *system, fgbVolumeEnvelope *env, const uint8_t newVolume, const fgbVoiceType type) {
	uint8_t oldVolume = env->currentVolume;
	env->currentVolume = newVolume;
#if FGB_APU_VOLUME_CHANGED_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Apply Volume from %u to %u", system->cpu.state.totalTickCycles, (type + 1), oldVolume, newVolume);
#endif
}

static void fgb__TickVolumeEnvelope(fgbSystem *system, fgbVolumeEnvelope *env, const fgbVoiceType type) {
	bool changed = false;

	uint8_t oldVolume = env->currentVolume;

	if (env->period) {
		if (env->isEnabled && --env->timer <= 0) {
			env->timer = env->period;
			if (env->isIncreasing && oldVolume < FGB__MAX_VOLUME_ENVELOPE_VOL) {
				uint8_t newVolume = oldVolume + 1;
				fgb__ApplyVolumeEnvelope(system, env, newVolume, type);
				changed = true;
			} else if (!env->isIncreasing && oldVolume > 0) {
				uint8_t newVolume = oldVolume - 1;
				fgb__ApplyVolumeEnvelope(system, env, newVolume, type);
				changed = true;
			} else {
				env->isEnabled = false;
			}
		}
	} else {
		env->timer = FGB__MAX_VOLUME_ENVELOPE_PERIOD;
	}
}

//
// Frequency Sweep
// 
// Change the square wave frequency over a period of time (Only supported by the Sweep Voice)
// 
// - The sweep is enabled only, when either the shift register or the period are set
// 
// - When it overflows (frequency > 2047), the sweep voice must be turned off
// 
// Based on: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep
//

#define FGB__MAX_FREQUENCY_SWEEP_FREQUENCY 2047

#define FGB__MAX_SWEEP_PERIOD 8

static void fgb__InitFrequencySweep(fgbFrequencySweep *sweep) {
	sweep->isDecrease = false;
	sweep->period = 0;
	sweep->shift = 0;
	sweep->shadow = 0;
}

static void fgb__StopFrequencySweep(fgbFrequencySweep *sweep) {
	sweep->isDecrease = false;
	sweep->timer = 0;
	sweep->shift = 0;
	sweep->period = 0;
}

static uint8_t fgb__GetFrequencySweepRegister(const fgbFrequencySweep *sweep) {
	fgbFrequencySweepRegister reg = {
		.shift = sweep->shift,
		.isDecrease = sweep->isDecrease,
		.period = sweep->period,
	};
	return reg.u8;
}

// Gets the frequency sweep timer in range of 1-8 (Obscure Behavior: 0 is is always 8)
static inline int32_t fgb__GetFrequencySweepTimer(const uint8_t period) {
	return period == 0 ? FGB__MAX_SWEEP_PERIOD : period;
}

// Increments/Decrements the current shadow frequency and returns the result.
// Note that due to the shift operation, an underflow cannot happen
static inline uint16_t fgb__ComputeFrequencyForSweep(fgbFrequencySweep *sweep) {
	uint16_t delta = sweep->shadow >> sweep->shift;

	// Due to the shift operation, we can never underflow
	FGB_ASSERT(!(sweep->shadow == 0 && delta > 0));
	
	uint16_t result;
	if (sweep->isDecrease) {
		result = sweep->shadow - delta;
		sweep->wasLastCalculationDecrease = true;
	} else {
		result = sweep->shadow + delta;
	}

	return result;
}

// Clocks the frequency sweep, which may change the frequency
// It is ticked every 128 Hz by the frame sequencer - which is internally clocked at 8192 Hz due to the CPU speed of ~4 MHz
static bool fgb__TickFrequencySweep(fgbSystem *system, fgbFrequencySweep *sweep, fgbSoundFrequency *freq, const bool voiceEnabled, bool *overflow) {
	if (!voiceEnabled || !sweep->isEnabled) {
		return false;
	}

	uint16_t period = fgb__GetFrequencySweepTimer(sweep->period);
	if (--sweep->timer <= 0) {
		if (period) {
			sweep->timer = period;
			uint16_t oldFrequency = sweep->shadow;
			uint16_t newFreq = fgb__ComputeFrequencyForSweep(sweep);
			if (newFreq > FGB__MAX_FREQUENCY_SWEEP_FREQUENCY) {
				*overflow = true;
			} else {
				bool result = false;

				if (sweep->shift) {
					sweep->shadow = newFreq;
					fgb__SetSoundPeriod(freq, newFreq);

#if FGB_APU_FREQUENCY_SWEEP_UPDATED_LOGGING
					FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-1] Changed frequency by sweep from %u to %u", system->cpu.state.totalTickCycles, oldFrequency, sweep->shadow);
#endif
					result = true;
				}

				// Perform another overflow check
				if (fgb__ComputeFrequencyForSweep(sweep) > FGB__MAX_FREQUENCY_SWEEP_FREQUENCY) {
					*overflow = true;
				}

				return result;
			}
		} else {
			sweep->timer = FGB__MAX_SWEEP_PERIOD;
		}
	}
	return false;
}

// Called when the gameboy wrote to NR14 and the voice gets triggered
static void fgb__TriggerFrequencySweep(fgbSystem *system, fgbFrequencySweep *sweep, const uint16_t frequency, bool *overflow) {
	sweep->isEnabled = sweep->period || sweep->shift;
	sweep->shadow = frequency;
	sweep->timer = fgb__GetFrequencySweepTimer(sweep->period);
	sweep->wasLastCalculationDecrease = false;

#if FGB_APU_FREQUENCY_SWEEP_CHANGED_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-1] Trigger Frequency Sweep: Frequency: %u, Period %u, Shift: %u, IsDec: %s", system->cpu.state.totalTickCycles, frequency, sweep->period, sweep->shift, sweep->isDecrease ? "yes" : "no");
#endif

	// If sweep shift is set, we may get an overflow
	if (sweep->shift && fgb__ComputeFrequencyForSweep(sweep) > FGB__MAX_FREQUENCY_SWEEP_FREQUENCY) {
		*overflow = true;
	}	
}

//
// Base Voice
//
static void fgb__InitBaseVoice(fgbVoice *voice) {
	voice->isEnabled = false;
	voice->isPowered = false;
	voice->isMuted = false;
	voice->isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = false;
	voice->isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = false;
}

static void fgb__PowerOffBaseVoice(fgbVoice *voice) {
	voice->isEnabled = false;
	voice->isPowered = false;
	voice->isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = false;
	voice->isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = false;
}

static void fgb__EnableBaseVoice(fgbSystem *system, fgbVoice *voice, const fgbVoiceType type, const bool enable, const char *reason) {
	voice->isEnabled = enable;

#if FGB_APU_VOICE_ENABLE_LOGGING
	if (enable)
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Enable voice reason: %s", system->cpu.state.totalTickCycles, (uint8_t)(type + 1), reason);
	else
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Disable voice reason: %s", system->cpu.state.totalTickCycles, (uint8_t)(type + 1), reason);
#endif
}

static uint8_t fgb__GetSoundLengthTimerWaveDutyRegister(const fgbSoundLengthTimer *lengthTimer, const uint8_t dutyCycle) {
	fgbSoundLengthTimerWaveDutyRegister reg = {
		.length = lengthTimer->length,
		.waveDuty = dutyCycle,
	};
	return reg.u8;
}

static uint8_t fgb__GetPeriodHighAndControlRegister(const fgbSoundFrequency *freq, const fgbSoundLengthTimer *length) {
	fgbPeriodHighAndControlRegister reg = {
		.periodHigh = freq->high,
		.isLengthEnabled = length->isEnabled,
		.isTriggered = false,
	};
	return reg.u8;
}

static void fgb__SetSquareWaveSoundLengthTimer(fgbSystem *system, fgbSoundLengthTimer *lengthTimer, fgbSquareWave *squareWave, const fgbSoundLengthTimerWaveDutyRegister reg, const fgbVoiceType type) {
	if (system->apu.state.isPowerOn) {
		squareWave->dutyCycle = reg.waveDuty;
	}

	fgb__ChangeSoundLengthTimer(lengthTimer, reg.length);

#if FGB_APU_LENGTH_CHANGED_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Set Sound Length / Duty: Len %u, Timer: %d, Duty: %u", system->cpu.state.totalTickCycles, (type + 1), lengthTimer->length, lengthTimer->timer, squareWave->dutyCycle);
#endif
}

static void fgb__SetVolumeEnvelope(fgbSystem *system, fgbVoice *voice, fgbVolumeEnvelope *env, const fgbVolumeEnvelopeRegister reg, const fgbVoiceType type) {
	env->initialVolume = reg.initialVolume;

#if FGB_APU_VOLUME_SET_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Set Volume Envelope: Initial: %u, IsInc: %s, Period: %u", system->cpu.state.totalTickCycles, (type + 1), env->initialVolume, env->isIncreasing ? "yes" : "no", env->period);
#endif

	// DAC is enabled when initial volume or increasing is non-zero
	voice->isPowered = reg.initialVolume || reg.isInc;

	if (!voice->isPowered) {
		fgb__EnableBaseVoice(system, voice, type, false, "Powered off due to volume envelope");
	}

	// Zombie Volume Adjustment for some games
	if (voice->isEnabled) {
		uint8_t oldVolume = env->currentVolume;
		if (env->period == 0 && env->isEnabled) {
			uint8_t newVolume = (oldVolume + 1) & FGB__MAX_VOLUME_ENVELOPE_VOL;
			fgb__ApplyVolumeEnvelope(system, env, newVolume, type);
			env->zombieStep = reg.u8 == 9;
		} else if (env->zombieStep > 0) {
			if (env->zombieStep == 1 && reg.u8 == 0x11) {
				env->zombieStep++;
			} else if (env->zombieStep == 2 && reg.u8 == 0x18) {
				env->zombieStep++;
				uint8_t newVolume = (oldVolume + FGB__MAX_VOLUME_ENVELOPE_VOL - 1) & FGB__MAX_VOLUME_ENVELOPE_VOL;
				fgb__ApplyVolumeEnvelope(system, env, newVolume, type);
			} else {
				env->zombieStep = 0;
			}
		}
	}

	env->isIncreasing = reg.isInc;
	env->period = reg.period;
}

// Called when the gameboy writes to NR10: No sweep is executed here, just the values applied for the next sweep.
// It returns false when the voice needs to be deactivated, due to subtraction mode obscure behavior - false otherwise.
static void fgb__SetFrequencySweep(fgbSystem *system, fgbVoice *voice, fgbFrequencySweep *sweep, const fgbFrequencySweepRegister reg) {
	bool oldIsDecrease = sweep->isDecrease;

	sweep->shift = reg.shift;
	sweep->isDecrease = reg.isDecrease;
	sweep->period = reg.period;

	// Obscure behavior: When the old direction is subtraction and the new direction is increase, but the last calculation was decrease - we disable the voice
	if (oldIsDecrease && !sweep->isDecrease && sweep->wasLastCalculationDecrease) {
		fgb__EnableBaseVoice(system, voice, fgbVoiceType_Sweep, false, "Frequency sweep direction is inverted");
	}
}

static bool fgb__SetNRx4(fgbSystem *system, fgbVoice *voice, fgbSoundFrequency *freq, fgbSoundLengthTimer *lengthTimer, const uint8_t value, const fgbVoiceType type) {
	fgbPeriodHighAndControlRegister reg = { .u8 = value };

	fgbAPU *apu = &system->apu;

	bool trigger = reg.isTriggered;
	bool wasLengthEnabled = lengthTimer->isEnabled;
	bool isLengthEnabled = reg.isLengthEnabled;

#if FGB_APU_NRx4_SET_LOGGING
	FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-%u] Set NRx4, Len-Enabled: %s, Trigger: yes", system->cpu.state.totalTickCycles, (type + 1), isLengthEnabled ? "yes" : "no", trigger ? "yes" : "no");
#endif

	lengthTimer->isEnabled = isLengthEnabled;

	// Only voice 0 and 1 has a frequency, so this is optional
	if (freq != NULL) {
		fgb__SetSoundPeriodHigh(system, freq, reg.periodHigh, type);
	}

	// Extra Length Clocking happens when the next frame in sequencer is not a length timer frame
	// But only when the length switches from disabled to enabled
	bool isNextFrameLength = (apu->state.frameSequencer.step % 2) == 0;
	if (!wasLengthEnabled && lengthTimer->isEnabled && !isNextFrameLength && lengthTimer->length > 0) {
		--lengthTimer->length;
		if (!trigger && lengthTimer->length == 0) {
			fgb__EnableBaseVoice(system, voice, type, false, "Length Timer was zero right at the beginning");
		}
	}

	if (trigger) {
		// Again weird behavior regarding length timer
		if (lengthTimer->length == 0) {
			lengthTimer->length = lengthTimer->maxLength;
			if (lengthTimer->isEnabled && !isNextFrameLength) {
				--lengthTimer->length;
			}
		}

		// Only enable voice when the DAC is powered
		fgb__EnableBaseVoice(system, voice, type, voice->isPowered, "Triggered and DAC powered");
	}

	return trigger;
}

static void fgb__TriggerVolumeEnvelope(fgbSystem *system, const fgbFrameSequencer *frameSeq, fgbVolumeEnvelope *env, const fgbVoiceType type) {
	fgb__ApplyVolumeEnvelope(system, env, env->initialVolume, type);

	env->timer = fgb__GetVolumeEnvelopTimer(env->period);
	env->isEnabled = true;

	// If the next frame in the sequencer will update the envelope, increment the timer
	if ((frameSeq->step + 1) == 7) {
		env->timer++;
	}
}

//
// Voice 1: Sweep + Tone
//
static void fgb__InitSweepVoice(fgbSweepVoice *voice) {
	fgb__InitBaseVoice(&voice->base);

	fgb__InitSquareWave(&voice->squareWave);
	fgb__InitSoundLengthTimer(&voice->length, 64);
	fgb__InitVolumeEnvelope(&voice->envelope);
	fgb__InitFrequencySweep(&voice->sweep);
}

static void fgb__PowerOffSweepVoice(fgbSystem *system, fgbSweepVoice *voice) {
	fgb__PowerOffBaseVoice(&voice->base);

	fgb__StopSquareWave(&voice->squareWave);
	fgb__StopSoundLengthTimer(&voice->length);
	fgb__StopVolumeEnvelope(&voice->envelope);
	fgb__StopFrequencySweep(&voice->sweep);
}

static void fgb__SetSweepVoiceNRx4(fgbSystem *system, fgbSweepVoice *voice, const uint8_t value) {
	fgbAPU *apu = &system->apu;

	bool trigger = fgb__SetNRx4(system, &voice->base, &voice->freq, &voice->length, value, fgbVoiceType_Sweep);

	fgb__SetSquareWavePeriod(&voice->squareWave, voice->freq.period);

	if (trigger) {
#if FGB_APU_VOICE_TRIGGER_LOGGING
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-1] Trigger with Initial-Volume %u, Square-Wave Period: %u, Timer: %d, Duty: %u", system->cpu.state.totalTickCycles, voice->envelope.initialVolume, voice->squareWave.period, voice->squareWave.timer, voice->squareWave.dutyCycle);
#endif

		fgb__TriggerVolumeEnvelope(system, &apu->state.frameSequencer, &voice->envelope, fgbVoiceType_Sweep);

		bool overflow = false;
		fgb__TriggerFrequencySweep(system, &voice->sweep, voice->freq.period, &overflow);
		if (overflow) {
			fgb__EnableBaseVoice(system, &voice->base, fgbVoiceType_Sweep, false, "Sweep Overflow");
		}

		fgb__StartSquareWave(&voice->squareWave);
	}
	return;
}

static void fgb__TickSweepVoice(fgbSweepVoice *voice) {
	fgb__TickSquareWave(&voice->squareWave);
}

static uint8_t fgb__GetSweepVoiceSample(fgbSweepVoice *voice) {
	if (!voice->base.isEnabled || !voice->base.isPowered || voice->base.isMuted)
		return 0;
	return fgb__GetSquareWaveSample(&voice->squareWave, voice->envelope.currentVolume);
}

static uint8_t fgb__GetSweepVoiceRegister(fgbSystem *system, const fgbSweepVoice *voice, const fgbSoundRegType reg) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
			return fgb__GetFrequencySweepRegister(&voice->sweep);

		case fgbSoundRegType_NRx1:
			return fgb__GetSoundLengthTimerWaveDutyRegister(&voice->length, voice->squareWave.dutyCycle);

		case fgbSoundRegType_NRx2:
			return fgb__GetVolumeEnvelopeRegister(&voice->envelope);

		case fgbSoundRegType_NRx3:
			// Write-Only
			return 0xFF;

		case fgbSoundRegType_NRx4:
			return fgb__GetPeriodHighAndControlRegister(&voice->freq, &voice->length);

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-1] Unsupported reg type of %u", reg);
			return 0xFF;
	}
}

static void fgb__SetSweepVoiceRegister(fgbSystem *system, fgbSweepVoice *voice, const fgbSoundRegType reg, const uint8_t value) {
	fgbAPU *apu = &system->apu;

	switch (reg) {
		case fgbSoundRegType_NRx0:
		{
			fgbFrequencySweepRegister reg = { .u8 = value };
			fgb__SetFrequencySweep(system, &voice->base, &voice->sweep, reg);
			return;
		}

		case fgbSoundRegType_NRx1:
		{
			fgbSoundLengthTimerWaveDutyRegister reg = { .u8 = value };
			fgb__SetSquareWaveSoundLengthTimer(system, &voice->length, &voice->squareWave, reg, fgbVoiceType_Sweep);
			return;
		}

		case fgbSoundRegType_NRx2:
		{
			fgbVolumeEnvelopeRegister reg = { .u8 = value };
			fgb__SetVolumeEnvelope(system, &voice->base, &voice->envelope, reg, fgbVoiceType_Sweep);
			return;
		}

		case fgbSoundRegType_NRx3:
		{
			fgbPeriodLowRegister reg = { .u8 = value };
			fgb__SetSoundPeriodLow(system, &voice->freq, reg.periodLow, fgbVoiceType_Sweep);
			fgb__SetSquareWavePeriod(&voice->squareWave, voice->freq.period);
			return;
		}

		case fgbSoundRegType_NRx4:
		{
			fgb__SetSweepVoiceNRx4(system, voice, value);
			return;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-1] Unsupported reg type of %u", reg);
			return;
	}
}

//
// Voice 2: Tone
//
static void fgb__InitToneVoice(fgbToneVoice *voice) {
	fgb__InitBaseVoice(&voice->base);

	fgb__InitSquareWave(&voice->squareWave);
	fgb__InitSoundLengthTimer(&voice->length, 64);
	fgb__InitVolumeEnvelope(&voice->envelope);
}

static void fgb__PowerOffToneVoice(fgbSystem *system, fgbToneVoice *voice) {
	fgb__PowerOffBaseVoice(&voice->base);

	fgb__StopSquareWave(&voice->squareWave);
	fgb__StopSoundLengthTimer(&voice->length);
	fgb__StopVolumeEnvelope(&voice->envelope);
}

static void fgb__SetToneVoiceNRx4(fgbSystem *system, fgbToneVoice *voice, const uint8_t value) {
	fgbAPU *apu = &system->apu;

	bool trigger = fgb__SetNRx4(system, &voice->base, &voice->freq, &voice->length, value, fgbVoiceType_Tone);

	fgb__SetSquareWavePeriod(&voice->squareWave, voice->freq.period);

	if (trigger) {
#if FGB_APU_VOICE_TRIGGER_LOGGING
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-2] Trigger with Initial-Volume %u, Square-Wave Period: %u, Timer: %d, Duty: %u", system->cpu.state.totalTickCycles, voice->envelope.initialVolume, voice->squareWave.period, voice->squareWave.timer, voice->squareWave.dutyCycle);
#endif

		fgb__TriggerVolumeEnvelope(system, &apu->state.frameSequencer, &voice->envelope, fgbVoiceType_Tone);

		fgb__StartSquareWave(&voice->squareWave);
	}

	return;
}

static void fgb__TickToneVoice(fgbToneVoice *voice) {
	fgb__TickSquareWave(&voice->squareWave);
}

static uint8_t fgb__GetToneVoiceSample(fgbToneVoice *voice) {
	if (!voice->base.isEnabled || !voice->base.isPowered || voice->base.isMuted)
		return 0;
	return fgb__GetSquareWaveSample(&voice->squareWave, voice->envelope.currentVolume);
}

static uint8_t fgb__GetToneVoiceRegister(fgbSystem *system, const fgbToneVoice *voice, const fgbSoundRegType reg) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
			// Unused
			return 0xFF;

		case fgbSoundRegType_NRx1:
			return fgb__GetSoundLengthTimerWaveDutyRegister(&voice->length, voice->squareWave.dutyCycle);

		case fgbSoundRegType_NRx2:
			return fgb__GetVolumeEnvelopeRegister(&voice->envelope);

		case fgbSoundRegType_NRx3:
			// Write-Only
			return 0xFF;

		case fgbSoundRegType_NRx4:
			return fgb__GetPeriodHighAndControlRegister(&voice->freq, &voice->length);

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-2] Unsupported reg type of %u", reg);
			return 0xFF;
	}
}

static void fgb__SetToneVoiceRegister(fgbSystem *system, fgbToneVoice *voice, const fgbSoundRegType reg, const uint8_t value) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
		{
			// Unused
			return;
		}

		case fgbSoundRegType_NRx1:
		{
			fgbSoundLengthTimerWaveDutyRegister reg = { .u8 = value };
			fgb__SetSquareWaveSoundLengthTimer(system, &voice->length, &voice->squareWave, reg, fgbVoiceType_Tone);
			return;
		}

		case fgbSoundRegType_NRx2:
		{
			fgbVolumeEnvelopeRegister reg = { .u8 = value };
			fgb__SetVolumeEnvelope(system, &voice->base, &voice->envelope, reg, fgbVoiceType_Tone);
			return;
		}

		case fgbSoundRegType_NRx3:
		{
			fgbPeriodLowRegister reg = { .u8 = value };
			fgb__SetSoundPeriodLow(system, &voice->freq, reg.periodLow, fgbVoiceType_Tone);
			fgb__SetSquareWavePeriod(&voice->squareWave, voice->freq.period);
			return;
		}

		case fgbSoundRegType_NRx4:
		{
			fgb__SetToneVoiceNRx4(system, voice, value);
			return;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-2] Unsupported reg type of %u", reg);
			return;
	}
}

//
// Voice 3: Wave
//

#define FGB__APU_WAVE_CORRUPTION_TICKS FGB__APU_TICKS

#define FGB__APU_WAVE_TRIGGER_DELAY_TICKS (FGB__APU_TICKS * 3)

// Map wave output level to a shift value, that is applied to a wave sample (shifting with 4 mutes the sample)
static uint8_t fgb__WaveVolumeShiftTable[4] = { 4, 0, 1, 2 };

// Map wave output level to a range of 0-15
static uint8_t fgb__WaveVolumeRangeTable[4] = { 0, 15, 7, 3 };

static void fgb__SetWaveVoicePeriod(fgbWaveVoice *voice) {
	voice->period = (2048 - voice->freq.period) * 2;
}

static void fgb__InitWaveVoice(fgbWaveVoice *voice) {
	fgb__InitBaseVoice(&voice->base);

	fgb__InitSoundLengthTimer(&voice->length, 256);

	voice->isPlaying = false;
	voice->period = 0;
	voice->timer = 0;
	voice->wavePosition = 0;
	voice->outputLevel = 0;
	voice->outputSample = 0;
}

static void fgb__PowerOffWaveVoice(fgbSystem *system, fgbWaveVoice *voice) {
	fgb__PowerOffBaseVoice(&voice->base);

	fgb__StopSoundLengthTimer(&voice->length);

	voice->isPlaying = false;
	voice->period = 0;
	voice->timer = 0;
	voice->wavePosition = 0;
	voice->outputLevel = 0;
	voice->outputSample = 0;
}

static uint8_t fgb__GetWaveVoicePattern(const fgbWaveVoice *voice, const uint8_t patternIndex) {
	FGB_ASSERT(patternIndex < 16);
	if (voice->base.isEnabled) {
		if (!voice->isPlaying) {
			uint8_t currentIndex = voice->wavePosition >> 1;
			return voice->patternRAM.entries[currentIndex].u8;
		}
		return 0xFF;
	} else {
		return voice->patternRAM.entries[patternIndex].u8;
	}
}

static void fgb__SetWaveVoicePattern(fgbWaveVoice *voice, const uint8_t patternIndex, const uint8_t value) {
	FGB_ASSERT(patternIndex < 16);
	if (voice->base.isEnabled) {
		if (!voice->isPlaying) {
			FGB_ASSERT(voice->wavePosition < 32);
			uint8_t currentIndex = voice->wavePosition >> 1;
			voice->patternRAM.entries[currentIndex].u8 = value;
		}
	} else {
		voice->patternRAM.entries[patternIndex].u8 = value;
	}
}

static void fgb__SetWaveVoiceNRx4(fgbSystem *system, fgbWaveVoice *voice, const uint8_t value) {
	bool trigger = fgb__SetNRx4(system, &voice->base, &voice->freq, &voice->length, value, fgbVoiceType_Wave);

	fgb__SetWaveVoicePeriod(voice);

	if (trigger) {
#if FGB_APU_VOICE_TRIGGER_LOGGING
		uint8_t volume = fgb__WaveVolumeRangeTable[voice->outputLevel];
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-3] Trigger with Initial-Volume %u", system->cpu.state.totalTickCycles, volume);
#endif

		if (voice->isPlaying) {
			// Triggering the wave voice, while its already playing will corrupt the wave RAM on DMG
			if (voice->timer == FGB__APU_WAVE_CORRUPTION_TICKS) {
				FGB_ASSERT(voice->wavePosition < 32);
				uint8_t position = (voice->wavePosition + 1) & 31;
				uint8_t byte = voice->patternRAM.entries[position >> 1].u8;
				switch (position >> 3) {
					case 0:
						voice->patternRAM.entries[0].u8 = 0;
						break;

					case 1:
					case 2:
					case 3:
						FGB_MEMCOPY(voice->patternRAM.entries, &voice->patternRAM.entries[(position >> 1) & 12].u8, 4);
						break;
				}
			}
		}

		voice->wavePosition = 0;
		voice->timer = voice->period + FGB__APU_WAVE_TRIGGER_DELAY_TICKS;
		voice->isPlaying = true;
		voice->outputSample = 0;

	}
}

static uint8_t fgb__GetWaveVoiceRegister(fgbSystem *system, const fgbWaveVoice *voice, const fgbSoundRegType reg) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
		{
			fgbWavePowerRegister reg = {
				.isPowered = voice->base.isPowered, 
				.unused = 0,
			};
			return reg.u8;
		}

		case fgbSoundRegType_NRx1: 
		{
			// The length is 8-bit on the wave voice
			return voice->length.length & 0xFF;
		}

		case fgbSoundRegType_NRx2:
		{
			fgbWaveOutputLevelRegister reg = {
				.outputLevel = voice->outputLevel,
			};
			return reg.u8;
		}
			
		case fgbSoundRegType_NRx3:
			// Write-Only
			return 0xFF;

		case fgbSoundRegType_NRx4:
		{
			fgbPeriodHighAndControlRegister reg = {
				.isLengthEnabled = voice->length.isEnabled,
				.periodHigh = voice->freq.high,
				.isTriggered = false,
			};
			return reg.u8;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-3] Unsupported reg type of %u", reg);
			return 0xFF;
	}
}

static void fgb__SetWaveVoiceRegister(fgbSystem *system, fgbWaveVoice *voice, const fgbSoundRegType reg, const uint8_t value) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
		{
			fgbWavePowerRegister reg = { .u8 = value };
			voice->base.isPowered = reg.isPowered;

			if (!voice->base.isPowered) {
				fgb__EnableBaseVoice(system, &voice->base, fgbVoiceType_Wave, false, "DAC is not powered");
				voice->isPlaying = false;
			}

			return;
		}

		case fgbSoundRegType_NRx1:
		{
			fgb__ChangeSoundLengthTimer(&voice->length, value);
			return;
		}

		case fgbSoundRegType_NRx2:
		{
			fgbWaveOutputLevelRegister reg = { .u8 = value };

			// Store output level for shifting the 4-sample to the correct length
			voice->outputLevel = reg.outputLevel;

			// Store volume shift to convert to 4-bit
			voice->volumeShift = fgb__WaveVolumeShiftTable[reg.outputLevel];

			return;
		}

		case fgbSoundRegType_NRx3:
		{
			fgbPeriodLowRegister reg = { .u8 = value };
			fgb__SetSoundPeriodLow(system, &voice->freq, reg.periodLow, fgbVoiceType_Wave);
			fgb__SetWaveVoicePeriod(voice);
			return;
		}

		case fgbSoundRegType_NRx4:
		{
			fgb__SetWaveVoiceNRx4(system, voice, value);
			return;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-3] Unsupported reg type of %u", reg);
	}
}

static void fgb__TickWaveVoice(fgbWaveVoice *voice) {
	if (--voice->timer == 0) {
		voice->timer = voice->period;

		voice->wavePosition = (voice->wavePosition + 1) & 0x1F;

		if (voice->base.isEnabled) {
			uint8_t ramPosition = voice->wavePosition / 2;

			uint8_t value;
			if ((voice->wavePosition & 0x1) == 0) {
				value = voice->patternRAM.entries[ramPosition].lower;
			} else {
				value = voice->patternRAM.entries[ramPosition].upper;
			}

			voice->outputSample = value >> voice->volumeShift;
		} else {
			voice->outputSample = 0;
		}
	}
}

static uint8_t fgb__GetWaveVoiceSample(fgbWaveVoice *voice) {
	if (!voice->base.isEnabled || !voice->base.isPowered || voice->base.isMuted)
		return 0;
	return voice->outputSample;
}

//
// Voice 4: Noise
//

#define FGB__MAX_NOISE_CLOCK_SHIFT 13

static void fgb__SetNoiseVoicePeriod(fgbNoiseVoice *voice) {
	FGB_ASSERT(voice->divisorIndex < FGB__NOISE_DIVISOR_COUNT);
	uint8_t divisor = fgb__noiseDivTable[voice->divisorIndex];
	voice->period = divisor << voice->clockShift;
}

static void fgb__InitNoiseVoice(fgbNoiseVoice *voice) {	
	fgb__InitBaseVoice(&voice->base);

	fgb__InitSoundLengthTimer(&voice->length, 64);
	fgb__InitVolumeEnvelope(&voice->envelope);

	voice->timer = 0;
	voice->lfsr = 0;
	voice->clockShift = 0;
	voice->is7Bit = false;
	voice->divisorIndex = 0;
}

static void fgb__PowerOffNoiseVoice(fgbSystem *system, fgbNoiseVoice *voice) {
	fgb__PowerOffBaseVoice(&voice->base);

	fgb__StopSoundLengthTimer(&voice->length);
	fgb__StopVolumeEnvelope(&voice->envelope);

	voice->timer = 0;
	voice->lfsr = 0;
	voice->clockShift = 0;
	voice->is7Bit = false;
	voice->divisorIndex = 0;
}

static void fgb__SetNoiseVoiceNRx4(fgbSystem *system, fgbNoiseVoice *voice, const uint8_t value) {
	fgbAPU *apu = &system->apu;

	bool trigger = fgb__SetNRx4(system, &voice->base, NULL, &voice->length, value, fgbVoiceType_Noise);

	if (trigger) {
#if FGB_APU_VOICE_TRIGGER_LOGGING
		FGB__DEBUG(system, fgb__KindName_APU, "[%llu] [Voice-4] Trigger with Initial-Volume %u", system->cpu.state.totalTickCycles, voice->envelope.initialVolume);
#endif

		fgb__SetNoiseVoicePeriod(voice);

		fgb__TriggerVolumeEnvelope(system, &apu->state.frameSequencer, &voice->envelope, fgbVoiceType_Noise);

		voice->lfsr = 0x7FFF;
		voice->timer = voice->period;
	}
}

static uint8_t fgb__GetNoiseVoiceRegister(fgbSystem *system, const fgbNoiseVoice *voice, const fgbSoundRegType reg) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
			// Unused
			return 0xFF;

		case fgbSoundRegType_NRx1:
		{
			fgbNoiseLengthTimerRegister reg = { .length = voice->length.length };
			return reg.u8;
		}
	
		case fgbSoundRegType_NRx2:
			return fgb__GetVolumeEnvelopeRegister(&voice->envelope);

		case fgbSoundRegType_NRx3:
		{
			fgbNoiseFrequencyRandomnessRegister reg = { 0 };
			reg.clockDivider = voice->divisorIndex & 0b11;
			reg.lfsrWidth = voice->is7Bit;
			reg.clockShift = voice->clockShift;
			return reg.u8;
		}

		case fgbSoundRegType_NRx4:
		{
			fgbNoiseControlRegister reg = { 0 };
			reg.isLengthEnabled = voice->length.isEnabled;
			reg.isTriggered = false;
			return reg.u8;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-4] Unsupported reg type of %u", reg);
			return 0xFF;
	}
}

static void fgb__SetNoiseVoiceRegister(fgbSystem *system, fgbNoiseVoice *voice, const fgbSoundRegType reg, const uint8_t value) {
	switch (reg) {
		case fgbSoundRegType_NRx0:
		{
			// Unused
			return;
		}

		case fgbSoundRegType_NRx1:
		{
			fgbNoiseLengthTimerRegister reg = { .u8 = value };
			fgb__ChangeSoundLengthTimer(&voice->length, reg.length);
			return;
		}

		case fgbSoundRegType_NRx2:
		{
			fgbVolumeEnvelopeRegister reg = { .u8 = value };
			fgb__SetVolumeEnvelope(system, &voice->base, &voice->envelope, reg, fgbVoiceType_Noise);
			return;
		}

		case fgbSoundRegType_NRx3:
		{
			fgbNoiseFrequencyRandomnessRegister reg = { .u8 = value };
			voice->clockShift = reg.clockShift;
			voice->divisorIndex = reg.clockDivider;
			voice->is7Bit = reg.lfsrWidth;
			fgb__SetNoiseVoicePeriod(voice);
			return;
		}

		case fgbSoundRegType_NRx4:
		{
			fgb__SetNoiseVoiceNRx4(system, voice, value);
			return;
		}

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_APU, "[Voice-4] Unsupported reg type of %u", reg);
	}
}

static void fgb__TickNoiseVoice(fgbNoiseVoice *voice) {
	if (--voice->timer <= 0) {
		voice->timer = voice->period;
		if (voice->clockShift <= FGB__MAX_NOISE_CLOCK_SHIFT) {
			uint8_t xorResult = (voice->lfsr & 0x1) ^ ((voice->lfsr >> 1 & 0x1));
			voice->lfsr >>= 1;
			voice->lfsr |= xorResult << 14;
			if (voice->is7Bit) {
				voice->lfsr &= ~0x40;
				voice->lfsr |= xorResult << 6;
			}
		}
	}
}

static uint8_t fgb__GetNoiseVoiceSample(fgbNoiseVoice *voice) {
	if (!voice->base.isEnabled || !voice->base.isPowered || voice->base.isMuted || ((voice->lfsr & 0x1) != 0))
		return 0;
	return voice->envelope.currentVolume;
}

//
// Audio Master Control (NR52/FF26)
//

static void fgb__APUZeroAudioRegisters(fgbSystem *system, fgbAPU *apu) {
	for (uint16_t address = 0xFF10; address < 0xFF26; ++address) {
		fgb__APUWrite(system, address, 0);
	}

	// Test that all registers are returning the OR result
	for (uint16_t address = 0xFF10; address < 0xFF26; ++address) {
		uint8_t value = fgb__APURead(system, address);
		uint8_t offset = address - 0xFF10;
		uint8_t addon = fgb__APU_ReadORTab[offset];
		FGB_ASSERT((value & addon) == addon);
	}
}

static inline uint8_t fgb__APUGetAudioMasterControlRegister(fgbSystem *system, fgbAPU *apu) {
	fgbAudioMasterControlRegister reg = {
		.audioOnOff = apu->state.isPowerOn,
		.unused = 0b111,
		.onVoice1 = apu->voices.sweep.base.isEnabled,
		.onVoice2 = apu->voices.tone.base.isEnabled,
		.onVoice3 = apu->voices.wave.base.isEnabled,
		.onVoice4 = apu->voices.noise.base.isEnabled,
	};
	return reg.u8;
}

static void fgb__APUSetAudioMasterControlRegister(fgbSystem *system, fgbAPU *apu, const uint8_t value) {
	// Remember sound length timer's
	uint16_t oldLengthTimers[4] = {
		apu->voices.sweep.length.timer,
		apu->voices.tone.length.timer,
		apu->voices.wave.length.timer,
		apu->voices.noise.length.timer,
	};

	fgbAudioMasterControlRegister reg = { .u8 = value };
	if (apu->state.isPowerOn && !reg.audioOnOff) {
		apu->state.isPowerOn = false;

		// Power off voices
		fgb__PowerOffSweepVoice(system, &apu->voices.sweep);
		fgb__PowerOffToneVoice(system, &apu->voices.tone);
		fgb__PowerOffWaveVoice(system, &apu->voices.wave);
		fgb__PowerOffNoiseVoice(system, &apu->voices.noise);

		// Stop frame sequencer
		fgb__StopFrameSequencer(&apu->state.frameSequencer);

		// Clear out all APU registers, except (FF26 to FF3F)
		fgb__APUZeroAudioRegisters(system, apu);
	} else if (!apu->state.isPowerOn && reg.audioOnOff) {
		// Initialize voices
		fgb__InitSweepVoice(&apu->voices.sweep);
		fgb__InitToneVoice(&apu->voices.tone);
		fgb__InitWaveVoice(&apu->voices.wave);
		fgb__InitNoiseVoice(&apu->voices.noise);

		// Start frame sequencer
		fgb__StartFrameSequencer(&system->timer, &apu->state.frameSequencer);

		// Sound powered on
		apu->state.isPowerOn = true;
	}

	if (reg.audioOnOff) {
		// Re-initialize sound length timer's
		apu->voices.sweep.length.timer = oldLengthTimers[0];
		apu->voices.tone.length.timer = oldLengthTimers[1];
		apu->voices.wave.length.timer = oldLengthTimers[2];
		apu->voices.noise.length.timer = oldLengthTimers[3];
	}
}

//
// Sound Panning (NR51/FF25)
//

static inline uint8_t fgb__APUGetSoundPanningRegister(fgbSystem *system, fgbAPU *apu) {
	fgbSoundPanningRegister reg = {
		.sweepRight = apu->voices.sweep.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT],
		.toneRight = apu->voices.tone.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT],
		.waveRight = apu->voices.wave.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT],
		.noiseRight = apu->voices.noise.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT],
		.sweepLeft = apu->voices.sweep.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT],
		.toneLeft = apu->voices.tone.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT],
		.waveLeft = apu->voices.wave.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT],
		.noiseLeft = apu->voices.noise.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT],
	};
	return  reg.u8;
}

static inline void fgb__APUSetSoundPanningRegister(fgbSystem *system, fgbAPU *apu, const uint8_t value) {
	fgbSoundPanningRegister reg = { .u8 = value };
	apu->voices.sweep.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = reg.sweepRight;
	apu->voices.tone.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = reg.toneRight;
	apu->voices.wave.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = reg.waveRight;
	apu->voices.noise.base.isSpeakerEnabled[FGB__APU_SPEAKER_RIGHT] = reg.noiseRight;
	apu->voices.sweep.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = reg.sweepRight;
	apu->voices.tone.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = reg.toneRight;
	apu->voices.wave.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = reg.waveRight;
	apu->voices.noise.base.isSpeakerEnabled[FGB__APU_SPEAKER_LEFT] = reg.noiseRight;
}

//
// Master Volume & VIN Panning (NR50/FF24)
//

static inline uint8_t fgb__APUGetMasterVolumeVINRegister(fgbSystem *system, fgbAPU *apu) {
	fgbAudioMasterVolumeVINRegister reg = {
		.rightVolume = fgb__PackVolumeTo3BITS(apu->state.stereoVolume[FGB__APU_SPEAKER_RIGHT]),
		.hasRight = apu->state.stereoEnabled[FGB__APU_SPEAKER_RIGHT],
		.leftVolume = fgb__PackVolumeTo3BITS(apu->state.stereoVolume[FGB__APU_SPEAKER_LEFT]),
		.hasLeft = apu->state.stereoEnabled[FGB__APU_SPEAKER_LEFT],
	};
	return  reg.u8;
}

static inline void fgb__APUSetMasterVolumeVINRegister(fgbSystem *system, fgbAPU *apu, const uint8_t value) {
	fgbAudioMasterVolumeVINRegister reg = { .u8 = value };
	apu->state.stereoVolume[FGB__APU_SPEAKER_RIGHT] = fgb__UnpackVolumeFrom3BITS(reg.rightVolume);
	apu->state.stereoVolume[FGB__APU_SPEAKER_LEFT] = fgb__UnpackVolumeFrom3BITS(reg.leftVolume);
	apu->state.stereoEnabled[FGB__APU_SPEAKER_RIGHT] = reg.hasRight;
	apu->state.stereoEnabled[FGB__APU_SPEAKER_LEFT] = reg.hasLeft;
}

//
// APU Core
//

#define FGB__APU_NR10 0xFF10
#define FGB__APU_NR11 0xFF11
#define FGB__APU_NR12 0xFF12
#define FGB__APU_NR13 0xFF13
#define FGB__APU_NR14 0xFF14

#define FGB__APU_NR21 0xFF16
#define FGB__APU_NR22 0xFF17
#define FGB__APU_NR23 0xFF18
#define FGB__APU_NR24 0xFF19

#define FGB__APU_NR30 0xFF1A
#define FGB__APU_NR31 0xFF1B
#define FGB__APU_NR32 0xFF1C
#define FGB__APU_NR33 0xFF1D
#define FGB__APU_NR34 0xFF1E

#define FGB__APU_NR41 0xFF20
#define FGB__APU_NR42 0xFF21
#define FGB__APU_NR43 0xFF22
#define FGB__APU_NR44 0xFF23

#define FGB__APU_NR50 0xFF24
#define FGB__APU_NR51 0xFF25
#define FGB__APU_NR52 0xFF26

static void fgb__APUInit(fgbSystem *system, const uint32_t sampleRate) {
	fgbAPU *apu = &system->apu;

	fgbClearStruct(&apu->buffer);
	fgbClearStruct(&apu->voices.sweep);
	fgbClearStruct(&apu->voices.tone);
	fgbClearStruct(&apu->voices.wave);
	fgbClearStruct(&apu->voices.noise);
	fgbClearStruct(&apu->state);

	apu->state.sampleRate = sampleRate;

	apu->state.cyclesPerSample = FGB_MAX_CPU_CYCLES / sampleRate;
	apu->state.sampleCycleTimer = 0;

	apu->state.stereoEnabled[FGB__APU_SPEAKER_LEFT] = false;
	apu->state.stereoEnabled[FGB__APU_SPEAKER_RIGHT] = false;
	apu->state.stereoVolume[FGB__APU_SPEAKER_LEFT] = 1.0f;
	apu->state.stereoVolume[FGB__APU_SPEAKER_RIGHT] = 1.0f;

	float chargeFactorRate = (float)FGB_MAX_CPU_CYCLES / (float)sampleRate;
	apu->state.highPassFilter.chargeFactor = powf(0.999958f, chargeFactorRate);
	apu->state.highPassFilter.capacitor = 0.0f;

	fgb__InitSweepVoice(&apu->voices.sweep);
	fgb__InitToneVoice(&apu->voices.tone);
	fgb__InitWaveVoice(&apu->voices.wave);
	fgb__InitNoiseVoice(&apu->voices.noise);
	fgb__InitFrameSequencer(&system->timer, &apu->state.frameSequencer);

	apu->state.isPowerOn = false;

	// Enable APU first, so subsequent writes succeed 
	fgb__APUWrite(system, FGB__APU_NR52, 0xf1);
	fgb__APUWrite(system, FGB__APU_NR11, 0x80);
	fgb__APUWrite(system, FGB__APU_NR12, 0xf3);
	fgb__APUWrite(system, FGB__APU_NR14, 0x80);
	fgb__APUWrite(system, FGB__APU_NR50, 0x77);
	fgb__APUWrite(system, FGB__APU_NR51, 0xf3);

	// Very important: Fill out the wave pattern ram with something other than zero
	FGB_MEMCOPY(&apu->voices.wave.patternRAM.m[0], &fgb__APU_DefaultRegister[32], FGB_WAVE_PATTERN_RAM_SIZE);

	FGB_ASSERT(apu->state.isPowerOn);
}

FGB_API bool fgbIsAudioVoiceMuted(const fgbSystem *system, const fgbVoiceType type) {
	if (system == NULL) {
		return false;
	}
	const fgbAPU *apu = &system->apu;
	switch (type) {
		case fgbVoiceType_Sweep:
			return apu->voices.sweep.base.isMuted;
		case fgbVoiceType_Tone:
			return apu->voices.tone.base.isMuted;
		case fgbVoiceType_Wave:
			return apu->voices.wave.base.isMuted;
		case fgbVoiceType_Noise:
			return apu->voices.noise.base.isMuted;
		default:
			return false;
	}
}

FGB_API void fgbSetAudioVoiceMute(fgbSystem *system, const fgbVoiceType type, const bool mute) {
	if (system == NULL) {
		return;
	}
	fgbAPU *apu = &system->apu;
	switch (type) {
		case fgbVoiceType_Sweep:
			apu->voices.sweep.base.isMuted = mute;
			break;
		case fgbVoiceType_Tone:
			apu->voices.tone.base.isMuted = mute;
			break;
		case fgbVoiceType_Wave:
			apu->voices.wave.base.isMuted = mute;
			break;
		case fgbVoiceType_Noise:
			apu->voices.noise.base.isMuted = mute;
			break;
		default:
			break;
	}
}

static inline fgbVoiceState fgb__GetVoiceState(const fgbVoice *voice) {
	if (voice->isMuted) {
		return fgbVoiceState_Muted;
	}
	if (voice->isPowered) {
		if (voice->isEnabled)
			return fgbVoiceState_Active;
		else
			return fgbVoiceState_Powered;
	}
	return fgbVoiceState_Off;
}

FGB_API fgbVoiceState fgbGetAudioVoiceState(const fgbSystem *system, const fgbVoiceType type) {
	if (system == NULL) {
		return false;
	}
	const fgbAPU *apu = &system->apu;
	switch (type) {
		case fgbVoiceType_Sweep:
			return fgb__GetVoiceState(&apu->voices.sweep.base);
		case fgbVoiceType_Tone:
			return fgb__GetVoiceState(&apu->voices.tone.base);
		case fgbVoiceType_Wave:
			return fgb__GetVoiceState(&apu->voices.wave.base);
		case fgbVoiceType_Noise:
			return fgb__GetVoiceState(&apu->voices.noise.base);
		default:
			return fgbVoiceState_Off;
	}
}

FGB_API float fgbGetAudioVoiceVolume(const fgbSystem *system, const fgbVoiceType type) {
	if (system == NULL) {
		return 0.0f;
	}
	const fgbAPU *apu = &system->apu;
	switch (type) {
		case fgbVoiceType_Sweep:
			return !apu->voices.sweep.base.isMuted ? apu->voices.sweep.envelope.currentVolume / (float)FGB__MAX_VOLUME_ENVELOPE_VOL : 0.0f;
		case fgbVoiceType_Tone:
			return !apu->voices.tone.base.isMuted ? apu->voices.tone.envelope.currentVolume / (float)FGB__MAX_VOLUME_ENVELOPE_VOL : 0.0f;
		case fgbVoiceType_Wave:
			return !apu->voices.wave.base.isMuted ? fgb__WaveVolumeRangeTable[apu->voices.wave.outputLevel] / (float)FGB__MAX_VOLUME_ENVELOPE_VOL : 0.0f;
		case fgbVoiceType_Noise:
			return !apu->voices.noise.base.isMuted ? apu->voices.noise.envelope.currentVolume / (float)FGB__MAX_VOLUME_ENVELOPE_VOL : 0.0f;
		default:
			return 0.0f;
	}
}

FGB_API float fgbGetAudioSpeakerVolume(const fgbSystem *system, const fgbSpeakerType type) {
	const fgbAPU *apu = &system->apu;
	if (type == fgbSpeakerType_Right) {
		return apu->state.stereoVolume[FGB__APU_SPEAKER_RIGHT];
	} else {
		return apu->state.stereoVolume[FGB__APU_SPEAKER_LEFT];
	}
}

FGB_API bool fgbIsAudioPowered(const fgbSystem *system) {
	if (system == NULL) {
		return false;
	}
	return system->apu.state.isPowerOn;
}

static void fgb__InitAudioBuffer(fgbAudioBuffer *buffer) {
	for (uint8_t i = 0; i < 2; ++i) {
		fgbClearStruct(&buffer->buffers[i]);
	}
	fgb__InterlockedExchange64(&buffer->fillCount, 0);
	fgb__InterlockedExchange64(&buffer->activeBufferIndex, 0);
}

static void fgb__WriteSamplesToAudioBuffer(fgbAudioBuffer *buffer, const uint8_t leftSample, const uint8_t rightSample) {
	uint64_t activeBufferIndex = fgb__InterlockedRead64(&buffer->activeBufferIndex);

	fgbAudioSampleBuffer *activeBuffer = &buffer->buffers[1 - activeBufferIndex];

	int64_t fillCount = fgb__InterlockedExchangeIncrement64(&activeBuffer->fillCount);

	// Write samples to the active buffer
	FGB_ASSERT(fillCount <= FGB_APU_MAX_FRAME_COUNT);
	int64_t head = fgb__InterlockedExchangeAdd64(&activeBuffer->headPosition, 2);
	activeBuffer->samples[(head + 0) % FGB_APU_MAX_SAMPLE_COUNT] = leftSample;
	activeBuffer->samples[(head + 1) % FGB_APU_MAX_SAMPLE_COUNT] = rightSample;
	
	if (fillCount >= FGB_APU_MAX_FRAME_COUNT) {
		// Buffer is full, switch to the next buffer and clear the new buffer
		int64_t newIndex = (activeBufferIndex + 1) & 1;
		fgb__InterlockedExchange64(&buffer->activeBufferIndex, newIndex);
		fgb__InterlockedExchange64(&buffer->fillCount, 0);

		// Clear new buffer
		fgbAudioSampleBuffer *newBuffer = &buffer->buffers[1 - newIndex];
		fgb__InterlockedExchange64(&newBuffer->fillCount, 0);
		fgb__InterlockedExchange64(&newBuffer->tailPosition, 0);
		fgb__InterlockedExchange64(&newBuffer->headPosition, 0);
	}
}

static uint32_t fgb__ReadSamplesFromAudioBuffer(fgbAudioBuffer *buffer, const uint32_t frameCount, uint8_t *outSamples) {
	uint64_t activeBufferIndex = fgb__InterlockedRead64(&buffer->activeBufferIndex);

	fgbAudioSampleBuffer *activeBuffer = &buffer->buffers[activeBufferIndex];

	uint64_t availableFrameCount = FGB_MAX(0, fgb__InterlockedRead64(&activeBuffer->fillCount));
	FGB_ASSERT(availableFrameCount <= SIZE_MAX);

	uint64_t numFrames = availableFrameCount;
	if (availableFrameCount > frameCount) {
		numFrames = frameCount;
	} else if (availableFrameCount < frameCount) {
		size_t remainingSize = (frameCount - (size_t)availableFrameCount) * FGB_APU_FRAME_SIZE;
		size_t writtenSize = (size_t)availableFrameCount * FGB_APU_FRAME_SIZE;
		FGB_MEMSET((uint8_t *)outSamples + writtenSize, 0, remainingSize);
		numFrames = availableFrameCount;
	}

	uint64_t readSampleIndex = fgb__InterlockedRead64(&activeBuffer->tailPosition);

	for (uint64_t frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
		for (uint8_t channelIndex = 0; channelIndex < 2; ++channelIndex) {
			uint8_t sample = activeBuffer->samples[(readSampleIndex + frameIndex * 2 + channelIndex) % FGB_APU_MAX_SAMPLE_COUNT];
			outSamples[frameIndex * 2 + channelIndex] = sample;
		}
	}

	fgb__InterlockedExchange64(&activeBuffer->tailPosition, (readSampleIndex + numFrames * 2) % FGB_APU_MAX_SAMPLE_COUNT);

	fgb__InterlockedExchangeAdd64(&buffer->fillCount, -(int64_t)numFrames);

	return frameCount;
}

// Tries to fetch N Audio-Frames (Two stereo samples) from the audio buffer
// If there are less than the required amount, the remaining is written as zero
// This is called from the outside, typically the audio playback callback that is ticked by the sound device
FGB_API uint32_t fgbFetchAudioSamples(fgbSystem *system, const uint32_t frameCount, uint8_t *outSamples) {
	fgbAPU *apu = &system->apu;

	if (!apu->state.isPowerOn) {
		return 0;
	}

	return fgb__ReadSamplesFromAudioBuffer(&apu->buffer, frameCount, outSamples);
}

// Clip the linear value into range of 0.0 to 1.0
static inline float fgb__AudioClipLinear(const float value) {
	return FGB_MAX(0.0f, FGB_MIN(1.0f, value));
}

// Clip the sample value into range of -1.0 to 1.0
static inline float fgb__AudioClipSample(const float value) {
	return FGB_MAX(-1.0f, FGB_MIN(1.0f, value));
}

static void fgb__UpdateVoices(fgbSystem *system, fgbAPU *apu) {
	fgbSweepVoice *sweepVoice = &apu->voices.sweep;
	fgbToneVoice *toneVoice = &apu->voices.tone;
	fgbWaveVoice *waveVoice = &apu->voices.wave;
	fgbNoiseVoice *noiseVoice = &apu->voices.noise;

#if !FGB_APU_DISABLE_VOICE_SWEEP
	fgb__TickSweepVoice(sweepVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_TONE
	fgb__TickToneVoice(toneVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_WAVE
	fgb__TickWaveVoice(waveVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_NOISE
	fgb__TickNoiseVoice(noiseVoice);
#endif
}

static void fgb__UpdateFrameSequencer(fgbSystem *system, fgbAPU *apu) {
	fgbSweepVoice *sweepVoice = &apu->voices.sweep;
	fgbToneVoice *toneVoice = &apu->voices.tone;
	fgbWaveVoice *waveVoice = &apu->voices.wave;
	fgbNoiseVoice *noiseVoice = &apu->voices.noise;

	uint8_t frameStep = 0;
	if (fgb__TickFrameSequencer(&system->timer, &apu->state.frameSequencer, &frameStep)) {
		// Tick Frequency Sweep
		if (frameStep == 2 || frameStep == 6) {
#if !FGB_APU_DISABLE_VOICE_SWEEP
			bool overflow = false;
			uint16_t oldFrequency = sweepVoice->sweep.shadow;
			if (fgb__TickFrequencySweep(system, &sweepVoice->sweep, &sweepVoice->freq, sweepVoice->base.isEnabled, &overflow)) {
				fgb__SetSquareWavePeriod(&sweepVoice->squareWave, sweepVoice->freq.period);
			}
			if (overflow) {
				fgb__EnableBaseVoice(system, &sweepVoice->base, fgbVoiceType_Sweep, false, "Sweep Overflow");
			}
#endif
		}

		// Tick Sound Length Timer
		if (frameStep % 2 == 0) {

#if !FGB_APU_DISABLE_VOICE_SWEEP
			// Sweep Voice
			if (fgb__TickSoundLengthTimer(&sweepVoice->length) == fgbSoundLengthState_EndReached) {
				fgb__EnableBaseVoice(system, &sweepVoice->base, fgbVoiceType_Sweep, false, "Length Timer");
			}
#endif

#if !FGB_APU_DISABLE_VOICE_TONE
			// Tone Voice
			if (fgb__TickSoundLengthTimer(&toneVoice->length) == fgbSoundLengthState_EndReached) {
				fgb__EnableBaseVoice(system, &toneVoice->base, fgbVoiceType_Tone, false, "Length Timer");
			}
#endif

#if !FGB_APU_DISABLE_VOICE_WAVE
			// Wave Voice
			if (fgb__TickSoundLengthTimer(&waveVoice->length) == fgbSoundLengthState_EndReached) {
				fgb__EnableBaseVoice(system, &waveVoice->base, fgbVoiceType_Wave, false, "Length Timer");
			}
#endif

#if !FGB_APU_DISABLE_VOICE_NOISE
			// Noise Voice
			if (fgb__TickSoundLengthTimer(&noiseVoice->length) == fgbSoundLengthState_EndReached) {
				fgb__EnableBaseVoice(system, &noiseVoice->base, fgbVoiceType_Noise, false, "Length Timer");
			}
#endif
		}

		// Tick Volume Envelope
		if (frameStep == 7) {
#if !FGB_APU_DISABLE_VOICE_SWEEP
			// Sweep Voice
			fgb__TickVolumeEnvelope(system, &sweepVoice->envelope, fgbVoiceType_Sweep);
#endif

#if !FGB_APU_DISABLE_VOICE_TONE
			// Tone Voice
			fgb__TickVolumeEnvelope(system, &toneVoice->envelope, fgbVoiceType_Tone);
#endif

#if !FGB_APU_DISABLE_VOICE_NOISE
			// Noise Voice
			fgb__TickVolumeEnvelope(system, &noiseVoice->envelope, fgbVoiceType_Noise);
#endif
		}
	}
}

static float fgb__HighPassFilter(fgbHighPassFilter *hpf, const float input, const bool isDACEnabled) {
	float result = 0.0f;
	if (isDACEnabled) {
		result = input - hpf->capacitor;
		hpf->capacitor = input - result * hpf->chargeFactor;
	}
	return result;
}

static void fgb__GenerateSteroSamples(fgbSystem *system, fgbAPU *apu, uint8_t samples[2]) {
	fgbSweepVoice *sweepVoice = &apu->voices.sweep;
	fgbToneVoice *toneVoice = &apu->voices.tone;
	fgbWaveVoice *waveVoice = &apu->voices.wave;
	fgbNoiseVoice *noiseVoice = &apu->voices.noise;

	// Generated mono samples for each voice, each is already multiplied by its own volume
	uint8_t sweep = 0;
	uint8_t tone = 0;
	uint8_t wave = 0;
	uint8_t noise = 0;

	if (apu->state.isPowerOn) {
#if !FGB_APU_DISABLE_VOICE_SWEEP
		sweep = fgb__GetSweepVoiceSample(sweepVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_TONE
		tone = fgb__GetToneVoiceSample(toneVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_WAVE
		wave = fgb__GetWaveVoiceSample(waveVoice);
#endif
#if !FGB_APU_DISABLE_VOICE_NOISE
		noise = fgb__GetNoiseVoiceSample(noiseVoice);
#endif
	}

	// Ensure that voice samples are in volume range
	FGB_ASSERT(sweep <= FGB__MAX_VOLUME_ENVELOPE_VOL);
	FGB_ASSERT(tone <= FGB__MAX_VOLUME_ENVELOPE_VOL);
	FGB_ASSERT(wave <= FGB__MAX_VOLUME_ENVELOPE_VOL);
	FGB_ASSERT(noise <= FGB__MAX_VOLUME_ENVELOPE_VOL);

	// To not get into clipping issues, we scale each sample down by 1/4
	const float sampleScale = 1.0f / (float)FGB__APU_VOICE_COUNT;

	// Inverse to scale samples by the volume in range of 0-15
	const float volumeEnvelopeScale = 1.0f / (float)FGB__MAX_VOLUME_ENVELOPE_VOL;

	// Scale samples into range of 0.0 - 1.0 or rather 0.0 - 0.25
	float monoSamples[4] = {
		sweep * volumeEnvelopeScale * sampleScale,
		tone * volumeEnvelopeScale * sampleScale,
		wave * volumeEnvelopeScale * sampleScale,
		noise * volumeEnvelopeScale * sampleScale,
	};

	// Mix the samples together in range of 0.0 to 1.0 and clip it
	float monoMixed = fgb__AudioClipLinear(monoSamples[0] + monoSamples[1] + monoSamples[2] + monoSamples[3]);

	// Convert it into range of -1.0 to 1.0
	float monoMixedRange = monoMixed * 2.0f - 1.0f;

	// Highpass filtering
	bool dacsOn = sweepVoice->base.isPowered || toneVoice->base.isPowered || waveVoice->base.isPowered || noiseVoice->base.isPowered;
	float filtered = fgb__HighPassFilter(&apu->state.highPassFilter, monoMixedRange, dacsOn);

	// Clip it to be in range of -1.0 to 1.0
	float clipped = fgb__AudioClipSample(filtered);

	// Convert back into linear range of 0.0 to 1.0
	float linear = (clipped + 1.0f) * 0.5f;

	// Stereo enable flags are ignored, so we or it with true - because NR50 is mostly set to 0x77 - disabling the speakers, which we dont want
	for (uint8_t speakerIndex = 0; speakerIndex < 2; ++speakerIndex) {
		bool isSpeakerEnabled = apu->state.stereoEnabled[speakerIndex] | true;
		float volume = isSpeakerEnabled ? apu->state.stereoVolume[speakerIndex] : 0.0f;
		float sample = linear * volume;
		samples[speakerIndex] = (uint8_t)(sample * 255.0f);
	}
}

// This is called every CPU cycle, the CPU clocks at the 4194304 Hz - so for example, a 8-bit bus read is 4 cycles, so would be called 4-times.
static bool fgb__APUTick(fgbSystem *system) {
	fgbAPU *apu = &system->apu;

	fgbSweepVoice *sweepVoice = &apu->voices.sweep;
	fgbToneVoice *toneVoice = &apu->voices.tone;
	fgbWaveVoice *waveVoice = &apu->voices.wave;
	fgbNoiseVoice *noiseVoice = &apu->voices.noise;

	// Update voices and the frame sequencer when the APU is powered on
	if (apu->state.isPowerOn) {
		fgb__UpdateVoices(system, apu);
		fgb__UpdateFrameSequencer(system, apu);
	}	

	uint8_t samples[2] = { 0 };

	// Sample timer reached? Reset timer to zero, generate and write out the stereo samples
	FGB_ASSERT(apu->state.cyclesPerSample > 0);
	if (++apu->state.sampleCycleTimer >= apu->state.cyclesPerSample) {
		apu->state.sampleCycleTimer = 0;
		fgb__GenerateSteroSamples(system, apu, samples);
		fgb__WriteSamplesToAudioBuffer(&apu->buffer, samples[0], samples[1]);
	}

	return true;
}

static uint8_t fgb__APURead_Direct(fgbSystem *system, const uint16_t address) {
	FGB_ASSERT(address >= FGB__BUS_ADDRESS_SOUND_REGISTER_FROM);
	fgbAPU *apu = &system->apu;

	uint16_t offset = address - 0xFF10;

	if (offset < 20) {
		uint8_t voiceIndex = offset / 5;
		fgbSoundRegType regType = fgbSoundRegType_NRx0 + offset % 5;
		switch (voiceIndex) {
			case 0:
				return fgb__GetSweepVoiceRegister(system, &apu->voices.sweep, regType);
			case 1:
				return fgb__GetToneVoiceRegister(system, &apu->voices.tone, regType);
			case 2:
				return fgb__GetWaveVoiceRegister(system, &apu->voices.wave, regType);
			case 3:
				return fgb__GetNoiseVoiceRegister(system, &apu->voices.noise, regType);
			default:
				FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported sound register type '%u' for address $%04X", regType, address);
				return 0xFF;
		}
	} else if (offset == 20) {
		// Control register NR50: Master Volume & VIN panning
		return fgb__APUGetMasterVolumeVINRegister(system, apu);
	} else if (offset == 21) {
		// Control register NR51: Channel stereo enable/panning
		return fgb__APUGetSoundPanningRegister(system, apu);
	} else if (offset == 22) {
		// Control register NR52: Sound/channel enable
		return fgb__APUGetAudioMasterControlRegister(system, apu);
	} else if (offset < 32) {
		// Unused registers
		return 0xFF;
	} else if (offset < 48) {
		// Wave Pattern RAM
		uint8_t patternIndex = offset - 32;
		return fgb__GetWaveVoicePattern(&apu->voices.wave, patternIndex);
	}
	return 0xFF;
}

static uint8_t fgb__APURead(fgbSystem *system, const uint16_t address) {
	if (address >= FGB__BUS_ADDRESS_SOUND_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SOUND_REGISTER_TO) {
		fgbAPU *apu = &system->apu;

		uint8_t raw = fgb__APURead_Direct(system, address);

		uint8_t offset = address - FGB__BUS_ADDRESS_SOUND_REGISTER_FROM;

		uint8_t addon = fgb__APU_ReadORTab[offset];

		uint8_t result = raw | addon;

#if FGB_APU_READ_REGISTER_IO_LOGGING
		const char *audioRegisterName = fgb__apuSoundRegisterLabels[offset];
		FGB__DEBUG(system, fgb__KindName_APU, "Read $%02X (0b" FGB__BYTE_TO_BINARY_PATTERN ") from register '%s' (%04X) -> Raw is $%02X (0b" FGB__BYTE_TO_BINARY_PATTERN ")", result, FGB__BYTE_TO_BINARY(result), audioRegisterName, address, raw, FGB__BYTE_TO_BINARY(raw));
#endif
		return result;
	}

	FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported read from address '%04X'", address);
	return 0xFF;
}

static void fgb__APUWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbAPU *apu = &system->apu;

	if (address >= FGB__BUS_ADDRESS_SOUND_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SOUND_REGISTER_TO) {
		uint16_t offset = address - FGB__BUS_ADDRESS_SOUND_REGISTER_FROM;

#if FGB_APU_WRITE_REGISTER_IO_LOGGING
		const char *audioRegisterName = fgb__apuSoundRegisterLabels[offset];
		FGB__DEBUG(system, fgb__KindName_APU, "Write $%02X (0b" FGB__BYTE_TO_BINARY_PATTERN ") to register '%s' (%04X)", value, FGB__BYTE_TO_BINARY(value), audioRegisterName, address);
#endif

		if (offset < 20) {
			if (!apu->state.isPowerOn) {
				return;
			}
			uint8_t voiceIndex = offset / 5;
			uint8_t nrOffset = offset % 5;
			fgbSoundRegType regType = fgbSoundRegType_NRx0 + nrOffset;
			switch (voiceIndex) {
				case 0:
#if !FGB_APU_DISABLE_VOICE_SWEEP
					fgb__Breakpoint(system, fgbBreakpointType_APUVoice1Write);
					fgb__SetSweepVoiceRegister(system, &apu->voices.sweep, regType, value);
#endif
					return;
				case 1:
#if !FGB_APU_DISABLE_VOICE_TONE
					fgb__Breakpoint(system, fgbBreakpointType_APUVoice2Write);
					fgb__SetToneVoiceRegister(system, &apu->voices.tone, regType, value);
#endif
					return;
				case 2:
#if !FGB_APU_DISABLE_VOICE_WAVE
					fgb__Breakpoint(system, fgbBreakpointType_APUVoice3Write);
					fgb__SetWaveVoiceRegister(system, &apu->voices.wave, regType, value);
#endif
					return;
				case 3:
#if !FGB_APU_DISABLE_VOICE_NOISE
					fgb__Breakpoint(system, fgbBreakpointType_APUVoice4Write);
					fgb__SetNoiseVoiceRegister(system, &apu->voices.noise, regType, value);
#endif
					return;
				default:
					FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported sound register type '%u' for address $%04X", regType, address);
					return;
			}
		} else if (offset == 20) {
			// Control register NR50: Master Volume & VIN panning
			if (apu->state.isPowerOn) {
				fgb__Breakpoint(system, fgbBreakpointType_APUNR50Write);
				fgb__APUSetMasterVolumeVINRegister(system, apu, value);
			}
			return;
		} else if (offset == 21) {
			// Control register NR51: Channel stereo enable/panning
			if (apu->state.isPowerOn) {
				fgb__Breakpoint(system, fgbBreakpointType_APUNR51Write);
				fgb__APUSetSoundPanningRegister(system, apu, value);
			}
			return;
		} else if (offset == 22) {
			// Control register NR52: Sound on/off
			fgb__Breakpoint(system, fgbBreakpointType_APUNR52Write);
			fgb__APUSetAudioMasterControlRegister(system, apu, value);
			return;
		} else if (offset < 32) {
			// Unused registers
			return;
		} else if (offset < 48) {
			// Wave Pattern RAM
			uint8_t patternIndex = offset - 32;
			fgb__SetWaveVoicePattern(&apu->voices.wave, patternIndex, value);
			return;
		}
	}

	FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported write '$%02X' to address '%04X'", value, address);
}

// ********************************************************************************************************************
// 
// PPU Implementation
// 
// ********************************************************************************************************************

// Gets start address that is shared with background and window tiles (BIT 4: 0 = 0x8800, 1 = 0x8000 )
static inline uint16_t fgbGetPPUBackgroundWindowTilesArea(const fgbLCDRegister *lcd) {
	return !lcd->lcdc.backgroundWindowTilesAreaSelect ? 0x8800 : 0x8000;
}

// Gets start address that is for background data (BIT 3, 0 = 0x9800, 1 = 0x9C00)
static inline uint16_t fgbGetPPUBackgroundDataArea(const fgbLCDRegister *lcd) {
	return !lcd->lcdc.backgroundDataAreaSelect ? 0x9800 : 0x9C00;
}

// Gets start address that is for window data (BIT 6, 0 = 0x9800, 1 = 0x9C00)
static inline uint16_t fgbGetPPUWindowDataArea(const fgbLCDRegister *lcd) {
	return !lcd->lcdc.windowDataAreaSelect ? 0x9800 : 0x9C00;
}

static void fgb__PPUPaletteSet(fgbMonochromePalette *targetPalette, const fgbMonochromeColors *sourceColors, const uint8_t index, const uint8_t data) {
	fgbColor *target;
	const fgbColor *source;
	if (index == 1) {
		source = sourceColors->sprite0;
		target = targetPalette->sprite0;
	} else if (index == 2) {
		source = sourceColors->sprite1;
		target = targetPalette->sprite1;
	} else {
		source = sourceColors->background;
		target = targetPalette->background;
	}

	target[0] = source[(data >> 0) & 0b11];
	target[1] = source[(data >> 2) & 0b11];
	target[2] = source[(data >> 4) & 0b11];
	target[3] = source[(data >> 6) & 0b11];
}

static void fgb__PPUPaletteInit(fgbMonochromePalette *targetPalette, const fgbMonochromeColors *sourceColors) {
	for (int i = 0; i < 4; ++i) {
		targetPalette->system[i] = sourceColors->system[i];
		targetPalette->background[i] = sourceColors->background[i];
		targetPalette->sprite0[i] = sourceColors->sprite0[i];
		targetPalette->sprite1[i] = sourceColors->sprite1[i];
	}
}

static void fgb__PPUPaletteReload(const fgbLCDRegister *lcd, fgbMonochromePalette *targetPalette, const fgbMonochromeColors *sourceColors) {
	for (int i = 0; i < 4; ++i) {
		targetPalette->system[i] = sourceColors->system[i];
	}
	fgb__PPUPaletteSet(targetPalette, sourceColors, 0, lcd->bgp.u8);
	fgb__PPUPaletteSet(targetPalette, sourceColors, 1, lcd->obp0.u8);
	fgb__PPUPaletteSet(targetPalette, sourceColors, 2, lcd->obp1.u8);
}

static inline void fgb__PPUSetMode(fgbSystem *system, fgbLCDRegister *lcd, const fgbPPUMode mode) {
	lcd->stat.lcdMode = mode;

	fgb__Breakpoint(system, fgbBreakpointType_LCDControlMode);
}

// How many total dots/ticks per second (~59.7 fps)
#define FGB__PPU_FRAME_DOTS 70224

// Only 40 active sprites can be drawn, any additional one must be skipped
#define FGB__PPU_MAX_SPRITES 40

// Supports only 10 sprites in a line, more will be not considered in the OAM search mode
#define FGB__PPU_MAX_SPRITES_PER_LINE 10

// Mode 0: Horizontal Blank Timing: The dots/ticks per one horizontal line
#define FGB__PPU_HORIZONTAL_BLANK_DOTS 456

// Mode 2: OAM Scan Timing (Searching for OBJs which overlap line, 80 dots)
#define FGB__PPU_OAM_SCAN_DOTS 80

// Vertical Blank Timing (Mode 1, 4560 dots, 10 Scanlines)
#define FGB__PPU_VERTICAL_BLANK_DOTS (FGB__PPU_HORIZONTAL_BLANK_DOTS * 10)
// Vertical Blank Line Count (Mode 1, only 0-143 is visible on the screen, 144-153 is used for doing the V-Blank)
#define FGB__PPU_VERTICAL_BLANK_LINE_COUNT (FGB_DISPLAY_HEIGHT + 10)

static inline uint8_t fgb__PPUDecodeColorIndex(const uint8_t first, const uint8_t second, const uint8_t bit) {
	FGB_ASSERT(bit >= 0 && bit <= 7);

	uint8_t mask = 1 << bit; // Create a mask, so we can clear the bits we don't care

	// Get the bit of the first byte and move it to the least significant bit plus 0
	// Get the bit of the second byte and move it to the least significant bit plus 1
	uint8_t hi = !!(first & (1 << bit)) << 0;
	uint8_t lo = !!(second & (1 << bit)) << 1;

	// Now we can simply or them together
	// Because there are in the format 0b00000001 and 0b00000010
	// Resulting in 00 or 01 or 10 or 11
	uint8_t result = hi | lo; // 

	return result;
}

static void fgb__PPUUpdateTileRow(fgbPPU *ppu, const fgbTileLine *line, const uint32_t startPixelX, const uint32_t startPixelY, const uint32_t scanlineWidth, fgbColor *outPixels) {
	uint8_t first = line->lower.value;
	uint8_t second = line->upper.value;
	for (uint8_t x = 0; x < FGB_TILE_WIDTH; ++x) {
		uint8_t bit = 7 - x;
		uint8_t colorIndex = fgb__PPUDecodeColorIndex(first, second, bit);
		fgbColor color = ppu->currentMonochromeColors.background[colorIndex];
		outPixels[startPixelY * scanlineWidth + startPixelX + x] = color;
	}
}

static void fgb__PPUVideoRAMUpdated(fgbPPU *ppu, const uint16_t addressIndex) {
	if (ppu->state.isDisplayEnabled && addressIndex < FGB_VRAM_TILEMAP_SIZE) {
		uint16_t normalizedAddress = addressIndex & 0xFFFE;

		uint16_t normalizedIndex = normalizedAddress;
		uint32_t tileIndex = normalizedIndex / FGB_TILE_SIZE;
		FGB_ASSERT(tileIndex < FGB_TOTAL_TILE_COUNT);

		uint32_t tileRow = tileIndex / FGB_TILEMAP_HORIZONTAL_COUNT;
		uint32_t tileColumn = tileIndex % FGB_TILEMAP_HORIZONTAL_COUNT;

		uint32_t lineY = (normalizedIndex / 2) % FGB_TILE_HEIGHT;

		uint32_t pixelX = tileColumn * FGB_TILE_WIDTH;
		uint32_t pixelY = tileRow * FGB_TILE_HEIGHT + lineY;

		const fgbVRAMBank *bank = &ppu->vram.bank0;

		const fgbTile *tile = &bank->tiles.tiles[tileIndex];

		const fgbTileLine *line = &tile->lines[lineY];

		fgb__PPUUpdateTileRow(ppu, line, pixelX, pixelY, FGB_TILEMAP_WIDTH, ppu->tilemap);

		ppu->state.isVRAMUpdated = true;
	}
}

static void fgb__PPUVideoRAMFullUpdate(fgbPPU *ppu) {
	if (!ppu->state.isDisplayEnabled)
		return;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	for (uint32_t tileRow = 0; tileRow < FGB_TILEMAP_VERTICAL_COUNT; ++tileRow) {
		for (uint32_t tileCol = 0; tileCol < FGB_TILEMAP_HORIZONTAL_COUNT; ++tileCol) {
			uint32_t tileIndex = tileRow * FGB_TILEMAP_HORIZONTAL_COUNT + tileCol;

			FGB_ASSERT(tileIndex < FGB_TOTAL_TILE_COUNT);
			const fgbTile *tile = &bank->tiles.tiles[tileIndex];

			uint32_t pixelX = FGB_TILE_WIDTH * tileCol;
			uint32_t pixelY = FGB_TILE_HEIGHT * tileRow;

			for (uint8_t lineIndex = 0; lineIndex < 8; ++lineIndex) {
				const fgbTileLine *line = &tile->lines[lineIndex];
				fgb__PPUUpdateTileRow(ppu, line, pixelX, pixelY + lineIndex, FGB_TILEMAP_WIDTH, ppu->tilemap);
			}
		}
	}

	ppu->state.isVRAMUpdated = true;
}

static void fgb__PPUBackgroundMapUpdate(fgbPPU *ppu) {
	if (!ppu->state.isDisplayEnabled)
		return;

	fgbLCDRegister *lcd = &ppu->lcd;

	uint32_t scanlineWidth = FGB_BACKGROUND_MAP_WIDTH;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	for (uint32_t tileRow = 0; tileRow < FGB_TILEDATA_VERTICAL_COUNT; ++tileRow) {
		for (uint32_t tileCol = 0; tileCol < FGB_TILEDATA_HORIZONTAL_COUNT; ++tileCol) {
			uint16_t dataIndex = tileRow * FGB_TILEDATA_HORIZONTAL_COUNT + tileCol;

			bool useMap9800 = !lcd->lcdc.backgroundDataAreaSelect;
			bool useTiles8800 = !lcd->lcdc.backgroundWindowTilesAreaSelect;

			fgbBackgroundMapTileInfo info = ppu->backgroundMap.tilesInfos[dataIndex];
			useMap9800 = !info.backgroundDataAreaSelect;
			useTiles8800 = !info.backgroundWindowTilesAreaSelect;

			uint32_t dataOffset = useMap9800 ? 0 : 1024; // 9800 or 9C00

			uint16_t dataPosition = dataOffset + dataIndex;

			FGB_ASSERT(dataPosition < FGB_TILEMAP_TOTAL_SIZE);
			uint8_t tileID = bank->data.m[dataPosition];

			uint32_t pixelX = FGB_TILE_WIDTH * tileCol;
			uint32_t pixelY = FGB_TILE_HEIGHT * tileRow;

			uint16_t tileAddress;

			if (useTiles8800) {
				tileAddress = 0x8800 + ((int8_t)tileID + 128) * 16;
			} else {
				tileAddress = 0x8000 + tileID * 16;
			}

			const fgbTile *tile = (const fgbTile *)(&ppu->vram.m[tileAddress - 0x8000]);

			for (uint8_t lineIndex = 0; lineIndex < 8; ++lineIndex) {
				const fgbTileLine *line = &tile->lines[lineIndex];
				fgb__PPUUpdateTileRow(ppu, line, pixelX, pixelY + lineIndex, scanlineWidth, ppu->backgroundMap.colors);
			}
		}
	}

	ppu->backgroundMap.scrollX = ppu->lcd.scx;
	ppu->backgroundMap.scrollY = ppu->lcd.scy;

	ppu->state.isBackgroundMapUpdated = true;
}

static void fgb__PPUDisplayClear(fgbPPU *ppu, const fgbColor color) {
	for (uint32_t i = 0, c = FGB_DISPLAY_HEIGHT * FGB_DISPLAY_WIDTH; i < c; ++i) {
		ppu->display[i] = color;
	}
}

// Function that raises an LCD STAT interrupt on certain conditions based on the GB manual
static void fgb__PPUStatusInterruptTest(fgbSystem *system, const bool vBlankTrigger) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	bool canRaise = false;

	canRaise |= (lcd->stat.lcdMode == fgbPPUMode_HBlank && lcd->stat.mode0IntSelect);
	canRaise |= (lcd->stat.lcdMode == fgbPPUMode_VBlank && lcd->stat.mode1IntSelect);
	canRaise |= (lcd->stat.lcdMode == fgbPPUMode_OAMSearch && lcd->stat.mode2IntSelect);
	canRaise |= (vBlankTrigger && lcd->stat.mode2IntSelect);
	canRaise |= (lcd->stat.lycIntSelect && lcd->stat.lycEqualsLY);

	if (canRaise) {
		if (!ppu->state.hasLCDStatusInterruptRequested) {
			fgb__InterruptRequest(system, fgbInterruptType_LCDStatus, "LCD mode or LY = LYC or VBlank Trigger");
			ppu->state.hasLCDStatusInterruptRequested = true;
		}
	} else {
		ppu->state.hasLCDStatusInterruptRequested = false;
	}
}

static void fgb__PPUCompareLine(fgbSystem *system) {
	fgbIORegisters *io = &system->io;
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (lcd->lyc == lcd->ly) {
		lcd->stat.lycEqualsLY = 1;
	} else {
		lcd->stat.lycEqualsLY = 0;
	}

	fgb__PPUStatusInterruptTest(system, false);
}

static void fgb__PPUResetLine(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	ppu->state.windowLine = 0;

	lcd->ly = 0;

	fgb__PPUCompareLine(system);
}

//
// DMA Transfer
//
static void fgb__PPUOAMWrite(fgbSystem *system, const uint16_t address, const uint8_t value);

static bool fgb__IsDMAActive(const fgbDMA *dma) {
	return dma->isActive;
}

static void fgb__DMAStart(fgbSystem *system, const uint8_t base) {
	fgbDMA *dma = &system->ppu.dma;

	fgbClearStruct(dma);

	dma->isActive = true;
	dma->offset = 0;
	dma->delay = 2;
	dma->base = base;

#if FGB_PPU_DMA_TRANSFER_LOGGING
	FGB__DEBUG(system, fgb__KindName_DMA, "DMA Start with base '%02X'", base);
#endif
}

static bool fgb__DMATick(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;

	fgbDMA *dma = &ppu->dma;

	if (!dma->isActive) {
		return true;
	}

	if (dma->delay > 0) {
		--dma->delay;

		if (dma->delay == 0) {
			// Only log out that the DMA is transferring, do not transfer here, wait one additional tick
#if FGB_PPU_DMA_TRANSFER_LOGGING
			FGB__DEBUG(system, fgb__KindName_DMA, "DMA Transfer with base '%02X' and offset '%02X' started", dma->base, dma->offset);
#endif
		}

		return true;
	}

	uint16_t index = dma->offset;
	uint16_t address = (dma->base * 0x100) + index;

	uint8_t read = fgb__BusRead8_Direct(system, address);

	fgb__PPUOAMWrite(system, index, read);

	++dma->offset;

	if (dma->offset < 0xA0) {
		dma->isActive = true;
	} else {
#if FGB_PPU_DMA_TRANSFER_LOGGING
		FGB__DEBUG(system, fgb__KindName_DMA, "DMA Transfer with base '%02X' and offset '%02X' complete", dma->base, dma->offset);
#endif
		dma->isActive = false;
	}

	return true;
}

//
// OAM Handling
//
static inline bool fgb__IsPPUOAMAllowedRead(const fgbPPU *ppu) {
	bool result = !fgb__IsDMAActive(&ppu->dma) && (ppu->lcd.stat.lcdMode == fgbPPUMode_HBlank || ppu->lcd.stat.lcdMode == fgbPPUMode_VBlank);
	return result;
}

static inline bool fgb__IsPPUOAMAllowedWrite(const fgbPPU *ppu) {
	bool result = !fgb__IsDMAActive(&ppu->dma) && (ppu->lcd.stat.lcdMode == fgbPPUMode_HBlank || ppu->lcd.stat.lcdMode == fgbPPUMode_VBlank);
	return result;
}

static uint8_t fgb__PPUOAMRead(fgbSystem *system, const uint16_t address) {
	const fgbPPU *ppu = &system->ppu;
	const fgbLCDRegister *lcd = &ppu->lcd;

	return ppu->oam.m[address - FGB__BUS_ADDRESS_PPU_OAM_FROM];
}

static void fgb__PPUOAMWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbPPU *ppu = &system->ppu;
	fgbIORegisters *io = &system->io;
	fgbLCDRegister *lcd = &ppu->lcd;

	uint16_t index = (address >= FGB__BUS_ADDRESS_PPU_OAM_FROM) ? address - FGB__BUS_ADDRESS_PPU_OAM_FROM : address;
	FGB_ASSERT(index < FGB_OAM_TABLE_SIZE);

	ppu->oam.m[index] = value;
}

//
// Video RAM (Tile Data, Character Data)
//
static inline bool fgb__IsPPUVRAMAllowedRead(const fgbPPU *ppu) {
	fgbLCDStatus stat = ppu->lcd.stat;
	return stat.lcdMode != fgbPPUMode_PixelTransfer;
}

static inline bool fgb__IsPPUVRAMAllowedWrite(const fgbPPU *ppu) {
	fgbLCDStatus stat = ppu->lcd.stat;
	return stat.lcdMode != fgbPPUMode_PixelTransfer;
}

static uint8_t fgb__PPUTilesRead(fgbSystem *system, const uint16_t address) {
	const fgbPPU *ppu = &system->ppu;
	const fgbLCDRegister *lcd = &ppu->lcd;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	uint16_t index = address - FGB__BUS_ADDRESS_PPU_TILES_FROM;
	return bank->tiles.m[index];
}

static void fgb_PPUTilesWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbPPU *ppu = &system->ppu;
	fgbIORegisters *io = &system->io;
	fgbLCDRegister *lcd = &ppu->lcd;

	fgbVRAMBank *bank = &ppu->vram.bank0;

	uint16_t index = address - FGB__BUS_ADDRESS_PPU_TILES_FROM;
	bank->tiles.m[index] = value;
}

static uint8_t fgb__PPUDataRead(fgbSystem *system, const uint16_t address) {
	const fgbPPU *ppu = &system->ppu;
	const fgbLCDRegister *lcd = &ppu->lcd;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	uint16_t index = address - FGB__BUS_ADDRESS_PPU_DATA_FROM;
	uint8_t result = bank->data.m[index];
	return result;
}

static void fgb__PPUDataWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbPPU *ppu = &system->ppu;
	fgbIORegisters *io = &system->io;
	fgbLCDRegister *lcd = &ppu->lcd;

	fgbVRAMBank *bank = &ppu->vram.bank0;

	uint16_t index = address - FGB__BUS_ADDRESS_PPU_DATA_FROM;
	bank->data.m[index] = value;
}

//
// LCD Register
//

static uint8_t fgb__PPULCDRegisterRead(fgbSystem *system, const uint16_t address) {
	const fgbPPU *ppu = &system->ppu;
	const fgbLCDRegister *lcd = &ppu->lcd;

	switch (address) {
		case 0xFF40:
			return lcd->lcdc.u8;
		case 0xFF41:
			return lcd->stat.u8;
		case 0xFF42:
			return lcd->scy;
		case 0xFF43:
			return lcd->scx;
		case 0xFF44:
			return lcd->ly;
		case 0xFF45:
			return lcd->lyc;
		case 0xFF46:
			return 0xFF; // DMA is write-only
		case 0xFF47:
			return lcd->bgp.u8;
		case 0xFF48:
			return lcd->obp0.u8;
		case 0xFF49:
			return lcd->obp1.u8;
		case 0xFF4A:
			return lcd->wy;
		case 0xFF4B:
			return lcd->wx;
		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported LCD read from address '%04X'", address);
			return 0xFF;
	}
}

static void fgb__PPULCDRegisterWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbPPU *ppu = &system->ppu;
	fgbIORegisters *io = &system->io;
	fgbLCDRegister *lcd = &ppu->lcd;

	switch (address) {
		case 0xFF40:
		{
			bool wasLCDTurnedOn = lcd->lcdc.lcdEnabled;

			lcd->lcdc.u8 = value;

			if (wasLCDTurnedOn && !lcd->lcdc.lcdEnabled) {
				fgb__Breakpoint(system, fgbBreakpointType_LCDControlPower);

				// LCD is going to be turned off
				if (lcd->stat.lcdMode != fgbPPUMode_VBlank) {
					FGB__WARN(system, fgb__KindName_PPU, "The LCD may not be turned off in mode '%u', turn off only in VBlank mode", lcd->stat.lcdMode);
				}

				fgb__PPUDisplayClear(ppu, ppu->currentMonochromeColors.system[1]);

				fgb__PPUSetMode(system, lcd, fgbPPUMode_HBlank);

				lcd->ly = 0;

				ppu->state.lcdOffTicks = 0;
			} else if (!wasLCDTurnedOn && lcd->lcdc.lcdEnabled) {
				// LCD is going to be turned on
				fgb__Breakpoint(system, fgbBreakpointType_LCDControlPower);

				fgb__PPUCompareLine(system);

				fgb__PPUStatusInterruptTest(system, false);

				ppu->state.lineTicks = 0;
				ppu->state.frameTicks = 0;
			}

		} break;
		case 0xFF41:
		{
			// Reset mode and LYC always when STAT was forcefully changed
			fgbPPUMode oldMode = lcd->stat.lcdMode;
			lcd->stat.u8 = value & 0b11111000;
			fgbPPUMode newMode = lcd->stat.lcdMode;

			if (oldMode != newMode) {
				fgb__Breakpoint(system, fgbBreakpointType_LCDControlMode);
			}

			if (lcd->lcdc.lcdEnabled) {
				fgb__PPUStatusInterruptTest(system, false);
			}

		} break;
		case 0xFF42:
			lcd->scy = value;
			break;
		case 0xFF43:
			lcd->scx = value;
			break;
		case 0xFF44:

			// NOTE(final): Changing ly is not allowed, so it will be reset to zero - which may break things
			fgb__PPUResetLine(system);

			break;
		case 0xFF45:

			lcd->lyc = value;

			if (lcd->lcdc.lcdEnabled) {
				fgb__PPUCompareLine(system);
			}

			break;
		case 0xFF46:
			lcd->dma = value;

			fgb__DMAStart(system, value);
			break;
		case 0xFF47:
			lcd->bgp.u8 = value;

			fgb__PPUPaletteSet(&ppu->currentMonochromeColors, &system->systemMonochromeColors, 0, value);
			break;
		case 0xFF48:
			lcd->obp0.u8 = value;

			fgb__PPUPaletteSet(&ppu->currentMonochromeColors, &system->systemMonochromeColors, 1, value & 0b11111100);
			break;
		case 0xFF49:
			lcd->obp1.u8 = value;
			fgb__PPUPaletteSet(&ppu->currentMonochromeColors, &system->systemMonochromeColors, 2, value & 0b11111100);
			break;
		case 0xFF4A:
			lcd->wy = value;
			break;
		case 0xFF4B:
			lcd->wx = value;
			break;
		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported LCD write '$%02X' to address '%04X'", value, address);
			break;
	}
}

static uint8_t fgb__PPURead(fgbSystem *system, const uint16_t address) {
	fgbPPU *ppu = &system->ppu;
	fgbIORegisters *io = &system->io;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (address >= FGB__BUS_ADDRESS_PPU_TILES_FROM && address <= FGB__BUS_ADDRESS_PPU_TILES_TO) {
#if FGB_PPU_VRAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUVRAMAllowedRead(ppu)) {
			FGB__WARN(system, fgb__KindName_PPU, "VRAM read access violation on address '$%04X' during Pixel Transfer Mode", address);
		}
#endif
		return fgb__PPUTilesRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_DATA_FROM && address <= FGB__BUS_ADDRESS_PPU_DATA_TO) {
#if FGB_PPU_VRAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUVRAMAllowedRead(ppu)) {
			FGB__WARN(system, fgb__KindName_PPU, "VRAM read access violation on address '$%04X' during Pixel Transfer Mode", address);
		}
#endif
		return fgb__PPUDataRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_OAM_FROM && address <= FGB__BUS_ADDRESS_PPU_OAM_TO) {
#if FGB_OAM_VRAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUOAMAllowedRead(ppu)) {
			if (fgb__IsDMAActive(&ppu->dma))
				FGB__WARN(system, fgb__KindName_PPU, "OAM read access violation on address '$%04X' DMA Transfer Mode", address);
			else
				FGB__WARN(system, fgb__KindName_PPU, "OAM read access violation on address '$%04X' during OAM Mode", address);
		}
#endif
		return fgb__PPUOAMRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_FROM && address <= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_TO) {
		return fgb__PPULCDRegisterRead(system, address);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported read from address '%04X'", address);
		return 0xFF;
	}
}

static void fgb__PPUWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (address >= FGB__BUS_ADDRESS_PPU_TILES_FROM && address <= FGB__BUS_ADDRESS_PPU_TILES_TO) {
#if FGB_PPU_VRAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUVRAMAllowedWrite(ppu)) {
			FGB__WARN(system, fgb__KindName_PPU, "VRAM write access violation on address '$%04X' during Pixel Transfer Mode", address);
		}
#endif
		fgb_PPUTilesWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PPU_DATA_FROM && address <= FGB__BUS_ADDRESS_PPU_DATA_TO) {
#if FGB_PPU_VRAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUVRAMAllowedWrite(ppu)) {
			FGB__WARN(system, fgb__KindName_PPU, "VRAM write access violation on address '$%04X' during Pixel Transfer Mode", address);
		}
#endif
		fgb__PPUDataWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PPU_OAM_FROM && address <= FGB__BUS_ADDRESS_PPU_OAM_TO) {
#if FGB_PPU_OAM_ACCESS_WARNING_LOGGING
		if (!fgb__IsPPUOAMAllowedWrite(ppu)) {
			if (fgb__IsDMAActive(&ppu->dma))
				FGB__WARN(system, fgb__KindName_PPU, "OAM write access violation on address '$%04X' DMA Transfer Mode", address);
			else
				FGB__WARN(system, fgb__KindName_PPU, "OAM write access violation on address '$%04X' during OAM Mode", address);
		}
#endif
		fgb__PPUOAMWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_FROM && address <= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_TO) {
		fgb__PPULCDRegisterWrite(system, address, value);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Unsupported write '$%02X' to address '%04X'", value, address);
	}
}

//
// Pixel FIFO
//
static inline bool fgb__IsPPUFIFOFull(const fgbPixelFIFO *fifo) {
	return !(fifo->len == 0 || fifo->len == 8);
}

static void fgb__PPUFIFOClear(fgbPixelFIFO *fifo) {
	fifo->in = fifo->out = fifo->len = 0;
}

static void fgb__PPUFIFOPush(fgbPixelFIFO *fifo, const fgbPixel pixel) {
	FGB_ASSERT(fifo->len < FGB_PIXEL_FIFO_LENGTH);
	fifo->pixels[fifo->in] = pixel;
	fifo->in = (fifo->in + 1) % FGB_PIXEL_FIFO_LENGTH;
	fifo->len++;
}

static bool fgb__PPUFIFOPop(fgbPixelFIFO *fifo, fgbPixel *outPixel) {
	if (fifo->len == 0) {
		return false;
	}
	*outPixel = fifo->pixels[fifo->out];
	fifo->out = (fifo->out + 1) % FGB_PIXEL_FIFO_LENGTH;
	fifo->len--;
	return true;
}

//
// Entire PPU Pipeline
// 
// Mode 0: HBlank
// Mode 1: VBlank
// Mode 2: OAM Search
// Mode 3: Pixel Transfer -> Uses FIFO to push one pixel per tick and fetches 8 pixels if possible into the FIFO
//
static void fgb__PPUPipelineReset(fgbPPUPipeline *pipeline) {
	pipeline->state.lineX = 0;
	pipeline->state.pushX = 0;
	pipeline->state.fifoX = 0;
	pipeline->fetch.currentX = 0;
	pipeline->fetch.state = fgbPPUFetchState_Tile;

	fgb__PPUFIFOClear(&pipeline->fifo);
}

// Window Range: WX=0..166, WY=0..143
// TopLeft: WX=7, WY=0
static bool fgb__PPUIsWindowVisible(const fgbLCDRegister *lcd) {
	if (!lcd->lcdc.windowEnable)
		return false;
	bool result = (lcd->wx >= 0 && lcd->wx <= (FGB_DISPLAY_WIDTH + 6)) && (lcd->wy >= 0 && lcd->wy < FGB_DISPLAY_HEIGHT);
	return result;
}

static void fgb__PPUIncrementLine(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (fgb__PPUIsWindowVisible(lcd)) {
		if (lcd->ly >= lcd->wy && lcd->ly < lcd->wy + FGB_DISPLAY_HEIGHT) {
			++ppu->state.windowLine;
		}
	}

	++lcd->ly;

	fgb__PPUCompareLine(system);
}

static void fgb__PPUPipelineFetchOAMEntries(fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	// Entries are sorted from smallest X to largest X
	fgbLineSpriteEntry *item = pipeline->sprites.first;
	while (item != NULL) {
		int x = (item->entry.x - 8) + (lcd->scx % 8);

		if ((x >= pipeline->fetch.currentX && x < pipeline->fetch.currentX + 8) ||
			((x + 8) >= pipeline->fetch.currentX && (x + 8) < pipeline->fetch.currentX + 8)) {
			pipeline->fetch.entries[pipeline->fetch.entryCount++] = item->entry;
		}

		item = item->next;

		// Either we are the end of the OAM linked list or we already got 3 sprite entries
		if (item == NULL || pipeline->fetch.entryCount >= 3) {
			break;
		}
	}
}

static void fgb__PPUPipelineLoadWindowTileID(fgbSystem *system, fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	uint16_t windowDataStartAddress = fgbGetPPUWindowDataArea(lcd);
	uint16_t windowDataOffset = (pipeline->tilePos.y * FGB_TILEDATA_HORIZONTAL_COUNT) + pipeline->tilePos.x;
	uint16_t windowDataAddress = windowDataStartAddress + windowDataOffset;
	uint16_t windowDataIndex = windowDataAddress - 0x9800;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	uint8_t tileId = bank->data.m[windowDataIndex];

	uint16_t tilesDataArea = fgbGetPPUBackgroundWindowTilesArea(lcd);

	pipeline->fetch.tileID = tilesDataArea == 0x8800 ? 128 + (int8_t)tileId : tileId;
}

static void fgb__PPUPipelineLoadBackgroundTileID(fgbSystem *system, fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	uint16_t backgroundDataStartAddress = fgbGetPPUBackgroundDataArea(lcd);
	uint16_t backgroundDataOffset = (pipeline->tilePos.y * FGB_TILEDATA_HORIZONTAL_COUNT) + pipeline->tilePos.x;
	uint16_t backgroundDataAddress = backgroundDataStartAddress + backgroundDataOffset;
	uint16_t backgroundDataIndex = backgroundDataAddress - 0x9800;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	uint8_t tileId = bank->data.m[backgroundDataIndex];

	uint16_t tilesDataArea = fgbGetPPUBackgroundWindowTilesArea(lcd);

	pipeline->fetch.tileID = tilesDataArea == 0x8800 ? 128 + (int8_t)tileId : tileId;
}

static fgbPixel fgb__PPUFetchSpritePixel(fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline, const uint8_t xPos) {
	for (uint8_t entryIndex = 0; entryIndex < pipeline->fetch.entryCount; ++entryIndex) {
		const fgbOAMEntry *entry = pipeline->fetch.entries + entryIndex;

		int x = (entry->x - 8) + (lcd->scx % 8);

		if (x + 8 < xPos) {
			// FIFO was faster than we where, so discard that pixel
			continue;
		}

		int offset = xPos - x;

		if (offset < 0 || offset > 7) {
			// We are outside to the left or to the right
			continue;
		}

		uint8_t bit = 7 - offset;

		if (entry->horizontalFlip) {
			bit = offset;
		}

		uint8_t low = pipeline->fetch.entryTileData[(entryIndex * 2) + 0];
		uint8_t high = pipeline->fetch.entryTileData[(entryIndex * 2) + 1];

		uint8_t colorIndex = fgb__PPUDecodeColorIndex(low, high, bit);

		if (colorIndex == 0 || !ppu->state.isSpritesEnabled) {
			// This sprite is transparent, try the next one
			continue;
		}

		fgbPixel pixel = {
			.spritePriority = !entry->backgroundPriority,
			.type = entry->paletteNumber ? fgbPixelType_Sprite1 : fgbPixelType_Sprite0,
			.value = colorIndex,
			.color = entry->paletteNumber ? ppu->currentMonochromeColors.sprite1[colorIndex] : ppu->currentMonochromeColors.sprite0[colorIndex]
		};

		return pixel;
	}

	fgbPixel empty = { 0 };
	return empty;
}

static void fgb__PPUPipelineLoadSpriteData(fgbSystem *system, fgbPPU *ppu, const uint8_t tileLineIndex) {
	fgbPPUPipeline *pipeline = &ppu->pipeline;
	fgbLCDRegister *lcd = &ppu->lcd;

	uint8_t spriteHeight = lcd->lcdc.objSize ? 16 : 8;

	uint8_t y = lcd->ly;

	const fgbVRAMBank *bank = &ppu->vram.bank0;

	for (uint8_t entryIndex = 0; entryIndex < pipeline->fetch.entryCount; ++entryIndex) {
		const fgbOAMEntry *entry = pipeline->fetch.entries + entryIndex;

		// NOTE(final): Remember we have to add 16 to the Y-position, because sprites at 0x10 (16) at the very top
		uint8_t tileY = ((y + 16) - entry->y) * 2;

		if (entry->verticalFlip) {
			// Flipped vertically
			tileY = ((spriteHeight * 2) - 2) - tileY; // Why -2?
		}

		uint8_t tileID = entry->tileID;

		if (spriteHeight == 16) {
			tileID &= ~(1); // Remove last bit
		}
	 
		uint16_t index = (tileID * FGB_TILE_SIZE) + tileY + tileLineIndex;

		pipeline->fetch.entryTileData[(entryIndex * 2) + tileLineIndex] = bank->tiles.m[index];
	}
}

static fgbPixel fgb__PPUPixelMix(fgbPixel background, fgbPixel sprite) {
	fgbPixel result = { 0 };
	if (sprite.value == 0) {
		// Sprite pixel is transparent
		result = background;
	} else {
		if (!sprite.spritePriority && background.value != 0) {
			// Background has higher priority
			result = background;
		} else {
			// Sprite has higher priority or sprite wins by value
			result = sprite;
		}
	}
	return result;
}

static void fgb__PPUPipelineFetch(fgbSystem *system, fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	fgbPPUFetchRegister *fetch = &pipeline->fetch;
	uint32_t totalTileCount = FGB_TOTAL_TILE_COUNT;
	uint8_t tileSize = FGB_TILE_SIZE;
	switch (pipeline->fetch.state) {
		case fgbPPUFetchState_Tile:
		{
			fetch->entryCount = 0;

			uint8_t tilePosX = (fetch->currentX + lcd->scx) / 8;
			uint8_t tilePosY = (lcd->ly + lcd->scy) / 8;
			ppu->pipeline.tilePos.x = tilePosX % FGB_TILEDATA_HORIZONTAL_COUNT;
			ppu->pipeline.tilePos.y = tilePosY % FGB_TILEDATA_VERTICAL_COUNT;
			ppu->pipeline.tileType = fgbPixelType_None;
			ppu->pipeline.state.offsetY = (lcd->ly + lcd->scy) % 8;

			// Background enabled?
			if (lcd->lcdc.backgroundEnabled) {
				ppu->pipeline.tileType = fgbPixelType_Background;

				// Load background tile ID
				fgb__PPUPipelineLoadBackgroundTileID(system, ppu, lcd, pipeline);

				// If window is enabled and it is inside the display for X position, overwrite the tile infos
				if (fgb__PPUIsWindowVisible(lcd)) {
					if (fetch->currentX + 7 >= lcd->wx && fetch->currentX + 7 < lcd->wx + FGB_DISPLAY_WIDTH + 14) {
						if (lcd->ly >= lcd->wy && lcd->ly < lcd->wy + FGB_DISPLAY_HEIGHT) {
							uint8_t windowTilePosX = ((fetch->currentX + 7 - lcd->wx) / 8);
							uint8_t windowTilePosY = ppu->state.windowLine / 8;

							ppu->pipeline.tilePos.x = windowTilePosX % FGB_TILEDATA_HORIZONTAL_COUNT;
							ppu->pipeline.tilePos.y = windowTilePosY % FGB_TILEDATA_VERTICAL_COUNT;
							ppu->pipeline.tileType = fgbPixelType_Window;
							ppu->pipeline.state.offsetY = ppu->state.windowLine % 8;

							// Load window tile ID
							fgb__PPUPipelineLoadWindowTileID(system, ppu, lcd, pipeline);
						}
					}
				}
			}

			// Sprites are enabled and we found sprites in the buffer
			if (lcd->lcdc.objEnable && pipeline->sprites.count > 0) {
				FGB_ASSERT(pipeline->sprites.first != NULL);
				fgb__PPUPipelineFetchOAMEntries(ppu, lcd, pipeline);
			}

			// Update background map tile info
			uint16_t tileInfosIndex = ppu->pipeline.tilePos.y * FGB_TILEDATA_HORIZONTAL_COUNT + ppu->pipeline.tilePos.x;
			fgbBackgroundMapTileInfo tileInfo = { 0 };
			tileInfo.type = ppu->pipeline.tileType;
			tileInfo.backgroundDataAreaSelect = lcd->lcdc.backgroundDataAreaSelect;
			tileInfo.backgroundWindowTilesAreaSelect = lcd->lcdc.backgroundWindowTilesAreaSelect;
			ppu->backgroundMap.tilesInfos[tileInfosIndex] = tileInfo;

			// We got the tile ID, so we can load the actual lines for the tiles for it consisting of two-bytes
			// starting with the first one
			fetch->state = fgbPPUFetchState_Data0;

			// From the tile we can reconstruct the X position, so we can increase the fetch X here
			fetch->currentX += 8;
		} break;

		case fgbPPUFetchState_Data0:
		case fgbPPUFetchState_Data1:
		{
			uint8_t tileLineIndex = fetch->state - fgbPPUFetchState_Data0;

			uint16_t tileMapStartAddress = fgbGetPPUBackgroundWindowTilesArea(lcd);
			uint16_t tileOffset = (pipeline->fetch.tileID * FGB_TILE_SIZE) + (pipeline->state.offsetY * 2);
			uint16_t tileAddress = tileMapStartAddress + tileOffset + tileLineIndex;
			uint16_t tilePosition = tileAddress - 0x8000;

			const fgbVRAMBank *bank = &ppu->vram.bank0;

			fetch->tileLine.m[tileLineIndex] = bank->tiles.m[tilePosition];

			fgb__PPUPipelineLoadSpriteData(system, ppu, tileLineIndex);

			if (fetch->state == fgbPPUFetchState_Data0) {
				// First byte of the line is done, go fetch the last byte on the next tick
				fetch->state = fgbPPUFetchState_Data1;
			} else {
				// Try to push it out directly, this will wait automatically if fifo is full
				fetch->state = fgbPPUFetchState_Push;
			}
		} break;

		case fgbPPUFetchState_Waiting:
		{
			// No space was left in the FIFO, so wait one tick before trying again
			fetch->state = fgbPPUFetchState_Push;
		} break;

		case fgbPPUFetchState_Push:
		{
			if (fgb__IsPPUFIFOFull(&pipeline->fifo)) {
				fetch->state = fgbPPUFetchState_Waiting;
				return;
			}

			fgbTileLine line = fetch->tileLine;

			for (uint8_t xOffset = 0; xOffset < FGB_TILE_WIDTH; ++xOffset) {

				uint8_t bit = 7 - xOffset;

				uint8_t colorIndex = fgb__PPUDecodeColorIndex(line.lower.value, line.upper.value, bit);

				fgbPixelType backgroundTileType = pipeline->tileType;

				if (backgroundTileType == fgbPixelType_Background && !ppu->state.isBackgroundEnabled) {
					colorIndex = 0;
				} else if (backgroundTileType == fgbPixelType_Window && !ppu->state.isWindowEnabled) {
					colorIndex = 0;
				}

				fgbPixel backgroundPixel = {
					.spritePriority = false,
					.type = backgroundTileType,
					.value = colorIndex,
					.color = ppu->currentMonochromeColors.background[colorIndex]
				};

				fgbPixel spritePixel = fgb__PPUFetchSpritePixel(ppu, lcd, pipeline, pipeline->state.fifoX);

				fgbPixel mixedPixel = fgb__PPUPixelMix(backgroundPixel, spritePixel);

				fgb__PPUFIFOPush(&pipeline->fifo, mixedPixel);

				fgb__Breakpoint(system, fgbBreakpointType_PPUFIFOPush);

				pipeline->state.fifoX++;
			}

			fetch->state = fgbPPUFetchState_Tile;
		} break;

		default:
			break;
	}
}

static void fgb__PPUPipelinePushPixel(fgbSystem *system, fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	if (!fgb__IsPPUFIFOFull(&pipeline->fifo)) {
		return;
	}

	fgbPixel pixel;
	if (!fgb__PPUFIFOPop(&pipeline->fifo, &pixel)) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_PPU, "Failed to pop pixel from FIFO!");
	}
	fgb__Breakpoint(system, fgbBreakpointType_PPUFIFOPop);

	// Is is pixel inside the screen? If so render the actual pixel out
	if ((pipeline->state.lineX >= lcd->scx % 8) || (pixel.type == fgbPixelType_Window)) {
		fgbColor color = { 0 };
		switch (pixel.type) {
			case fgbPixelType_Sprite0:
				color = ppu->currentMonochromeColors.sprite0[pixel.value];
				break;
			case fgbPixelType_Sprite1:
				color = ppu->currentMonochromeColors.sprite1[pixel.value];
				break;
			case fgbPixelType_Background:
			case fgbPixelType_Window:
				color = ppu->currentMonochromeColors.background[pixel.value];
				break;
			default:
				color = ppu->currentMonochromeColors.background[0];
				break;
		}

		ppu->display[(lcd->ly * FGB_DISPLAY_WIDTH) + pipeline->state.pushX] = color;
		fgb__Breakpoint(system, fgbBreakpointType_PPUDrawPixel);

		++pipeline->state.pushX;
	}

	++pipeline->state.lineX;
}

static void fgb__PPUPipelineTick(fgbSystem *system, fgbPPU *ppu, fgbLCDRegister *lcd, fgbPPUPipeline *pipeline) {
	if (!(ppu->state.lineTicks & 1)) {
		fgb__PPUPipelineFetch(system, ppu, lcd, pipeline);
	}

	fgb__PPUPipelinePushPixel(system, ppu, lcd, pipeline);
}

static void fgb__PPUInsertSpriteEntry(fgbPPUPipeline *pipeline, fgbLineSpriteEntry *toInsert) {
	fgbLineSpriteEntry *insReg = pipeline->sprites.first;
	fgbLineSpriteEntry *prev = NULL;
	while (insReg != NULL) {
		if (insReg->entry.x > toInsert->entry.x) {
			FGB_ASSERT(prev != NULL);
			prev->next = toInsert;
			toInsert->next = insReg;
			break;
		}

		if (insReg->next == NULL) {
			insReg->next = toInsert;
			break;
		}

		prev = insReg;
		insReg = insReg->next;
	}
}

static void fgb__PPUSearchSprites(fgbSystem *system, fgbPPU *ppu) {
	fgbPPUPipeline *pipeline = &ppu->pipeline;
	fgbLCDRegister *lcd = &ppu->lcd;

	// Reset found sprites first
	fgbClearStruct(&pipeline->sprites);

	uint8_t y = lcd->ly;

	uint8_t spriteHeight = lcd->lcdc.objSize ? 16 : 8;

	for (uint8_t oamEntryIndex = 0; oamEntryIndex < FGB_MAX_OAM_ENTRY_COUNT; ++oamEntryIndex) {
		const fgbOAMEntry *oamEntry = ppu->oam.entries + oamEntryIndex;

		if (!oamEntry->x) {
			// Not visible, because 0 is the same as -8
			continue;
		}

		if (pipeline->sprites.count >= FGB__PPU_MAX_SPRITES_PER_LINE) {
			// Only 10 sprites per line are allowed
			break;
		}

		int16_t entryY = oamEntry->y - 16;
		if (entryY <= y && entryY + spriteHeight > y) {

			fgbLineSpriteEntry *newEntry = &pipeline->sprites.buffer[pipeline->sprites.count++];
			newEntry->entry = *oamEntry;
			newEntry->next = NULL;

			// We are the very first sprite or
			// We are smaller than the last sprite, so we can insert outself to the very end
			if (pipeline->sprites.first == NULL || oamEntry->x < pipeline->sprites.first->entry.x) {
				newEntry->next = pipeline->sprites.first;
				pipeline->sprites.first = newEntry;
			} else {
				// Insert the sprite entry in the chain by sorting it in
				FGB_ASSERT(pipeline->sprites.first != NULL);
				fgb__PPUInsertSpriteEntry(pipeline, newEntry);
			}
		}
	}
}

static void fgb__PPUModeOAMSearch(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbPPUPipeline *pipeline = &ppu->pipeline;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (ppu->state.lineTicks >= FGB__PPU_OAM_SCAN_DOTS) {
		fgb__PPUSetMode(system, lcd, fgbPPUMode_PixelTransfer);

		// OAM search is done, reset pipeline so we can start fetching and pushing pixels
		fgb__PPUPipelineReset(pipeline);

		ppu->state.lineTicks = 0;
	}

	if (ppu->state.lineTicks == 1) {
		fgb__PPUSearchSprites(system, ppu);
	}
}

static void fgb__PPUModePixelTransfer(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;
	fgbPPUPipeline *pipeline = &ppu->pipeline;

	fgb__PPUPipelineTick(system, ppu, lcd, pipeline);

	if (ppu->pipeline.state.pushX >= FGB_DISPLAY_WIDTH) {
		ppu->state.lineTicks = 0;

		fgb__PPUFIFOClear(&pipeline->fifo);

		fgb__PPUSetMode(system, lcd, fgbPPUMode_HBlank);

		fgb__PPUStatusInterruptTest(system, false);
	}
}

// Depending on the scroll X position, we may have different ticks for the horizontal blank mode, so this function computes this
static uint32_t fgb__PPUHorizontalBlankTicks(const fgbLCDRegister *lcd) {
	switch (lcd->scx & 7) {
		case 0:
			return 204;
		case 1:
		case 2:
		case 3:
		case 4:
			return 200;
		case 5:
		case 6:
		case 7:
			return 196;
		default:
			return 0;
	}
}

static void fgb__PPUModeHorizontalBlank(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

#if 0
	const uint32_t blankTicks = FGB__PPU_HORIZONTAL_BLANK_DOTS;
#else
	const uint32_t blankTicks = fgb__PPUHorizontalBlankTicks(lcd);
#endif

	if (ppu->state.lineTicks >= blankTicks) {
		fgb__PPUIncrementLine(system);

		if (lcd->ly >= FGB_DISPLAY_HEIGHT) {
			fgb__PPUSetMode(system, lcd, fgbPPUMode_VBlank);

			// Only fire the VBlank interrupt, when the LCD is actually enabled
			if (lcd->lcdc.lcdEnabled) {
				fgb__InterruptRequest(system, fgbInterruptType_VerticalBlank, "VBlank");
			}

			
		} else {
			fgb__PPUSetMode(system, lcd, fgbPPUMode_OAMSearch);
		}

		ppu->state.lineTicks = 0;
	}
}

static void fgb__PPUModeVerticalBlank(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	if (ppu->state.lineTicks >= FGB__PPU_HORIZONTAL_BLANK_DOTS) {
		fgb__PPUIncrementLine(system);

		if (lcd->ly >= FGB__PPU_VERTICAL_BLANK_LINE_COUNT) {
			fgb__PPUSetMode(system, lcd, fgbPPUMode_OAMSearch);

			fgb__PPUResetLine(system);

			fgb__PPUStatusInterruptTest(system, false);

			// Update tiles and background map
			fgb__PPUVideoRAMFullUpdate(&system->ppu);
			fgb__PPUBackgroundMapUpdate(&system->ppu);

			// Our frame is now finished
			ppu->state.frameCount++;
			ppu->state.isFrameFinished = true;
			ppu->state.frameTicks = 0;

			fgb__Breakpoint(system, fgbBreakpointType_PPUFrameEnd);
		}

		ppu->state.lineTicks = 0;
	}
}

static bool fgb__PPUTick(fgbSystem *system) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;
	fgbPPUPipeline *pipeline = &ppu->pipeline;

	// The LCD is turned off, do not execute any PPU operations - until the LCD is turned on by writing to FF40
	if (!lcd->lcdc.lcdEnabled) {
		ppu->state.lcdOffTicks++;

		// Notify that we want to refresh the frame every 1/60 of a second, so rendering doesn't get stuck
		if (ppu->state.lcdOffTicks >= FGB__PPU_FRAME_DOTS) {
			ppu->state.lcdOffTicks -= FGB__PPU_FRAME_DOTS;

			fgb__PPUDisplayClear(ppu, ppu->currentMonochromeColors.system[1]);

			ppu->state.frameCount++;
			ppu->state.isFrameFinished = true;
		}

		return true;
	}

	// The PPU does nothing until this flag is reset
	if (ppu->state.isFrameFinished) {
		return true;
	}

#if FGB_PPU_TICK_LOGGING
	FGB__DEBUG(system, fgb__KindName_PPU,
		"[%u] "
		"PPU F/L Ticks: %u/%u, Mode: %u, LY: %u, LYC: %u, "
		"F/P/L/FI/O X: %u/%u/%u/%u/%u, "
		"TP: %u x %u, T-ID: %u, "
		"Fetch: %u, LCDC: %02X, STAT: %02X, BGP: %02X",
		ppu->state.frameCount,
		ppu->state.frameTicks,
		ppu->state.lineTicks,
		lcd->stat.lcdMode,
		lcd->ly,
		lcd->lyc,
		pipeline->state.fetchX,
		pipeline->state.pushX,
		pipeline->state.lineX,
		pipeline->state.fifoX,
		pipeline->offsetY,
		pipeline->tilePos.x,
		pipeline->tilePos.y,
		pipeline->fetchedTileID,
		pipeline->state,
		lcd->lcdc.u8,
		lcd->stat.u8,
		lcd->bgp.u8);
#endif

	// Raise a breakpoint when a frame begins (Everything is reset to zero and the mode is OAM Search)
	if (ppu->lcd.ly == 0 && ppu->state.windowLine == 0 && 
		ppu->pipeline.fetch.currentX == 0 && ppu->pipeline.state.lineX == 0 && ppu->pipeline.state.pushX == 0 &&
		ppu->state.lineTicks == 0 && 
		lcd->stat.lcdMode == fgbPPUMode_OAMSearch) {
		fgb__Breakpoint(system, fgbBreakpointType_PPUFrameBegin);
	}

	++ppu->state.frameTicks;

	++ppu->state.lineTicks;

	switch (lcd->stat.lcdMode) {

		case fgbPPUMode_OAMSearch:
			fgb__PPUModeOAMSearch(system);
			break;

		case fgbPPUMode_PixelTransfer:
			fgb__PPUModePixelTransfer(system);
			break;

		case fgbPPUMode_HBlank:
			fgb__PPUModeHorizontalBlank(system);
			break;

		case fgbPPUMode_VBlank:
			fgb__PPUModeVerticalBlank(system);
			break;

		default:
			break;
	}

	return true;
}

static void fgb__PPUInit(fgbSystem *system, const bool isDisplayEnabled) {
	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;

	// Init Window X/Y
	lcd->wy = 0;
	lcd->wx = 0;

	// Init Scroll X/Y
	lcd->scx = 0;
	lcd->scy = 0;

	// Init Line
	lcd->ly = 0;
	lcd->lyc = 0;
	ppu->state.windowLine = 0;

	// Init control and status register
	lcd->lcdc.u8 = 0b10010001;
	lcd->bgp.u8 = 0xFC;
	lcd->obp0.u8 = 0xFF;
	lcd->obp1.u8 = 0xFF;

	lcd->stat.u8 = 0;

	lcd->stat.lcdMode = fgbPPUMode_OAMSearch;

	// Init ticks and frame counter
	ppu->state.lineTicks = 0;
	ppu->state.lcdOffTicks = 0;
	ppu->state.frameCount = 0;

	// Init Pipeline
	fgb__PPUPipelineReset(&ppu->pipeline);

	ppu->state.isFrameFinished = false;
	ppu->state.isVRAMUpdated = false;
	ppu->state.isBackgroundMapUpdated = false;
	ppu->state.isDisplayEnabled = isDisplayEnabled;

	ppu->state.hasLCDStatusInterruptRequested = false;

	ppu->state.isBackgroundEnabled = true;
	ppu->state.isWindowEnabled = true;
	ppu->state.isSpritesEnabled = true;

	fgb__PPUPaletteInit(&ppu->currentMonochromeColors, &system->systemMonochromeColors);

	fgb__PPUDisplayClear(ppu, ppu->currentMonochromeColors.system[0]);
}

// ********************************************************************************************************************
// Timer Implementation
// ********************************************************************************************************************
static int fgb__globalTimerClockToShiftTable[] = {
	[fgbTimerClockType_00_1024] = 9,
	[fgbTimerClockType_01_16] = 3,
	[fgbTimerClockType_10_64] = 5,
	[fgbTimerClockType_11_256] = 7,
};

static void fgb__TimerTickTIMA(fgbSystem *system, const uint16_t lastDiv, const uint16_t newDiv) {
	fgbTimer *timer = &system->timer;
	fgbTimerRegister *tr = &timer->reg;
	if (tr->tac.isEnabled) {
		fgbTimerClockType clockType = (fgbTimerClockType)tr->tac.clock;
		int shift = fgb__globalTimerClockToShiftTable[clockType];
		bool shouldUpdate = ((lastDiv >> shift) & 0b1) && (!((newDiv >> shift) & 0b1));
		if (shouldUpdate) {
			tr->counter++;
			if (tr->counter == 0) {
				timer->reloadCycles = 0;
				timer->isReloading = true;
			}
		}
	}
}

static uint8_t fgb__TimerRead(const fgbTimer *timer, const uint16_t address) {
	switch (address) {
		case 0xFF04:
			return (timer->divider >> 8) & 0xFF;
		case 0xFF05:
			return timer->reg.counter;
		case 0xFF06:
			return timer->reg.modulo;
		case 0xFF07:
			return timer->reg.tac.u8 & 0b11111000;
		default:
			return 0;
	}
}

static void fgb__TimerTACChanged(fgbTimer *timer, const fgbTimerControlRegister newTac) {
	if (!newTac.isEnabled) {
		return;
	}
	uint8_t oldClock = timer->reg.tac.clock;
	uint8_t newClock = newTac.clock;

	int oldShift = fgb__globalTimerClockToShiftTable[oldClock];
	int newShift = fgb__globalTimerClockToShiftTable[newClock];

	if (!((timer->divider >> oldShift) & 0b1) && ((timer->divider >> newShift) & 0b1)) {
		timer->reg.counter++;
		if (timer->reg.counter == 0) {
			timer->reloadCycles = 0;
			timer->isReloading = true;
		}
	}
}

static void fgb__TimerWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbTimer *timer = &system->timer;
	switch (address) {
		case 0xFF04:
			fgb__TimerTickTIMA(system, timer->divider, 0);
			timer->divider = 0;
			break;
		case 0xFF05:
			// Only writeable when TMA is *NOT* written to TIMA
			if (!timer->interruptCycles) {
				timer->reg.counter = value;
				timer->isReloading = false;
				timer->reloadCycles = 0;
			}
			break;
		case 0xFF06:
			timer->reg.modulo = value;

			// Update TIMA, when an interrupt is raised while writing TMA
			if (timer->interruptCycles) {
				timer->reg.counter = timer->reg.modulo;
			}
			break;
		case 0xFF07: {
			fgbTimerControlRegister oldTac = timer->reg.tac;
			fgbTimerControlRegister newTac = { .u8 = value };

			if (oldTac.isEnabled != newTac.isEnabled) {
				fgb__TimerTickTIMA(system, timer->divider, 0);
				if (timer->isReloading && timer->reloadCycles == 0) {
					timer->reloadCycles = 0;
					timer->isReloading = false;
					timer->reg.counter = timer->reg.modulo;
					fgb__InterruptRequest(system, fgbInterruptType_Timer, "TAC Changed & Overflow");
				}
			}

			fgb__TimerTACChanged(timer, newTac);

			timer->reg.tac.u8 = value;
		} break;

		default:
			break;
	}
}

static bool fgb__TimerTick(fgbSystem *system) {
	fgbTimer *timer = &system->timer;
	if (timer->interruptCycles > 0) {
		timer->interruptCycles++;
		if (timer->interruptCycles == 4) {
			timer->interruptCycles = 0;
		}
	}
	if (timer->isReloading) {
		timer->reloadCycles++;
		if (timer->reloadCycles == 4) {
			timer->reloadCycles = 0;
			timer->interruptCycles = 1;
			timer->isReloading = false;
			timer->reg.counter = timer->reg.modulo;
			fgb__InterruptRequest(system, fgbInterruptType_Timer, "Timer Overflow");
		}
	}

	uint16_t lastDivider = timer->divider;
	timer->divider++;
	fgb__TimerTickTIMA(system, lastDivider, timer->divider);

	return true;
}

static void fgb__TimerInit(fgbTimer *timer) {
	fgbClearStruct(&timer->reg);
	timer->divider = 0xABCC;
	timer->reg.divider = timer->divider >> 8;
}

// ********************************************************************************************************************
// BUS Implementation
// ********************************************************************************************************************
static uint8_t fgb__WorkRamRead(fgbSystem *system, const uint16_t address) {
	if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK0_TO) {
		return system->ram.work.bank0.m[address - FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM];
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_TO) {
		const fgbWorkRAMBank *bank = &system->ram.work.bank1_to_N[0];
		return bank->m[address - FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM];
	} else if (address >= FGB__BUS_ADDRESS_SHADOW_RAM_FROM && address <= FGB__BUS_ADDRESS_SHADOW_RAM_TO) {
		uint16_t newAddress = address - 0x2000; // Redirect to work ram
		return fgb__WorkRamRead(system, newAddress);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_RAM, "Unsupported read from address '%04X'", address);
		return 0;
	}
}

static void fgb__WorkRamWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK0_TO) {
		system->ram.work.bank0.m[address - FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM] = value;
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_TO) {
		fgbWorkRAMBank *bank = &system->ram.work.bank1_to_N[0];
		bank->m[address - FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM] = value;
	} else if (address >= FGB__BUS_ADDRESS_SHADOW_RAM_FROM && address <= FGB__BUS_ADDRESS_SHADOW_RAM_TO) {
		uint16_t newAddress = address - 0x2000; // Redirect to work ram
		fgb__WorkRamWrite(system, newAddress, value);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_RAM, "Unsupported write '$%02X' to address '%04X'", value, address);
	}
}

static uint8_t fgb__HighRamRead(fgbSystem *system, const uint16_t address) {
	return system->ram.high.m[address - FGB__BUS_ADDRESS_HIGH_RAM_FROM];
}

static void fgb__HighRamWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	system->ram.high.m[address - FGB__BUS_ADDRESS_HIGH_RAM_FROM] = value;
}

//
// Joypad
//
#define FGB__JOYPAD_BUTTON_PRESSED false
#define FGB__JOYPAD_BUTTON_RELEASED true
#define FGB__JOYPAD_BUTTON_SELECTION false
#define FGB__JOYPAD_DIRECTION_SELECTION false

#define FGB__JOYPAD_BUTTON_A_OR_RIGHT_MASK (1 << 0)
#define FGB__JOYPAD_BUTTON_B_OR_LEFT_MASK (1 << 1)
#define FGB__JOYPAD_BUTTON_SELECT_OR_UP_MASK (1 << 2)
#define FGB__JOYPAD_BUTTON_START_OR_DOWN_MASK (1 << 3)

static inline bool fgb__IsControllerButtonDown(const fgbButtonState state) {
	bool result = state >= fgbButtonState_Pressed;
	return result;
}

static fgbJoypadRegister fgb__JoypadReadRegister(const fgbJoypadState *joypad) {
	const fgbJoypadRegister oldReg = joypad->reg;

	const fgbControllerState *controller = &joypad->currentState;

	fgbJoypadRegister newReg = {.u8 = 0xCF };
	newReg.buttonSelection = oldReg.buttonSelection;
	newReg.directionSelection = oldReg.directionSelection;
	newReg.unused = 0b11;

	bool buttonsSelected = oldReg.buttonSelection == FGB__JOYPAD_BUTTON_PRESSED;
	bool directionsSelected = oldReg.directionSelection == FGB__JOYPAD_BUTTON_PRESSED;
	bool bothAreSelected = buttonsSelected && directionsSelected;

	// Update buttons
	if (buttonsSelected) {
		bool isA = fgb__IsControllerButtonDown(controller->a);
		bool isB = fgb__IsControllerButtonDown(controller->b);
		bool isSelect = fgb__IsControllerButtonDown(controller->select);
		bool isStart = fgb__IsControllerButtonDown(controller->start);
		newReg.dpadRight_Or_ButtonA = isA ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadLeft_Or_ButtonB = isB ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadUp_Or_Select = isSelect ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadDown_Or_Start = isStart ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
	}

	// Update directions
	if (directionsSelected) {
		bool isRight = fgb__IsControllerButtonDown(controller->right);
		bool isLeft = fgb__IsControllerButtonDown(controller->left);
		bool isUp = fgb__IsControllerButtonDown(controller->up);
		bool isDown = fgb__IsControllerButtonDown(controller->down);
		newReg.dpadRight_Or_ButtonA = isRight ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadLeft_Or_ButtonB = isLeft ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadUp_Or_Select = isUp ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
		newReg.dpadDown_Or_Start = isDown ? FGB__JOYPAD_BUTTON_PRESSED : FGB__JOYPAD_BUTTON_RELEASED;
	}

	return newReg;
}

static inline bool fgb__IsJoypadButtonPressedDown(const fgbJoypadRegister last, const fgbJoypadRegister insReg, const uint8_t buttonMask) {
	bool wasSet = last.u8 & buttonMask;
	bool isSet = insReg.u8 & buttonMask;
	bool result = wasSet && !isSet;
	return result;
}

static void fgb__JoypadInterrupt(fgbSystem *system, fgbInterrupts *ir, fgbJoypadState *joypad) {
	fgbJoypadRegister last = joypad->lastButtonStates;

	fgbJoypadRegister insReg = fgb__JoypadReadRegister(joypad);

	bool rightOrADown = fgb__IsJoypadButtonPressedDown(last, insReg, FGB__JOYPAD_BUTTON_A_OR_RIGHT_MASK);
	bool leftOrBDown = fgb__IsJoypadButtonPressedDown(last, insReg, FGB__JOYPAD_BUTTON_B_OR_LEFT_MASK);
	bool upOrSelectDown = fgb__IsJoypadButtonPressedDown(last, insReg, FGB__JOYPAD_BUTTON_SELECT_OR_UP_MASK);
	bool downOrStartDown = fgb__IsJoypadButtonPressedDown(last, insReg, FGB__JOYPAD_BUTTON_START_OR_DOWN_MASK);

	bool buttonsSelected = insReg.buttonSelection == FGB__JOYPAD_BUTTON_PRESSED;
	bool directionsSelected = insReg.directionSelection == FGB__JOYPAD_BUTTON_PRESSED;

	bool anySelection = (buttonsSelected && !directionsSelected) || (!buttonsSelected && directionsSelected);

	// Only raise a request when buttons or directions are selected (but not both) and a button goes from "not pressed" to "pressed"
	bool canRaise = anySelection && (rightOrADown || leftOrBDown || upOrSelectDown || downOrStartDown);
	if (canRaise) {
		fgb__InterruptRequest(system, fgbInterruptType_Joypad, "Joypad");
	}

	joypad->lastButtonStates = insReg;
}

static void fgb__JoypadWriteRegister(fgbSystem *system, fgbJoypadState *joypad, const fgbJoypadRegister newReg) {
	fgbJoypadRegister *destReg = &joypad->reg;
	destReg->buttonSelection = newReg.buttonSelection;
	destReg->directionSelection = newReg.directionSelection;
	fgb__JoypadInterrupt(system, &system->interrupts, joypad);
}

FGB_API void fgbSetButtonState(fgbSystem *system, const fgbButtonType button, const bool isDown) {
	if (system == NULL || button >= 8) {
		return;
	}

	fgbJoypadState *joypad = &system->joypad;

	fgbControllerState *oldState = &joypad->currentState;
	fgbControllerState *requestedState = &joypad->requestedState;

	uint8_t index = (uint8_t)button;

	fgbButtonState oldButton = oldState->m[index];
	fgbButtonState newButton;
	if (isDown) {
		if (oldButton == fgbButtonState_Released) {
			newButton = fgbButtonState_Pressed;
		} else {
			newButton = fgbButtonState_Hold;
		}
	} else {
		newButton = fgbButtonState_Released;
	}
	requestedState->m[index] = newButton;

	joypad->isStateChanged = true;
}

FGB_API void fgbClearButtons(fgbSystem *system) {
	if (system == NULL) {
		return;
	}

	fgbJoypadState *joypad = &system->joypad;

	fgbClearStruct(&joypad->requestedState);

	joypad->isStateChanged = true;
}

//
// Serial
//
static uint8_t fgb__SerialRead(const fgbSerial *serial, const uint16_t address) {
	switch (address) {
		case 0xFF01:
			return serial->state.reg.sc;
		case 0xFF02:
			return serial->state.reg.sb;
		default:
			return 0;
	}
}

static void fgb__SerialWrite(fgbSerial *serial, const uint16_t address, const uint8_t value) {
	switch (address) {
		case 0xFF01:
			serial->state.reg.sc = value;
			break;
		case 0xFF02:
			serial->state.reg.sb = value;
			break;
		default:
			break;
	}
}

//
// IO
//
static void fgb__InterruptsWrite(fgbSystem *system, const uint16_t address, const uint8_t value);
static uint8_t fgb__InterruptsRead(fgbSystem *system, const uint16_t address);

static uint8_t fgb__IORead(fgbSystem *system, const uint16_t address) {
	if (address == FGB__BUS_ADDRESS_CONTROLLER_REGISTER) {
		fgbJoypadRegister reg = fgb__JoypadReadRegister(&system->joypad);
		return reg.u8;
	}

	if (address == FGB__BUS_ADDRESS_INTERRUPT_REQUEST_REGISTER || address == FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER) {
		return fgb__InterruptsRead(system, address);
	}

	if (address >= FGB__BUS_ADDRESS_TIMER_REGISTER_FROM && address <= FGB__BUS_ADDRESS_TIMER_REGISTER_TO) {
		return fgb__TimerRead(&system->timer, address);
	} else if (address >= FGB__BUS_ADDRESS_SOUND_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SOUND_REGISTER_TO) {
		return fgb__APURead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_FROM && address <= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_TO) {
		return fgb__PPURead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_SERIAL_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SERIAL_REGISTER_TO) {
		return fgb__SerialRead(&system->serial, address);
	}

	if (address >= FGB__BUS_ADDRESS_IO_REGISTERS_FROM && address <= FGB__BUS_ADDRESS_IO_REGISTERS_TO) {
		return system->io.m[address - FGB__BUS_ADDRESS_IO_REGISTERS_FROM];
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_IO, "Unsupported read from address '%04X'", address);
		return 0;
	}
}

static void fgb__IOWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	if (address == FGB__BUS_ADDRESS_CONTROLLER_REGISTER) {
		fgbJoypadRegister reg = { .u8 = value };
		fgb__JoypadWriteRegister(system, &system->joypad, reg);
		return;
	}

	if (address == FGB__BUS_ADDRESS_INTERRUPT_REQUEST_REGISTER || address == FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER) {
		fgb__InterruptsWrite(system, address, value);
		return;
	}

	if (address >= FGB__BUS_ADDRESS_TIMER_REGISTER_FROM && address <= FGB__BUS_ADDRESS_TIMER_REGISTER_TO) {
		fgb__TimerWrite(system, address, value);
		return;
	} else if (address >= FGB__BUS_ADDRESS_SOUND_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SOUND_REGISTER_TO) {
		fgb__APUWrite(system, address, value);
		return;
	} else if (address >= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_FROM && address <= FGB__BUS_ADDRESS_PPU_LCD_REGISTER_TO) {
		fgb__PPUWrite(system, address, value);
		return;
	} else if (address >= FGB__BUS_ADDRESS_SERIAL_REGISTER_FROM && address <= FGB__BUS_ADDRESS_SERIAL_REGISTER_TO) {
		fgb__SerialWrite(&system->serial, address, value);
		return;
	}

	if (address >= FGB__BUS_ADDRESS_IO_REGISTERS_FROM && address <= FGB__BUS_ADDRESS_IO_REGISTERS_TO) {
		system->io.m[address - FGB__BUS_ADDRESS_IO_REGISTERS_FROM] = value;
		return;
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_IO, "Unsupported write '$%02X' to address '$%04X'", value, address);
	}
}

static void fgb__GamePakWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	if (!system->gamePak.isValid || system->gamePak.rom.length == 0) {
		if (!system->boot.state.isActive)
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_GamePak, "Unsupported gamePak write of '$%02X' to address '$%04X' -> GamePak not loaded", value, address);
		return;
	}
	fgbMemoryBankController *mbc = &system->mbc;
	mbc->write((struct fgbSystem *)system, (struct fgbMemoryBankController *)mbc, address, value);
}

static uint8_t fgb__GamePakRead(fgbSystem *system, const uint16_t address) {
	if (!system->gamePak.isValid || system->gamePak.rom.length == 0) {
		if (!system->boot.state.isActive)
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_GamePak, "Unsupported gamePak read from address '$%04X' -> GamePak not loaded", address);
		return 0xFF;
	}
	fgbMemoryBankController *mbc = &system->mbc;
	return mbc->read((struct fgbSystem *)system, (struct fgbMemoryBankController *)mbc, address);
}

static uint8_t fgb__BootRead8(fgbSystem *system, const uint16_t address) {
	if (address == FGB__BUS_ADDRESS_BOOT_ROM_REGISTER) {
		return system->boot.state.reg;
	} else if (address < 0x100) {
		return system->boot.rom.data[address];
	}
	return 0;
}

static void fgb__BootWrite8(fgbSystem *system, const uint16_t address, const uint8_t value) {
	if (address == FGB__BUS_ADDRESS_BOOT_ROM_REGISTER) {
		system->boot.state.reg = value;
		system->boot.state.isActive = false;
	}
}

static inline uint8_t fgb__BusRead8_Direct(fgbSystem *system, const uint16_t address) {
	if (system->boot.state.isActive && address >= FGB__BUS_ADDRESS_BOOT_FROM && address <= FGB__BUS_ADDRESS_BOOT_TO) {
		return fgb__BootRead8(system, address);
	}
	if (address >= FGB__BUS_ADDRESS_ROM_FROM && address <= FGB__BUS_ADDRESS_ROM_TO) {
		return fgb__GamePakRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_VRAM_FROM && address <= FGB__BUS_ADDRESS_PPU_VRAM_TO) {
		return fgb__PPURead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_EXTERNAL_RAM_FROM && address <= FGB__BUS_ADDRESS_EXTERNAL_RAM_TO) {
		return fgb__GamePakRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK0_TO) {
		return fgb__WorkRamRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_TO) {
		return fgb__WorkRamRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_SHADOW_RAM_FROM && address <= FGB__BUS_ADDRESS_SHADOW_RAM_TO) {
		return fgb__WorkRamRead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PPU_OAM_FROM && address <= FGB__BUS_ADDRESS_PPU_OAM_TO) {
		return fgb__PPURead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_PROHIBITED_FROM && address <= FGB__BUS_ADDRESS_PROHIBITED_TO) {
		return 0; // Not allowed, don't warn here
	} else if (address >= FGB__BUS_ADDRESS_IO_REGISTERS_FROM && address <= FGB__BUS_ADDRESS_IO_REGISTERS_TO) {
		return fgb__IORead(system, address);
	} else if (address >= FGB__BUS_ADDRESS_HIGH_RAM_FROM && address <= FGB__BUS_ADDRESS_HIGH_RAM_TO) {
		return fgb__HighRamRead(system, address);
	} else if (address == FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER) {
		return fgb__IORead(system, address);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Bus, "Unsupported read from address '%04X'", address);
		return 0;
	}
}

static uint8_t fgb__BusRead8(fgbSystem *system, const uint16_t address, const bool tick) {
	uint8_t read = fgb__BusRead8_Direct(system, address);
#if FGB_BUS_LOGGING
	FGB__DEBUG(system, fgb__KindName_Bus, "[%2u] [%8zu] Read U8 '%02X' from address '%04X'", system->ppu.frameCount, system->cpu.state.totalTickCycles, read, address);
#endif
	if (tick) {
		fgb__HWTick4(system);
	}
	return read;
}

static inline void fgb__BusWrite8_Direct(fgbSystem *system, const uint16_t address, const uint8_t value) {
	if (address == FGB__BUS_ADDRESS_BOOT_ROM_REGISTER) {
		fgb__BootWrite8(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_ROM_FROM && address <= FGB__BUS_ADDRESS_ROM_TO) {
		fgb__GamePakWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PPU_VRAM_FROM && address <= FGB__BUS_ADDRESS_PPU_VRAM_TO) {
		fgb__PPUWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_EXTERNAL_RAM_FROM && address <= FGB__BUS_ADDRESS_EXTERNAL_RAM_TO) {
		fgb__GamePakWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK0_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK0_TO) {
		fgb__WorkRamWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_FROM && address <= FGB__BUS_ADDRESS_WORK_RAM_BANK1_OR_N_TO) {
		fgb__WorkRamWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_SHADOW_RAM_FROM && address <= FGB__BUS_ADDRESS_SHADOW_RAM_TO) {
		fgb__WorkRamWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PPU_OAM_FROM && address <= FGB__BUS_ADDRESS_PPU_OAM_TO) {
		fgb__PPUWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_PROHIBITED_FROM && address <= FGB__BUS_ADDRESS_PROHIBITED_TO) {
		return; // Not allowed, but don't warn here
	} else if (address >= FGB__BUS_ADDRESS_IO_REGISTERS_FROM && address <= FGB__BUS_ADDRESS_IO_REGISTERS_TO) {
		fgb__IOWrite(system, address, value);
	} else if (address >= FGB__BUS_ADDRESS_HIGH_RAM_FROM && address <= FGB__BUS_ADDRESS_HIGH_RAM_TO) {
		fgb__HighRamWrite(system, address, value);
	} else if (address == FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER) {
		fgb__IOWrite(system, address, value);
	} else {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Bus, "Unsupported U8 write to address '$%04X' with value '$%02X'", address, value);
	}
}

static void fgb__BusWrite8(fgbSystem *system, const uint16_t address, const uint8_t value, const bool tick) {
	fgb__BusWrite8_Direct(system, address, value);
#if FGB_BUS_LOGGING
	FGB__DEBUG(system, fgb__KindName_Bus, "[%2u] [%8zu] Written U8 '%02X' to address '%04X'", system->ppu.frameCount, system->cpu.state.totalTickCycles, value, address);
#endif
	if (tick) {
		fgb__HWTick4(system);
	}
}

static uint16_t fgb__BusRead16(fgbSystem *system, const uint16_t address, const bool tick) {
	uint8_t low = fgb__BusRead8(system, address + 0, tick);
	uint8_t high = fgb__BusRead8(system, address + 1, tick);

	fgbValue imm = { 0 };
	imm.ulow = low;
	imm.uhigh = high;

#if FGB_BUS_LOGGING
	FGB__DEBUG(system, fgb__KindName_Bus, "[%2u] [%8zu] Read U16 '%04X' from address '%04X'", system->ppu.frameCount, system->cpu.state.totalTickCycles, imm.u16, address);
#endif

	return imm.u16;
}

static void fgb__BusWrite16(fgbSystem *system, const uint16_t address, const uint16_t value, const bool tick) {
	fgbValue imm = { 0 };
	imm.u16 = value;

	fgb__BusWrite8(system, address + 0, imm.ulow, tick);
	fgb__BusWrite8(system, address + 1, imm.uhigh, tick);

#if FGB_BUS_LOGGING
	FGB__DEBUG(system, fgb__KindName_Bus, "[%2u] [%8zu] Written U16 '%04X' to address '%04X'", system->ppu.frameCount, system->cpu.state.totalTickCycles, value, address);
#endif
}

//
// Public API for bus read/write
//

FGB_API uint8_t fgbBusRead8(fgbSystem *system, const uint16_t address) {
	return fgb__BusRead8_Direct(system, address);
}

FGB_API void fgbBusWrite8(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgb__BusWrite8_Direct(system, address, value);
}



// ********************************************************************************************************************
// Tables Implementation
// ********************************************************************************************************************
static const char *fgb__instructionTypeNameTable[] = {
	[fgbInstructionType_None] = "None",
	[fgbInstructionType_NOP] = "NOP",
	[fgbInstructionType_ADC] = "ADC",
	[fgbInstructionType_ADD] = "ADD",
	[fgbInstructionType_AND] = "AND",
	[fgbInstructionType_BIT] = "BIT",
	[fgbInstructionType_CALL] = "CALL",
	[fgbInstructionType_CCF] = "CCF",
	[fgbInstructionType_CP] = "CP",
	[fgbInstructionType_CPL] = "CPL",
	[fgbInstructionType_DAA] = "DAA",
	[fgbInstructionType_DEC] = "DEC",
	[fgbInstructionType_DI] = "DI",
	[fgbInstructionType_EI] = "EI",
	[fgbInstructionType_HALT] = "HALT",
	[fgbInstructionType_INC] = "INC",
	[fgbInstructionType_JP] = "JP",
	[fgbInstructionType_JR] = "JR",
	[fgbInstructionType_LD] = "LD",
	[fgbInstructionType_OR] = "OR",
	[fgbInstructionType_POP] = "POP",
	[fgbInstructionType_PREFIX] = "PREFIX",
	[fgbInstructionType_PUSH] = "PUSH",
	[fgbInstructionType_RES] = "RES",
	[fgbInstructionType_RET] = "RET",
	[fgbInstructionType_RETI] = "RETI",
	[fgbInstructionType_RL] = "RL",
	[fgbInstructionType_RLA] = "RLA",
	[fgbInstructionType_RLC] = "RLC",
	[fgbInstructionType_RLCA] = "RLCA",
	[fgbInstructionType_RR] = "RR",
	[fgbInstructionType_RRA] = "RRA",
	[fgbInstructionType_RRC] = "RRC",
	[fgbInstructionType_RRCA] = "RRCA",
	[fgbInstructionType_RST] = "RST",
	[fgbInstructionType_SBC] = "SBC",
	[fgbInstructionType_SCF] = "SCF",
	[fgbInstructionType_SET] = "SET",
	[fgbInstructionType_SLA] = "SLA",
	[fgbInstructionType_SRA] = "SRA",
	[fgbInstructionType_SRL] = "SRL",
	[fgbInstructionType_STOP] = "STOP",
	[fgbInstructionType_SUB] = "SUB",
	[fgbInstructionType_SWAP] = "SWAP",
	[fgbInstructionType_XOR] = "XOR",
};

static const char *fgb__addressingModeNameTable[] = {
	[fgbAddressingMode_Implied] = "Implied",
	[fgbAddressingMode_Constant] = "Constant",
	[fgbAddressingMode_Constant_MemReg] = "Constant_MemReg",
	[fgbAddressingMode_Constant_Reg] = "Constant_Reg",
	[fgbAddressingMode_I8] = "I8",
	[fgbAddressingMode_MemA16_Reg] = "MemA16_Reg",
	[fgbAddressingMode_MemConstantOffsetA8_Reg] = "MemConstantOffsetA8_Reg",
	[fgbAddressingMode_MemConstantOffsetReg_Reg] = "MemConstantOffsetReg_Reg",
	[fgbAddressingMode_MemReg] = "MemReg",
	[fgbAddressingMode_MemReg_Reg] = "MemReg_Reg",
	[fgbAddressingMode_MemReg_U8] = "MemReg_U8",
	[fgbAddressingMode_MemRegDec_Reg] = "MemRegDec_Reg",
	[fgbAddressingMode_MemRegInc_Reg] = "MemRegInc_Reg",
	[fgbAddressingMode_Reg] = "Reg",
	[fgbAddressingMode_Reg_I8] = "Reg_I8",
	[fgbAddressingMode_Reg_MemA16] = "Reg_MemA16",
	[fgbAddressingMode_Reg_MemConstantOffsetA8] = "Reg_MemConstantOffsetA8",
	[fgbAddressingMode_Reg_MemConstantOffsetReg] = "Reg_MemConstantOffsetReg",
	[fgbAddressingMode_Reg_MemReg] = "Reg_MemReg",
	[fgbAddressingMode_Reg_MemRegDec] = "Reg_MemRegDec",
	[fgbAddressingMode_Reg_MemRegInc] = "Reg_MemRegInc",
	[fgbAddressingMode_Reg_Reg] = "Reg_Reg",
	[fgbAddressingMode_Reg_RegOffsetI8] = "Reg_RegOffsetI8",
	[fgbAddressingMode_Reg_U16] = "Reg_U16",
	[fgbAddressingMode_Reg_U8] = "Reg_U8",
	[fgbAddressingMode_U16] = "U16",
};

static const char *fgb__registerToNameTable[] = {
	[fgbRegisterType_A] = "A",
	[fgbRegisterType_F] = "F",
	[fgbRegisterType_B] = "B",
	[fgbRegisterType_C] = "C",
	[fgbRegisterType_D] = "D",
	[fgbRegisterType_E] = "E",
	[fgbRegisterType_H] = "H",
	[fgbRegisterType_L] = "L",
	[fgbRegisterType_AF] = "AF",
	[fgbRegisterType_BC] = "BC",
	[fgbRegisterType_DE] = "DE",
	[fgbRegisterType_HL] = "HL",
	[fgbRegisterType_SP] = "SP",
	[fgbRegisterType_PC] = "PC",
};

// Non-Prefix Instructions
static const fgbInstruction fgb__instructionTable[] = {
	[0x00] = {fgbInstructionType_NOP, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0x01] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U16, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_None, "----", 0x0000, 3, 3, 3, 16},
	[0x02] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_A, "----", 0x0000, 1, 2, 2, 8},
	[0x03] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x04] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x05] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x06] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x07] = {fgbInstructionType_RLCA, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "000C", 0x0000, 1, 1, 1, 8},
	[0x08] = {fgbInstructionType_LD, fgbAddressingMode_MemA16_Reg, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_None, "----", 0x0000, 3, 5, 5, 16},
	[0x09] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_BC, "-0HC", 0x0000, 1, 2, 2, 16},
	[0x0A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_BC, "----", 0x0000, 1, 2, 2, 8},
	[0x0B] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x0C] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x0D] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x0E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x0F] = {fgbInstructionType_RRCA, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "000C", 0x0000, 1, 1, 1, 8},
	[0x10] = {fgbInstructionType_STOP, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0x11] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U16, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_None, "----", 0x0000, 3, 3, 3, 16},
	[0x12] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_A, "----", 0x0000, 1, 2, 2, 8},
	[0x13] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x14] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x15] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x16] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x17] = {fgbInstructionType_RLA, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "000C", 0x0000, 1, 1, 1, 8},
	[0x18] = {fgbInstructionType_JR, fgbAddressingMode_I8, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 2, 3, 3, 0},
	[0x19] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_DE, "-0HC", 0x0000, 1, 2, 2, 16},
	[0x1A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_DE, "----", 0x0000, 1, 2, 2, 8},
	[0x1B] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x1C] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x1D] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x1E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x1F] = {fgbInstructionType_RRA, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "000C", 0x0000, 1, 1, 1, 8},
	[0x20] = {fgbInstructionType_JR, fgbAddressingMode_I8, fgbConditionType_NotZero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 2, 2, 3, 0},
	[0x21] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U16, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 3, 3, 3, 16},
	[0x22] = {fgbInstructionType_LD, fgbAddressingMode_MemRegInc_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_A, "----", 0x0000, 1, 2, 2, 8},
	[0x23] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x24] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x25] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x26] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x27] = {fgbInstructionType_DAA, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "Z-0C", 0x0000, 1, 1, 1, 8},
	[0x28] = {fgbInstructionType_JR, fgbAddressingMode_I8, fgbConditionType_Zero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 2, 2, 3, 0},
	[0x29] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_HL, "-0HC", 0x0000, 1, 2, 2, 16},
	[0x2A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemRegInc, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x2B] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x2C] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x2D] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x2E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x2F] = {fgbInstructionType_CPL, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "-11-", 0x0000, 1, 1, 1, 8},
	[0x30] = {fgbInstructionType_JR, fgbAddressingMode_I8, fgbConditionType_NotCarry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 2, 2, 3, 0},
	[0x31] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U16, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_None, "----", 0x0000, 3, 3, 3, 16},
	[0x32] = {fgbInstructionType_LD, fgbAddressingMode_MemRegDec_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_A, "----", 0x0000, 1, 2, 2, 8},
	[0x33] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x34] = {fgbInstructionType_INC, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z0H-", 0x0000, 1, 3, 3, 8},
	[0x35] = {fgbInstructionType_DEC, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z1H-", 0x0000, 1, 3, 3, 8},
	[0x36] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_U8, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 2, 3, 3, 8},
	[0x37] = {fgbInstructionType_SCF, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "-001", 0x0000, 1, 1, 1, 8},
	[0x38] = {fgbInstructionType_JR, fgbAddressingMode_I8, fgbConditionType_Carry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 2, 2, 3, 0},
	[0x39] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_SP, "-0HC", 0x0000, 1, 2, 2, 16},
	[0x3A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemRegDec, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x3B] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_None, "----", 0x0000, 1, 2, 2, 16},
	[0x3C] = {fgbInstructionType_INC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z0H-", 0x0000, 1, 1, 1, 8},
	[0x3D] = {fgbInstructionType_DEC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z1H-", 0x0000, 1, 1, 1, 8},
	[0x3E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x3F] = {fgbInstructionType_CCF, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "-00C", 0x0000, 1, 1, 1, 8},
	[0x40] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x41] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x42] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x43] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x44] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x45] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x46] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x47] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x48] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x49] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x4A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x4B] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x4C] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x4D] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x4E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x4F] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x50] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x51] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x52] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x53] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x54] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x55] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x56] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x57] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x58] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x59] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x5A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x5B] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x5C] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x5D] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x5E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x5F] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x60] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x61] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x62] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x63] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x64] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x65] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x66] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x67] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x68] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x69] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x6A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x6B] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x6C] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x6D] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x6E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x6F] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x70] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_B, "----", 0x0000, 1, 2, 2, 8},
	[0x71] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_C, "----", 0x0000, 1, 2, 2, 8},
	[0x72] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_D, "----", 0x0000, 1, 2, 2, 8},
	[0x73] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_E, "----", 0x0000, 1, 2, 2, 8},
	[0x74] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_H, "----", 0x0000, 1, 2, 2, 8},
	[0x75] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_L, "----", 0x0000, 1, 2, 2, 8},
	[0x76] = {fgbInstructionType_HALT, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0x77] = {fgbInstructionType_LD, fgbAddressingMode_MemReg_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_A, "----", 0x0000, 1, 2, 2, 8},
	[0x78] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "----", 0x0000, 1, 1, 1, 8},
	[0x79] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "----", 0x0000, 1, 1, 1, 8},
	[0x7A] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "----", 0x0000, 1, 1, 1, 8},
	[0x7B] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "----", 0x0000, 1, 1, 1, 8},
	[0x7C] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "----", 0x0000, 1, 1, 1, 8},
	[0x7D] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "----", 0x0000, 1, 1, 1, 8},
	[0x7E] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 8},
	[0x7F] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "----", 0x0000, 1, 1, 1, 8},
	[0x80] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x81] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x82] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x83] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x84] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x85] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x86] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z0HC", 0x0000, 1, 2, 2, 8},
	[0x87] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x88] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x89] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x8A] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x8B] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x8C] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x8D] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x8E] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z0HC", 0x0000, 1, 2, 2, 8},
	[0x8F] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z0HC", 0x0000, 1, 1, 1, 8},
	[0x90] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x91] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x92] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x93] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x94] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x95] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x96] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z1HC", 0x0000, 1, 2, 2, 8},
	[0x97] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x98] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x99] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x9A] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x9B] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x9C] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x9D] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0x9E] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z1HC", 0x0000, 1, 2, 2, 8},
	[0x9F] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xA0] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA1] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA2] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA3] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA4] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA5] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA6] = {fgbInstructionType_AND, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z010", 0x0000, 1, 2, 2, 8},
	[0xA7] = {fgbInstructionType_AND, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z010", 0x0000, 1, 1, 1, 8},
	[0xA8] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z000", 0x0000, 1, 1, 1, 8},
	[0xA9] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z000", 0x0000, 1, 1, 1, 8},
	[0xAA] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z000", 0x0000, 1, 1, 1, 8},
	[0xAB] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z000", 0x0000, 1, 1, 1, 8},
	[0xAC] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z000", 0x0000, 1, 1, 1, 8},
	[0xAD] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z000", 0x0000, 1, 1, 1, 8},
	[0xAE] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z000", 0x0000, 1, 2, 2, 8},
	[0xAF] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB0] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB1] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB2] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB3] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB4] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB5] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB6] = {fgbInstructionType_OR, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z000", 0x0000, 1, 2, 2, 8},
	[0xB7] = {fgbInstructionType_OR, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z000", 0x0000, 1, 1, 1, 8},
	[0xB8] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_B, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xB9] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xBA] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_D, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xBB] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_E, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xBC] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_H, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xBD] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_L, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xBE] = {fgbInstructionType_CP, fgbAddressingMode_Reg_MemReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_HL, "Z1HC", 0x0000, 1, 2, 2, 8},
	[0xBF] = {fgbInstructionType_CP, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_A, "Z1HC", 0x0000, 1, 1, 1, 8},
	[0xC0] = {fgbInstructionType_RET, fgbAddressingMode_Implied, fgbConditionType_NotZero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 2, 5, 0},
	[0xC1] = {fgbInstructionType_POP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_None, "----", 0x0000, 1, 3, 3, 16},
	[0xC2] = {fgbInstructionType_JP, fgbAddressingMode_U16, fgbConditionType_NotZero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 4, 0},
	[0xC3] = {fgbInstructionType_JP, fgbAddressingMode_U16, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 4, 4, 0},
	[0xC4] = {fgbInstructionType_CALL, fgbAddressingMode_U16, fgbConditionType_NotZero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 6, 0},
	[0xC5] = {fgbInstructionType_PUSH, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_BC, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 16},
	[0xC6] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z0HC", 0x0000, 2, 2, 2, 8},
	[0xC7] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 0},
	[0xC8] = {fgbInstructionType_RET, fgbAddressingMode_Implied, fgbConditionType_Zero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 2, 5, 0},
	[0xC9] = {fgbInstructionType_RET, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 0},
	[0xCA] = {fgbInstructionType_JP, fgbAddressingMode_U16, fgbConditionType_Zero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 4, 0},
	[0xCB] = {fgbInstructionType_PREFIX, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0xCC] = {fgbInstructionType_CALL, fgbAddressingMode_U16, fgbConditionType_Zero, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 6, 0},
	[0xCD] = {fgbInstructionType_CALL, fgbAddressingMode_U16, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 6, 6, 0},
	[0xCE] = {fgbInstructionType_ADC, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z0HC", 0x0000, 2, 2, 2, 8},
	[0xCF] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0008, 1, 4, 4, 0},
	[0xD0] = {fgbInstructionType_RET, fgbAddressingMode_Implied, fgbConditionType_NotCarry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 2, 5, 0},
	[0xD1] = {fgbInstructionType_POP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_None, "----", 0x0000, 1, 3, 3, 16},
	[0xD2] = {fgbInstructionType_JP, fgbAddressingMode_U16, fgbConditionType_NotCarry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 4, 0},
	[0xD3] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xD4] = {fgbInstructionType_CALL, fgbAddressingMode_U16, fgbConditionType_NotCarry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 6, 0},
	[0xD5] = {fgbInstructionType_PUSH, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_DE, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 16},
	[0xD6] = {fgbInstructionType_SUB, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z1HC", 0x0000, 2, 2, 2, 8},
	[0xD7] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0010, 1, 4, 4, 0},
	[0xD8] = {fgbInstructionType_RET, fgbAddressingMode_Implied, fgbConditionType_Carry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 2, 5, 0},
	[0xD9] = {fgbInstructionType_RETI, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 0},
	[0xDA] = {fgbInstructionType_JP, fgbAddressingMode_U16, fgbConditionType_Carry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 4, 0},
	[0xDB] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xDC] = {fgbInstructionType_CALL, fgbAddressingMode_U16, fgbConditionType_Carry, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 3, 3, 6, 0},
	[0xDD] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xDE] = {fgbInstructionType_SBC, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z1HC", 0x0000, 2, 2, 2, 8},
	[0xDF] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0018, 1, 4, 4, 0},
	[0xE0] = {fgbInstructionType_LD, fgbAddressingMode_MemConstantOffsetA8_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0xFF00, 2, 3, 3, 8},
	[0xE1] = {fgbInstructionType_POP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 1, 3, 3, 16},
	[0xE2] = {fgbInstructionType_LD, fgbAddressingMode_MemConstantOffsetReg_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_A, "----", 0xFF00, 1, 2, 2, 8},
	[0xE3] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xE4] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xE5] = {fgbInstructionType_PUSH, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 16},
	[0xE6] = {fgbInstructionType_AND, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z010", 0x0000, 2, 2, 2, 8},
	[0xE7] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0020, 1, 4, 4, 0},
	[0xE8] = {fgbInstructionType_ADD, fgbAddressingMode_Reg_I8, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_None, "00HC", 0x0000, 2, 4, 4, 16},
	[0xE9] = {fgbInstructionType_JP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0xEA] = {fgbInstructionType_LD, fgbAddressingMode_MemA16_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0000, 3, 4, 4, 8},
	[0xEB] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xEC] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xED] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xEE] = {fgbInstructionType_XOR, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0xEF] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0028, 1, 4, 4, 0},
	[0xF0] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemConstantOffsetA8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0xFF00, 2, 3, 3, 8},
	[0xF1] = {fgbInstructionType_POP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_AF, fgbRegisterType_None, "ZNHC", 0x0000, 1, 3, 3, 16},
	[0xF2] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemConstantOffsetReg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_C, "----", 0xFF00, 1, 2, 2, 8},
	[0xF3] = {fgbInstructionType_DI, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0xF4] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xF5] = {fgbInstructionType_PUSH, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_AF, fgbRegisterType_None, "----", 0x0000, 1, 4, 4, 16},
	[0xF6] = {fgbInstructionType_OR, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0xF7] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0030, 1, 4, 4, 0},
	[0xF8] = {fgbInstructionType_LD, fgbAddressingMode_Reg_RegOffsetI8, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_SP, "00HC", 0x0000, 2, 3, 3, 16},
	[0xF9] = {fgbInstructionType_LD, fgbAddressingMode_Reg_Reg, fgbConditionType_Always, fgbRegisterType_SP, fgbRegisterType_HL, "----", 0x0000, 1, 2, 2, 16},
	[0xFA] = {fgbInstructionType_LD, fgbAddressingMode_Reg_MemA16, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0000, 3, 4, 4, 8},
	[0xFB] = {fgbInstructionType_EI, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 1, 1, 0},
	[0xFC] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xFD] = {fgbInstructionType_None, fgbAddressingMode_Implied, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0000, 1, 0, 0, 0},
	[0xFE] = {fgbInstructionType_CP, fgbAddressingMode_Reg_U8, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z1HC", 0x0000, 2, 2, 2, 8},
	[0xFF] = {fgbInstructionType_RST, fgbAddressingMode_Constant, fgbConditionType_Always, fgbRegisterType_None, fgbRegisterType_None, "----", 0x0038, 1, 4, 4, 0},
};

// CB-Prefix Instructions
static const fgbInstruction fgb__prefixInstructionTable[] = {
	[0x00] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x01] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x02] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x03] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x04] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x05] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x06] = {fgbInstructionType_RLC, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x07] = {fgbInstructionType_RLC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x08] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x09] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x0A] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x0B] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x0C] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x0D] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x0E] = {fgbInstructionType_RRC, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x0F] = {fgbInstructionType_RRC, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x10] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x11] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x12] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x13] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x14] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x15] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x16] = {fgbInstructionType_RL, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x17] = {fgbInstructionType_RL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x18] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x19] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x1A] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x1B] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x1C] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x1D] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x1E] = {fgbInstructionType_RR, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x1F] = {fgbInstructionType_RR, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x20] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x21] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x22] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x23] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x24] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x25] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x26] = {fgbInstructionType_SLA, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x27] = {fgbInstructionType_SLA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x28] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x29] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x2A] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x2B] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x2C] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x2D] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x2E] = {fgbInstructionType_SRA, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x2F] = {fgbInstructionType_SRA, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x30] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x31] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x32] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x33] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x34] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x35] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x36] = {fgbInstructionType_SWAP, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z000", 0x0000, 2, 4, 4, 8},
	[0x37] = {fgbInstructionType_SWAP, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z000", 0x0000, 2, 2, 2, 8},
	[0x38] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x39] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x3A] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x3B] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x3C] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x3D] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x3E] = {fgbInstructionType_SRL, fgbAddressingMode_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z00C", 0x0000, 2, 4, 4, 8},
	[0x3F] = {fgbInstructionType_SRL, fgbAddressingMode_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z00C", 0x0000, 2, 2, 2, 8},
	[0x40] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x41] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x42] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x43] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x44] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x45] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x46] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0000, 2, 3, 3, 8},
	[0x47] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0000, 2, 2, 2, 8},
	[0x48] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x49] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x4A] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x4B] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x4C] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x4D] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x4E] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0001, 2, 3, 3, 8},
	[0x4F] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0001, 2, 2, 2, 8},
	[0x50] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x51] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x52] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x53] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x54] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x55] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x56] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0002, 2, 3, 3, 8},
	[0x57] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0002, 2, 2, 2, 8},
	[0x58] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x59] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x5A] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x5B] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x5C] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x5D] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x5E] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0003, 2, 3, 3, 8},
	[0x5F] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0003, 2, 2, 2, 8},
	[0x60] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x61] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x62] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x63] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x64] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x65] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x66] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0004, 2, 3, 3, 8},
	[0x67] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0004, 2, 2, 2, 8},
	[0x68] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x69] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x6A] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x6B] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x6C] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x6D] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x6E] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0005, 2, 3, 3, 8},
	[0x6F] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0005, 2, 2, 2, 8},
	[0x70] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x71] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x72] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x73] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x74] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x75] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x76] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0006, 2, 3, 3, 8},
	[0x77] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0006, 2, 2, 2, 8},
	[0x78] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x79] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x7A] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x7B] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x7C] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x7D] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x7E] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "Z01-", 0x0007, 2, 3, 3, 8},
	[0x7F] = {fgbInstructionType_BIT, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "Z01-", 0x0007, 2, 2, 2, 8},
	[0x80] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x81] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x82] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x83] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x84] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x85] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x86] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 2, 4, 4, 8},
	[0x87] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0x88] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x89] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x8A] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x8B] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x8C] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x8D] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x8E] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0001, 2, 4, 4, 8},
	[0x8F] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0x90] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x91] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x92] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x93] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x94] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x95] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x96] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0002, 2, 4, 4, 8},
	[0x97] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0x98] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x99] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x9A] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x9B] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x9C] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x9D] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0x9E] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0003, 2, 4, 4, 8},
	[0x9F] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xA0] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA1] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA2] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA3] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA4] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA5] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA6] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0004, 2, 4, 4, 8},
	[0xA7] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xA8] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xA9] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xAA] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xAB] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xAC] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xAD] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xAE] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0005, 2, 4, 4, 8},
	[0xAF] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xB0] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB1] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB2] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB3] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB4] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB5] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB6] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0006, 2, 4, 4, 8},
	[0xB7] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xB8] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xB9] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xBA] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xBB] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xBC] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xBD] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xBE] = {fgbInstructionType_RES, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0007, 2, 4, 4, 8},
	[0xBF] = {fgbInstructionType_RES, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xC0] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC1] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC2] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC3] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC4] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC5] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC6] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0000, 2, 4, 4, 8},
	[0xC7] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0000, 2, 2, 2, 8},
	[0xC8] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xC9] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xCA] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xCB] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xCC] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xCD] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xCE] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0001, 2, 4, 4, 8},
	[0xCF] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0001, 2, 2, 2, 8},
	[0xD0] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD1] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD2] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD3] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD4] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD5] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD6] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0002, 2, 4, 4, 8},
	[0xD7] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0002, 2, 2, 2, 8},
	[0xD8] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xD9] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xDA] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xDB] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xDC] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xDD] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xDE] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0003, 2, 4, 4, 8},
	[0xDF] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0003, 2, 2, 2, 8},
	[0xE0] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE1] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE2] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE3] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE4] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE5] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE6] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0004, 2, 4, 4, 8},
	[0xE7] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0004, 2, 2, 2, 8},
	[0xE8] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xE9] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xEA] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xEB] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xEC] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xED] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xEE] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0005, 2, 4, 4, 8},
	[0xEF] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0005, 2, 2, 2, 8},
	[0xF0] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF1] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF2] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF3] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF4] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF5] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF6] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0006, 2, 4, 4, 8},
	[0xF7] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0006, 2, 2, 2, 8},
	[0xF8] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_B, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xF9] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_C, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xFA] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_D, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xFB] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_E, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xFC] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_H, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xFD] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_L, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
	[0xFE] = {fgbInstructionType_SET, fgbAddressingMode_Constant_MemReg, fgbConditionType_Always, fgbRegisterType_HL, fgbRegisterType_None, "----", 0x0007, 2, 4, 4, 8},
	[0xFF] = {fgbInstructionType_SET, fgbAddressingMode_Constant_Reg, fgbConditionType_Always, fgbRegisterType_A, fgbRegisterType_None, "----", 0x0007, 2, 2, 2, 8},
};

FGB_API fgbInstructionTable fgbGetInstructionTable(void) {
	fgbInstructionTable result = { 0 };
	result.normal = &fgb__instructionTable[0];
	result.prefix = &fgb__prefixInstructionTable[0];
	return result;
}

// ********************************************************************************************************************
// String Formatting
// ********************************************************************************************************************
FGB_API const char *fgbGetRegisterName(const fgbRegisterType reg) {
	if (reg < FGB_ARRAYCOUNT(fgb__registerToNameTable)) {
		return fgb__registerToNameTable[reg];
	}
	return NULL;
}

FGB_API const char *fgbGetInstructionName(const fgbInstructionType type) {
	if (type < FGB_ARRAYCOUNT(fgb__instructionTypeNameTable))
		return fgb__instructionTypeNameTable[type];
	return NULL;
}

FGB_API const char *fgbGetAddressingModeName(const fgbAddressingMode mode) {
	if (mode < FGB_ARRAYCOUNT(fgb__addressingModeNameTable))
		return fgb__addressingModeNameTable[mode];
	return NULL;
}

static size_t fgb__FormatInstruction(fgbSystem *system, const fgbInstructionRegister *current, char *destBuffer, const size_t maxDestBufferLen) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_Core, "String");

	const char *instructionName = fgbGetInstructionName(current->instruction.type);

	const char *addressingModeName = fgbGetAddressingModeName(current->instruction.mode);

	const bool constantMemOffsetAsLDH = true;

	uint8_t opcode = current->opcode;

	const fgbDataRegister *data = &current->data;

	switch (current->instruction.mode) {
		case fgbAddressingMode_Implied:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s", instructionName);

		case fgbAddressingMode_Constant:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s $%04X", instructionName, current->instruction.value & 0xFFFF);

		case fgbAddressingMode_Constant_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %d, (%s)", instructionName, (int)current->instruction.value, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Constant_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %d, %s", instructionName, (int)current->instruction.value, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_I8:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s $%02X", instructionName, data->value.ulow);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s I8", instructionName);

		case fgbAddressingMode_U16:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s $%04X", instructionName, data->value.u16);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s U16", instructionName);

		case fgbAddressingMode_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s", instructionName, fgbGetRegisterName(current->instruction.regA));
		case fgbAddressingMode_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s)", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Reg_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %s", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_Reg_RegOffsetI8:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %s+%d", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB), current->data.offset);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %s+I8", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_Reg_MemConstantOffsetReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X+%s)", instructionName, fgbGetRegisterName(current->instruction.regA), current->instruction.value & 0xFFFF, fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_MemConstantOffsetReg_Reg: {
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X+%s), %s", instructionName, current->instruction.value & 0xFFFF, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));
		}

		case fgbAddressingMode_Reg_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s)", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_Reg_MemA16:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X)", instructionName, fgbGetRegisterName(current->instruction.regA), current->data.sourceAddress);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (A16)", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Reg_MemRegInc:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s+)", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_Reg_MemRegDec:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s-)", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_MemReg_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s), %s", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_MemReg_U8:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s), $%02X", instructionName, fgbGetRegisterName(current->instruction.regA), data->value.ulow);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s), U8", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Reg_U8:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, $%02X", instructionName, fgbGetRegisterName(current->instruction.regA), data->value.ulow);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, U8", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Reg_I8:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %d", instructionName, fgbGetRegisterName(current->instruction.regA), data->value.slow);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, I8", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_Reg_U16:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, $%04X", instructionName, fgbGetRegisterName(current->instruction.regA), data->value.u16);
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, U16", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_MemA16_Reg:
			if (data->hasData)
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X), %s", instructionName, data->targetAddress, fgbGetRegisterName(current->instruction.regA));
			else
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (A16), %s", instructionName, fgbGetRegisterName(current->instruction.regA));

		case fgbAddressingMode_MemRegDec_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s-), %s", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_MemRegInc_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s+), %s", instructionName, fgbGetRegisterName(current->instruction.regA), fgbGetRegisterName(current->instruction.regB));

		case fgbAddressingMode_MemConstantOffsetA8_Reg:
		{
			uint16_t constant = current->instruction.value & 0xFFFF;
			uint8_t offset = (data->targetAddress & ~constant) & 0xFF;

			if (constantMemOffsetAsLDH) {
				if (data->hasData)
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH ($%02X), %s", instructionName, offset, fgbGetRegisterName(current->instruction.regA));
				else
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH (A8), %s", instructionName, fgbGetRegisterName(current->instruction.regA));
			} else {
				if (data->hasData)
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X+$%02X), %s", instructionName, constant, offset, fgbGetRegisterName(current->instruction.regA));
				else
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X+A8), %s", instructionName, constant, fgbGetRegisterName(current->instruction.regA));
			}
		}

		case fgbAddressingMode_Reg_MemConstantOffsetA8:
		{
			// Use LDH instead
			uint16_t constant = current->instruction.value & 0xFFFF;
			uint8_t offset = (data->sourceAddress & ~constant) & 0xFF;
			if (constantMemOffsetAsLDH) {
				if (data->hasData)
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH %s, ($%02X)", instructionName, fgbGetRegisterName(current->instruction.regA), offset);
				else
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH %s, (A8)", instructionName, fgbGetRegisterName(current->instruction.regA));
			} else {
				if (data->hasData)
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X+$%02X)", instructionName, fgbGetRegisterName(current->instruction.regA), constant, offset);
				else
					return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X+A8)", instructionName, fgbGetRegisterName(current->instruction.regA), constant);
			}
		}

		default:
			FGB__WARN(system, sysName, "Addressing mode '%s' not implemented", addressingModeName);
			return 0;
	}
}

// ********************************************************************************************************************
// CPU Implementation
// ********************************************************************************************************************
static bool fgb__IsRegister8Bit(const fgbRegisterType type) {
	switch (type) {
		case fgbRegisterType_A:
		case fgbRegisterType_F:
		case fgbRegisterType_B:
		case fgbRegisterType_C:
		case fgbRegisterType_D:
		case fgbRegisterType_E:
		case fgbRegisterType_H:
		case fgbRegisterType_L:
			return true;
		default:
			return false;
	}
}

static bool fgb__IsRegister16Bit(const fgbRegisterType type) {
	switch (type) {
		case fgbRegisterType_AF:
		case fgbRegisterType_BC:
		case fgbRegisterType_DE:
		case fgbRegisterType_HL:
		case fgbRegisterType_SP:
		case fgbRegisterType_PC:
			return true;
		default:
			return false;
	}
}

static inline uint8_t fgb__ReadRegister8_Direct(const fgbCPURegisters *regs, const fgbRegisterType type) {
	switch (type) {
		case fgbRegisterType_A:
			return regs->a;
		case fgbRegisterType_F:
			return regs->f.flags;
		case fgbRegisterType_B:
			return regs->b;
		case fgbRegisterType_C:
			return regs->c;
		case fgbRegisterType_D:
			return regs->d;
		case fgbRegisterType_E:
			return regs->e;
		case fgbRegisterType_H:
			return regs->h;
			return true;
		case fgbRegisterType_L:
			return regs->l;
		default:
			return 0;
	}
}

static uint8_t fgb__ReadRegister8(const fgbCPURegisters *regs, const fgbRegisterType type) {
	uint8_t read = fgb__ReadRegister8_Direct(regs, type);
	return read;
}

static inline uint16_t fgb__ReadRegister16_Direct(const fgbCPURegisters *regs, const fgbRegisterType type) {
	switch (type) {
		case fgbRegisterType_AF:
			return regs->af;
		case fgbRegisterType_BC:
			return regs->bc;
		case fgbRegisterType_DE:
			return regs->de;
		case fgbRegisterType_HL:
			return regs->hl;
		case fgbRegisterType_SP:
			return regs->sp;
		case fgbRegisterType_PC:
			return regs->pc;
		default:
			return 0;
	}
}

static uint16_t fgb__ReadRegister16(const fgbCPURegisters *regs, const fgbRegisterType type) {
	uint16_t read = fgb__ReadRegister16_Direct(regs, type);
	const char *regName = fgbGetRegisterName(type);
	return read;
}

static inline void fgb__WriteRegister8_Direct(fgbCPURegisters *regs, const fgbRegisterType type, const uint8_t value) {
	switch (type) {
		case fgbRegisterType_A:
			regs->a = value;
			break;
		case fgbRegisterType_F:
			regs->f.flags = value;
			break;
		case fgbRegisterType_B:
			regs->b = value;
			break;
		case fgbRegisterType_C:
			regs->c = value;
			break;
		case fgbRegisterType_D:
			regs->d = value;
			break;
		case fgbRegisterType_E:
			regs->e = value;
			break;
		case fgbRegisterType_H:
			regs->h = value;
			break;
		case fgbRegisterType_L:
			regs->l = value;
			break;
		default:
			break;
	}
}

static void fgb__WriteRegister8(fgbCPURegisters *regs, const fgbRegisterType type, const uint8_t value) {
	const char *regName = fgbGetRegisterName(type);
	fgb__WriteRegister8_Direct(regs, type, value);
}

static inline void fgb__WriteRegister16_Direct(fgbCPURegisters *regs, const fgbRegisterType type, const uint16_t value) {
	switch (type) {
		case fgbRegisterType_AF:
			regs->af = value;
			break;
		case fgbRegisterType_BC:
			regs->bc = value;
			break;
		case fgbRegisterType_DE:
			regs->de = value;
			break;
		case fgbRegisterType_HL:
			regs->hl = value;
			break;
		case fgbRegisterType_SP:
			regs->sp = value;
			break;
		case fgbRegisterType_PC:
			regs->pc = value;
			break;
		default:
			break;
	}
}

static void fgb__WriteRegister16(fgbCPURegisters *regs, const fgbRegisterType type, const uint16_t value) {
	const char *regName = fgbGetRegisterName(type);
	fgb__WriteRegister16_Direct(regs, type, value);
}

typedef struct {
	uint8_t newValue;
	bool isZero;
	bool isOverflow;
	bool isHalfOverflow;
} fgb__Alu8Result;

static fgb__Alu8Result fgb__Add8(const uint8_t oldValue, const uint8_t addend) {
	uint8_t newValue = oldValue + addend;
	bool isZero = newValue == 0;
	bool isOverflow = (uint16_t)(oldValue + addend) > 0xFF;
	bool isHalfOverflow = (oldValue & 0xF) + (addend & 0xF) > 0xF;
	return (fgb__Alu8Result) { newValue, isZero, isOverflow, isHalfOverflow };
}

static fgb__Alu8Result fgb__Sub8(const uint8_t oldValue, const uint8_t subtrahend) {
	uint8_t difference = oldValue - subtrahend;
	bool isZero = difference == 0;
	bool isOverflow = oldValue < subtrahend;
	bool isHalfOverflow = (oldValue & 0xF) - (subtrahend & 0xF) < 0;
	return (fgb__Alu8Result) { difference, isZero, isOverflow, isHalfOverflow };
}

static fgb__Alu8Result fgb__Inc8(const uint8_t oldValue) {
	uint8_t newValue = oldValue + 1;
	bool isZero = newValue == 0;
	bool isOverflow = false;
	bool isHalfOverflow = (newValue & 0x0F) == 0x00;
	return (fgb__Alu8Result) { newValue, isZero, isOverflow, isHalfOverflow };
}

static fgb__Alu8Result fgb__Dec8(const uint8_t oldValue) {
	uint8_t newValue = oldValue - 1;
	bool isZero = newValue == 0;
	bool isOverflow = true;
	bool isHalfOverflow = (newValue & 0x0F) == 0x0F;
	return (fgb__Alu8Result) { newValue, isZero, isOverflow, isHalfOverflow };
}

static char fgb__FlagChar(int8_t v) {
	if (v == 1)
		return '1';
	else if (v == 0)
		return '0';
	return '-';
}

static void fgb__SetFlags(fgbCPURegisters *r, int8_t z, int8_t n, int8_t h, int8_t c) {
	char flagsText[5] = { 0 };
	flagsText[0] = fgb__FlagChar(z);
	flagsText[1] = fgb__FlagChar(n);
	flagsText[2] = fgb__FlagChar(h);
	flagsText[3] = fgb__FlagChar(c);

	if (z != -1) {
		r->f.zeroFlag = z == 1;
	}
	if (n != -1) {
		r->f.negativeFlag = n == 1;
	}
	if (h != -1) {
		r->f.halfCarryFlag = h == 1;
	}
	if (c != -1) {
		r->f.fullCarryFlag = c == 1;
	}
}

static bool fgb__StackPush16(fgbSystem *system, const uint16_t value) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_Core, "Stack");

	fgbCPURegisters *r = &system->cpu.registers;

	uint16_t oldSP = fgb__ReadRegister16(r, fgbRegisterType_SP);

	uint16_t newSP = oldSP - 2;

	fgb__WriteRegister16(r, fgbRegisterType_SP, newSP);

	fgb__BusWrite16(system, newSP, value, true);

	return true;
}

static uint16_t fgb__StackPop16(fgbSystem *system) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_Core, "Stack");

	fgbCPURegisters *r = &system->cpu.registers;

	uint16_t oldSP = fgb__ReadRegister16(r, fgbRegisterType_SP);

	uint16_t newSP = oldSP + 2;

	fgb__WriteRegister16(r, fgbRegisterType_SP, newSP);

	uint16_t result = fgb__BusRead16(system, oldSP, true);

	return result;
}

static uint8_t fgb__StackPop8(fgbSystem *system) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_Core, "Stack");

	fgbCPURegisters *r = &system->cpu.registers;

	uint16_t oldSP = fgb__ReadRegister16(r, fgbRegisterType_SP);

	uint16_t newSP = oldSP + 1;

	fgb__WriteRegister16(r, fgbRegisterType_SP, newSP);

	uint8_t result = fgb__BusRead8(system, oldSP, true);

	return result;
}

#define FGB__INSTRUCTION_PROC(name) bool name(fgbSystem *system, const fgbInstructionRegister *current)
typedef FGB__INSTRUCTION_PROC(fgb__InstructionProc);

static bool fgb__InstructionProc_NOP(fgbSystem *system, const fgbInstructionRegister *current) {
	return true;
}

static bool fgb__InstructionProc_DI(fgbSystem *system, const fgbInstructionRegister *current) {
	system->interrupts.isMasterEnabled = false;
	system->interrupts.ticksEnableIME = 0;
	return true;
}

static bool fgb__InstructionProc_EI(fgbSystem *system, const fgbInstructionRegister *current) {
	system->interrupts.ticksEnableIME = 4;
	return true;
}

static bool fgb__EvaluateCondition(const fgbSystem *system, const fgbConditionType condition) {
	const fgbFlagsRegister f = system->cpu.registers.f;
	switch (condition) {
		case fgbConditionType_Zero:
			return f.zeroFlag;
		case fgbConditionType_NotZero:
			return !f.zeroFlag;
		case fgbConditionType_Carry:
			return f.fullCarryFlag;
		case fgbConditionType_NotCarry:
			return !f.fullCarryFlag;
		case fgbConditionType_Always:
			return true;
		default:
			return false;
	}
}

static bool fgb__Goto(fgbSystem *system, const fgbInstructionRegister *current, const char *sysName, const uint16_t address, const bool pushToStack) {
	fgbCPURegisters *r = &system->cpu.registers;

	if (fgb__EvaluateCondition(system, current->instruction.condition)) {
		if (pushToStack) {
			uint16_t pc = r->pc;
			if (!fgb__StackPush16(system, pc)) {
				FGB__ERROR(system, sysName, "Unable to push PC '$%04X' to the stack", pc);
				return false;
			}
		}

		r->pc = address;

		system->cpu.instructionRegister.wasBranchTaken = true;

		if (current->instruction.mode == fgbAddressingMode_Reg && fgb__IsRegister16Bit(current->instruction.regA)) {
			// Do nothing, because when a JP address is read from a register directly, no additional cycles are required
			int x = 42;
		} else {
			fgb__HWTick4(system);
		}
	}

	return true;
}

static bool fgb__InstructionProc_JP(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "JP");

	FGB_ASSERT(current->data.hasData);
	uint16_t address = current->data.value.u16;

	return fgb__Goto(system, current, sysName, address, false);
}

static bool fgb__InstructionProc_JR(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "JR");

	FGB_ASSERT(current->data.hasData);
	int8_t offset = current->data.value.slow;

	uint16_t address = system->cpu.registers.pc + offset;

	return fgb__Goto(system, current, sysName, address, false);
}

static bool fgb__InstructionProc_CALL(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "CALL");

	FGB_ASSERT(current->data.hasData);
	uint16_t address = current->data.value.u16;

	return fgb__Goto(system, current, sysName, address, true);
}

static bool fgb__InstructionProc_RST(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RST");

	uint16_t address = current->instruction.value & 0xFFFF;

	return fgb__Goto(system, current, sysName, address, true);
}


static bool fgb__Return(fgbSystem *system, const fgbInstructionRegister *current, const char *sysName) {
	fgbCPURegisters *r = &system->cpu.registers;

	if (current->instruction.condition != fgbConditionType_Always) {
		fgb__HWTick4(system);
	}

	if (fgb__EvaluateCondition(system, current->instruction.condition)) {
		uint16_t newPC = fgb__StackPop16(system);

		fgb__HWTick4(system);

		r->pc = newPC;

		system->cpu.instructionRegister.wasBranchTaken = true;
	}

	return true;
}

static bool fgb__InstructionProc_RET(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RET");

	return fgb__Return(system, current, sysName);
}

static bool fgb__InstructionProc_RETI(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RETI");

	bool result = fgb__Return(system, current, sysName);

	system->interrupts.isMasterEnabled = true;

	return result;
}

static bool fgb__InstructionProc_PUSH(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "PUSH");

	fgbCPURegisters *r = &system->cpu.registers;

	FGB_ASSERT(current->data.hasData);
	FGB_ASSERT(current->data.value.isWide);
	FGB_ASSERT(current->instruction.mode == fgbAddressingMode_Reg);

	uint16_t value = current->data.value.u16;

	// 2-3 th cycle
	if (!fgb__StackPush16(system, value)) {
		FGB__ERROR(system, sysName, "Unable to push '$%04X' to the stack", value);
		return false;
	}

	// 4th cycle
	fgb__HWTick4(system);

	return true;
}

static bool fgb__InstructionProc_POP(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "POP");

	fgbCPURegisters *r = &system->cpu.registers;

	FGB_ASSERT(current->instruction.mode == fgbAddressingMode_Reg);

	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(fgb__IsRegister16Bit(destReg));

	uint16_t value = fgb__StackPop16(system);

	if (destReg == fgbRegisterType_AF) {
		// NOTE(final): POP AF masks out the used flags (4-bits) the other 4-bits are zero
		value = value & 0xFFF0;

		// TODO(final): POP AF needs to update all the flags, but this is missed here and the blorrg test succeeded - weird
	}

	fgb__WriteRegister16(r, destReg, value);

	return true;
}

static bool fgb__InstructionProc_INC(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "INC");

	fgbCPURegisters *r = &system->cpu.registers;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(data->hasData);

	if (!data->isMemoryTarget) {
		fgbRegisterType destReg = current->instruction.regA;

		if (fgb__IsRegister16Bit(destReg)) {
			uint16_t oldValue = data->value.u16;

			uint16_t newValue = oldValue + 1;

			fgb__WriteRegister16(r, destReg, newValue);

			fgb__HWTick4(system);

			return true;
		} else {
			FGB_ASSERT(fgb__IsRegister8Bit(destReg));

			uint8_t oldValue = data->value.ulow;

			fgb__Alu8Result incRes = fgb__Inc8(oldValue);

			uint8_t newValue = incRes.newValue;

			fgb__WriteRegister8(r, destReg, newValue);

			fgb__SetFlags(r, incRes.isZero, false, incRes.isHalfOverflow, -1);

			return true;
		}
	} else {
		uint16_t address = data->targetAddress;

		// INC is a 8-bit operation
		FGB_ASSERT(current->instruction.dataLength == 8);

		uint8_t oldValue = data->value.ulow;

		fgb__Alu8Result incRes = fgb__Inc8(oldValue);

		uint8_t newValue = incRes.newValue;

		fgb__BusWrite8(system, address, newValue, true);

		fgb__SetFlags(r, incRes.isZero, false, incRes.isHalfOverflow, -1);

		return true;
	}
}

static bool fgb__InstructionProc_DEC(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "DEC");

	fgbCPURegisters *r = &system->cpu.registers;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(data->hasData);

	if (!data->isMemoryTarget) {
		fgbRegisterType destReg = current->instruction.regA;

		if (fgb__IsRegister16Bit(destReg)) {
			uint16_t oldValue = data->value.u16;

			uint16_t newValue = oldValue - 1;

			fgb__WriteRegister16(r, destReg, newValue);

			fgb__HWTick4(system);
		} else {
			FGB_ASSERT(fgb__IsRegister8Bit(destReg));

			uint8_t oldValue = data->value.ulow;

			fgb__Alu8Result decRes = fgb__Dec8(oldValue);

			uint8_t newValue = decRes.newValue;
			fgb__WriteRegister8(r, destReg, newValue);

			fgb__SetFlags(r, decRes.isZero, true, decRes.isHalfOverflow, -1);
		}

		return true;
	} else {
		FGB_ASSERT(current->instruction.dataLength == 8);

		uint16_t address = data->targetAddress;

		uint8_t oldValue = data->value.ulow;

		fgb__Alu8Result decRes = fgb__Dec8(oldValue);

		uint8_t newValue = decRes.newValue;
		fgb__BusWrite8(system, address, newValue, true);

		fgb__SetFlags(r, decRes.isZero, true, decRes.isHalfOverflow, -1);

		return true;
	}
}

static bool fgb__InstructionProc_XOR(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "XOR");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	uint8_t newValue = oldValue ^ current->data.value.ulow;

	fgb__WriteRegister8(r, destReg, newValue);

	fgb__SetFlags(r, newValue == 0, false, false, false);

	return true;
}

static bool fgb__InstructionProc_AND(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "AND");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	uint8_t newValue = oldValue & current->data.value.ulow;

	fgb__WriteRegister8(r, destReg, newValue);

	fgb__SetFlags(r, newValue == 0, false, true, false);

	return true;
}

static bool fgb__InstructionProc_OR(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "OR");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	uint8_t newValue = oldValue | current->data.value.ulow;

	fgb__WriteRegister8(r, destReg, newValue);

	fgb__SetFlags(r, newValue == 0, false, false, false);

	return true;
}

static bool fgb__InstructionProc_ADD(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "ADD");

	fgbCPURegisters *r = &system->cpu.registers;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(!data->isMemoryTarget);
	FGB_ASSERT(data->hasData);

	fgbRegisterType destReg = current->instruction.regA;

	if (fgb__IsRegister16Bit(destReg)) {
		uint16_t oldValue = fgb__ReadRegister16(r, destReg);

		uint16_t newValue;
		if (data->value.isWide) {
			uint16_t addend = data->value.u16;
			newValue = oldValue + addend;
		} else {
			if (data->value.isSign) {
				int8_t addend = data->value.slow;
				int32_t tmp = oldValue + addend;
				newValue = (uint16_t)tmp & 0xFFFF;
			} else {
				uint8_t addend = data->value.ulow;
				newValue = oldValue + addend;
			}
		}

		int8_t isZero;
		int8_t isHalfCarry;
		int8_t isFullCarry;

		if (destReg == fgbRegisterType_SP) {
			// NOTE(final): $E8 Needs special treatment, because it computes flags differenly and is signed
			uint16_t fetch = data->value.u16;
			isZero = 0;
			isHalfCarry = (oldValue & 0xF) + (fetch & 0xF) >= 0x10;
			isFullCarry = (int)(oldValue & 0xFF) + (int)(fetch & 0xFF) >= 0x100;
		} else {
			isZero = -1;
			isHalfCarry = (newValue & 0xFFF) < (oldValue & 0xFFF);
			isFullCarry = (newValue & 0xFFFF) < (oldValue & 0xFFFF);
		}


		uint16_t targetValue = newValue & 0xFFFF;

		fgb__WriteRegister16(r, destReg, targetValue);

		if (current->instruction.mode == fgbAddressingMode_Reg_I8) {
			// NOTE(final): $E8 requires two additional cycles
			fgb__HWTick4(system);
			fgb__HWTick4(system);
		} else
			fgb__HWTick4(system);

		fgb__SetFlags(r, isZero, false, isHalfCarry, isFullCarry);
	} else {
		FGB_ASSERT(fgb__IsRegister8Bit(destReg));
		FGB_ASSERT(!data->value.isSign);

		uint8_t oldValue = fgb__ReadRegister8(r, destReg);

		uint8_t addend = current->data.value.ulow;

		fgb__Alu8Result addRes = fgb__Add8(oldValue, addend);

		uint8_t newValue = addRes.newValue;

		fgb__WriteRegister8(r, destReg, newValue);

		fgb__SetFlags(r, addRes.isZero, false, addRes.isHalfOverflow, addRes.isOverflow);
	}

	return true;
}

static bool fgb__InstructionProc_ADC(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "ADC");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	bool c = r->f.fullCarryFlag;

	uint8_t f = current->data.value.ulow;

	uint16_t sum = oldValue + f + c;

	uint8_t newValue = sum & 0xFF;

	bool isZero = newValue == 0;
	bool isHalfCarry = ((oldValue & 0xF) + (f & 0xF) + c) > 0xF;
	bool isFullCarry = sum > 0xFF;

	fgb__WriteRegister8(r, destReg, newValue);

	fgb__SetFlags(r, isZero, false, isHalfCarry, isFullCarry);

	return true;
}

static bool fgb__InstructionProc_SBC(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "SBC");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	bool c = r->f.fullCarryFlag;

	uint8_t f = current->data.value.ulow;
	uint8_t subtrahend = f + c;
	uint16_t newValue = oldValue - subtrahend;

	bool isZero = (oldValue - subtrahend) == 0;
	bool isHalfCarry = ((int)oldValue & 0xF) - ((int)f & 0xF) - ((int)c) < 0;
	bool isFullCarry = ((int)oldValue) - ((int)f) - ((int)c) < 0;

	fgb__WriteRegister8(r, destReg, newValue & 0xFF);

	fgb__SetFlags(r, isZero, true, isHalfCarry, isFullCarry);

	return true;
}

static bool fgb__InstructionProc_SUB(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "SUB");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	uint8_t subtrahend = current->data.value.ulow;

	fgb__Alu8Result subRes = fgb__Sub8(oldValue, subtrahend);

	uint8_t newValue = subRes.newValue;

	fgb__WriteRegister8(r, destReg, newValue);

	fgb__SetFlags(r, subRes.isZero, true, subRes.isHalfOverflow, subRes.isOverflow);

	return true;
}

static bool fgb__InstructionProc_CP(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "CP");

	fgbCPURegisters *r = &system->cpu.registers;

	// Only Register A is allowed as destination
	fgbRegisterType destReg = current->instruction.regA;
	FGB_ASSERT(destReg == fgbRegisterType_A);

	FGB_ASSERT(!current->data.isMemoryTarget);
	FGB_ASSERT(current->data.hasData);

	uint8_t value = fgb__ReadRegister8(r, destReg);

	FGB_ASSERT(current->data.hasData);
	uint8_t subtrahend = current->data.value.ulow;

	fgb__Alu8Result subRes = fgb__Sub8(value, subtrahend);

	fgb__SetFlags(r, subRes.isZero, true, subRes.isHalfOverflow, subRes.isOverflow);

	return true;
}

typedef fgb__Alu8Result fgb__CB_AluFunc(const uint8_t value, const fgbFlagsRegister flags);

// Rotates the contents of operand m to the left.
// r and (HL) are used for operand m.
// When B = 85h, (HL) = 0, and CY = 0,
// RLC B ; B <- 0Bh, CY <- 1, Z <- 0, H <- 0, N <- 0
// RLC (HL) ; (HL) <- 00h, CY <- 0, Z <- 1, H <- 0, N <- 0
static fgb__Alu8Result fgb__RLC_U8(const uint8_t value, const fgbFlagsRegister flags) {
	bool c = false;
	uint8_t newValue = (value << 1) & 0xFF;

	if ((value & (1 << 7)) != 0) {
		newValue |= 1;
		c = true;
	}

	fgb__Alu8Result result = { 0 };
	result.isZero = newValue == 0;
	result.isHalfOverflow = false;
	result.isOverflow = c;
	result.newValue = newValue;
	return result;
}

// Rotates the contents of operand m to the right.
// r and (HL) are used for operand m.
// When C = 1h, (HL) = 0h, CY = 0,
// RRC C ; C <- 80h, CY <- 1, Z <- 0, H <- 0, N <- 0
// RRC (HL) ; (HL) <- 00h, CY <- 0, Z <- 1, H <- 0, N <- 0
static fgb__Alu8Result fgb__RRC_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = value >> 1;
	newValue |= (value << 7);

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = value & 1;
	result.newValue = newValue;
	return result;
}


// Rotates the contents of operand m to the left.
// r and (HL) are used for operand m.
// When L = 80h, (HL) = 11h, and CY = 0,
// RL L ; L <- 00h, CY <- 1, Z <- 1, H <- 0, N <- 0
// RL (HL) ; (HL) <- 22h, CY <- 0, Z <- 0, H <- 0, N <- 0
static fgb__Alu8Result fgb__RL_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = value << 1;
	newValue |= flags.fullCarryFlag;

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = !!(value & 0x80);
	result.newValue = newValue;
	return result;
}

// Rotates the contents of operand m to the right.
// r and (HL) are used for operand m.
// When A = 1h, (HL) = 8Ah, CY = 0,
// RR A ; A <- 00h, CY <- 1, Z <- 1, H <- 0, N <- 0
// RR (HL) ; (HL) <- 45h, CY <- 0, Z <- 0, H <- 0, N <- 0
static fgb__Alu8Result fgb__RR_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = value >> 1;
	newValue |= (flags.fullCarryFlag << 7);

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = value & 1;
	result.newValue = newValue;
	return result;
}

// Shifts the contents of operand m to the left.
// r and (HL) are used for operand m.
// When D = 80h, (HL) = FFh, and CY = 0,
// SLA D ; D <- 00h, CY <- 1, Z <- 1, H <- 0, N <- 0
// SLA (HL) ; (HL) <- FEh, CY <- 1, Z <- 0, H <- 0, N <- 0
static fgb__Alu8Result fgb__SLA_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = value << 1;

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = !!(value & 0x80);
	result.newValue = newValue;
	return result;
}

// Shifts the contents of operand m to the right.
// r and (HL) are used for operand m.
// When A = 8Ah, (HL) = 01h, and CY = 0,
// SRA D ; A <- C5h, CY <- 0, Z <- 0, H <- 0, N <- 0
// SRA (HL) ; (HL) <- 00h, CY <- 1, Z <- 1, H <- 0, N <- 0
static fgb__Alu8Result fgb__SRA_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = (int8_t)value >> 1;

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = value & 1;
	result.newValue = newValue;
	return result;
}

// Shifts the contents of the lower-order 4 bits (0-3) of operand m unmodified to the higher-order 4 bits (4-7)
// of that operand and shifts the contents of the higher-order 4 bits to the lower-order 4 bits.
// r and (HL) are used for operand m.
// When A = 00h and (HL) = F0h,
// SWAP A ; A <- 00h, Z <- 1, H <- 0, N <- 0, CY <- 0
// SWAP (HL) ; (HL) <- 0Fh, Z <- 0, H <- 0, N <- 0, CY <- 0
static fgb__Alu8Result fgb__SWAP_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = ((value & 0xF0) >> 4) | ((value & 0xF) << 4);

	fgb__Alu8Result result = { 0 };
	result.isZero = newValue == 0;
	result.isHalfOverflow = false;
	result.isOverflow = false;
	result.newValue = newValue;
	return result;
}

// Shifts the contents of operand m to the right.
// r and (HL) are used for operand m.
// When A = 01h, (HL) = FFh, CY + 0,
// SRL A ; A <- 00h, CY <- 1, Z <- 1, H <- 0, N <- 0
// SRL (HL) ; (HL) <- 7Fh, CY <- 1, Z <- 0, H <- 0, N <- 0
static fgb__Alu8Result fgb__SRL_U8(const uint8_t value, const fgbFlagsRegister flags) {
	uint8_t newValue = value >> 1;

	fgb__Alu8Result result = { 0 };
	result.isZero = !newValue;
	result.isHalfOverflow = false;
	result.isOverflow = value & 1;
	result.newValue = newValue;
	return result;
}

static fgb__CB_AluFunc *fgb__Alu8Funcs[256] = {
	[fgbInstructionType_RLC] = fgb__RLC_U8,
	[fgbInstructionType_RRC] = fgb__RRC_U8,
	[fgbInstructionType_RL] = fgb__RL_U8,
	[fgbInstructionType_RR] = fgb__RR_U8,
	[fgbInstructionType_SRA] = fgb__SRA_U8,
	[fgbInstructionType_SLA] = fgb__SLA_U8,
	[fgbInstructionType_SRL] = fgb__SRL_U8,
	[fgbInstructionType_SWAP] = fgb__SWAP_U8,
};

#define FGB__STR_CONCAT(a, b) a # b
#define FGB__STR_CAST(s) #s

static bool fgb__InstructionProc_RLCA(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RLCA");

	const fgbRegisterType destReg = fgbRegisterType_A;

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	fgb__Alu8Result shiftRes = fgb__RLC_U8(oldValue, r->f);

	uint8_t newValue = shiftRes.newValue;
	fgb__WriteRegister8(r, destReg, newValue);

	bool fullCarry = shiftRes.isOverflow;
	fgb__SetFlags(r, false, false, false, fullCarry);

	return true;
}

static bool fgb__InstructionProc_RRCA(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RRCA");

	const fgbRegisterType destReg = fgbRegisterType_A;

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	fgb__Alu8Result shiftRes = fgb__RRC_U8(oldValue, r->f);

	uint8_t newValue = shiftRes.newValue;
	fgb__WriteRegister8(r, destReg, newValue);

	bool fullCarry = shiftRes.isOverflow;
	fgb__SetFlags(r, false, false, false, fullCarry);

	return true;
}

static bool fgb__InstructionProc_RLA(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RLA");

	const fgbRegisterType destReg = fgbRegisterType_A;

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	fgb__Alu8Result shiftRes = fgb__RL_U8(oldValue, r->f);

	uint8_t newValue = shiftRes.newValue;
	fgb__WriteRegister8(r, destReg, newValue);

	bool fullCarry = shiftRes.isOverflow;
	fgb__SetFlags(r, false, false, false, fullCarry);

	return true;
}

static bool fgb__InstructionProc_RRA(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RRA");

	const fgbRegisterType destReg = fgbRegisterType_A;

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, destReg);

	fgb__Alu8Result shiftRes = fgb__RR_U8(oldValue, r->f);

	uint8_t newValue = shiftRes.newValue;
	fgb__WriteRegister8(r, destReg, newValue);

	bool fullCarry = shiftRes.isOverflow;
	fgb__SetFlags(r, false, false, false, fullCarry);

	return true;
}

static bool fgb__InstructionProc_RotateShifts(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *instructionName = fgbGetInstructionName(current->instruction.type);

	char sysName[20] = { 0 };
	fgb__StringFormat(sysName, FGB_ARRAYCOUNT(sysName), "%s::%s", fgb__KindName_CPU, instructionName);

	fgb__CB_AluFunc *func = fgb__Alu8Funcs[current->instruction.type];
	if (func == NULL) {
		FGB__ERROR(system, sysName, "No ALU procedure for instruction '%s' defined", instructionName);
		return false;
	}

	fgbCPURegisters *r = &system->cpu.registers;

	fgbRegisterType destReg = current->instruction.regA;

	uint8_t oldValue = current->data.value.ulow;

	if (!current->data.isMemoryTarget && current->instruction.mode == fgbAddressingMode_Implied) {
		destReg = fgbRegisterType_A;
		oldValue = fgb__ReadRegister8(r, destReg);
	}

	if (!current->data.isMemoryTarget) {
		FGB_ASSERT(fgb__IsRegister8Bit(destReg));

		fgb__Alu8Result shiftRes = func(oldValue, r->f);

		uint8_t value = shiftRes.newValue;
		fgb__WriteRegister8(r, destReg, value);

		bool isZero = shiftRes.isZero;
		bool fullCarry = shiftRes.isOverflow;
		fgb__SetFlags(r, isZero, false, false, fullCarry);

		return true;
	} else {
		uint16_t address = current->data.targetAddress;

		fgb__Alu8Result shiftRes = func(oldValue, r->f);

		uint8_t value = shiftRes.newValue;
		fgb__BusWrite8(system, address, value, true);

		bool isZero = shiftRes.isZero;
		bool fullCarry = shiftRes.isOverflow;
		fgb__SetFlags(r, isZero, false, false, fullCarry);

		return true;
	}

	return false;
}

static bool fgb__InstructionProc_LD(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "LD");

	fgbCPURegisters *r = &system->cpu.registers;

	const fgbDataRegister *data = &current->data;

	fgbAddressingMode mode = current->instruction.mode;

	FGB_ASSERT(data->hasData);

	if (!data->isMemoryTarget) {
		fgbRegisterType destReg = current->instruction.regA;

		if (fgb__IsRegister16Bit(destReg)) {
			uint16_t value = data->value.isWide ? data->value.u16 : data->value.ulow;

			uint16_t sourceValue = value;

			if (mode == fgbAddressingMode_Reg_RegOffsetI8) {
				// NOTE(final): Reload source register value, because to update the flags, we need the value after the set
				fgbRegisterType sourceReg = current->instruction.regB;
				sourceValue = fgb__ReadRegister16_Direct(r, sourceReg);
			}

			fgb__WriteRegister16(r, destReg, value);

			if (mode == fgbAddressingMode_Reg_RegOffsetI8) {
				// NOTE(final): LD HL, SP + i8 requires three emulator cycles, so we add one because the opcode was one bus-read and the I8 was one
				int8_t offset = data->offset;
				uint8_t hflag = (sourceValue & 0xF) + (offset & 0xF) >= 0x10;
				uint8_t cflag = (sourceValue & 0xFF) + (offset & 0xFF) >= 0x100;
				fgb__SetFlags(r, 0, 0, hflag, cflag);
				fgb__HWTick4(system);
			} else if (mode == fgbAddressingMode_Reg_Reg) {
				// NOTE(final): LD SP, HL requires two emulator cycles, so we add one because the opcode was one bus-read
				fgb__HWTick4(system);
			}

			return true;
		} else {
			FGB_ASSERT(fgb__IsRegister8Bit(destReg));

			uint8_t value = data->value.ulow;

			fgb__WriteRegister8(r, destReg, value);

			return true;
		}
	} else {

		uint16_t targetAddress = data->targetAddress;

		if (data->value.isWide) {
			uint16_t value = data->value.u16;
			fgb__BusWrite16(system, targetAddress, value, true);
			return true;
		} else {
			uint8_t value = data->value.ulow;
			fgb__BusWrite8(system, targetAddress, value, true);
			return true;
		}
	}

	FGB__ERROR(system, sysName, "Unsupported Adressing Mode '%s'", fgbGetAddressingModeName(current->instruction.mode));
	return false;
}

static bool fgb__InstructionProc_DAA(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "DAA");

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, fgbRegisterType_A);

	uint8_t u = 0;

	bool hflag = r->f.halfCarryFlag;
	bool nflag = r->f.negativeFlag;
	bool cflag = r->f.fullCarryFlag;

	if (hflag || (!nflag && (oldValue & 0xF) > 9)) {
		u = 6;
	}

	int8_t fullCarry = 0;
	if (cflag || (!nflag && oldValue > 0x99)) {
		u |= 0x60;
		fullCarry = 1;
	}

	uint8_t newValue = oldValue + (nflag ? -u : u);
	fgb__WriteRegister8(r, fgbRegisterType_A, newValue);

	fgb__SetFlags(r, newValue == 0, -1, 0, fullCarry);

	return true;
}

static bool fgb__InstructionProc_CPL(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "CPL");

	fgbCPURegisters *r = &system->cpu.registers;

	uint8_t oldValue = fgb__ReadRegister8(r, fgbRegisterType_A);

	uint8_t newValue = ~oldValue;

	fgb__WriteRegister8(r, fgbRegisterType_A, newValue);

	fgb__SetFlags(r, -1, 1, 1, -1);

	return true;
}

static bool fgb__InstructionProc_SCF(fgbSystem *system, const fgbInstructionRegister *current) {
	fgbCPURegisters *r = &system->cpu.registers;
	fgb__SetFlags(r, -1, 0, 0, 1);
	return true;
}

static bool fgb__InstructionProc_CCF(fgbSystem *system, const fgbInstructionRegister *current) {
	fgbCPURegisters *r = &system->cpu.registers;
	bool carryFlag = r->f.fullCarryFlag;
	fgb__SetFlags(r, -1, 0, 0, carryFlag ^ 1);
	return true;
}

static bool fgb__InstructionProc_HALT(fgbSystem *system, const fgbInstructionRegister *current) {
	fgbCPU *cpu = &system->cpu;
	fgbInterrupts *ir = &system->interrupts;

	if (ir->ticksEnableIME > 0) {
		// Special case for IE
		ir->ticksEnableIME = 0;
		ir->isMasterEnabled = true;
		cpu->registers.pc--;
	} else {
		uint8_t enable = ir->enable.u8;
		uint8_t request = ir->request.u8;

		cpu->state.type = fgbCPUStateType_Halt;

		// Force HALT bug
		if (!ir->isMasterEnabled && (request & enable & 0x1F)) {
			cpu->state.skipPC = true;
		}
	}

	return true;
}

static bool fgb__InstructionProc_STOP(fgbSystem *system, const fgbInstructionRegister *current) {
	fgbCPURegisters *r = &system->cpu.registers;

	// NOTE(final): STOP skips the next instruction always
	r->pc++;

	return true;
}

static bool fgb__InstructionProc_BIT(fgbSystem *system, const fgbInstructionRegister *current) {
	fgbCPURegisters *r = &system->cpu.registers;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(data->hasData);

	uint8_t bit = current->instruction.value & 0xFF;

	uint8_t value = current->data.value.ulow;

	bool zeroFlag = !(value & (1 << bit));
	fgb__SetFlags(r, zeroFlag, 0, 1, -1);

	return true;
}

static bool fgb__InstructionProc_RES(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "RES");

	fgbCPURegisters *r = &system->cpu.registers;

	fgbRegisterType destReg = current->instruction.regA;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(data->hasData);

	uint8_t bit = current->instruction.value & 0xFF;

	uint8_t oldValue = current->data.value.ulow;

	uint8_t newValue = oldValue & ~(1 << bit);

	if (data->isMemoryTarget) {
		uint16_t targetAddress = data->targetAddress;

		fgb__BusWrite8(system, targetAddress, newValue, true);
	} else {
		FGB_ASSERT(fgb__IsRegister8Bit(destReg));

		fgb__WriteRegister8(r, destReg, newValue);
	}

	return true;
}

static bool fgb__InstructionProc_SET(fgbSystem *system, const fgbInstructionRegister *current) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "SET");

	fgbCPURegisters *r = &system->cpu.registers;

	fgbRegisterType destReg = current->instruction.regA;

	const fgbDataRegister *data = &current->data;

	FGB_ASSERT(data->hasData);

	uint8_t bit = current->instruction.value & 0xFF;

	uint8_t oldValue = current->data.value.ulow;

	uint8_t newValue = oldValue | (1 << bit);

	if (data->isMemoryTarget) {
		uint16_t targetAddress = data->targetAddress;

		fgb__BusWrite8(system, targetAddress, newValue, true);
	} else {
		FGB_ASSERT(fgb__IsRegister8Bit(destReg));

		fgb__WriteRegister8(r, destReg, newValue);
	}

	return true;
}

static fgb__InstructionProc *fgb__instructionProcedures[] = {
	[fgbInstructionType_NOP] = fgb__InstructionProc_NOP,

	// Jump
	[fgbInstructionType_JP] = fgb__InstructionProc_JP,
	[fgbInstructionType_JR] = fgb__InstructionProc_JR,
	[fgbInstructionType_CP] = fgb__InstructionProc_CP,
	[fgbInstructionType_CALL] = fgb__InstructionProc_CALL,
	[fgbInstructionType_RET] = fgb__InstructionProc_RET,
	[fgbInstructionType_RETI] = fgb__InstructionProc_RETI,
	[fgbInstructionType_PUSH] = fgb__InstructionProc_PUSH,
	[fgbInstructionType_POP] = fgb__InstructionProc_POP,
	[fgbInstructionType_RST] = fgb__InstructionProc_RST,

	// Load
	[fgbInstructionType_LD] = fgb__InstructionProc_LD,

	// Arithmetic
	[fgbInstructionType_INC] = fgb__InstructionProc_INC,
	[fgbInstructionType_DEC] = fgb__InstructionProc_DEC,
	[fgbInstructionType_XOR] = fgb__InstructionProc_XOR,
	[fgbInstructionType_OR] = fgb__InstructionProc_OR,
	[fgbInstructionType_AND] = fgb__InstructionProc_AND,
	[fgbInstructionType_ADD] = fgb__InstructionProc_ADD,
	[fgbInstructionType_SUB] = fgb__InstructionProc_SUB,
	[fgbInstructionType_ADC] = fgb__InstructionProc_ADC,
	[fgbInstructionType_SBC] = fgb__InstructionProc_SBC,

	// Rotate with A
	[fgbInstructionType_RLCA] = fgb__InstructionProc_RLCA,
	[fgbInstructionType_RLA] = fgb__InstructionProc_RLA,
	[fgbInstructionType_RRCA] = fgb__InstructionProc_RRCA,
	[fgbInstructionType_RRA] = fgb__InstructionProc_RRA,

	// CB
	[fgbInstructionType_RLC] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_RRC] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_RL] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_RR] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_SLA] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_SRA] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_SRL] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_SWAP] = fgb__InstructionProc_RotateShifts,
	[fgbInstructionType_BIT] = fgb__InstructionProc_BIT,
	[fgbInstructionType_RES] = fgb__InstructionProc_RES,
	[fgbInstructionType_SET] = fgb__InstructionProc_SET,

	// Special
	[fgbInstructionType_DI] = fgb__InstructionProc_DI,
	[fgbInstructionType_EI] = fgb__InstructionProc_EI,
	[fgbInstructionType_DAA] = fgb__InstructionProc_DAA,
	[fgbInstructionType_CPL] = fgb__InstructionProc_CPL,
	[fgbInstructionType_SCF] = fgb__InstructionProc_SCF,
	[fgbInstructionType_CCF] = fgb__InstructionProc_CCF,
	[fgbInstructionType_HALT] = fgb__InstructionProc_HALT,
	[fgbInstructionType_STOP] = fgb__InstructionProc_STOP,
};

static fgb__InstructionProc *fgb__GetInstructionProcedure(const fgbInstructionType type) {
	if (type < FGB_ARRAYCOUNT(fgb__instructionProcedures))
		return fgb__instructionProcedures[type];
	return NULL;
}

static bool fgb__ExecuteInstruction(fgbSystem *system) {
	fgbCPU *cpu = &system->cpu;

	fgbInstructionRegister *insReg = &cpu->instructionRegister;

	const fgbInstruction *instruction = &insReg->instruction;

	const char *instructionName = fgbGetInstructionName(instruction->type);

	fgb__InstructionProc *proc = fgb__GetInstructionProcedure(instruction->type);
	if (proc == NULL) {
		FGB__ERROR(system, fgb__KindName_CPU, "No procedure found for instruction '%s', op-code: $%02X", instructionName, insReg->opcode);
		return false;
	}

	if (!proc(system, insReg)) {
		FGB__ERROR(system, fgb__KindName_CPU, "Execution failed for instruction '%s', op-code: $%02X", instructionName, insReg->opcode);
		return false;
	}

	return true;
}

static bool fgb__FetchInstruction(fgbSystem *system) {
	if (system == NULL) {
		FGB__ERROR(system, fgb__KindName_CPU, "Invalid fetch arguments");
		return false;
	}

	fgbCPU *cpu = &system->cpu;

	uint16_t startAddress = system->cpu.registers.pc;

	uint16_t pc = system->cpu.registers.pc;

	uint8_t firstOpCode = fgb__BusRead8(system, pc, true);

	system->cpu.registers.pc++;

	if (cpu->state.skipPC) {
		cpu->state.skipPC = false;
		system->cpu.registers.pc--;
	}

	const fgbInstruction *instruction = &fgb__instructionTable[firstOpCode];

	if (instruction == NULL || instruction->type == fgbInstructionType_None) {
		FGB__ERROR(system, fgb__KindName_CPU, "No instruction for opcode '%02X' found", firstOpCode);
		return false;
	}

	bool isPrefixInstruction = false;
	uint8_t secondOpCode = 0;
	if (instruction->type == fgbInstructionType_PREFIX) {
		isPrefixInstruction = true;

		pc = system->cpu.registers.pc;

		secondOpCode = fgb__BusRead8(system, pc, true);

		system->cpu.registers.pc++;

		if (cpu->state.skipPC) {
			cpu->state.skipPC = false;
			system->cpu.registers.pc--;
		}

		instruction = &fgb__prefixInstructionTable[secondOpCode];

		if (instruction == NULL || instruction->type == fgbInstructionType_None) {
			FGB__ERROR(system, fgb__KindName_CPU, "No CB instruction for opcode '%02X' found", secondOpCode);
			return false;
		}
	}

	cpu->instructionRegister.instruction = *instruction;
	cpu->instructionRegister.opcode = isPrefixInstruction ? secondOpCode : firstOpCode;
	cpu->instructionRegister.isPrefixInstruction = isPrefixInstruction;

	return true;
}

static void fgb__SetFetchedValueU8(fgbDataRegister *data, const uint8_t immediate) {
	data->value.ulow = immediate;
	data->value.isWide = false;
	data->value.isSign = false;
	data->hasData = true;
}

static void fgb__SetFetchedValueS8(fgbDataRegister *data, const int8_t immediate) {
	data->value.slow = immediate;
	data->value.isWide = false;
	data->value.isSign = true;
	data->hasData = true;
}

static void fgb__SetFetchedValueU16(fgbDataRegister *data, const uint16_t immediate) {
	data->value.u16 = immediate;
	data->value.isWide = true;
	data->value.isSign = false;
	data->hasData = true;
}

static void fgb__SetFetchedTargetMemory(fgbDataRegister *data, const uint16_t address) {
	data->targetAddress = address;
	data->isMemoryTarget = true;
	data->hasData = true;
}

static void fgb__SetFetchedSourceMemory(fgbDataRegister *data, const uint16_t address) {
	data->sourceAddress = address;
	data->isMemoryTarget = false;
	data->hasData = true;
}

static void fgb__SetFetchedOffet(fgbDataRegister *data, const int8_t offset) {
	data->offset = offset;
	data->hasData = true;
}

static bool fgb__FetchData(fgbSystem *system) {
	const char *sysName = FGB__SYSNAME_ADDON(fgb__KindName_CPU, "Fetch");

	fgbCPU *cpu = &system->cpu;

	fgbInstructionRegister *insReg = &cpu->instructionRegister;

	fgbDataRegister *data = &insReg->data;

	const fgbInstruction *instruction = &insReg->instruction;

	fgbCPURegisters *r = &cpu->registers;

	uint16_t pc = cpu->registers.pc;

	const fgbAddressingMode mode = instruction->mode;

	switch (mode) {
		case fgbAddressingMode_Implied:
			return true;

		case fgbAddressingMode_Constant:
			fgb__SetFetchedValueU16(data, instruction->value & 0xFFFF);
			return true;

		case fgbAddressingMode_Constant_Reg:
		{
			fgbRegisterType destReg = instruction->regA;

			uint8_t value = fgb__ReadRegister8(r, destReg);

			fgb__SetFetchedValueU8(data, value);

			return true;
		}

		case fgbAddressingMode_Constant_MemReg:
		{
			fgbRegisterType destReg = instruction->regA;

			FGB_ASSERT(fgb__IsRegister16Bit(destReg));

			uint16_t targetAddress = fgb__ReadRegister16(r, destReg);

			fgb__SetFetchedTargetMemory(data, targetAddress);

			uint8_t value = fgb__BusRead8(system, targetAddress, true);
			fgb__SetFetchedValueU8(data, value);

			return true;
		}

		case fgbAddressingMode_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			if (fgb__IsRegister16Bit(destReg)) {
				uint16_t value = fgb__ReadRegister16(r, destReg);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(destReg));

				uint8_t value = fgb__ReadRegister8(r, destReg);

				fgb__SetFetchedValueU8(data, value);
			}
			return true;
		}

		case fgbAddressingMode_Reg_Reg:
		{
			fgbRegisterType sourceReg = instruction->regB;
			if (fgb__IsRegister16Bit(sourceReg)) {
				uint16_t value = fgb__ReadRegister16(r, sourceReg);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

				uint8_t value = fgb__ReadRegister8(r, sourceReg);

				fgb__SetFetchedValueU8(data, value);
			}
			return true;
		}

		case fgbAddressingMode_Reg_RegOffsetI8:
		{
			fgbRegisterType sourceReg = instruction->regB;

			FGB_ASSERT(fgb__IsRegister16Bit(sourceReg));

			uint8_t offset = fgb__BusRead8(system, pc, true);

			r->pc++;

			uint16_t source = fgb__ReadRegister16(r, sourceReg);

			uint32_t tmp = source + (int8_t)offset;

			uint32_t value = tmp & 0xFFFF;

			fgb__SetFetchedValueU16(data, value);

			fgb__SetFetchedOffet(data, (int8_t)offset);

			return true;
		}

		case fgbAddressingMode_Reg_U16:
		case fgbAddressingMode_U16:
		{
			uint16_t value = fgb__BusRead16(system, pc, true);

			r->pc += 2;

			fgb__SetFetchedValueU16(data, value);

			return true;
		}

		case fgbAddressingMode_I8:
		case fgbAddressingMode_Reg_I8:
		case fgbAddressingMode_Reg_U8:
		{
			uint8_t value = fgb__BusRead8(system, pc, true);

			r->pc++;

			if (mode == fgbAddressingMode_Reg_U8)
				fgb__SetFetchedValueU8(data, value);
			else
				fgb__SetFetchedValueS8(data, value);

			return true;
		}

		case fgbAddressingMode_Reg_MemRegInc:
		case fgbAddressingMode_Reg_MemRegDec:
		case fgbAddressingMode_Reg_MemReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			FGB_ASSERT(fgb__IsRegister16Bit(sourceReg));

			uint16_t address = fgb__ReadRegister16(r, sourceReg);

			if (fgb__IsRegister16Bit(destReg)) {
				uint16_t value = fgb__BusRead16(system, address, true);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(destReg));

				uint8_t value = fgb__BusRead8(system, address, true);

				fgb__SetFetchedValueU8(data, value);
			}

			if (mode == fgbAddressingMode_Reg_MemRegDec) {
				FGB_ASSERT(sourceReg == fgbRegisterType_HL);
				fgb__WriteRegister16(r, sourceReg, address - 1);
			} else if (mode == fgbAddressingMode_Reg_MemRegInc) {
				FGB_ASSERT(sourceReg == fgbRegisterType_HL);
				fgb__WriteRegister16(r, sourceReg, address + 1);
			}

			return true;
		}

		case fgbAddressingMode_MemA16_Reg:
		{
			fgbRegisterType sourceReg = instruction->regA;

			uint16_t address = fgb__BusRead16(system, pc, true);

			r->pc += 2;

			if (fgb__IsRegister16Bit(sourceReg)) {
				uint16_t sourceValue = fgb__ReadRegister16(r, sourceReg);

				fgb__SetFetchedValueU16(data, sourceValue);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

				uint8_t sourceValue = fgb__ReadRegister8(r, sourceReg);

				fgb__SetFetchedValueU8(data, sourceValue);
			}

			fgb__SetFetchedTargetMemory(data, address);

			return true;
		}

		case fgbAddressingMode_Reg_MemA16:
		{
			fgbRegisterType destReg = instruction->regA;

			uint16_t address = fgb__BusRead16(system, pc, true);

			fgb__SetFetchedSourceMemory(data, address);

			r->pc += 2;

			if (fgb__IsRegister16Bit(destReg)) {
				uint16_t value = fgb__BusRead16(system, address, true);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(destReg));

				uint8_t value = fgb__BusRead8(system, address, true);

				fgb__SetFetchedValueU8(data, value);
			}

			return true;
		}

		case fgbAddressingMode_Reg_MemConstantOffsetA8:
		{
			fgbRegisterType destReg = instruction->regA;

			uint8_t addressAddon = fgb__BusRead8(system, pc, true);

			r->pc++;

			uint16_t constant = instruction->value & 0xFFFF;

			uint16_t address = constant | addressAddon;

			fgb__SetFetchedSourceMemory(data, address);

			if (fgb__IsRegister16Bit(destReg)) {
				uint16_t value = fgb__BusRead16(system, address, true);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(destReg));

				uint8_t value = fgb__BusRead8(system, address, true);

				fgb__SetFetchedValueU8(data, value);

				return true;
			}

			return true;
		}

		case fgbAddressingMode_MemConstantOffsetA8_Reg:
		{
			fgbRegisterType sourceReg = instruction->regA;

			uint8_t addressAddon = fgb__BusRead8(system, pc, true);

			r->pc++;

			if (fgb__IsRegister16Bit(sourceReg)) {
				uint16_t sourceValue = fgb__ReadRegister16(r, sourceReg);

				fgb__SetFetchedValueU16(data, sourceValue);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

				uint8_t sourceValue = fgb__ReadRegister8(r, sourceReg);

				fgb__SetFetchedValueU8(data, sourceValue);
			}

			uint16_t constant = instruction->value & 0xFFFF;

			uint16_t address = constant | addressAddon;

			fgb__SetFetchedTargetMemory(data, address);

			return true;
		}

		case fgbAddressingMode_MemReg:
		{
			// NOTE: Instructions such INC (HL), DEC (HL) are 8-bit operations, so we dont use the register to decide the data-length (8 or 16 bit)

			fgbRegisterType destReg = instruction->regA;

			uint16_t address = fgb__ReadRegister16(r, destReg);

			fgb__SetFetchedTargetMemory(data, address);

			if (instruction->dataLength == 16) {
				uint16_t value = fgb__BusRead16(system, address, true);

				fgb__SetFetchedValueU16(data, value);
			} else {
				uint8_t value = fgb__BusRead8(system, address, true);

				fgb__SetFetchedValueU8(data, value);
			}

			return true;
		}

		case fgbAddressingMode_MemReg_U8:
		{
			fgbRegisterType addressReg = instruction->regA;

			FGB_ASSERT(fgb__IsRegister16Bit(addressReg));

			uint16_t targetAddress = fgb__ReadRegister16(r, addressReg);

			fgb__SetFetchedTargetMemory(data, targetAddress);

			uint8_t value = fgb__BusRead8(system, pc, true);

			r->pc++;

			fgb__SetFetchedValueU8(data, value);

			return true;
		}

		case fgbAddressingMode_MemReg_Reg:
		case fgbAddressingMode_MemRegDec_Reg:
		case fgbAddressingMode_MemRegInc_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			uint16_t address = fgb__ReadRegister16(r, destReg);

			if (fgb__IsRegister16Bit(sourceReg)) {
				uint16_t sourceValue = fgb__ReadRegister16(r, sourceReg);

				fgb__SetFetchedValueU16(data, sourceValue);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

				uint8_t sourceValue = fgb__ReadRegister8(r, sourceReg);
				fgb__SetFetchedValueU8(data, sourceValue);
			}

			if (mode == fgbAddressingMode_MemRegDec_Reg) {
				fgb__WriteRegister16(r, destReg, address - 1);
			} else if (mode == fgbAddressingMode_MemRegInc_Reg) {
				fgb__WriteRegister16(r, destReg, address + 1);
			}

			fgb__SetFetchedTargetMemory(data, address);

			return true;
		}

		case fgbAddressingMode_Reg_MemConstantOffsetReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			uint16_t startAddress = instruction->value & 0xFFFF;

			FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

			uint8_t offset = fgb__ReadRegister8(r, sourceReg);

			uint16_t sourceAddress = startAddress + offset;
			fgb__SetFetchedSourceMemory(data, sourceAddress);

			if (fgb__IsRegister16Bit(destReg)) {
				uint16_t value = fgb__BusRead16(system, sourceAddress, true);

				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(destReg));

				uint8_t value = fgb__BusRead8(system, sourceAddress, true);

				fgb__SetFetchedValueU8(data, value);
			}

			return true;
		}

		case fgbAddressingMode_MemConstantOffsetReg_Reg:
		{
			fgbRegisterType offsetReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			uint16_t startAddress = instruction->value & 0xFFFF;

			uint8_t offset = fgb__ReadRegister8(r, offsetReg);

			uint16_t targetAddress = startAddress + offset;
			fgb__SetFetchedTargetMemory(data, targetAddress);

			if (fgb__IsRegister16Bit(sourceReg)) {
				uint16_t value = fgb__ReadRegister16(r, sourceReg);
				fgb__SetFetchedValueU16(data, value);
			} else {
				FGB_ASSERT(fgb__IsRegister8Bit(sourceReg));

				uint8_t value = fgb__ReadRegister8(r, sourceReg);
				fgb__SetFetchedValueU8(data, value);
			}

			return true;
		}

		default:
			FGB__ERROR(system, sysName, "Addressing mode '%s' is not implemented, instruction '$%02X %s'", fgbGetAddressingModeName(mode), insReg->opcode, fgbGetInstructionName(insReg->instruction.type));
			return false;
	}
}

// ********************************************************************************************************************
// Interrupt Implementation
// ********************************************************************************************************************
#define FGB__INTERRUPT_ADDRESS_VERTICAL_BLANK 0x40
#define FGB__INTERRUPT_ADDRESS_LCD_STATUS 0x48
#define FGB__INTERRUPT_ADDRESS_TIMER 0x50
#define FGB__INTERRUPT_ADDRESS_SERIAL 0x58
#define FGB__INTERRUPT_ADDRESS_JOYPAD 0x60

static const uint16_t fgb__InterruptVectorTable[0x100] = {
	[fgbInterruptType_VerticalBlank] = FGB__INTERRUPT_ADDRESS_VERTICAL_BLANK,
	[fgbInterruptType_LCDStatus] = FGB__INTERRUPT_ADDRESS_LCD_STATUS,
	[fgbInterruptType_Timer] = FGB__INTERRUPT_ADDRESS_TIMER,
	[fgbInterruptType_Serial] = FGB__INTERRUPT_ADDRESS_SERIAL,
	[fgbInterruptType_Joypad] = FGB__INTERRUPT_ADDRESS_JOYPAD,
};

static const char *fgb__InterruptNameTable[0x100] = {
	[fgbInterruptType_VerticalBlank] = "Vertical Blank",
	[fgbInterruptType_LCDStatus] = "LCD Status",
	[fgbInterruptType_Timer] = "Timer",
	[fgbInterruptType_Serial] = "Serial",
	[fgbInterruptType_Joypad] = "Joypad",
};

static void fgb__InterruptsInit(fgbInterrupts *ir) {
	fgbClearStruct(ir);

	ir->enable.u8 = 0;
	ir->request.u8 = 0;
	ir->isMasterEnabled = false;

	ir->ticksEnableIME = 0;
	ir->ticksRequestInterruptDelay = 0;
}

static void fgb__InterruptRequest(fgbSystem *system, const fgbInterruptType type, const char *reason) {
	fgbInterrupts *ir = &system->interrupts;
	if ((ir->request.u8 & type) == 0) {
#if FGB_INTERRUPT_LOGGING
		FGB__DEBUG(system, fgb__KindName_Core, "-> Interrupt Request '%u' -> %s", type, reason);
#endif
		ir->request.u8 |= type;
	}
	if (type == fgbInterruptType_VerticalBlank) {
		// NOTE(final): VBlank requests require 4 ticks to be visible as pending interrupt
		ir->ticksRequestInterruptDelay = 4;
	}
}

static bool fgb__InterruptServe(fgbSystem *system, const fgbInterruptType type) {
	const uint16_t vector = fgb__InterruptVectorTable[type];

	uint16_t pc = system->cpu.registers.pc;
	if (!fgb__StackPush16(system, pc)) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Interrupts, "Failed pushing PC '$%04X' to stack", pc);
		return false;
	}

	system->cpu.registers.pc = vector;

	system->interrupts.request.u8 &= ~type;

	system->cpu.state.type = fgbCPUStateType_Normal;

	system->interrupts.isMasterEnabled = false;

	return true;
}

static fgbInterruptType fgb__InterruptPending(const fgbInterrupts *ir) {
	fgbInterruptsFlags flags = { .u8 = ir->request.u8 & ir->enable.u8 & 0b11111 };
	if (flags.u8 == 0) {
		return fgbInterruptType_None;
	}
	if (flags.vblank && ir->ticksRequestInterruptDelay == 0) {
		return fgbInterruptType_VerticalBlank;
	} else if (flags.stat) {
		return fgbInterruptType_LCDStatus;
	} else if (flags.timer) {
		return fgbInterruptType_Timer;
	} else if (flags.serial) {
		return fgbInterruptType_Serial;
	} else if (flags.joypad) {
		return fgbInterruptType_Joypad;
	} else {
		return fgbInterruptType_None;
	}
}

static void fgb__InterruptsWrite(fgbSystem *system, const uint16_t address, const uint8_t value) {
	fgbInterrupts *ir = &system->interrupts;
	switch (address) {
		case FGB__BUS_ADDRESS_INTERRUPT_REQUEST_REGISTER:
			ir->request.u8 = value;
			break;
		case FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER:
			ir->enable.u8 = value;
			break;
		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_IO, "Unsupported interrupt write of '%02X' to address '%04X'", value, address);
			break;
	}
}

static uint8_t fgb__InterruptsRead(fgbSystem *system, const uint16_t address) {
	const fgbInterrupts *ir = &system->interrupts;
	switch (address) {
		case FGB__BUS_ADDRESS_INTERRUPT_REQUEST_REGISTER:
			return ir->request.u8;
		case FGB__BUS_ADDRESS_INTERRUPT_ENABLE_REGISTER:
			return ir->enable.u8;
		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_IO, "Unsupported interrupt read from address '%04X'", address);
			return 0xFF;
	}
}

static void fgb__HandleInterruptTicks(fgbInterrupts *ir, const bool wasServed) {
	// NOTE(final): Decrease delay pending request ticks for interrupt types such as VBlank
	if (!wasServed && ir->ticksRequestInterruptDelay > 0) {
		FGB_ASSERT((ir->ticksRequestInterruptDelay % 4) == 0);
		ir->ticksRequestInterruptDelay -= 4;
	}

	// NOTE(final): Enable master interrupt, when master enable pending ticks reaches zero
	if (!wasServed && ir->ticksEnableIME > 0) {
		FGB_ASSERT((ir->ticksEnableIME % 4) == 0);
		ir->ticksEnableIME -= 4;
		if (ir->ticksEnableIME == 0) {
			ir->isMasterEnabled = true;
		}
	}
}

// ********************************************************************************************************************
// Core Implementation
// ********************************************************************************************************************
FGB_API bool fgbPause(fgbSystem *system) {
	if (system == NULL || system->state == fgbEmulationState_Error) {
		return false;
	}

	system->state = fgbEmulationState_Paused;

	return true;
}

FGB_API bool fgbStep(fgbSystem *system) {
	if (system == NULL || system->state == fgbEmulationState_Error) {
		return false;
	}

	system->state = fgbEmulationState_Step;

	return true;
}

FGB_API bool fgbMicroStep(fgbSystem *system) {
	if (system == NULL || system->state == fgbEmulationState_Error) {
		return false;
	}

	system->state = fgbEmulationState_MicroStep;

	return true;
}

FGB_API bool fgbResume(fgbSystem *system) {
	if (system == NULL || system->state == fgbEmulationState_Running) {
		return false;
	}

	system->state = fgbEmulationState_Running;

	return false;
}

typedef enum {
	fgb__PrintInstructionFlags_None = 0,
	fgb__PrintInstructionFlags_Console = 1 << 0,
	fgb__PrintInstructionFlags_File = 1 << 1,
	fgb__PrintInstructionFlags_ExcludeFlags = 1 << 2,
} fgb__PrintInstructionFlags;

static void fgb__PrintCurrentInstruction(fgbSystem *system, fgbInstructionRegister *insReg, const fgbCPURegisters *regs, const uint32_t frameIndex, const char *prefix, const fgb__PrintInstructionFlags flags) {
	const char *instructionName = fgbGetInstructionName(insReg->instruction.type);

	uint16_t startPC = insReg->startPC;
	fgbTickCycles startCycles = insReg->startTicks;

	bool withoutFlags = (flags & fgb__PrintInstructionFlags_ExcludeFlags) != 0;

	bool conOut = (flags & fgb__PrintInstructionFlags_Console) != 0;

	bool fileOut = (flags & fgb__PrintInstructionFlags_File) != 0;

	static char instructionText[256] = { 0 };

	FGB_MEMSET(instructionText, 0, sizeof(instructionText));
	if (!fgb__FormatInstruction(system, insReg, instructionText, FGB_ARRAYCOUNT(instructionText))) {
		fgb__StringFormat(instructionText, FGB_ARRAYCOUNT(instructionText), "%s", instructionName);
	}

	uint8_t byte0 = fgb__BusRead8_Direct(system, startPC + 0);
	uint8_t byte1 = fgb__BusRead8_Direct(system, startPC + 1);
	uint8_t byte2 = fgb__BusRead8_Direct(system, startPC + 2);
	uint8_t byte3 = fgb__BusRead8_Direct(system, startPC + 3);

	char flagsStr[5];
	flagsStr[0] = regs->f.zeroFlag ? 'Z' : '-';
	flagsStr[1] = regs->f.negativeFlag ? 'N' : '-';
	flagsStr[2] = regs->f.halfCarryFlag ? 'H' : '-';
	flagsStr[3] = regs->f.fullCarryFlag ? 'C' : '-';
	flagsStr[4] = 0;

	static char outputBuffer[512] = { 0 };

	FGB_MEMSET(outputBuffer, 0, sizeof(outputBuffer));

	fgb__StringFormat(
		outputBuffer, FGB_ARRAYCOUNT(outputBuffer),
		"[%2u] [%8zu] [%04X] (%02X) %-16s {Flags: %s} {Regs: AF = %02X, %02X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, PC: %04X}",
		frameIndex, startCycles, startPC, insReg->opcode, instructionText, flagsStr, regs->a, regs->f.flags, regs->bc, regs->de, regs->hl, regs->sp, regs->pc);

	if (!withoutFlags) {
	} else {
		fgb__StringFormat(
			outputBuffer, FGB_ARRAYCOUNT(outputBuffer),
			"%s %04X: %-24s (%02X %02X %02X %02X)",
			prefix, startPC, instructionText, byte0, byte1, byte2, byte3);
	}

	if (conOut) {
		FGB__DEBUG(system, fgb__KindName_Core, outputBuffer);
	}
}

static fgbRegisterType GetRegisterTypeFromName(const char *name) {
	for (int i = fgbRegisterType_A; i < FGB_REGISTER_TYPE_COUNT; ++i) {
		const char *regName = fgb__registerToNameTable[i];
		if (strcmp(regName, name) == 0) {
			return (fgbRegisterType)i;
		}
	}
	return fgbRegisterType_None;
}

static void fgb__HWTick4(fgbSystem *system) {
	fgbMemoryCycles memoryCycles = 1;
	fgbTickCycles tickCycles = 4;

	// DMA always takes 4 cpu machine cycles (Comes first, before any other system!)
	fgb__MicroStep(system, fgbMicroStepType_DMA);
	if (!fgb__DMATick(system)) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed updating DMA");
		return;
	}

	for (fgbTickCycles tickCycle = 0; tickCycle < tickCycles; ++tickCycle) {
		fgb__MicroStep(system, fgbMicroStepType_CPUTick);

		for (fgbMemoryCycles memoryCycle = 0; memoryCycle < memoryCycles; ++memoryCycle) {
			fgb__MicroStep(system, fgbMicroStepType_HardwareTick);

			fgb__MicroStep(system, fgbMicroStepType_Timer);
			if (!fgb__TimerTick(system)) {
				FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed updating timer");
				return;
			}

			fgb__MicroStep(system, fgbMicroStepType_PPU);
			if (!fgb__PPUTick(system)) {
				FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed updating PPU");
				return;
			}

			fgb__MicroStep(system, fgbMicroStepType_APU);
			if (!fgb__APUTick(system)) {
				FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed updating APU");
				return;
			}

			system->cpu.state.currentMemoryCycles++;
		}

		system->cpu.state.totalTickCycles++;
	}

	
}

FGB_API fgbTestResultType fgbRunTest(const fgbCallbacks *callbacks, const fgbGamePak *gamePak, const uint64_t maxTickCount, fgbRunTestFunc *func, fgbTestResultData *data) {
	if (callbacks == NULL || gamePak == NULL || data == NULL) {
		return fgbTestResultType_InvalidArguments;
	}

	if (!gamePak->isValid) {
		return fgbTestResultType_InvalidGamePak;
	}

	fgbClearStruct(data);

	fgbSystem *system = (fgbSystem *)fgb__AllocateMemory(callbacks, sizeof(fgbSystem));
	if (system == NULL) {
		return fgbTestResultType_OutOfMemory;
	}

	fgbTestResultType result = fgbTestResultType_Success;

	fgbConfiguration config = { 0 };
	config.isScreenDisabled = true;

	fgbInitResult initRes = fgbInit(system, &config, gamePak);

	uint64_t tickCount = 0;

	if (initRes == fgbInitResult_InvalidArguments) {
		result = fgbTestResultType_InvalidArguments;
		goto done;
	} else if (initRes == fgbInitResult_InvalidGamePak) {
		result = fgbTestResultType_InvalidGamePak;
		goto done;
	}

	bool testCompleted = false;

	while (!testCompleted && ((maxTickCount == 0) || (tickCount < maxTickCount))) {
		++tickCount;

		if (func != NULL) {
			if (!func(system, data)) {
				result = fgbTestResultType_FailedProcessingTest;
				goto done;
			}
		}

		if (data->finished) {
			testCompleted = true;
			break;
		}

		if (!fgbTick(system)) {
			const char *message = NULL;
			switch (system->error.type) {
				case fgbErrorType_InfiniteLoop:
					message = "Infinite Loop Detected";
					break;

				case fgbErrorType_ExecutionError:
					message = system->error.message;
					break;
			}

			if (data->outputLen == 0) {
				fgb__StringCopy(message, data->output, FGB_ARRAYCOUNT(data->output));
				data->outputLen = FGB_STRLEN(data->output);
			}

			result = fgbTestResultType_EmulationFailed;
			goto done;
		}
	}

	if (data->success) {
		result = fgbTestResultType_Success;
	} else {
		result = fgbTestResultType_NotPassed;
	}

done:
	data->cpuCycles = system->cpu.state.totalTickCycles;
	data->tickCycles = tickCount;

	fgbShutdown(system);

	fgb__FreeMemory(callbacks, system);

	return result;
}

static void fgb__SetupDMG(fgbCPURegisters *regs) {
	regs->pc = 0x100;
	regs->sp = 0xfffe;

	regs->a = 0x01;
	regs->f.flags = 0xb0;

	regs->b = 0x00;
	regs->c = 0x13;

	regs->d = 0x00;
	regs->e = 0xd8;

	regs->h = 0x01;
	regs->l = 0x4d;
}

static void fgb__InitExternalRAM(fgbSystem *system, fgbGamePak *gamePak) {
	// Clear external RAM
	bool hasExternalRAM = gamePak->ram.memory.length > 0;
	if (hasExternalRAM) {
		FGB_MEMSET(gamePak->ram.memory.data, 0, gamePak->ram.memory.length);
		gamePak->ram.isDirty = false;
		gamePak->ram.requestSave = false;
		gamePak->ram.lastSaveTime = 0;
	}

	// Try to load the external RAM from the file
	bool supportsBattery = gamePak->info.features & fgbGamePakFeature_BATTERY;
	if (hasExternalRAM && supportsBattery) {
		fgb__ExternalRAMLoad(system, gamePak);
		gamePak->ram.isDirty = false;
		gamePak->ram.requestSave = false;
		gamePak->ram.lastSaveTime = FGB_CURRENT_TICKS();
	}
}

static void fgb__PowerOn(fgbSystem *system, const bool isScreenEnabled, const uint32_t sampleRate) {
	FGB__INFO(system, fgb__KindName_Core, "Power On");
	FGB__INFO(system, fgb__KindName_Core, "");

	fgbCPU *cpu = &system->cpu;
	fgbBoot *boot = &system->boot;
	fgbCPURegisters *regs = &cpu->registers;
	fgbIORegisters *io = &system->io;
	fgbTimer *timer = &system->timer;
	fgbSerial *serial = &system->serial;
	fgbInterrupts *interrupts = &system->interrupts;
	fgbInstructionRegister *insReg = &cpu->instructionRegister;
	fgbPPU *ppu = &system->ppu;
	fgbJoypadState *joypad = &system->joypad;
	fgbAPU *apu = &system->apu;
	fgbGamePak *gamePak = &system->gamePak;

	// Reset structs
	fgbClearStruct(regs);
	fgbClearStruct(cpu);
	fgbClearStruct(timer);
	fgbClearStruct(serial);
	fgbClearStruct(interrupts);
	fgbClearStruct(insReg);
	fgbClearStruct(ppu);
	fgbClearStruct(joypad);
	fgbClearStruct(apu);

	fgb__TimerInit(timer);

	fgb__PPUInit(system, isScreenEnabled);

	fgb__APUInit(system, sampleRate);

	fgb__MBCInit(system, gamePak, &system->mbc);

	fgb__InitExternalRAM(system, gamePak);

	fgb__InterruptsInit(interrupts);

	system->coreType = fgbCoreType_DMG;

	if (boot->rom.isEnabled) {
		boot->state.isActive = true;
		boot->state.reg = 0;

		// NOTE(final): CPU register already cleared
	} else {
		boot->state.isActive = false;
		boot->state.reg = 0;

		fgb__SetupDMG(regs);
	}
}

static void fgb__ExecuteReset(fgbSystem *system, const fgbResetState resetState) {
	FGB_ASSERT(resetState != fgbResetState_None);

	FGB__INFO(system, fgb__KindName_Core, "Reset");
	FGB__INFO(system, fgb__KindName_Core, "");

	fgb__PowerOn(system, system->ppu.state.isDisplayEnabled, system->apu.state.sampleRate);

	system->ppu.state.isFrameFinished = true;
	system->ppu.state.isVRAMUpdated = true;
	system->ppu.state.isBackgroundMapUpdated = true;

	if (resetState == fgbResetState_Pause) {
		system->state = fgbEmulationState_Paused;
	} else {
		system->state = fgbEmulationState_Running;
	}

	system->resetState = fgbResetState_None;
}

static bool fgb__FetchDecodeExecute(fgbSystem *system, const uint32_t startFrameIndex, const fgbCPURegisters *startRegs) {
	fgbCPU *cpu = &system->cpu;
	fgbPPU *ppu = &system->ppu;
	fgbCPURegisters *regs = &cpu->registers;
	fgbInstructionRegister *insReg = &cpu->instructionRegister;
	fgbLog *log = &system->log;
	fgbGamePak *gamePak = &system->gamePak;

	//
	// Fetch Instruction
	// 
	if (!fgb__FetchInstruction(system)) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed to fetch instruction at address '$%04X'", insReg->startPC);
		return false;
	}

	//
	// Fetch Data
	// 
	const char *instructionName = fgbGetInstructionName(insReg->instruction.type);
	const char *addressingModeName = fgbGetAddressingModeName(insReg->instruction.mode);

	uint16_t fetchAddress = cpu->registers.pc;
	if (!fgb__FetchData(system)) {
		fgb__PrintCurrentInstruction(system, insReg, startRegs, startFrameIndex, "FETCH", fgb__PrintInstructionFlags_Console);
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed to fetch data for instruction '%s', mode '%s' at address '$%04X'", instructionName, addressingModeName, fetchAddress);
		return false;
	}

#if FGB_CPU_INSTRUCTION_TICK_LOGGING
	fgb__PrintCurrentInstruction(system, insReg, startRegs, startFrameIndex, "", fgb__PrintInstructionFlags_Console);
#endif

	//
	// Execute
	//
	if (!fgb__ExecuteInstruction(system)) {
		fgb__PrintCurrentInstruction(system, insReg, startRegs, startFrameIndex, "EXEC", fgb__PrintInstructionFlags_Console);
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed to execute instruction '%s', mode '%s'", instructionName, addressingModeName);
		return false;
	}

	// In the instruction are machine cycles defined, 1 equals 4 cpu cycles, so we multiply it by 4
	fgbTickCycles branchCycles = insReg->instruction.branchCycles * 4;
	fgbTickCycles normalCycles = insReg->instruction.normalCycles * 4;
	fgbTickCycles minCycles = FGB_MIN(branchCycles, normalCycles);
	fgbTickCycles maxCycles = FGB_MAX(branchCycles, normalCycles);

	fgbTickCycles cyclesAdded = cpu->state.totalTickCycles - insReg->startTicks;

	// Match cycles
	if (cyclesAdded < minCycles) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Expect at least %uu cycles, but got %uu in instruction '%s', mode '%s'", minCycles, cyclesAdded, instructionName, addressingModeName);
		return false;
	}
	if (cyclesAdded > maxCycles) {
		FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Expect no more than %uu cycles, but got %uu in instruction '%s', mode '%s'", maxCycles, cyclesAdded, instructionName, addressingModeName);
		return false;
	}
	if (cpu->instructionRegister.wasBranchTaken) {
		if (cyclesAdded != branchCycles) {
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Expect exactly %uu cycles for a jump, but got %uu in instruction '%s', mode '%s'", branchCycles, cyclesAdded, instructionName, addressingModeName);
			return false;
		}
	} else {
		if (cyclesAdded != normalCycles) {
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Expect exactly %uu cycles for a normal instruction, but got %uu in instruction '%s', mode '%s'", normalCycles, cyclesAdded, instructionName, addressingModeName);
			return false;
		}
	}

	return true;
}

static void fgb__PPUResetForTick(fgbPPU *ppu) {
	fgbPPUState *state = &ppu->state;

	// Do not touch any data/registers here, otherwise we get in trouble!

	// Reset frame states, if needed
	if (state->isFrameFinished) {
		state->isFrameFinished = false;
		state->lineTicks = 0;
		state->frameTicks = 0;
	}

	// Reset video ram state, if needed
	if (state->isVRAMUpdated) {
		state->isVRAMUpdated = false;
	}

	// Reset background map state, if needed
	if (state->isBackgroundMapUpdated) {
		state->isBackgroundMapUpdated = false;
	}

}

FGB_API bool fgbTick(fgbSystem *system) {
	if (system == NULL || (!system->gamePak.isValid && !system->boot.state.isActive)) {
		return false;
	}

	//
	// Reset system, if requested from the outside to prevent access to memory while running, etc.
	//
	if (system->resetState != fgbResetState_None) {
		fgb__ExecuteReset(system, system->resetState);
		return true;
	}

	//
	// State Switch
	//
	switch (system->state) {
		case fgbEmulationState_Error:
			return false;

		case fgbEmulationState_Paused:
			return true;

		case fgbEmulationState_Step:
			system->state = fgbEmulationState_Paused;
			break;

		case fgbEmulationState_Breakpoint:
		case fgbEmulationState_MicroStep:
			system->state = fgbEmulationState_Step;
			break;

		default:
			break;
	}

	fgbCPU *cpu = &system->cpu;
	fgbPPU *ppu = &system->ppu;
	fgbCPURegisters *regs = &cpu->registers;
	fgbInstructionRegister *insReg = &cpu->instructionRegister;
	fgbLog *log = &system->log;
	fgbGamePak *gamePak = &system->gamePak;
	fgbError *error = &system->error;
	fgbJoypadState *joypad = &system->joypad;
	fgbInterrupts *ir = &system->interrupts;

	// Reset any error
	error->type = fgbErrorType_None;
	error->message[0] = 0;

	//
	// Joypad State Change
	//
	if (joypad->isStateChanged) {
		joypad->isStateChanged = false;
		joypad->currentState = joypad->requestedState;
		fgb__JoypadInterrupt(system, ir, joypad);
	}

	//
	// Reset instruction register and cycles
	// Reset PPU States, such as isFrameFinished, etc.
	// Remember stuff for debug purposes
	//
	fgbClearStruct(insReg);
	insReg->startPC = regs->pc;
	insReg->startTicks = cpu->state.totalTickCycles;

	cpu->state.currentMemoryCycles = 0;

	uint32_t startFrameIndex = ppu->state.frameCount;
	fgbCPURegisters startRegs = cpu->registers;

	fgb__PPUResetForTick(ppu);

	bool interruptWasServed = false;

	//
	// Leave HALT mode
	//
	if (cpu->state.type == fgbCPUStateType_Halt) {
		// Decrease timer to leave HALT mode
		if (cpu->state.ticksLeaveHaltMode > 0) {
			FGB_ASSERT((cpu->state.ticksLeaveHaltMode % 4) == 0);
			cpu->state.ticksLeaveHaltMode -= 4;

			if (cpu->state.ticksLeaveHaltMode == 0) {
				cpu->state.type = fgbCPUStateType_Normal;
			}
		}

		// Start timer to that will leave HALT mode
		if ((cpu->state.type == fgbCPUStateType_Halt) && (fgb__InterruptPending(ir) != fgbInterruptType_None) && (cpu->state.ticksLeaveHaltMode == 0)) {
			cpu->state.ticksLeaveHaltMode = 12;
		}
	}

	//
	// Process CPU state
	//
	switch (cpu->state.type) {
		case fgbCPUStateType_Normal: {
			// Serve interrupt when: IME is enabled, a pending interrupt was found
			fgbInterruptType pendingInterrupt = fgb__InterruptPending(ir);
			if (ir->isMasterEnabled && (pendingInterrupt != fgbInterruptType_None)) {
				if (!fgb__InterruptServe(system, pendingInterrupt)) {
					const char *interruptName = fgb__InterruptNameTable[pendingInterrupt];
					FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Failed serve interrupt type '%s'", interruptName);
					return false;
				}
				interruptWasServed = true;
			} else {
				// Fetch -> Decode -> Execute
				if (!fgb__FetchDecodeExecute(system, startFrameIndex, &startRegs)) {
					FGB_ASSERT(error->type != fgbErrorType_None);
					return false;
				}
			}
		} break;

		case fgbCPUStateType_Halt: {
			// NOTE(final): HALT can only be exit due to any interrupt request and won't exit immediatelly, it takes at least 12 cycles
			fgb__HWTick4(system);
		} break;

		case fgbCPUStateType_Stop: {
			// TODO(final): Implement this correctly!
		} break;

		default:
			FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, "Unsupported CPU State: %d", system->cpu.state.type);
			return false;
	}

	// Handle enable of IME or decrease pending interrupt cycles
	fgb__HandleInterruptTicks(ir, interruptWasServed);

	// Put the machine to a faulted state, when there is an infinite loop detected and track the error
	if ((cpu->state.lastPC == regs->pc && cpu->state.lastSP == regs->sp) && (!system->interrupts.isMasterEnabled && !system->interrupts.enable.u8)) {
		FGB__Failure(system, fgbErrorType_InfiniteLoop, fgb__KindName_Core, "Infinity Loop Detected");
		return false;
	}

	// Remember last PC and SP, so we can detect infinite loops
	cpu->state.lastPC = regs->pc;
	cpu->state.lastSP = regs->sp;

	//
	// Save external RAM, if needed
	//
	bool ramSaveRequest = gamePak->ram.requestSave;
	bool hasRAM = gamePak->ram.memory.length > 0;
	bool hasBattery = gamePak->info.features & fgbGamePakFeature_BATTERY;
	bool ramWasChanged = gamePak->ram.isDirty;
	bool ramSaveTimeExceeded = (FGB_CURRENT_TICKS() - gamePak->ram.lastSaveTime) >= FGB__EXTERNAL_RAM_SAVE_TIME_DELTA_MS;
	if (ramSaveRequest && hasRAM && hasBattery && ramWasChanged && ramSaveTimeExceeded) {
		gamePak->ram.requestSave = false;
		gamePak->ram.isDirty = false;
		gamePak->ram.lastSaveTime = FGB_CURRENT_TICKS();
		fgb__ExternalRAMSave(system, gamePak);
	}

	return true;
}

static bool fgb__MemoryIsZero(const uint8_t *mem, const size_t size) {
	bool notZero = false;
	for (size_t i = 0; i < size; ++i) {
		if (mem[i]) {
			notZero |= true;
			break;
		}
	}
	return !notZero;
}

//
// Test all very important structs and registers, so values and bit fields matches the specs
//
static bool fgb__CoreTest(fgbSystem *system) {
	char errorStr[64] = { 0 };

	bool result = false;

#define CORE_TEST_ASSERT(exp, format, ...) { if (!(exp)) { FGB__Failure(system, fgbErrorType_ExecutionError, fgb__KindName_Core, format, ## __VA_ARGS__); goto failed; } }

	//
	// fgbValue
	//
	{
		fgbValue value = { 0 };
		value.u16 = 0; value.ulow = 42;
		CORE_TEST_ASSERT(value.ulow == 42 && value.uhigh == 0, "fgbValue U8 Low failed");
		value.u16 = 0; value.uhigh = 42;
		CORE_TEST_ASSERT(value.ulow == 0 && value.uhigh == 42, "fgbValue U8 High failed");
		value.u16 = 0; value.slow = -33;
		CORE_TEST_ASSERT(value.slow == -33 && value.shigh == 0, "fgbValue S8 Low failed");
		value.u16 = 0; value.shigh = -33;
		CORE_TEST_ASSERT(value.slow == 0 && value.shigh == -33, "fgbValue S8 High failed");

		value.u16 = 0; value.u16 = 0x00CD;
		CORE_TEST_ASSERT(value.ulow == 0xCD && value.uhigh == 0, "fgbValue U16 failed");
		value.u16 = 0; value.u16 = 0xFEDC;
		CORE_TEST_ASSERT(value.ulow == 0xDC && value.uhigh == 0xFE, "fgbValue U16 failed");

		value.u16 = 0xFFFF; value.isWide = true;
		CORE_TEST_ASSERT(value.u16 == 0xFFFF && value.isWide, "fgbValue Wide failed");
		value.u16 = 0xABCD; value.isSign = true;
		CORE_TEST_ASSERT(value.u16 == 0xABCD && value.isSign, "fgbValue Sign failed");
	}

	//
	// Flags Register
	//
	{
		fgbFlagsRegister f = { 0 };
		f.flags = 0; f.zeroFlag = 1;
		CORE_TEST_ASSERT(f.flags == 0b10000000, "Flag Bit 0 failed");
		f.flags = 0; f.negativeFlag = 1;
		CORE_TEST_ASSERT(f.flags == 0b01000000, "Flag Bit 1 failed");
		f.flags = 0; f.halfCarryFlag = 1;
		CORE_TEST_ASSERT(f.flags == 0b00100000, "Flag Bit 2 failed");
		f.flags = 0; f.fullCarryFlag = 1;
		CORE_TEST_ASSERT(f.flags == 0b00010000, "Flag Bit 3 failed");
		f.flags = 0; f.empty = 0b1111;
		CORE_TEST_ASSERT(f.flags == 0b00001111, "Flag Bit 4-7 failed");
		f.flags = 0b11110000;
		CORE_TEST_ASSERT(f.zeroFlag == 1 && f.negativeFlag == 1 && f.halfCarryFlag == 1 && f.fullCarryFlag == 1 && f.empty == 0, "All Flags set failed");
		f.flags = 0b00001111;
		CORE_TEST_ASSERT(f.zeroFlag == 0 && f.negativeFlag == 0 && f.halfCarryFlag == 0 && f.fullCarryFlag == 0 && f.empty > 0, "All Flags NOT set failed");
	}

	//
	// CPURegister
	//
	{
		fgbCPURegisters reg = { 0 };

		reg = (fgbCPURegisters){ 0 }; reg.pc = 0xABCD;
		CORE_TEST_ASSERT(reg.pc == 0xABCD, "PC register failed");

		reg = (fgbCPURegisters){ 0 }; reg.sp = 0xFEDC;
		CORE_TEST_ASSERT(reg.sp == 0xFEDC, "SP register failed");

		reg = (fgbCPURegisters){ 0 }; reg.af = 0x42CE;
		CORE_TEST_ASSERT(reg.af == 0x42CE && reg.a == 0x42 && reg.f.flags == 0xCE, "AF register failed");

		reg = (fgbCPURegisters){ 0 }; reg.a = 0x42;
		CORE_TEST_ASSERT(reg.a == 0x42 && reg.f.flags == 0, "A register failed");

		reg = (fgbCPURegisters){ 0 }; reg.f.flags = 0xB2;
		CORE_TEST_ASSERT(reg.f.flags == 0xB2 && reg.a == 0, "F register failed");
	}

	//
	// LCDC
	//
	{
		fgbLCDControl lcdc = { 0 };
		lcdc.u8 = 0; lcdc.backgroundEnabled = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00000001, "LCDC Bit 0 failed");
		lcdc.u8 = 0; lcdc.objEnable = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00000010, "LCDC Bit 1 failed");
		lcdc.u8 = 0; lcdc.objSize = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00000100, "LCDC Bit 2 failed");
		lcdc.u8 = 0; lcdc.backgroundDataAreaSelect = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00001000, "LCDC Bit 3 failed");
		lcdc.u8 = 0; lcdc.backgroundWindowTilesAreaSelect = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00010000, "LCDC Bit 4 failed");
		lcdc.u8 = 0; lcdc.windowEnable = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b00100000, "LCDC Bit 5 failed");
		lcdc.u8 = 0; lcdc.windowDataAreaSelect = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b01000000, "LCDC Bit 6 failed");
		lcdc.u8 = 0; lcdc.lcdEnabled = 1;
		CORE_TEST_ASSERT(lcdc.u8 == 0b10000000, "LCDC Bit 7 failed");
	}

	//
	// LCDS
	//
	{
		fgbLCDStatus lcds = { 0 };

		lcds.u8 = 0; lcds.lcdMode = 0b00;
		CORE_TEST_ASSERT(lcds.u8 == 0b00000000, "LCDS Bit 0-1 with 00 failed");
		lcds.u8 = 0; lcds.lcdMode = 0b01;
		CORE_TEST_ASSERT(lcds.u8 == 0b00000001, "LCDS Bit 0-1 with 01 failed");
		lcds.u8 = 0; lcds.lcdMode = 0b10;
		CORE_TEST_ASSERT(lcds.u8 == 0b00000010, "LCDS Bit 0-1 with 10 failed");
		lcds.u8 = 0; lcds.lcdMode = 0b11;
		CORE_TEST_ASSERT(lcds.u8 == 0b00000011, "LCDS Bit 0-1 with 11 failed");
		lcds.u8 = 0; lcds.lycEqualsLY = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b00000100, "LCDS Bit 2 failed");
		lcds.u8 = 0; lcds.mode0IntSelect = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b00001000, "LCDS Bit 3 failed");
		lcds.u8 = 0; lcds.mode1IntSelect = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b00010000, "LCDS Bit 4 failed");
		lcds.u8 = 0; lcds.mode2IntSelect = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b00100000, "LCDS Bit 5 failed");
		lcds.u8 = 0; lcds.lycIntSelect = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b01000000, "LCDS Bit 6 failed");
		lcds.u8 = 0; lcds.unused = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000000, "LCDS Bit 7 failed");

		lcds.u8 = 0b10000000; lcds.lcdMode = 0b00;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000000, "LCDS Bit 0-1 with 00 failed");
		lcds.u8 = 0b10000000; lcds.lcdMode = 0b01;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000001, "LCDS Bit 0-1 with 01 failed");
		lcds.u8 = 0b10000000; lcds.lcdMode = 0b10;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000010, "LCDS Bit 0-1 with 10 failed");
		lcds.u8 = 0b10000000; lcds.lcdMode = 0b11;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000011, "LCDS Bit 0-1 with 11 failed");

		lcds.u8 = 0b00000011; lcds.unused = 1;
		CORE_TEST_ASSERT(lcds.u8 == 0b10000011, "LCDS Bit 0-1 set and 7 set failed");

		lcds.u8 = 0b11111100;
		CORE_TEST_ASSERT(lcds.lycEqualsLY == 1, "LCDS BIT 2 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.mode0IntSelect == 1, "LCDS BIT 3 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.mode1IntSelect == 1, "LCDS BIT 4 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.mode2IntSelect == 1, "LCDS BIT 5 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.lycIntSelect == 1, "LCDS BIT 6 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.unused == 1, "LCDS BIT 7 failed for all, except BIT 0-1");
		CORE_TEST_ASSERT(lcds.lcdMode == 0, "LCDS Mode Mask set failed #4");

		lcds.u8 = 0; lcds.lcdMode = 0;
		lcds.u8 &= ~0b11;
		lcds.u8 |= 0x02;
		CORE_TEST_ASSERT(lcds.u8 == 0x02, "LCDS Mode Mask set failed #2");

		lcds.u8 = 0; lcds.lcdMode = true;
		lcds.u8 &= ~0b11;
		lcds.u8 |= 3;
		CORE_TEST_ASSERT(lcds.u8 == 0x03, "LCDS Mode Mask set failed #3");
	}

	//
	// Sound Register
	//
	{
		fgbSoundRegister sr = { 0 };
		sr.audioMasterControl.u8 = 0xAB;
		CORE_TEST_ASSERT(sr.m[0xFF26 - 0xFF10] == 0xAB, "Audio Master Control M Sync");

		sr.nr10.u8 = 0xDE;
		CORE_TEST_ASSERT(sr.m[0xFF10 - 0xFF10] == 0xDE, "NR10 M Sync");
	}

	result = true;

	goto done;

failed:

	FGB__BREAK;

	result = false;

done:
#undef CORE_TEST_ASSERT

	return result;
}

FGB_API fgbInitResult fgbInit(fgbSystem *system, const fgbConfiguration *config, const fgbGamePak *gamePak) {
	if (system == NULL) {
		return fgbInitResult_InvalidArguments;
	}

	fgbClearStruct(system);

	bool isScreenEnabled = true;

	// TODO(final): Copy the entire config structure instead!
	uint32_t targetSampleRate = 0;
	if (config != NULL) {
		system->log = config->log;
		system->boot.rom = config->bootROM;
		system->systemMonochromeColors = !fgb__MemoryIsZero((uint8_t *)&config->colors.m[0], sizeof(fgbMonochromeColors)) ? config->colors : FGB_DEFAULT_DMG_COLORS;
		system->directories = config->directories;
		system->debug = config->debug;
		system->callbacks = config->callbacks;
		isScreenEnabled = !config->isScreenDisabled;
		targetSampleRate = config->targetSampleRate;
	} else {
		system->systemMonochromeColors = FGB_DEFAULT_DMG_COLORS;
	}

	if (targetSampleRate == 0) {
		targetSampleRate = 44100;
	}

	if (!fgb__CoreTest(system)) {
		return fgbInitResult_CoreTestFailed;
	}

	FGB__INFO(system, fgb__KindName_Core, "");
	FGB__INFO(system, fgb__KindName_Core, "Initialize");
	FGB__INFO(system, fgb__KindName_Core, "");

	if (system->boot.rom.isEnabled && system->boot.rom.length < FGB_BOOT_MIN_ROM_SIZE) {
		return fgbInitResult_MissingBootROM;
	}

	if (!system->boot.rom.isEnabled && gamePak == NULL) {
		FGB__ERROR(system, fgb__KindName_Core, "Missing gamePak argument");
		return fgbInitResult_MissingGamePak;
	}

	if (gamePak != NULL && !gamePak->isValid && !system->boot.rom.isEnabled) {
		FGB__ERROR(system, fgb__KindName_Core, "Unsupported gamePak, rom size '%zu'", gamePak->rom.length);
		return fgbInitResult_InvalidGamePak;
	}

	if (system->boot.rom.isEnabled) {
		FGB__INFO(system, fgb__KindName_Core, "Using Boot ROM with size '%u'", system->boot.rom.length);
	}

	if (gamePak != NULL && gamePak->isValid) {
		FGB__INFO(system, fgb__KindName_Core, "Using GamePak '%s':", gamePak->info.title.text);
		system->gamePak = *gamePak;

		fgb__MBCInit(system, gamePak, &system->mbc);

		FGB__INFO(system, fgb__KindName_Core, "    Filepath: %s", gamePak->filePath);
		FGB__INFO(system, fgb__KindName_Core, "    Type: %s", fgbGetGamePakTypeName(gamePak->info.gamePakType));
		FGB__INFO(system, fgb__KindName_Core, "    Core: %s", fgbGetCoreTypeName(gamePak->info.coreType));
		FGB__INFO(system, fgb__KindName_Core, "    MBC: %s", fgbGetMemoryControllerTypeName(gamePak->info.mbcType));
		FGB__INFO(system, fgb__KindName_Core, "    ROM Size: %zu (%u Banks)", gamePak->rom.length, gamePak->info.romBankCount);
		FGB__INFO(system, fgb__KindName_Core, "    RAM Size: %zu (%u Banks)", gamePak->ram.memory.length, gamePak->info.ramBankCount);
		FGB__INFO(system, fgb__KindName_Core, "    Checksum: %u (%s)", gamePak->info.romChecksum, (gamePak->hasROMChecksumPassed ? "Passed" : "Failed"));
		FGB__INFO(system, fgb__KindName_Core, "    License: %s", (gamePak->hasLicensePassed ? "Passed" : "Failed"));

		char featuresBuffer[100] = { 0 };
		if (gamePak->info.features == fgbGamePakFeature_None) {
			fgb__StringAppend("None", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
		} else {
			if ((gamePak->info.features & fgbGamePakFeature_RAM) > 0) {
				if (FGB_STRLEN(featuresBuffer) > 0)
					fgb__StringAppend(" | ", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
				fgb__StringAppend("Ram", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
			}
			if ((gamePak->info.features & fgbGamePakFeature_BATTERY) > 0) {
				if (FGB_STRLEN(featuresBuffer) > 0)
					fgb__StringAppend(" | ", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
				fgb__StringAppend("Battery", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
			}
			if ((gamePak->info.features & fgbGamePakFeature_TIMER) > 0) {
				if (FGB_STRLEN(featuresBuffer) > 0)
					fgb__StringAppend(" | ", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
				fgb__StringAppend("Timer", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
			}
			if ((gamePak->info.features & fgbGamePakFeature_RUMBLE) > 0) {
				if (FGB_STRLEN(featuresBuffer) > 0)
					fgb__StringAppend(" | ", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
				fgb__StringAppend("Rumble", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
			}
			if ((gamePak->info.features & fgbGamePakFeature_SENSOR) > 0) {
				if (FGB_STRLEN(featuresBuffer) > 0)
					fgb__StringAppend(" | ", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
				fgb__StringAppend("Sensor", featuresBuffer, FGB_ARRAYCOUNT(featuresBuffer));
			}
		}
		FGB__INFO(system, fgb__KindName_Core, "    Features: %s", featuresBuffer);
	}

	FGB__INFO(system, fgb__KindName_Core, "");

	if (config != NULL) {
		system->state = config->paused ? fgbEmulationState_Paused : fgbEmulationState_Running;
	} else {
		system->state = fgbEmulationState_Running;
	}

	fgb__PowerOn(system, isScreenEnabled, targetSampleRate);

	FGB__INFO(system, fgb__KindName_Core, system->state == fgbEmulationState_Paused ? "Paused" : "Running");

	FGB__INFO(system, fgb__KindName_Core, "");

	return fgbInitResult_Success;
}

FGB_API void fgbShutdown(fgbSystem *system) {
	if (system == NULL) {
		return;
	}

	FGB__INFO(system, fgb__KindName_Core, "");
	FGB__INFO(system, fgb__KindName_Core, "Shutdown");
	FGB__INFO(system, fgb__KindName_Core, "");

	fgbGamePakUnload(&system->gamePak);

	fgbClearStruct(system);
}

FGB_API bool fgbReset(fgbSystem *system, const bool paused) {
	if (system == NULL || (!(system->gamePak.isValid || system->boot.state.isActive))) {
		return false;
	}

	fgbResetState newState = paused ? fgbResetState_Pause : fgbResetState_Running;

	if (system->state == fgbEmulationState_Running || 
		system->state == fgbEmulationState_MicroStep || 
		system->state == fgbEmulationState_Step || 
		system->state == fgbEmulationState_Breakpoint) {
		system->resetState = newState;
		return true;
	}

	fgb__ExecuteReset(system, newState);

	return true;
}

static fgbValue fgb__CreateValueU8(const uint8_t value) {
	fgbValue result = { 0 };
	result.isWide = false;
	result.isSign = false;
	result.ulow = value;
	return result;
}

static fgbValue fgb__CreateValueS8(const int8_t value) {
	fgbValue result = { 0 };
	result.isWide = false;
	result.isSign = true;
	result.slow = value;
	return result;
}

static fgbValue fgb__CreateValueU16(const uint16_t value) {
	fgbValue result = { 0 };
	result.isWide = true;
	result.isSign = false;
	result.u16 = value;
	return result;
}

static void fgb__AddConstantOperand(fgbDecodedInstruction *instruction, const uint16_t constant) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->constant = constant & 0xFFFF;
	op->type = fgbOperandType_Constant;
}

static void fgb__AddImmediateU8Operand(fgbDecodedInstruction *instruction, const uint8_t imm8) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->immediate = fgb__CreateValueU8(imm8);
	op->type = fgbOperandType_Immediate;
}

static void fgb__AddImmediateS8Operand(fgbDecodedInstruction *instruction, const int8_t imm8) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->immediate = fgb__CreateValueS8(imm8);
	op->type = fgbOperandType_Immediate;
}

static void fgb__AddImmediateU16Operand(fgbDecodedInstruction *instruction, const uint16_t imm16) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->immediate = fgb__CreateValueU16(imm16);
	op->type = fgbOperandType_Immediate;
}

static void fgb__AddRegisterOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_Register;
}

static void fgb__AddMemoryRegisterOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_MemoryFromRegister;
}

static void fgb__AddMemoryRegisterIncOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_MemoryFromRegisterInc;
}

static void fgb__AddMemoryRegisterDecOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_MemoryFromRegisterDec;
}

static void fgb__AddMemoryRegisterWithOffsetOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg, const int8_t offset) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_MemoryFromRegisterWithOffset;
	op->offset = fgb__CreateValueS8(offset);
}

static void fgb__AddRegisterWithOffsetOperand(fgbDecodedInstruction *instruction, const fgbRegisterType reg, const int8_t offset) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->reg = reg;
	op->type = fgbOperandType_RegisterWithOffset;
	op->offset = fgb__CreateValueS8(offset);
}

static void fgb__AddMemoryAddressOperand(fgbDecodedInstruction *instruction, const uint16_t address) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->type = fgbOperandType_MemoryAddress;
	op->address = address;
}

static void fgb__AddMemoryAddressWithOffsetOperand(fgbDecodedInstruction *instruction, const uint16_t address, const uint8_t offset) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->type = fgbOperandType_MemoryAddress;
	op->address = address;
	op->offset = fgb__CreateValueU8(offset);
}

static void fgb__AddMemoryConstantWithRegisterOffsetOperand(fgbDecodedInstruction *instruction, const uint16_t address, fgbRegisterType reg) {
	FGB_ASSERT(instruction->operandCount < FGB_ARRAYCOUNT(instruction->operands));
	fgbOperand *op = instruction->operands + instruction->operandCount++;
	op->type = fgbOperandType_MemoryAddressWithRegisterOffset;
	op->reg = reg;
	op->address = address;
}

FGB_API bool fgbDecodeInstruction(const fgbMemory *rom, const uint32_t pos, fgbDecodedInstruction *outInstruction) {
	if (rom == NULL || rom->data == NULL || outInstruction == NULL) {
		return false;
	}

	if (pos >= rom->length) {
		return false;
	}

	uint32_t currentPos = pos;

	uint8_t opcode = rom->data[currentPos++];

	bool isPrefix;

	const fgbInstruction *instruction;
	if (opcode == 0xCB) {
		if (currentPos >= rom->length) {
			return false;
		}
		opcode = rom->data[currentPos++];
		instruction = &fgb__prefixInstructionTable[opcode];
		isPrefix = true;
	} else {
		instruction = &fgb__instructionTable[opcode];
		isPrefix = false;
	}

	if (instruction == NULL)
		return false;

	fgbDecodedInstruction result = { 0 };
	result.type = instruction->type;
	result.mode = instruction->mode;
	result.opCode = opcode;
	result.isPrefix = isPrefix;
	result.position = pos;

	switch (instruction->mode) {
		case fgbAddressingMode_Implied:
			break;

		case fgbAddressingMode_Constant:
			fgb__AddConstantOperand(&result, instruction->value & 0xFFFF);
			break;

		case fgbAddressingMode_Constant_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddConstantOperand(&result, instruction->value & 0xFFFF);
			fgb__AddRegisterOperand(&result, destReg);
		} break;

		case fgbAddressingMode_Constant_MemReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddConstantOperand(&result, instruction->value & 0xFFFF);
			fgb__AddMemoryRegisterOperand(&result, destReg);
		} break;

		case fgbAddressingMode_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddRegisterOperand(&result, destReg);
			
		} break;

		case fgbAddressingMode_Reg_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;
			fgb__AddRegisterOperand(&result, destReg);
			fgb__AddRegisterOperand(&result, sourceReg);
		} break;

		case fgbAddressingMode_Reg_RegOffsetI8:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;
			fgb__AddRegisterOperand(&result, destReg);

			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t offset = rom->data[currentPos++];
			fgb__AddRegisterWithOffsetOperand(&result, sourceReg, (int8_t)offset);
		} break;

		case fgbAddressingMode_Reg_U16:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddRegisterOperand(&result, destReg);

			if (currentPos + 1 >= rom->length) {
				return false;
			}

			uint8_t low = rom->data[currentPos++];
			uint8_t high = rom->data[currentPos++];

			fgbValue imm = { 0 };
			imm.ulow = low;
			imm.uhigh = high;

			fgb__AddImmediateU16Operand(&result, imm.u16);
		} break;

		case fgbAddressingMode_U16:
		{
			if (currentPos + 1 >= rom->length) {
				return false;
			}

			uint8_t low = rom->data[currentPos++];
			uint8_t high = rom->data[currentPos++];

			fgbValue imm = { 0 };
			imm.ulow = low;
			imm.uhigh = high;

			fgb__AddImmediateU16Operand(&result, imm.u16);
		} break;

		case fgbAddressingMode_I8:
		{
			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t s8 = rom->data[currentPos++];
			fgb__AddImmediateS8Operand(&result, (int8_t)s8);
		} break;

		case fgbAddressingMode_Reg_I8:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddRegisterOperand(&result, destReg);

			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t s8 = rom->data[currentPos++];
			fgb__AddImmediateS8Operand(&result, (int8_t)s8);
		} break;

		case fgbAddressingMode_Reg_U8:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddRegisterOperand(&result, destReg);

			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t u8 = rom->data[currentPos++];
			fgb__AddImmediateU8Operand(&result, u8);
		} break;

		case fgbAddressingMode_Reg_MemRegInc:
		case fgbAddressingMode_Reg_MemRegDec:
		case fgbAddressingMode_Reg_MemReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			fgb__AddRegisterOperand(&result, destReg);

			if (instruction->mode == fgbAddressingMode_Reg_MemRegInc) {
				fgb__AddMemoryRegisterIncOperand(&result, sourceReg);
			} else if (instruction->mode == fgbAddressingMode_Reg_MemRegDec) {
				fgb__AddMemoryRegisterDecOperand(&result, sourceReg);
			} else {
				fgb__AddMemoryRegisterOperand(&result, sourceReg);
			}
		} break;

		case fgbAddressingMode_MemA16_Reg:
		{
			fgbRegisterType sourceReg = instruction->regA;

			if (currentPos + 1 >= rom->length) {
				return false;
			}

			uint8_t low = rom->data[currentPos++];
			uint8_t high = rom->data[currentPos++];

			fgbValue imm = { 0 };
			imm.ulow = low;
			imm.uhigh = high;

			fgb__AddMemoryAddressOperand(&result, imm.u16);

			fgb__AddRegisterOperand(&result, sourceReg);
		} break;

		case fgbAddressingMode_Reg_MemA16:
		{
			fgbRegisterType destReg = instruction->regA;

			fgb__AddRegisterOperand(&result, destReg);

			if (currentPos + 1 >= rom->length) {
				return false;
			}

			uint8_t low = rom->data[currentPos++];
			uint8_t high = rom->data[currentPos++];

			fgbValue imm = { 0 };
			imm.ulow = low;
			imm.uhigh = high;

			fgb__AddMemoryAddressOperand(&result, imm.u16);
		} break;

		case fgbAddressingMode_Reg_MemConstantOffsetA8:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddRegisterOperand(&result, destReg);

			uint16_t address = instruction->value & 0xFFFF;
			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t offset = rom->data[currentPos++];
			fgb__AddMemoryAddressWithOffsetOperand(&result, address, offset);
		} break;

		case fgbAddressingMode_MemConstantOffsetA8_Reg:
		{
			fgbRegisterType sourceReg = instruction->regA;

			uint16_t address = instruction->value & 0xFFFF;
			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t offset = rom->data[currentPos++];
			fgb__AddMemoryAddressWithOffsetOperand(&result, address, offset);

			fgb__AddRegisterOperand(&result, sourceReg);
		} break;

		case fgbAddressingMode_MemReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddMemoryRegisterOperand(&result, destReg);
		} break;

		case fgbAddressingMode_MemReg_U8:
		{
			fgbRegisterType destReg = instruction->regA;
			fgb__AddMemoryRegisterOperand(&result, destReg);

			if (currentPos >= rom->length) {
				return false;
			}
			uint8_t u8 = rom->data[currentPos++];
			fgb__AddImmediateU8Operand(&result, u8);
		} break;

		case fgbAddressingMode_MemReg_Reg:
		case fgbAddressingMode_MemRegDec_Reg:
		case fgbAddressingMode_MemRegInc_Reg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			if (instruction->mode == fgbAddressingMode_MemRegInc_Reg) {
				fgb__AddMemoryRegisterIncOperand(&result, destReg);
			} else if (instruction->mode == fgbAddressingMode_MemRegDec_Reg) {
				fgb__AddMemoryRegisterDecOperand(&result, destReg);
			} else {
				fgb__AddMemoryRegisterOperand(&result, destReg);
			}

			fgb__AddRegisterOperand(&result, sourceReg);
		} break;

		case fgbAddressingMode_Reg_MemConstantOffsetReg:
		{
			fgbRegisterType destReg = instruction->regA;
			fgbRegisterType offsetReg = instruction->regB;

			uint16_t startAddress = instruction->value & 0xFFFF;

			fgb__AddRegisterOperand(&result, destReg);

			fgb__AddMemoryConstantWithRegisterOffsetOperand(&result, startAddress, destReg);
		} break;

		case fgbAddressingMode_MemConstantOffsetReg_Reg:
		{
			fgbRegisterType offsetReg = instruction->regA;
			fgbRegisterType sourceReg = instruction->regB;

			uint16_t startAddress = instruction->value & 0xFFFF;

			fgb__AddMemoryConstantWithRegisterOffsetOperand(&result, startAddress, offsetReg);

			fgb__AddRegisterOperand(&result, sourceReg);
		} break;

		default:
			break;
	}

	uint32_t len = currentPos - pos;

	result.length = len;

	*outInstruction = result;

	return true;
}

FGB_API size_t fgbFormatInstruction(char *destBuffer, size_t maxDestBufferLen, const fgbDecodedInstruction *instruction) {
	if (instruction == NULL) {
		return 0;
	}

	uint8_t opcode = instruction->opCode;

	const char *instructionName = fgbGetInstructionName(instruction->type);

	const bool constantMemOffsetAsLDH = true;

	switch (instruction->mode) {
		case fgbAddressingMode_Implied:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s", instructionName);

		case fgbAddressingMode_Constant:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s $%04X", instructionName, instruction->operands[0].constant);

		case fgbAddressingMode_Constant_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %d, (%s)", instructionName, (int)instruction->operands[0].constant, fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_Constant_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %d, %s", instructionName, (int)instruction->operands[0].constant, fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_I8:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %d", instructionName, instruction->operands[0].immediate.slow);

		case fgbAddressingMode_U16:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s $%04X", instructionName, instruction->operands[0].immediate.u16);

		case fgbAddressingMode_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s", instructionName, fgbGetRegisterName(instruction->operands[0].reg));
		case fgbAddressingMode_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s)", instructionName, fgbGetRegisterName(instruction->operands[0].reg));

		case fgbAddressingMode_Reg_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %s", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_Reg_RegOffsetI8:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %s+%d", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg), instruction->operands[1].offset.slow);

		case fgbAddressingMode_Reg_MemConstantOffsetReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X+%s)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].address, fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemConstantOffsetReg_Reg: {
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X+%s), %s", instructionName, instruction->operands[0].address, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));
		}

		case fgbAddressingMode_Reg_MemReg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_Reg_MemA16:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].address);

		case fgbAddressingMode_Reg_MemRegInc:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s+)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_Reg_MemRegDec:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, (%s-)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemReg_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s), %s", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemReg_U8:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s), $%02X", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].immediate.ulow);

		case fgbAddressingMode_Reg_U8:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, $%02X", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].immediate.ulow);

		case fgbAddressingMode_Reg_I8:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, %d", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].immediate.slow);

		case fgbAddressingMode_Reg_U16:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, $%04X", instructionName, fgbGetRegisterName(instruction->operands[0].reg), instruction->operands[1].immediate.u16);

		case fgbAddressingMode_MemA16_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X), %s", instructionName, instruction->operands[0].address, fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemRegDec_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s-), %s", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemRegInc_Reg:
			return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s (%s+), %s", instructionName, fgbGetRegisterName(instruction->operands[0].reg), fgbGetRegisterName(instruction->operands[1].reg));

		case fgbAddressingMode_MemConstantOffsetA8_Reg:
		{
			uint16_t constant = instruction->operands[0].address;
			uint8_t offset = instruction->operands[0].offset.ulow;

			if (constantMemOffsetAsLDH) {
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH ($%02X), %s", instructionName, offset, fgbGetRegisterName(instruction->operands[1].reg));
			} else {
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s ($%04X+$%02X), %s", instructionName, constant, offset, fgbGetRegisterName(instruction->operands[1].reg));
			}
		}

		case fgbAddressingMode_Reg_MemConstantOffsetA8:
		{
			// Use LDH instead
			uint16_t constant = instruction->operands[1].address;
			uint8_t offset = instruction->operands[1].offset.ulow;
			if (constantMemOffsetAsLDH) {
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%sH %s, ($%02X)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), offset);
			} else {
				return fgb__StringFormat(destBuffer, maxDestBufferLen, "%s %s, ($%04X+$%02X)", instructionName, fgbGetRegisterName(instruction->operands[0].reg), constant, offset);
			}
		}

		default:
			return 0;
	}
}

FGB_API void fgbSetColorPalette(fgbSystem *system, const fgbMonochromeColors *colors) {
	if (system == NULL) {
		return;
	}

	if (colors != NULL)
		system->systemMonochromeColors = *colors;
	else
		system->systemMonochromeColors = FGB_DEFAULT_DMG_COLORS;

	fgbPPU *ppu = &system->ppu;
	fgbLCDRegister *lcd = &ppu->lcd;
	fgb__PPUPaletteReload(lcd, &ppu->currentMonochromeColors, &system->systemMonochromeColors);
}

FGB_API fgbEmulationState fgbGetState(const fgbSystem *system) {
	if (system == NULL) {
		return false;
	}
	return system->state;
}

FGB_API bool fgbIsFrameUpdated(const fgbSystem *system) {
	if (system == NULL) {
		return false;
	}
	return system->ppu.state.isFrameFinished;
}

FGB_API bool fgbIsVRAMUpdated(const fgbSystem *system) {
	if (system == NULL) {
		return false;
	}
	return system->ppu.state.isVRAMUpdated;
}

// ********************************************************************************************************************
// 
// Snapshot Export/Import/Save/Load Implementation
// 
// ********************************************************************************************************************

// Snapshot Magic Key
#define FGB_SNAPSHOT_MAGIC_KEY (uint32_t)FGB_FOURCC('F', 'G', 'B', 'S')

// Start/End of data Key
#define FGB_SNAPSHOT_SOD_KEY (uint32_t)FGB_FOURCC('F', 'S', 'O', 'D')
#define FGB_SNAPSHOT_EOD_KEY (uint32_t)FGB_FOURCC('F', 'E', 'O', 'D')

#pragma pack(push, 1)
typedef struct {
	// 4-byte FourCC Magic code
	uint32_t magic;
	// Snapshot version
	fgbSnapshotVersion version;
	// Date time
	fgbDateTime dateTime;
} fgb__SnapshotHeader;
#pragma pack(pop)

static char fgb__SnapshotFilePathBuffer[2048];

static bool fgb__IsGamePakInfoEqual(const fgbGamePakInfo *source, const fgbGamePakInfo *dest) {
	if (source == NULL || dest == NULL) {
		return false;
	}
	bool failed = false;
	failed |= (source->coreType != dest->coreType);
	failed |= (source->gamePakType != dest->gamePakType);
	failed |= (source->romSizeType != dest->romSizeType);
	failed |= (source->ramSizeType != dest->ramSizeType);
	failed |= (source->headerChecksum != dest->headerChecksum);
	failed |= (source->romChecksum != dest->romChecksum);
	if (failed) {
		return false;
	}

	if (!fgb__IsStringEqual(source->title.text, dest->title.text)) {
		return false;
	}

	return true;
}

static bool fgb__IsSnapshotAllowed(const fgbSystem *system) {
	if (system->state == fgbEmulationState_Error || !system->gamePak.isValid) {
		return false;
	}
#if defined(FGB_64BIT)
	return true;
#else
	return false;
#endif
}

FGB_API bool fgbSnapshotExport(fgbSystem *system, fgbSnapshot *outSnapshot) {
	if (system == NULL || outSnapshot == NULL) {
		return false;
	}

	if (!fgb__IsSnapshotAllowed(system)) {
		return false;
	}

	// Ensure that the system is paused, while taking the snapshot
	if (!fgbPause(system)) {
		return false;
	}

	fgbClearStruct(outSnapshot);

	// Game Info
	fgbCopyStruct(&system->gamePak.info, &outSnapshot->gameInfo);
	// MBC
	fgbCopyStruct(&system->mbc.data, &outSnapshot->mbcData);
	// External RAM
	fgbCopyStruct(&system->gamePak.ram, &outSnapshot->externalRam);

	// Internal RAM
	fgbCopyStruct(&system->ram, &outSnapshot->internalRam);

	// CPU
	fgbCopyStruct(&system->cpu, &outSnapshot->cpu);
	// IO
	fgbCopyStruct(&system->io, &outSnapshot->io);
	// Interrupts
	fgbCopyStruct(&system->interrupts, &outSnapshot->interrupts);
	// Joypad
	fgbCopyStruct(&system->joypad, &outSnapshot->joypad);
	// Monochrome colors
	fgbCopyStruct(&system->systemMonochromeColors, &outSnapshot->monochromeColors);
	// APU
	fgbCopyStruct(&system->apu, &outSnapshot->apu);

	// PPU
	fgbCopyStruct(&system->ppu, &outSnapshot->ppu);

	// Generate sprites linked list
	fgbClearStruct(&outSnapshot->ppu.pipeline.sprites);
	outSnapshot->ppu.pipeline.sprites.count = system->ppu.pipeline.sprites.count;
	outSnapshot->ppu.pipeline.sprites.first = NULL; // Don't care, its reconstructed later
	fgbLineSpriteEntry *inSpriteEntry = system->ppu.pipeline.sprites.first;
	size_t outSpriteEntryIndex = 0;
	while (inSpriteEntry != NULL) {
		fgbLineSpriteEntry *outSpriteEntry = &outSnapshot->ppu.pipeline.sprites.buffer[outSpriteEntryIndex++];
		outSpriteEntry->entry = inSpriteEntry->entry;
		outSpriteEntry->next = NULL; // Don't care, its reconstructed later
		inSpriteEntry = inSpriteEntry->next;
	}

	// Timer
	fgbCopyStruct(&system->timer, &outSnapshot->timer);
	// Serial
	fgbCopyStruct(&system->serial, &outSnapshot->serial);

	outSnapshot->version = fgbSnapshotVersion_Current;
	outSnapshot->dateTime = fgb__DateTimeQuery(&system->callbacks);

	return true;
}

FGB_API bool fgbAreSnapshotsSupported(const fgbSystem *system) {
	if (system == NULL) {
		return false;
	}

	if (!fgb__IsSnapshotAllowed(system)) {
		return false;
	}

	return true;
}

FGB_API bool fgbIsSnapshotValid(const fgbSystem *system, const fgbSnapshot *snapshot) {
	if (system == NULL || snapshot == NULL) {
		return false;
	}

	if (!fgb__IsSnapshotAllowed(system)) {
		return false;
	}

	if (!fgb__IsGamePakInfoEqual(&system->gamePak.info, &snapshot->gameInfo)) {
		return false;
	}

	return true;
}

FGB_API bool fgbSnapshotSaveToFile(const fgbSystem *system, const char *romFilePath, const uint8_t slotIndex, const fgbSnapshot *snapshot) {
#if !defined(FGB_64BIT)
	return false; // We don't support non 64-bit serialization, due to size_t and pointer differences -> Not great :-(
#endif

	if (system == NULL || romFilePath == NULL || snapshot == NULL || !system->gamePak.isValid) {
		return false;
	}

	const fgbCallbacks *cb = &system->callbacks;

	if (!fgb__BuildFilePath(cb, romFilePath, system->directories.snapshotFolderPath, fgbFileType_Snapshot, fgb__SnapshotFilePathBuffer, sizeof(fgb__SnapshotFilePathBuffer), slotIndex)) {
		return false;
	}

	const char *filePath = fgb__SnapshotFilePathBuffer;
	if (FGB_STRLEN(filePath) == 0)
		return false;

	fgbFileHandle fileHandle;
	if (!fgb__FileCreate(cb, filePath, &fileHandle)) {
        return false;
    }

	uint8_t sixteenBytes[16] = {0};

	const uint32_t sodKey = FGB_SNAPSHOT_SOD_KEY;
	const uint32_t eodKey = FGB_SNAPSHOT_EOD_KEY;

	// Header
	fgb__SnapshotHeader header = { 0 };
	header.magic = FGB_SNAPSHOT_MAGIC_KEY;
	header.version = snapshot->version;
	header.dateTime = snapshot->dateTime;
	fgb__FileWrite(cb, fileHandle, &header, sizeof(header));

	// Start-of-Data
	fgbClearStruct(sixteenBytes);
	FGB_MEMCOPY(&sixteenBytes[16 - sizeof(uint32_t)], &sodKey, 4);
	fgb__FileWrite(cb, fileHandle, &sixteenBytes, sizeof(sixteenBytes));

	// Game Info
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->gameInfo);
	// MBC
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->mbcData);
	// External RAM
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->externalRam);

	// Internal RAM
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->internalRam);

	// CPU
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->cpu);
	// IO
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->io);
	// Interrupts
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->interrupts);
	// Joypad
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->joypad);
	// Monochrome Colors
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->monochromeColors);

	// APU
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->apu);
	// PPU
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->ppu);
	// Timer
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->timer);
	// Serial
	fgb__FileWriteAutoStruct(cb, fileHandle, &snapshot->serial);

	// End-of-Data
	fgbClearStruct(sixteenBytes);
	FGB_MEMCOPY(&sixteenBytes[16 - sizeof(uint32_t)], &eodKey, 4);
	fgb__FileWrite(cb, fileHandle, &sixteenBytes, sizeof(sixteenBytes));

	fgb__FileFlush(cb, fileHandle);

    fgb__FileClose(cb, fileHandle);

	return true;
}

FGB_API bool fgbSnapshotLoadFromFile(const fgbSystem *system, const char *romFilePath, const uint8_t slotIndex, fgbSnapshot *snapshot) {
#if !defined(FGB_64BIT)
	return false; // We don't support non 64-bit serialization, due to size_t and pointer differences -> Not great :-(
#endif

	if (system == NULL || romFilePath == NULL || snapshot == NULL || !system->gamePak.isValid) {
		return false;
	}

	bool result = false;

	const fgbCallbacks *cb = &system->callbacks;

	if (!fgb__BuildFilePath(cb, romFilePath, system->directories.snapshotFolderPath, fgbFileType_Snapshot, fgb__SnapshotFilePathBuffer, sizeof(fgb__SnapshotFilePathBuffer), slotIndex)) {
		return false;
	}

	const char *filePath = fgb__SnapshotFilePathBuffer;
	if (FGB_STRLEN(filePath) == 0) {
		return false;
	}

	if (!fplFileExists(filePath)) {
		return false;
	}

	fgbFileHandle fileHandle;
	if (!fgb__FileOpen(cb, filePath, &fileHandle)) {
        return false;
    }

	size_t read;

	size_t headerSize = sizeof(fgb__SnapshotHeader);

	uint8_t sixteenBytes[16] = { 0 };

	// Header
	fgb__SnapshotHeader header = { 0 };
	read = fgb__FileRead(cb, fileHandle, &header, sizeof(header), headerSize);
	if (read != headerSize) {
		goto failed;
	}
	if (header.magic != FGB_SNAPSHOT_MAGIC_KEY) {
		goto failed;
	}
	if (header.version < fgbSnapshotVersion_First || header.version > fgbSnapshotVersion_Current) {
		goto failed;
	}

	snapshot->dateTime = header.dateTime;
	snapshot->version = header.version;

	// Start-of-Data
	read = fgb__FileRead(cb, fileHandle, &sixteenBytes, sizeof(sixteenBytes), 16);
	if (read != 16) {
		goto failed;
	}
	uint32_t sod = *(uint32_t *)(sixteenBytes + (16 - sizeof(uint32_t)));
	if (sod != FGB_SNAPSHOT_SOD_KEY) {
		goto failed;
	}

	// Game Info
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->gameInfo)) {
		goto failed;
	}
	// MBC
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->mbcData)) {
		goto failed;
	}
	// External RAM
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->externalRam)) {
		goto failed;
	}

	// Internal RAM
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->internalRam)) {
		goto failed;
	}

	// CPU
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->cpu)) {
		goto failed;
	}
	// IO
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->io)) {
		goto failed;
	}
	// Interrupts
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->interrupts)) {
		goto failed;
	}
	// Joypad
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->joypad)) {
		goto failed;
	}
	// Monochrome Colors
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->monochromeColors)) {
		goto failed;
	}

	// APU
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->apu)) {
		goto failed;
	}
	// PPU
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->ppu)) {
		goto failed;
	}
	// Timer
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->timer)) {
		goto failed;
	}
	// Serial
	if (!fgb__FileReadAutoStruct(cb, fileHandle, &snapshot->serial)) {
		goto failed;
	}

	// End-of-Data
	read = fgb__FileRead(cb, fileHandle, &sixteenBytes, sizeof(sixteenBytes), 16);
	if (read != 16) {
		goto failed;
	}
	uint32_t eod = *(uint32_t *)(sixteenBytes + (16 - sizeof(uint32_t)));
	if (eod != FGB_SNAPSHOT_EOD_KEY) {
		goto failed;
	}

	result = true;
	goto done;

failed:
	result = false;

done:
	fgb__FileClose(cb, fileHandle);

	return result;
}

FGB_API bool fgbSnapshotImport(fgbSystem *system, const fgbSnapshot *snapshot) {
	if (system == NULL || snapshot == NULL) {
		return false;
	}

	if (!fgbIsSnapshotValid(system, snapshot)) {
		return false;
	}

	fgb__ExecuteReset(system, fgbResetState_Pause);

	// Game Info
	
	// MBC
	fgbCopyStruct(&snapshot->mbcData, &system->mbc.data);
	// External RAM
	fgbCopyStruct(&snapshot->externalRam, &system->gamePak.ram);

	// Internal RAM
	fgbCopyStruct(&snapshot->internalRam, &system->ram);

	// CPU
	fgbCopyStruct(&snapshot->cpu, &system->cpu);
	// IO
	fgbCopyStruct(&snapshot->io, &system->io);
	// Interrupts
	fgbCopyStruct(&snapshot->interrupts, &system->interrupts);
	// Joypad
	fgbCopyStruct(&snapshot->joypad, &system->joypad);
	// Monochrome colors
	fgbCopyStruct(&snapshot->monochromeColors, &system->systemMonochromeColors);

	// APU
	fgbCopyStruct(&snapshot->apu, &system->apu);

	// PPU
	fgbCopyStruct(&snapshot->ppu, &system->ppu);

	// Reconstruct sprite linked list
	size_t spriteCount = FGB_MIN(FGB_MAX(0, system->ppu.pipeline.sprites.count), 10);
	if (spriteCount == 0) {
		system->ppu.pipeline.sprites.first = NULL;
	} else {
		system->ppu.pipeline.sprites.first = system->ppu.pipeline.sprites.buffer + 0;
		for (size_t i = 0; i < spriteCount; ++i) {
			fgbLineSpriteEntry *entry = system->ppu.pipeline.sprites.buffer + i;
			fgbLineSpriteEntry *nextEntry = ((i + 1) < spriteCount) ? entry + 1 : NULL;
			entry->next = nextEntry;
		}
	}

	// Timer
	fgbCopyStruct(&snapshot->timer, &system->timer);
	// Serial
	fgbCopyStruct(&snapshot->serial, &system->serial);

	return true;
}

#endif