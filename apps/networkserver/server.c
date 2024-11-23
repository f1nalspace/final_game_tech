#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <final_net.h>

int main(int argc, char **argv) {
	bool isBE = fplNetworkIsBigEndian();
	printf("Platform is %s\n", isBE ? "Big Endian" : "Little Endian");
	fplNetworkTest();
	return 0;
}