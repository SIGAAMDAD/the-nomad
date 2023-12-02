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


static inline const char *bzip2_strerror(int err)
{
	switch (err) {
	case BZ_DATA_ERROR: return "(BZ_DATA_ERROR) buffer provided to bzip2 was corrupted";
	case BZ_MEM_ERROR: return "(BZ_MEM_ERROR) memory allocation request made by bzip2 failed";
	case BZ_DATA_ERROR_MAGIC: return "(BZ_DATA_ERROR_MAGIC) buffer was not compressed with bzip2, it did not contain \"BZA\"";
	case BZ_IO_ERROR: return va("(BZ_IO_ERROR) failure to read or write, file I/O error");
	case BZ_UNEXPECTED_EOF: return "(BZ_UNEXPECTED_EOF) unexpected end of data stream";
	case BZ_OUTBUFF_FULL: return "(BZ_OUTBUFF_FULL) buffer overflow";
	case BZ_SEQUENCE_ERROR: return "(BZ_SEQUENCE_ERROR) bad function call error, please report this bug";
	case BZ_OK:
		break;
	};
	return "No Error... How?";
}

static inline void CheckBZIP2(int errcode, uint64_t buflen, const char *action)
{
	switch (errcode) {
	case BZ_OK:
	case BZ_RUN_OK:
	case BZ_FLUSH_OK:
	case BZ_FINISH_OK:
	case BZ_STREAM_END:
		return; // all good
	case BZ_CONFIG_ERROR:
	case BZ_DATA_ERROR:
	case BZ_DATA_ERROR_MAGIC:
	case BZ_IO_ERROR:
	case BZ_MEM_ERROR:
	case BZ_PARAM_ERROR:
	case BZ_SEQUENCE_ERROR:
	case BZ_OUTBUFF_FULL:
	case BZ_UNEXPECTED_EOF:
		Error("Failure on %s of %lu bytes. BZIP2 error reason:\n\t%s", action, buflen, bzip2_strerror(errcode));
		break;
	};
}

static char *Compress_BZIP2(void *buf, uint64_t buflen, uint64_t *outlen)
{
#ifdef _WIN32
	return NULL;
#else
	char *out, *newbuf;
	unsigned int len;
	int ret;

	Printf("Compressing %lu bytes with bzip2...", buflen);

	len = buflen;
	out = (char *)GetMemory(buflen);
	ret = BZ2_bzBuffToBuffCompress(out, &len, (char *)buf, buflen, 9, 4, 50);
	CheckBZIP2(ret, buflen, "compression");

	Printf("Successful compression of %lu to %u bytes with zlib", buflen, len);
	newbuf = (char *)GetMemory(len);
	memcpy(newbuf, out, len);
	FreeMemory(out);
	*outlen = len;

	return newbuf;
#endif
}

static inline const char *zlib_strerror(int err)
{
	switch (err) {
	case Z_DATA_ERROR: return "(Z_DATA_ERROR) buffer provided to zlib was corrupted";
	case Z_BUF_ERROR: return "(Z_BUF_ERROR) buffer overflow";
	case Z_STREAM_ERROR: return "(Z_STREAM_ERROR) bad params passed to zlib, please report this bug";
	case Z_MEM_ERROR: return "(Z_MEM_ERROR) memory allocation request made by zlib failed";
	case Z_OK:
		break;
	};
	return "No Error... How?";
}

static void *zalloc(voidpf opaque, uInt items, uInt size)
{
	(void)opaque;
	(void)items;
	return GetMemory(size);
}

static void zfreeMemory(voidpf opaque, voidpf address)
{
	(void)opaque;
	FreeMemory((void *)address);
}

static char *Compress_ZLIB(void *buf, uint64_t buflen, uint64_t *outlen)
{
	char *out, *newbuf;
	const uint64_t expectedLen = buflen / 2;
	int ret;

	out = (char *)GetMemory(buflen);

#if 0
	stream.zalloc = zalloc;
	stream.zfree = zfree;
	stream.opaque = Z_NULL;
	stream.next_in = (Bytef *)buf;
	stream.avail_in = buflen;
	stream.next_out = (Bytef *)out;
	stream.avail_out = buflen;

	ret = deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 9, );
	if (ret != Z_OK) {
		Error("Failed to compress buffer of %lu bytes (inflateInit2)", buflen);
		return NULL;
	}
	inflate

	do {
		ret = inflate(&stream, Z_SYNC_FLUSH);

		switch (ret) {
		case Z_NEED_DICT:
		case Z_STREAM_ERROR:
			ret = Z_DATA_ERROR;
			break;
		case Z_DATA_ERRO:
		case Z_MEM_ERROR:
			inflateEnd(&stream);
			Error("Failed to compress buffer of %lu bytes (inflate)", buflen);
			return NULL;
		};
		
		if (ret != Z_STREAM_END) {
			newbuf = (char *)Malloc(buflen * 2);
			memcpy(newbuf, out, buflen * 2);
			FreeMemory(out);
			out = newbuf;

			stream.next_out = (Bytef *)(out + buflen);
			stream.avail_out = buflen;
			buflen *= 2;
		}
	} while (ret != Z_STREAM_END);
#endif
	Printf("Compressing %lu bytes with zlib", buflen);

	ret = compress2((Bytef *)out, (uLongf *)outlen, (const Bytef *)buf, buflen, Z_BEST_COMPRESSION);
	if (ret != Z_OK)
		Error("Failure on compression of %lu bytes. ZLIB error reason:\n\t%s", buflen, zError(ret));
	
	Printf("Successful compression of %lu to %lu bytes with zlib", buflen, *outlen);
	newbuf = (char *)GetMemory(*outlen);
	memcpy(newbuf, out, *outlen);
	FreeMemory(out);

	return newbuf;
}

char *Compress(void *buf, uint64_t buflen, uint64_t *outlen, int compression)
{
	switch (compression) {
	case COMPRESS_BZIP2:
		return Compress_BZIP2(buf, buflen, outlen);
	case COMPRESS_ZLIB:
		return Compress_ZLIB(buf, buflen, outlen);
	default:
		break;
	};
	return (char *)buf;
}

static char *Decompress_BZIP2(void *buf, uint64_t buflen, uint64_t *outlen)
{
#ifdef _WIN32
	return NULL;
#else
	char *out, *newbuf;
	unsigned int len;
	int ret;

	Printf("Decompressing %lu bytes with bzip2", buflen);
	len = buflen * 2;
	out = (char *)GetMemory(buflen * 2);
	ret = BZ2_bzBuffToBuffDecompress(out, &len, (char *)buf, buflen, 0, 4);
	CheckBZIP2(ret, buflen, "decompression");

	Printf("Successful decompression of %lu to %u bytes with bzip2", buflen, len);
	newbuf = (char *)GetMemory(len);
	memcpy(newbuf, out, len);
	FreeMemory(out);
	*outlen = len;

	return newbuf;
#endif
}

static char *Decompress_ZLIB(void *buf, uint64_t buflen, uint64_t *outlen)
{
	char *out, *newbuf;
	int ret;

	Printf("Decompressing %lu bytes with zlib...", buflen);
	out = (char *)GetMemory(buflen * 2);
	*outlen = buflen * 2;
	ret = uncompress((Bytef *)out, (uLongf *)outlen, (const Bytef *)buf, buflen);
	if (ret != Z_OK)
		Error("Failure on decompression of %lu bytes. ZLIB error reason:\n\t:%s", buflen * 2, zError(ret));
	
	Printf("Successful decompression of %lu bytes to %lu bytes with zlib", buflen, *outlen);
	newbuf = (char *)GetMemory(*outlen);
	memcpy(newbuf, out, *outlen);
	FreeMemory(out);

	return newbuf;
}

char *Decompress(void *buf, uint64_t buflen, uint64_t *outlen, int compression)
{
	switch (compression) {
	case COMPRESS_BZIP2:
		return Decompress_BZIP2(buf, buflen, outlen);
	case COMPRESS_ZLIB:
		return Decompress_ZLIB(buf, buflen, outlen);
	default:
		break;
	};
	return (char *)buf;
}
