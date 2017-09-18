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
#ifndef NULL
#define NULL 0
#endif

#ifndef MAX_PATH
#ifdef _MAX_PATH
#define MAX_PATH _MAX_PATH
#else
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#endif
#endif

#define GLOBAL_CASE(A, B)			\
case A:								\
	printf("\t%s: %d\n", B, global);	\
	break;							\

#define QUICK_PRINT(A, B)			\
printf("%s: " B "\n", #A, A)		\

#define QUICK_PRINT_UTF16(A, B)		\
printf("%s: " B "\n", (wchar_t *)#A, A)		\

#define IS_BE() ('\xde\xad\xbe\xef' == 0xdeadbeef)

uint32_t swapUint32(char x[4]) {
	//*(uint32_t *)
	if (IS_BE())
		return *(uint32_t *)x;
	else { // uhh what
		uint32_t ret;
		char *b = (char *)&ret;
		b[3] = x[3];
		b[2] = x[2];
		b[1] = x[1];
		b[0] = x[0];
		return ret;
	}
}

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


uint32_t ParsePMX(FILE *fp, sint8_t *filename) {
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

	if (firstHeaderChunk.version != 2.0) {
		printf("ADD VERSION %.1f SUPPORT!\n", firstHeaderChunk.version);
		return -2;
	}
	uint8_t *globals = (uint8_t*)malloc(firstHeaderChunk.globalCount);
	
	QUICK_PRINT(firstHeaderChunk.magic, "0x%x");
	QUICK_PRINT(firstHeaderChunk.version, "%.1f");
	QUICK_PRINT(firstHeaderChunk.globalCount, "%d");
	
	fread(globals, firstHeaderChunk.globalCount, 1, fp);


	printGlobals(globals, firstHeaderChunk.globalCount);

	sint8_t *localModelName = readString(fp);
	sint8_t *universalModelName = readString(fp);
	sint8_t *localComments = readString(fp);
	sint8_t *universalComments = readString(fp);
	
	if (globals[0] == 1) {
		QUICK_PRINT(localModelName, "%s");
		QUICK_PRINT(universalModelName, "%s");
		QUICK_PRINT(localComments, "%s");
		QUICK_PRINT(universalComments, "%s");
	}
	else {
		QUICK_PRINT(localModelName, "%S");
		QUICK_PRINT(universalModelName, "%S");
		QUICK_PRINT(localComments, "%S");
		QUICK_PRINT(universalComments, "%S");
	}

	uint32_t vertexCount;
	fread(&vertexCount, sizeof(uint32_t), 1, fp);

	QUICK_PRINT(vertexCount, "%d");

#pragma pack(push)
#pragma pack(1) // set alignment to 1 byte
	typedef struct _vec_t {
		float x, y, z;
	} vec_t;
#pragma pack(pop) // restore

#pragma pack(push)
#pragma pack(1) // set alignment to 1 byte
	typedef struct _uv_t {
		float u, v;
	} uv_t;
#pragma pack(pop) // restore

	vec_t *vertList = (vec_t *)malloc(sizeof(vec_t) * vertexCount);
	vec_t *normalList = (vec_t *)malloc(sizeof(vec_t) * vertexCount);
	uv_t *uvList = (uv_t *)malloc(sizeof(uv_t) * vertexCount);
	char objFilename[MAX_PATH];
	sprintf(objFilename, "%s_out.obj", filename);
	FILE *obj = fopen(objFilename, "wb");
	if (!obj) {
		printf("Failed to create %s file!\n", objFilename);
		return -3;
	}

	for (uint32_t i = 0; i < vertexCount; i++) {
		vec_t vertex;
		fread(&vertex, sizeof(vec_t), 1, fp);
		vec_t normal;
		fread(&normal, sizeof(vec_t), 1, fp);
		uv_t uv;
		fread(&uv, sizeof(uv_t), 1, fp);
		if (globals[1] > 0) {
			float *appendixUV = (float *)malloc(sizeof(float) * globals[1]);
			fread(appendixUV, sizeof(float), globals[2], fp); // Unused for now
			free(appendixUV);
		}
		uint8_t weightType;
		fread(&weightType, sizeof(uint8_t), 1, fp);

		if (weightType == 0) { // BDEF1
			fseek(fp, globals[5], SEEK_CUR);
		}
		else if (weightType == 1) { // BDEF2
			fseek(fp, (globals[5] * 2) + sizeof(float), SEEK_CUR);
		}
		else if (weightType == 2) { // BDEF4
			fseek(fp, (globals[5] * 4) + (sizeof(float) * 4), SEEK_CUR);
		}
		else if (weightType == 3) { // SDEF
			fseek(fp, (globals[5] * 2) + sizeof(float) + ( sizeof(vec_t) * 3 ), SEEK_CUR);
		}
		else {
			printf("INVALID WEIGHT TYPE! %d\n", ftell(fp));
			fclose(obj);
			remove(objFilename);
			free(vertList);
			free(normalList);
			free(uvList);
			free(globals);
			return -3;
		}

		float edgeScale;
		fread(&edgeScale, sizeof(float), 1, fp);


		vertList[i] = vertex;
		normalList[i] = normal;
		uvList[i] = uv;

		char buf[1024];
		sprintf(buf, "v %.3f %.3f %.3f 1.000\nvn %.3f %.3f %.3f\nvt %.3f %.3f\n", vertex.x, vertex.y, vertex.z, normal.x, normal.y, normal.z, uv.u, uv.v);
		fwrite(buf, 1, strlen(buf), obj);
	}
	uint32_t faceCount;
	fread(&faceCount, sizeof(faceCount), 1, fp);
	QUICK_PRINT(faceCount, "%d");

	uint32_t *faces = (uint32_t *)malloc(faceCount * sizeof(uint32_t));
	uint32_t j = 0;
	for (uint32_t i = 0; i < faceCount; i++) {
		char val[4];
		memset(val, 0, 4);
		fread(&val, globals[2], 1, fp);
		faces[i] = swapUint32(val);
		//QUICK_PRINT(faces[i], "%d");
	}
	for (uint32_t i = 0; i < faceCount; i+= 3) {
		char buf[256];
		sprintf(buf, "f %d/%d %d/%d %d/%d\n", faces[i] + 1, faces[i] + 1, faces[i+1] + 1, faces[i + 1] + 1, faces[i+2] + 1, faces[i + 2] + 1);
		fwrite(buf, sizeof(uint8_t), strlen(buf), obj);
	}

	fclose(obj);
	free(faces);
	free(vertList);
	free(normalList);
	free(uvList);
	free(globals);

	return 0;
}

int main(int argc, char **argv) {
	printf("PMX2OBJ By ogniK! Last built "__DATE__" at "__TIME__"\n");
	printf("---------------------------------------------------------------------------\n");
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
	if ((errCode = ParsePMX(fp, argv[1])) != 0)
		printf("Failed to parse PMX with error code %02X!\n", errCode);
	else
		printf("Done! File saved to %s_out.obj\n", argv[1]);
	fclose(fp);
}
