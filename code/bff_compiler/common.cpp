#include "../bff_file/g_bff.h"
#define LOG_WARN(...) Con_Printf(__VA_ARGS__)
#define N_Error BFF_Error
#include "zone.h"

bff_short_t LittleShort(bff_short_t x)
{
#ifdef __BIG_ENDIAN__
	byte b1, b2;
	b1 = x & 0xFF;
	b2 = (x >> 8) & 0xFF;
	return (b1 << 8) + b2;
#else
	return x;
#endif
}

bff_float_t LittleFloat(bff_float_t x)
{
#ifdef __BIG_ENDIAN__
	union {byte b[4]; float f;} in, out;
	in.f = x;
	out.b[0] = in.b[3];
	out.b[1] = in.b[2];
	out.b[2] = in.b[1];
	out.b[3] = in.b[0];
	return out.f;
#else
	return x;
#endif
}

bff_int_t LittleInt(bff_int_t x)
{
#ifdef __BIG_ENDIAN__
	byte b[4];
	b[0] = x & 0xFF;
	b[1] = (x >> 8) & 0xFF;
	b[2] = (x >> 16) & 0xFF;
	b[3] = (x >> 24) & 0xFF;
	return ((bff_int_t)b[0]<<24) + ((bff_int_t)b[1]<<16) + ((bff_int_t)b[2]<<8) + b[3];
#else
	return x;
#endif
}

FILE* SafeOpen(const char* filepath, const char* mode)
{
	FILE* fp = fopen(filepath, mode);
	if (!fp) {
		BFF_Error("SafeOpen: failed to open file %s in mode %s, errno: %s", filepath, mode, strerror(errno));
	}
	return fp;
}


void* SafeMalloc(size_t size, const char *name)
{
	Con_Printf("SafeMalloc: allocating %lu bytes with malloc() for %s", size, name);
	void *p = malloc(size);
	if (p == NULL) {
		BFF_Error("SafeMalloc: malloc() failed on allocation of %li bytes, errno: %s", size, strerror(errno));
	}
	return p;
}


void __attribute__((noreturn)) BFF_Error(const char* fmt, ...)
{
	fprintf(stderr, "Error: ");
	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(stderr, fmt, argptr);
	va_end(argptr);
	fprintf(stderr, "\n");
	fflush(stderr);
	exit(EXIT_FAILURE);
}

void LoadFile(FILE* fp, void** buffer, bff_int_t* length)
{
	fseek(fp, 0L, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	void *buf = Z_Malloc(*length, TAG_STATIC, &buf, "filebuf");
	fread(buf, sizeof(char), *length, fp);
	*buffer = buf;
}

void Con_Printf(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(stdout, fmt, argptr);
	va_end(argptr);
	fprintf(stdout, "\n");
}

static void SafeRead(FILE* fp, void *data, size_t size)
{
	if (fread(data, size, 1, fp) == 0) {
		BFF_Error("SafeRead: failed to read %lu bytes from file, errno: %s", size, strerror(errno));
	}
}

void DecompileBFF(const char *filepath)
{
	Z_Init();
	FILE* fp = SafeOpen(filepath, "rb");

	bff_t* archive = (bff_t *)Z_Malloc(sizeof(bff_t), TAG_STATIC, &archive, "bffArchive");
	bffheader_t header;
	SafeRead(fp, &header, sizeof(bffheader_t));
	if (header.ident != BFF_IDENT) {
		BFF_Error("DecompileBFF: file isn't a bff archive");
	}
	if (header.magic != HEADER_MAGIC) {
		BFF_Error("DecompileBFF: header magic number is not correct");
	}
	if (!header.numChunks) {
		BFF_Error("DecompileBFF: bad chunk count");
	}

	SafeRead(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeRead(fp, archive->bffGamename, sizeof(archive->bffGamename));

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %lu\n",
	header.numChunks);

	archive->numChunks = header.numChunks;
	archive->chunkList = (bff_chunk_t *)Z_Malloc(sizeof(bff_chunk_t) * header.numChunks, TAG_STATIC, &archive->chunkList, "chunkList");

	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		SafeRead(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeRead(fp, &archive->chunkList[i].chunkSize, sizeof(bff_int_t));
		SafeRead(fp, &archive->chunkList[i].chunkType, sizeof(bff_int_t));
		Con_Printf(
			"<-------- CHUNK %lu -------->\n"
			"size: %lu\n"
			"type: %i\n"
			"name: %s\n",
		i, archive->chunkList[i].chunkSize, archive->chunkList[i].chunkType, archive->chunkList[i].chunkName);

		archive->chunkList[i].chunkBuffer = (char *)Z_Malloc(archive->chunkList[i].chunkSize, TAG_STATIC, &archive->chunkList[i].chunkBuffer, "chunkBuffer");
		SafeRead(fp, archive->chunkList[i].chunkBuffer, sizeof(char) * archive->chunkList[i].chunkSize);
	}
	fclose(fp);

	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		Z_Free(archive->chunkList[i].chunkBuffer);
	}
	Z_Free(archive->chunkList);
	Z_Free(archive);
}