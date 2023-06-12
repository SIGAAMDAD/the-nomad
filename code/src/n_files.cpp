#include "n_shared.h"
#include "g_bff.h"

#define MAX_FILE_HANDLES 1024
#define MAX_BFF_FILES 8
#define MAX_FILE_HASH 1024
#define FS_HashFileName Com_GenerateHashValue
#define MAX_TMP_FILES 16

typedef struct
{
	char name[MAX_BFF_CHUNKNAME];

	char *buffer;
	char *bufPtr;
	uint32_t size;
} fileInBFF_t;

typedef struct
{
	char name[256];
	fileInBFF_t *fileList;
	bff_t *handle;
	
	uint32_t numfiles;
	uint32_t handlesUsed;
	int32_t index;
} bffFile_t;

typedef union fileData
{
	FILE* fp;
	fileInBFF_t* chunk;
	void* stream;
	char* buffer;
} fileData;

typedef struct
{
	char name[MAX_GDR_PATH];

	fileData data;

	bffFile_t* bff;
	qboolean used;
	qboolean bffFile;
	qboolean tmpFile;
	int32_t bffIndex;
} fileHandle_t;

static fileHandle_t handles[MAX_FILE_HANDLES];
static bffFile_t* bffs[MAX_BFF_FILES];

static uint32_t fs_numHandles;
static uint32_t fs_totalArchives;
cvar_t fs_gamedir     = {"fs_gamedir",     "gamedata", 0.0f, 0, qfalse, TYPE_STRING, qtrue};
cvar_t fs_numArchives = {"fs_numArchives", "",         0.0f, 1, qfalse, TYPE_INT,    qtrue};
static GDRStr fs_homepath;
static bool fs_initialized;
static uint64_t fs_writeCount;
static uint64_t fs_readCount;
static int32_t fs_lastBffIndex;
static uint32_t fs_numTmpFiles;
static uint32_t fs_loadStack;

static bool FS_IsChunk(fileHandle_t* handle);
static bool FS_IsChunk(const char *filepath);
static fileInBFF_t* FS_GetBFFChunk(const char *filepath);
static bffFile_t* FS_GetChunkBFF(fileInBFF_t* chunk);
static char* FS_BuildOSPath(const char *base, const char *game, const char *npath);
static void FS_ReplaceSeparators(char *str);
static void FS_ForceFlush(file_t f);
static uint64_t FS_FileLength(FILE *f);
static qboolean FS_OpenFileInBFF(const char *chunkname, file_t *fd, bffFile_t *bff, fileInBFF_t *chunk, fileHandle_t *f);

static FILE* FS_FileForHandle(file_t f)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileForHandle: out of range");
	}
	if (handles[f].bffFile) {
		N_Error("FS_FileForHandle: can't get FILE on bff file");
	}
	if (!handles[f].data.fp) {
		N_Error("FS_FileForHandle: NULL");
	}
	return handles[f].data.fp;
}

static file_t FS_HandleForFile(void)
{
	file_t f;
	
	for (f = 1; f < MAX_FILE_HANDLES; f++) {
		if (!handles[f].used) {
			return f;
		}
	}
	Con_Printf("WARNING: no free file handle");
	return FS_INVALID_HANDLE;
}

static void FS_CloseBFF(fileHandle_t* f)
{
	bffFile_t* bff = bffs[f->bffIndex];

	f->bff = NULL;
	f->bffIndex = -1;
	f->bffFile = qfalse;

	Mem_Free(bff->fileList);
	BFF_CloseArchive(bff->handle);
}

static void FS_InitHandle(fileHandle_t* f)
{
	f->bffIndex = -1;
	f->bff = NULL;
	f->bffFile = qfalse;
	f->data.stream = NULL;
	f->tmpFile = qfalse;
	f->used = qfalse;
}

static void FS_InitBFFs(void)
{
	bff_t *bff;
	bffFile_t *file;
	char bffpath[12];
	char *ospath;
	
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		snprintf(bffpath, sizeof(bffpath), "bff%i.bff", i);

		ospath = FS_BuildOSPath(fs_homepath.c_str(), NULL, bffpath);
		bff = BFF_OpenArchive(ospath);

		bffs[i] = (bffFile_t *)Mem_Alloc(sizeof(bffFile_t));
		file = bffs[i];
	
		file->index = (int32_t)i;
		file->handle = bff;
		file->handlesUsed = 0;
		file->numfiles = bff->numChunks;
		file->fileList = (fileInBFF_t *)Mem_Alloc(sizeof(fileInBFF_t) * file->numfiles);

		N_strncpy(file->name, bff->bffGamename, 256);

		for (uint32_t c = 0; c < bff->numChunks; c++) {
			N_strncpy(file->fileList[c].name, bff->chunkList[c].chunkName, MAX_BFF_CHUNKNAME);
			file->fileList[c].buffer = bff->chunkList[c].chunkBuffer;
			file->fileList[c].size = bff->chunkList[c].chunkSize;
			file->fileList[c].bufPtr = bff->chunkList[c].chunkBuffer;
		}
	}
}

uint32_t FS_NumBFFs(void)
{
	return fs_totalArchives;
}

file_t FS_OpenBFF(int32_t index)
{
	file_t fd;
	fileHandle_t *f;
	bffFile_t *file;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (index == -1) {
		N_Error("FS_OpenBFF: invalid index");
	}
	if (index > fs_totalArchives) {
		Con_Printf("WARNING: index over fs_totalArchives (FS_OpenBFF)");
		return FS_INVALID_HANDLE;
	}
	fd = FS_HandleForFile();
	f = &handles[fd];
	FS_InitHandle(f);
	
	f->bff = bffs[index];
	f->bffFile = qtrue;
	f->bffIndex = index;
	N_strncpy(f->name, bffs[index]->name, MAX_GDR_PATH);
	f->used = qtrue;

	return fd;
}

uint64_t FS_LoadFile(const char *filepath, void **buffer)
{
	file_t fd = FS_FOpenRead(filepath);
	if (fd == FS_INVALID_HANDLE) {
		N_Error("FS_LoadFile: failed to load file %s", filepath);
	}

	uint64_t fsize = FS_FileLength(fd);
	void *buf = Hunk_TempAlloc(fsize);
	FS_Read(buf, fsize, fd);
	*buffer = buf;

	return fsize;
}

static bool FS_AllowedExtension(const char *filename, bool allowBFFs, const char **extension)
{
	static const char *extlist[] = { "dll", "so", "exe", "dylib", "qvm", "bff" };
	const char *e;
	uint32_t n;

	e = strrchr(filename, '.');

	if (allowBFFs)
		n = arraylen(extlist);
	else
		n = arraylen(extlist) - 1;

	for (uint32_t i = 0; i < n; i++) {
		if (N_strcasecmp(e, extlist[n])) {
			if (extension) {
				*extension = extlist[n];
			}
			return false;
		}
	}
	return true;
}

static void FS_CheckFilenameAllowed(const char *filename, const char *function, bool allowBFFs)
{
	const char *extension;
	if (!FS_AllowedExtension(filename, allowBFFs, &extension)) {
		N_Error("%s: not allowed to manipulate '%s' due to %s extension", function, filename, extension);
	}
}

void FS_Remove(const char *ospath)
{
	FS_CheckFilenameAllowed(ospath, __func__, qtrue);
	remove(FS_BuildOSPath(fs_homepath.c_str(), NULL, ospath));
}

uint64_t FS_FileLength(file_t f)
{
	fileHandle_t* file;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileLength: out of range");
	}

	file = &handles[f];

	if (FS_IsChunk(file)) {
		return file->data.chunk->size;
	}
	else {
		return FS_FileLength(file->data.fp);
	}
}

uint64_t FS_FileTell(file_t f)
{
	fileHandle_t* file;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileTell: out of range");
	}
	
	file = &handles[f];

	if (FS_IsChunk(file)) {
		return (uint64_t)(file->data.chunk->bufPtr - file->data.chunk->buffer);
	}
	else {
		return (uint64_t)ftell(file->data.fp);
	}
}

fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence)
{
	fileHandle_t* file;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileSeek: out of range");
	}
	
	file = &handles[f];

	if (FS_IsChunk(file)) {
		char *begin = file->data.chunk->buffer;

		if (whence == FS_SEEK_END && offset) {
			return -1;
		}
		else if (whence == FS_SEEK_CUR
		&& file->data.chunk->bufPtr + offset >= &file->data.chunk->buffer[file->data.chunk->size]) {
			return -1;
		}
		switch (whence) {
		case FS_SEEK_CUR:
			file->data.chunk->bufPtr += offset;
			break;
		case FS_SEEK_BEGIN:
			file->data.chunk->bufPtr = begin + offset;
			break;
		case FS_SEEK_END:
			file->data.chunk->bufPtr = &begin[file->data.chunk->size];
			break;
		};
	}
	else {
		uint32_t fwhence;
		switch (whence) {
		case FS_SEEK_CUR:
			fwhence = SEEK_CUR;
			break;
		case FS_SEEK_BEGIN:
			fwhence = SEEK_SET;
			break;
		case FS_SEEK_END:
			fwhence = SEEK_END;
			break;
		default:
			N_Error("FS_FileSeek: invalid whence");
		};
		fseek(file->data.fp, (long)offset, (int)fwhence);
	}
}

void FS_Init(void)
{
	fs_homepath = Sys_pwd();

	int numArchives = fs_numArchives.i;
	if (numArchives >= MAX_BFF_FILES) {
		N_Error("FS_Init: too many bff files");
	}

	fs_totalArchives = numArchives;

	FS_InitBFFs();

	for (uint32_t i = 0; i < MAX_FILE_HANDLES; i++) {
		handles[i].used = qfalse;
		handles[i].data.stream = NULL;
	}

	fs_lastBffIndex = -1;
	fs_readCount = 0;
	fs_writeCount = 0;
	fs_numHandles = 0;
	fs_initialized = true;
	fs_numTmpFiles = 0;
}

uint64_t FS_Write(const void *buffer, uint64_t size, file_t fd)
{
	fileHandle_t *f;
	uint64_t writeCount;

	if (fd <= FS_INVALID_HANDLE || fd >= MAX_FILE_HANDLES) {
		N_Error("FS_Write: out of range");
	}

	f = &handles[fd];

	// bff chunks are read-only
	if (FS_IsChunk(f)) {
		Con_Printf("WARNING: attempted to write to bff chunk");
	}
	else {
		writeCount = fwrite(buffer, 1, size, f->data.fp);
		if (writeCount != size) {
			N_Error("FS_Write: short write of %lu bytes, should have read %lu bytes", writeCount, size);
		}
		fs_writeCount += writeCount;
		return writeCount;
	}
	return 0;
}

uint64_t FS_Read(void *buffer, uint64_t size, file_t fd)
{
	fileHandle_t *f;
	uint64_t readCount;

	if (fd <= FS_INVALID_HANDLE || fd >= MAX_FILE_HANDLES) {
		N_Error("FS_Read: out of range");
	}

	f = &handles[fd];

	if (FS_IsChunk(f)) {
		if (f->data.chunk->bufPtr + size > &f->data.chunk->buffer[f->data.chunk->size]) {
			// should (logically) never happen, if it does, probably a code or bff corruption error
			N_Error("FS_Read: overread of %lu bytes", size);
		}
		memcpy(buffer, f->data.chunk->bufPtr, size);
		f->data.chunk->bufPtr += size;
		fs_readCount += size;
		return size;
	}
	// bff archives and normal files
	else {
		readCount = fread(buffer, 1, size, f->data.fp);
		if (readCount != size) {
			N_Error("FS_Read: short read of %lu bytes, should have read %lu bytes", readCount, size);
		}
		fs_readCount += readCount;
		return readCount;
	}
	return 0;
}

file_t FS_CreateTmp(char **name, const char *ext)
{
	file_t fd;
	fileHandle_t *f;
	static char *ospath;
	const char *tmpname;
	FILE *fp;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (fs_numTmpFiles >= MAX_TMP_FILES) {
		N_Error("FS_CreateTmp: too many temp files");
	}

	fd = FS_HandleForFile();
	f = &handles[fd];
	FS_InitHandle(f);

	if (ext)
		tmpname = va("tmpfile%i.%s", fs_numTmpFiles, ext);
	else
		tmpname = va("tmpfile%i.ntf", fs_numTmpFiles);

	ospath = FS_BuildOSPath(fs_homepath.c_str(), NULL, tmpname);
	f->tmpFile = qtrue;
	f->data.fp = Sys_FOpen(ospath, "wb+");
	if (!f->data.fp) {
		return FS_INVALID_HANDLE;
	}
	f->used = qtrue;
	N_strncpy(f->name, tmpname, MAX_GDR_PATH);
	*name = ospath;

#ifdef _NOMAD_DEBUG
	Con_Printf("DEBUG: created tempfile %s", ospath);
#endif

	fs_numTmpFiles++;

	return fd;
}

void* FS_GetBFFData(file_t handle)
{
	return (void *)handles[handle].bff->handle;
}

void FS_FClose(file_t f)
{
	fileHandle_t* handle;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FClose: out of range");
	}

	handle = &handles[f];
	if (!handle->used) {
		N_Error("FS_FClose: file closed twice");
	}

	// handle is a bff archive
	if (handle->bff && handle->bffFile && handle->bffIndex > -1 && !handle->data.stream) {
		FS_CloseBFF(handle);
	}
	else if (FS_IsChunk(handle)) {
		handle->bff = NULL;
		handle->bffIndex = -1;
		handle->bffFile = qfalse;
	}
	else {
		FS_ForceFlush(f);
		fclose(handle->data.fp);
		if (handle->tmpFile) { // its a temporary file, delete when closing
			FS_Remove(handle->name);
			handle->tmpFile = qfalse;
			fs_numTmpFiles--;
		}
	}

	handle->used = qfalse;
	handle->data.stream = NULL;
}

char* FS_GetOSPath(file_t f)
{
	fileHandle_t *file;
	static char *ospath;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_GetOSPath: out of range");
	}

	file = &handles[f];
	ospath = FS_BuildOSPath(fs_homepath.c_str(), NULL, file->name);

	return ospath;
}

file_t FS_FOpenWrite(const char *filepath)
{
	char *ospath;
	fileHandle_t *f;
	file_t fd;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!filepath) {
		N_Error("FS_FOpenWrite: NULL filepath");
	}
	if (!*filepath) {
		N_Error("FS_FOpenWrite: empty filepath");
	}

	fd = FS_HandleForFile();
	f = &handles[fd];
	FS_InitHandle(f);

	if (FS_IsChunk(filepath)) {
		N_Error("FS_FOpenWrite: cannot create a write stream for a bff chunk");
	}
	else {
		ospath = FS_BuildOSPath(fs_homepath.c_str(), NULL, filepath);
		N_strncpy(f->name, filepath, MAX_GDR_PATH);
		f->data.fp = Sys_FOpen(ospath, "wb");
		if (!f->data.fp) {
			N_Error("FS_FOpenWrite: failed to open file %s", ospath);
			return FS_INVALID_HANDLE;
		}
	}
	f->used = qtrue;
	return fd;
}

file_t FS_FOpenRead(const char *filepath)
{
	char *ospath;
	fileHandle_t* f;
	file_t fd;
	bffFile_t* bff;
	fileInBFF_t* chunk;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!filepath) {
		N_Error("FS_FOpenRead: NULL filepath");
	}
	if (!*filepath) {
		N_Error("FS_FOpenRead: empty filepath");
	}

	fd = FS_HandleForFile();
	f = &handles[fd];
	FS_InitHandle(f);

	if (FS_IsChunk(filepath)) {
		chunk = FS_GetBFFChunk(filepath);
		if (!chunk) {
			Con_Printf("WARNING: failed to load bff chunk %s", filepath);
			return FS_INVALID_HANDLE;
		}
		bff = FS_GetChunkBFF(chunk);
		if (!FS_OpenFileInBFF(filepath, &fd, bff, chunk, f)) {
			return FS_INVALID_HANDLE;
		}
	}
	// normal file
	else {
		ospath = FS_BuildOSPath(fs_homepath.c_str(), NULL, filepath);
		N_strncpy(f->name, filepath, MAX_GDR_PATH);
		f->data.fp = Sys_FOpen(ospath, "rb");
		if (!f->data.fp) {
			N_Error("FS_FOpenRead: failed to open file %s", ospath);
			return FS_INVALID_HANDLE;
		}
	}
	f->used = qtrue;
	return fd;
}

static void FS_ReplaceSeparators(char *str)
{
	char *s;

	for (s = str; *s; s++) {
		if (*s == PATH_SEP_FOREIGN) {
			*s = PATH_SEP;
		}
	}
}

static uint64_t FS_FileLength(FILE *f)
{
	uint64_t pos, end;

	pos = ftell(f);
	fseek(f, 0L, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);
	
	return end;
}

static void FS_ForceFlush(file_t f)
{
	FILE *fp;

	fp = FS_FileForHandle(f);
	setvbuf(fp, NULL, _IONBF, 0);
}

static char* FS_BuildOSPath(const char *base, const char *game, const char *npath)
{
	char temp[MAX_OSPATH*2+1];
	static char ospath[2][sizeof(temp)+MAX_OSPATH];
	static uint32_t toggle;

	toggle ^= 1;

	if (!game || !*game)
		game = fs_gamedir.s;
	
	if (npath)
		stbsp_snprintf(temp, sizeof(temp), "%c%s%c%s", PATH_SEP, game, PATH_SEP, npath);
	else
		stbsp_snprintf(temp, sizeof(temp), "%c%s", PATH_SEP, game);
	
	FS_ReplaceSeparators(temp);
	stbsp_snprintf(ospath[toggle], sizeof(ospath[0]), "%s%s", base, temp);

	return ospath[toggle];
}

static bool FS_IsChunk(fileHandle_t *handle)
{
	if (!handle->bff) return false;
	if (handle->bffIndex == -1) return false;
	if (!handle->bffFile) return false;
	return true;
}

static bool FS_IsChunk(const char *filepath)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		for (uint32_t c = 0; c < bffs[i]->numfiles; c++) {
			if (N_strncmp(filepath, bffs[i]->fileList[c].name, MAX_BFF_CHUNKNAME)) {
				return true;
			}
		}
	}
	return false;
}
static fileInBFF_t* FS_GetBFFChunk(const char *filepath)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		for (uint32_t c = 0; c < bffs[i]->numfiles; c++) {
			if (N_strncmp(filepath, bffs[i]->fileList[c].name, MAX_BFF_CHUNKNAME)) {
				return &bffs[i]->fileList[c];
			}
		}
	}
	return NULL;
}
static bffFile_t* FS_GetChunkBFF(fileInBFF_t* chunk)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		for (uint32_t c = 0; c < bffs[i]->numfiles; c++) {
			if (chunk == &bffs[i]->fileList[c]) {
				return bffs[i];
			}
		}
	}
	return NULL;
}

static qboolean FS_OpenFileInBFF(const char *chunkname, file_t *fd, bffFile_t *bff, fileInBFF_t *chunk, fileHandle_t *f)
{
	f->bffFile = qtrue;
	f->bff = bff;
	f->bffIndex = bff->index;
	f->data.chunk = chunk;
	fs_lastBffIndex = bff->index;

	N_strncpy(f->name, chunk->name, MAX_BFF_CHUNKNAME);

	bff->handlesUsed++;

	return qtrue;
}