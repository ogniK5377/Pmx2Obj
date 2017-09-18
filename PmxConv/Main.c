#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER // Get rid of fopen, fread, fwrite warnings
#pragma warning(disable : 4996)  
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define NULL 0

#define GLOBAL_CASE(A, B)			\
case A:								\
	printf("\t%s: %d\n", B, global);	\
	break;							\

int printGlobals(uint8_t *globals, uint8_t globalCount) {
	printf("Global count: %d\n--------------------------------\n", globalCount);
	for (uint8_t i = 0; i < globalCount; i++) {
		uint8_t global = globals[i];
		switch (i) {
		case 0:
			printf("\tText encoding: %s\n", global == 0 ? "UTF16LE" : "UTF8");
			break;

		GLOBAL_CASE(1, "Additional vec4 count")
		GLOBAL_CASE(2, "Vertex index size")
		GLOBAL_CASE(3, "Texture index size")
		GLOBAL_CASE(4, "Material index size")
		GLOBAL_CASE(5, "Bone index size")
		GLOBAL_CASE(6, "Morph index size")
		GLOBAL_CASE(7, "Rigidbody index size")

		default:
			break;
		}
	}
	printf("--------------------------------\n");
	return 0;
}

uint32_t ParsePMX(FILE *fp) {
#pragma pack(push)
#pragma pack(1) // set alignment to 1 byte
	typedef struct _headerFirst {
		uint32_t magic;
		float version;
		uint8_t globalCount;
	} _firstChunk;
#pragma pack(pop) // restore

	_firstChunk firstHeaderChunk;
	fread(&firstHeaderChunk, sizeof(_firstChunk), 1, fp);
	
	if (firstHeaderChunk.magic != 0x20584D50) {
		printf("Bad magic!\n");
		return -1;
	}
	uint8_t *globals = (uint8_t*)malloc(firstHeaderChunk.globalCount);
	fread(globals, firstHeaderChunk.globalCount, 1, fp);


	printGlobals(globals, firstHeaderChunk.globalCount);
	for (int i = 0; i < firstHeaderChunk.globalCount; i++) {
		printf("%d\n", (char*)(globals)[i]);
	}
	
	free(globals);

	return 0;
}

int main(int argc, char **argv) {
#ifdef _DEBUG
	argc = 2;
	argv[1] = "C:\\cunt\\testmodel.pmx";
#endif
	if (argc < 2) {
		printf("%s <FILE>\n", argv[0]);
		return 0;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		printf("Failed to open \"%s\"!\n", argv[1]);
		return 0;
	}
	uint32_t errCode;
	if ((errCode = ParsePMX(fp)) != 0)
		printf("Failed to parse PMX with error code %02X!\n", errCode);
	fclose(fp);
}
