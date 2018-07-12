#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <malloc.h>
#include <assert.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char **argv) {
	if(argc != 2) {
		return -1;
	}
	const char *filePath = argv[1];
	FILE *file;
	if(fopen_s(&file, filePath, "rb") == 0) {
		fseek(file, 0, SEEK_END);
		size_t size = ftell(file);
		fseek(file, 0, SEEK_SET);
		uint8_t *mem = (uint8_t *)malloc(size);
		fread_s(mem, size, size, 1, file);
		fclose(file);

		printf("const unsigned char dataArray[] = {\n");

		size_t maxColCount = 32;
		size_t remainingSize = size;
		const uint8_t *p = mem;
		size_t outByteCount = 0;
		while(remainingSize > 0) {
			size_t colCount = min(remainingSize, maxColCount);
			printf("\t");
			for(int col = 0; col < colCount; ++col) {
				uint8_t value = p[col];
				if(col > 0) {
					printf(",");
				}
				printf("0x%02x", value);
				++outByteCount;
			}
			remainingSize -= colCount;
			p += colCount;
			if(remainingSize > 0) {
				printf(",");
			}
			printf("\n");
		}

		assert(outByteCount == size);

		printf("};\n");
		printf("const size_t dataArraySize = sizeof(dataArray);\n");

		free(mem);
	}
}