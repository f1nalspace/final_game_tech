#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int64_t roundUp(size_t numToRound, size_t multiple)
{
	if(multiple == 0)
		return numToRound;

	int64_t remainder = numToRound % multiple;
	if(remainder == 0)
		return numToRound;

	return numToRound + multiple - remainder;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		return -1;
	}

	const char *filePath = argv[1];
	const char *dataName = "data";
	if(argc >= 3) {
		dataName = argv[2];
	}

	FILE *file;
	if(fopen_s(&file, filePath, "rb") != 0) {
		return(-1);
	}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	uint8_t *mem = (uint8_t *)malloc(size);
	if(mem == NULL) {
		fclose(file);
		return(-1);
	}
	fread_s(mem, size, size, 1, file);
	fclose(file);

	const char *dataTypeName;
	int byteCount;
	size_t maxColCount;

	int forceBytes = 8;

	if(size % 8 == 0 || forceBytes == 8) {
		dataTypeName = "uint64_t";
		byteCount = 8;
		maxColCount = 4;
	} else if(size % 4 == 0 || forceBytes == 4) {
		dataTypeName = "uint32_t";
		byteCount = 4;
		maxColCount = 8;
	} else if(size % 2 == 0 || forceBytes == 2) {
		dataTypeName = "uint16_t";
		byteCount = 2;
		maxColCount = 16;
	} else {
		dataTypeName = "uint8_t";
		byteCount = 1;
		maxColCount = 32;
	}

	size_t blockSize = roundUp(size, byteCount);
	size_t blockCount = blockSize / byteCount;

	printf("const %s data_%s[%zu] = {\n", dataTypeName, dataName, blockCount);

	size_t remainingSize = size;
	const uint8_t *p = mem;
	size_t outByteCount = 0;
	while(remainingSize > 0) {
		size_t colCount = min(remainingSize / byteCount, maxColCount);
		printf("\t");
		size_t readBytes = 0;
		if(colCount > 0) {
			for(size_t col = 0; col < colCount; ++col) {
				if(col > 0) {
					printf(",");
				}
				if(byteCount == 8) {
					uint64_t *x = (uint64_t *)p;
					uint64_t value = x[col];
					printf("0x%016llx", value);
					readBytes += 8;
				} else if(byteCount == 4) {
					uint32_t *x = (uint32_t *)p;
					uint32_t value = x[col];
					printf("0x%08x", value);
					readBytes += 4;
				} else if(byteCount == 2) {
					uint16_t *x = (uint16_t *)p;
					uint16_t value = x[col];
					printf("0x%04x", value);
					readBytes += 2;
				} else {
					uint8_t value = p[col];
					printf("0x%02x", value);
					++readBytes;
				}
			}
		} else {
			size_t pad = byteCount - remainingSize;

			uint64_t finalValue = 0;
			for(int i = 0; i < remainingSize; ++i) {
				uint8_t b8 = p[i];
				int bits = i * 8;
				finalValue |= (((uint64_t)b8 & 0xFF) << bits);
			}
			readBytes += remainingSize;

			if(byteCount == 8) {
				printf("0x%016llx\n", finalValue);
			} else if(byteCount == 4) {
				uint32_t *x = (uint32_t *)p;
				uint32_t value = (uint32_t)(finalValue & 0xFFFFFFFF);
				printf("0x%08x", value);
			} else if(byteCount == 2) {
				uint16_t value = (uint16_t)(finalValue & 0xFFFF);
				printf("0x%04x", value);
			} else {
				uint8_t value = (uint8_t)(finalValue & 0xFF);
				printf("0x%02x", value);
			}
		}
		outByteCount += readBytes;
		remainingSize -= readBytes;
		p += readBytes;
		if(remainingSize > 0) {
			printf(",");
		}
		if (remainingSize > 0)
			printf("\n");
	}

	assert(outByteCount == size);

	printf("};\n");
	printf("const size_t sizeOf_%s = %zu;\n", dataName, size);
	printf("const size_t blockSizeOf_%s = %zu;\n", dataName, blockSize);
	printf("const uint8_t *ptr_%s = (uint8_t *)data_%s;\n", dataName, dataName);

	free(mem);
}