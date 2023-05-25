#include "../src/n_shared.h"
#include "g_bff.h"
#include "../bff_compiler/zone.h"

FILE* SafeOpen(const char* filepath, const char* mode)
{
	FILE* fp = fopen(filepath, mode);
	if (!fp) {
		BFF_Error("SafeOpen: failed to open file %s in mode %s, errno: %s", filepath, mode, strerror(errno));
	}
	return fp;
}

#ifndef _NOMAD_VERSION

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
void LoadFile(FILE* fp, void** buffer, size_t* length)
{
	fseek(fp, 0L, SEEK_END);
	*length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	void *buf = SafeMalloc(*length);
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
#endif