#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER // Get rid of fopen, fread, fwrite warnings
#pragma warning(disable : 4996)  
#endif

typedef char sint8_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define NULL 0

#define GLOBAL_CASE(A, B)			\
case A:								\
	printf("\t%s: %d\n", B, global);	\
	break;							\

#define QUICK_PRINT(A, B)			\
printf("%s: " B "\n", #A, A);		\

int printGlobals(uint8_t *globals, uint8_t globalCount) {
	printf("--------------------------------\n");
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

sint8_t *readString(FILE *fp) {
	uint32_t len;
	fread(&len, sizeof(uint32_t), 1, fp);
	sint8_t *t = (sint8_t *)malloc(len + 1);
	memset(t, 0, len + 1);
	if (len > 0) {
		fread(t, 1, len, fp);
	}
	return t;
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
	
	QUICK_PRINT(firstHeaderChunk.magic, "0x%x")
	QUICK_PRINT(firstHeaderChunk.version, "%.1f")
	QUICK_PRINT(firstHeaderChunk.globalCount, "%d")
	
	fread(globals, firstHeaderChunk.globalCount, 1, fp);


	printGlobals(globals, firstHeaderChunk.globalCount);

	sint8_t *localModelName = readString(fp);
	sint8_t *universalModelName = readString(fp);
	sint8_t *localComments = readString(fp);
	sint8_t *universalComments = readString(fp);
	
	QUICK_PRINT(localModelName, "%s")
	QUICK_PRINT(universalModelName, "%s")
	QUICK_PRINT(localComments, "%s")
	QUICK_PRINT(universalComments, "%s")

#pragma pack(push)
#pragma pack(1) // set alignment to 1 byte
	typedef struct _headerCounts {
		uint32_t vertexCount;
		uint32_t surfaceCount;
		uint32_t textureCount;
		uint32_t materialCount;
		uint32_t boneCount;
		uint32_t morphCount;
		uint32_t diplayFrameCount;
		uint32_t rigidbodyCount;
		uint32_t jointCount;
	} _secondChunk;
#pragma pack(pop) // restore

	_secondChunk secondHeaderChunk;
	fread(&secondHeaderChunk, sizeof(_secondChunk), 1, fp);

	//printf("")


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
