//
// Header
//
#ifndef FINAL_DEBUG_H
#define FINAL_DEBUG_H

#include <final_platform_layer.h>

#if defined(FPL__ENABLE_DEBUG) || defined(FORCE_DEBUG)
#	define DEBUG_ENABLED
#endif

#if defined(DEBUG_ENABLED)

#if defined(FPL_IS_CPP)
#	define SUPPORTS_TIMED_BLOCK
#endif

typedef enum DebugType {
	DebugType_Unknown = 0,
	DebugType_FrameMarker,
	DebugType_BeginBlock,
	DebugType_EndBlock,
	DebugType_Count,
} DebugType;

typedef struct DebugEvent {
	uint64_t cycles;
	uint64_t time;
	char *guid;
	uint16_t threadID;
	uint16_t coreIndex;
	uint8_t type;
} DebugEvent;
fplStaticAssert(sizeof(DebugEvent) % 32 == 0);

#define MAX_DEBUG_EVENT_COUNT (16 * 65536)

typedef struct DebugTable {
	DebugEvent events[2][MAX_DEBUG_EVENT_COUNT];
	volatile uint64_t eventArrayIndex_EventIndex;
	volatile uint32_t currentEventArrayIndex;
} DebugTable;

typedef struct DebugMemory {
	void *storageBase;
	size_t storageSize;
} DebugMemory;

extern DebugTable *globalDebugTable;
extern DebugMemory *globalDebugMemory;
#endif // DEBUG_ENABLED

extern void InitDebug(const size_t storageSize);
extern void ReleaseDebug();

#if defined(DEBUG_ENABLED)

#define UniqueFileCounterString__(a, b, c, d) a "|" #b "|" #c "|" d
#define UniqueFileCounterString_(a, b, c, d) UniqueFileCounterString__(a, b, c, d)
#define DEBUG_NAME(name) UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__, name)

static void RecordDebugEvent(DebugType type, char *guid);

#define FRAME_MARKER() RecordDebugEvent(DebugType_FrameMarker, DEBUG_NAME("Frame Marker"))

#if defined(SUPPORTS_TIMED_BLOCK)
struct TimedBlock {
	TimedBlock(char *guid, U32 hitCountInit = 1) {
		BEGIN_BLOCK_(guid);
	}

	~TimedBlock() {
		END_BLOCK_();
	}
};

#	define TIMED_BLOCK__(guid, counter, ...) TimedBlock timedBlock_##counter(guid, ## __VA_ARGS__)
#	define TIMED_BLOCK_(guid, counter, ...) TIMED_BLOCK__(guid, counter, ## __VA_ARGS__)
#	define TIMED_BLOCK(name, ...) TIMED_BLOCK_(DEBUG_NAME(name), __COUNTER__, ## __VA_ARGS__)
#	define TIMED_FUNCTION(...) TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), ## __VA_ARGS__)
#else
#	define TIMED_BLOCK(...) 
#	define TIMED_FUNCTION(...) 
#endif

#define BEGIN_BLOCK_(guid) {RecordDebugEvent(DebugType_BeginBlock, guid);}
#define END_BLOCK_() {RecordDebugEvent(DebugType_EndBlock, DEBUG_NAME("::END_BLOCK::"));}

#define BEGIN_BLOCK(name) BEGIN_BLOCK_(DEBUG_NAME(name))
#define END_BLOCK() END_BLOCK_()


#else
#	define TIMED_BLOCK(...) 
#	define TIMED_FUNCTION(...) 
#	define BEGIN_BLOCK(...)
#	define END_BLOCK(...)
#	define FRAME_MARKER(...)
#endif

#endif // FINAL_DEBUG_H

//
// Implementation
//
#if defined(FINAL_DEBUG_IMPLEMENTATION) && !defined(FINAL_DEBUG_IMPLEMENTED)
#define FINAL_DEBUG_IMPLEMENTED

DebugTable *globalDebugTable = fpl_null;
DebugMemory *globalDebugMemory = fpl_null;

static void RecordDebugEvent(DebugType type, char* guid) {
	fplAssert(globalDebugTable);
	uint64_t arrayIndex_EventIndex = fplAtomicIncrementU64(&globalDebugTable->eventArrayIndex_EventIndex);
	uint32_t eventIndex = arrayIndex_EventIndex & 0xFFFFFFFF;
	fplAssert(eventIndex < fplArrayCount(globalDebugTable->events[0]));
	DebugEvent *ev = globalDebugTable->events[arrayIndex_EventIndex >> 32] + eventIndex;
	ev->cycles = fplCPURDTSC();
	ev->time = fplGetTimeInSecondsHP();
	ev->type = (uint8_t)type;
	ev->coreIndex = 0;
	ev->threadID = (uint16_t)fplGetCurrentThreadId();
	ev->guid = guid;
}

extern void InitDebug(const size_t storageSize) {
	fplAssert(globalDebugMemory == fpl_null);
	fplAssert(globalDebugTable == fpl_null);
	size_t totalSize = sizeof(DebugMemory) + 8 + sizeof(DebugTable) + 8 + storageSize;
	void *base = fplMemoryAllocate(totalSize);
	globalDebugMemory = (DebugMemory *)base;
	globalDebugMemory->storageBase = (uint8_t *)base + sizeof(DebugMemory) + 8 + sizeof(DebugTable) + 8;
	globalDebugMemory->storageSize = storageSize;
	globalDebugTable = (DebugTable *)((uint8_t *)base + sizeof(DebugMemory) + 8);
}

extern void ReleaseDebug() {
	if (globalDebugMemory != fpl_null) {
		fplAssert(globalDebugMemory->storageBase != fpl_null);
		size_t offsetToBase = 8 + sizeof(DebugMemory) + 8 + sizeof(DebugTable);
		void *base = (uint8_t *)globalDebugMemory->storageBase - offsetToBase;
		fplMemoryFree(base);
	}
	globalDebugTable = fpl_null;
	globalDebugMemory = fpl_null;
}

#endif // FINAL_DEBUG_IMPLEMENTATION