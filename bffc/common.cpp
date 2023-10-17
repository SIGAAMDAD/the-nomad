#include "../bff_file/g_bff.h"
#define LOG_WARN(...) Con_Printf(__VA_ARGS__)
#define N_Error BFF_Error
#include "zone.h"

#if 0
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
#endif

FILE* SafeOpen(const char* filepath, const char* mode)
{
	FILE* fp = fopen(filepath, mode);
	if (!fp) {
		BFF_Error("SafeOpen: failed to open file %s in mode %s, errno: %s", filepath, mode, strerror(errno));
	}
	return fp;
}

void* SafeRealloc(void *ptr, size_t nsize)
{
	void *p = realloc(ptr, nsize);
	if (p == NULL) {
		BFF_Error("SafeRealloc: realloc() failed on reallocation of %li bytes, errno: %s", nsize, strerror(errno));
	}
	return p;
}

void* SafeMalloc(size_t size, const char *name)
{
//	Con_Printf("SafeMalloc: allocating %lu bytes with malloc() for %s", size, name);
	void *p = malloc(size);
	if (p == NULL) {
		BFF_Error("SafeMalloc: malloc() failed on allocation of %li bytes, errno: %s", size, strerror(errno));
	}
	return p;
}


void BFF_Error(const char* fmt, ...)
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

void LoadFile(FILE* fp, void** buffer, int64_t* length)
{
	fseek(fp, 0L, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	void *buf = SafeMalloc(*length, "filebuf");
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

static inline const char *BFF_CompressionString(int compression)
{
	switch (compression) {
	case COMPRESSION_BZIP2: return "bzip2";
	case COMPRESSION_ZLIB: return "zlib";
	};
	return "None";
}

#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif

void DecompileBFF(const char *filepath)
{
	uint64_t offset, nameLen, size;
	bffheader_t header;
	char name[MAX_OSPATH];
	char gameName[MAX_BFF_PATH];
	FILE *fp;

	fp = SafeOpen(filepath, "rb");

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
	if (header.version != BFF_VERSION) {
		Con_Printf("======== WARNING: Version found in header isn't the same as this application's ========\n");
	}
	
	SafeRead(fp, gameName, sizeof(gameName));

	Con_Printf(
		"<-------- HEADER -------->\n"
		"name: %s\n"
		"total chunks: %lu\n"
		"compression: %s\n"
		"version: %hu\n",
	gameName, header.numChunks, BFF_CompressionString(header.compression), header.version);

	for (int64_t i = 0; i < header.numChunks; i++) {
		offset = ftell(fp);

		SafeRead(fp, &nameLen, sizeof(uint64_t));
		SafeRead(fp, name, nameLen);
		SafeRead(fp, &size, sizeof(int64_t));
		Con_Printf(
			"<-------- CHUNK %li -------->\n"
			"size: %3.03f KiB\n"
			"name: %s\n"
			"offset: %lu\n",
		i, ((float)size / 1024), name, offset);

		offset = ftell(fp);
		fseek(fp, offset + size, SEEK_SET);
	}
	fclose(fp);
}