// Runs the thread storage and the concurrent create/teardown tests of FPL_Test in a loop, to catch a rare hang.
// Prints one "ROUND <n>" line per finished round on stderr, threadstress.sh watches those lines.
// Usage: threadloop [rounds]
#define main FplTestOriginalMain
#include "../../demos/FPL_Test/fpl_test.c"
#undef main

#include <stdlib.h>

#define THREADLOOP_DEFAULT_ROUNDS 100

int main(int argc, char **argv) {
	int rounds = (argc > 1) ? atoi(argv[1]) : THREADLOOP_DEFAULT_ROUNDS;
	if (!fplPlatformInit(fplInitFlags_None, fpl_null)) {
		fprintf(stderr, "platform init failed\n");
		return 1;
	}
	for (int round = 0; round < rounds; ++round) {
		ThreadStorageTest(0);
		ThreadStorageTest(8);
		ThreadStorageTest(32);
		ThreadConcurrentCreateTest(4, 50);
		ThreadConcurrentCreateTest(8, 100);
		fprintf(stderr, "ROUND %d\n", round);
		fflush(stderr);
	}
	fplPlatformRelease();
	return 0;
}
