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
	uint32_t bytesRead;
	uint32_t size;
	qboolean touched;
} fileInBFF_t;

typedef struct
{
	char name[MAX_GDR_PATH];
	fileInBFF_t *fileList;
	bff_t *handle;
	
	uint64_t numfiles;
	uint64_t handlesUsed;
	uint32_t checksum;
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

typedef enum : uint64_t
{
	DIR_STATIC = 0,		// the game can do anything to the directory
	DIR_CONST,			// read-only directory
} diraccess_t;

typedef struct
{
	char *name; // gamedata/
	char *path; // /home/user/glnomad/gamedata/
} directory_t;

typedef struct searchpath_s
{
	directory_t dir;
	bffFile_t *bff;
	struct searchpath_s *next;
	diraccess_t access;
} searchpath_t;

static fileHandle_t handles[MAX_FILE_HANDLES];
static bffFile_t **fs_archives;

static searchpath_t *fs_basepath; // cwd without the other stuff
static searchpath_t *fs_gamepath; // either gamedata/ or a dedicated mod directory
static searchpath_t *fs_homepath; // ospath to the cwd

static uint32_t   fs_numHandles;
static uint32_t   fs_totalArchives;
static qboolean   fs_initialized;
static uint64_t   fs_writeCount;
static uint64_t   fs_readCount;
static int32_t    fs_lastBFFIndex;
static uint32_t   fs_numTmpFiles;
static uint32_t   fs_loadStack;
static const char *fs_homedir;

cvar_t fs_gamedir     = {"fs_gamedir",     "gamedata", 0.0f, 0, qfalse, TYPE_STRING, CVG_ENGINE, CVAR_SAVE | CVAR_ROM | CVAR_DEV};
cvar_t fs_numArchives = {"fs_numArchives", "",         0.0f, 1, qfalse, TYPE_INT, CVG_ENGINE, CVAR_SAVE | CVAR_ROM};

static uint32_t FS_ReturnPath( const char *zname, char *zpath, uint32_t *depth );
static qboolean FS_FilenameCompare(const char *s1, const char *s2);
static int FS_PathCmp(const char *s1, const char *s2);
static bool FS_IsChunk(fileHandle_t* handle);
static bool FS_IsChunk(const char *filepath);
static fileInBFF_t* FS_GetBFFChunk(const char *filepath);
static bffFile_t* FS_GetChunkBFF(fileInBFF_t* chunk);
static char* FS_BuildOSPath(const char *base, const char *game, const char *npath);
static void FS_ReplaceSeparators(char *str);
static void FS_ForceFlush(file_t f);
static uint64_t FS_FileLength(FILE *f);
static qboolean FS_BFFIsRegistered(const bffFile_t *bff);
static qboolean FS_OpenFileInBFF(const char *chunkname, file_t *fd, bffFile_t *bff, fileInBFF_t *chunk, fileHandle_t *f);
static uint32_t FS_MakeBFFChecksum(const bffFile_t *bff);
static void FS_RemoveFromCache(bffFile_t *bff);
static void FS_AddToCache(bffFile_t *bff);
static void FS_FreeUnused(void);
static void FS_FreeBFF(bffFile_t *bff);
static qboolean FS_IsCached(const bffFile_t *bff);
static bool FS_AllowedExtension(const char *filename, bool allowBFFs, const char **extension);

// command console functions
static void FS_ListArchives_f(void);
static void FS_ListRegisteredBFFs_f(void);

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

void FS_VM_FClose(file_t *f)
{
	if (!f) {
		return;
	}
	if (*f <= FS_INVALID_HANDLE || *f >= MAX_FILE_HANDLES) {
		return;
	}

	FS_FClose(*f);
	*f = FS_INVALID_HANDLE;
}

void FS_VM_FOpenWrite(const char *path, file_t *f)
{
	if (!f) {
#ifdef _NOMAD_DEBUG
		N_Error("FS_VM_FOpenWrite: NULL handle");
#else
		Con_Printf(ERROR, "FS_VM_FOpenWrite: NULL handle");
#endif
	}

	*f = FS_FOpenWrite(path);
}

void FS_VM_FOpenRead(const char *path, file_t *f)
{
	if (!f) {
#ifdef _NOMAD_DEBUG
		N_Error("FS_VM_FOpenRead: NULL handle");
#else
		Con_Printf(ERROR, "FS_VM_FOpenWrite: NULL handle");
#endif
	}

	*f = FS_FOpenRead(path);
}

static void FS_FreeBFF(bffFile_t *bff)
{
	uint32_t i;

	// free the base handle
	for (i = 0; i < bff->handle->numChunks; i++)
		Z_Free(bff->handle->chunkList[i].chunkBuffer);

	Z_Free(bff->handle->chunkList);
	Z_Free(bff->handle);

	// don't free it unless its not cached
	if (!FS_IsCached(bff))
		Z_Free(bff);
}

static void FS_CloseBFF(fileHandle_t* f)
{
	FS_FreeBFF(fs_archives[f->bffIndex]);
	f->bff = NULL;
	f->bffIndex = -1;
	f->bffFile = qfalse;
}

static void FS_InitHandle(fileHandle_t* f)
{
	f->bffIndex = -1;
	f->bff = NULL;
	f->bffFile = qfalse;
	f->data.stream = NULL;
	f->tmpFile = qfalse;
	f->used = qtrue;
}

static void FS_LoadFilesFromBFF(bff_chunk_t *chunkList, fileInBFF_t *fileList, uint32_t numchunks)
{
	uint32_t c;
	for (c = 0; c < numchunks; c++) {
		N_strncpyz(fileList[c].name, chunkList[c].chunkName, MAX_BFF_CHUNKNAME);
		fileList[c].buffer = chunkList[c].chunkBuffer;
		fileList[c].size = chunkList[c].chunkSize;
		fileList[c].bytesRead = 0;
		fileList[c].touched = qfalse;
	}
}

static void FS_InitBFFs(void)
{
	bff_t *bff;
	bffFile_t *file;
	searchpath_t *path, *prev;
	char bffpath[12];
	char *ospath;
	
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		snprintf(bffpath, sizeof(bffpath), "bff%i.bff", i);

		ospath = FS_BuildOSPath(fs_homedir, NULL, bffpath);
		bff = BFF_OpenArchive(ospath);

		fs_archives[i] = (bffFile_t *)Z_Malloc(sizeof(bffFile_t), TAG_CBFF, &fs_archives[i], "bffFile");
		file = fs_archives[i];
	
		file->index = (int32_t)i;
		file->handle = bff;
		file->handlesUsed = 0;
		file->numfiles = bff->numChunks;
		file->fileList = (fileInBFF_t *)Z_Malloc(sizeof(fileInBFF_t) * file->numfiles, TAG_CBFF, &file->fileList, "fileList");

		N_strncpyz(file->name, bff->bffGamename, 256);
		FS_LoadFilesFromBFF(bff->chunkList, file->fileList, file->numfiles);
	}

	prev = fs_gamepath;
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		path = (searchpath_t *)Hunk_Alloc(sizeof(*path), "bffpath", h_low);
		path->dir.name = Z_Strdup(fs_archives[i]->handle->bffGamename);
		path->dir.path = Z_Strdup(fs_archives[i]->handle->bffPathname);
		path->bff = fs_archives[i]->handle;
		path->access = DIR_CONST;

		prev->next = path;
		prev = path;
	}
}

/*
FS_ThePurge: frees all untouched/uncached bff files and chunks
*/
void FS_ThePurge(void)
{
	FS_FreeUnused();
	Z_FreeTags(TAG_UBFF, TAG_UBFF); // free it all

	if (fs_loadStack == 0) { // clear the hunk temp if the load stack is empty
		Z_FreeTags(TAG_FILE_USED, TAG_FILE_FREE);
	}
}

/*
FS_Initialized: returns a boolean whether or not FS_Init has been called
*/
qboolean FS_Initialized(void)
{
	return fs_initialized;
}

uint32_t FS_NumBFFs(void)
{
	return fs_totalArchives;
}

/*
FS_LoadStack: returns the filesystem's load stack
*/
uint32_t FS_LoadStack(void)
{
	return fs_loadStack;
}

int32_t FS_LastBFFIndex(void)
{
	return fs_lastBFFIndex;
}



void FS_FreeFile( void *buffer )
{
	if ( !fs_initialized ) {
		N_Error( "Filesystem call made without initialization" );
	}
	if ( !buffer ) {
		N_Error( "FS_FreeFile( NULL )" );
	}
	fs_loadStack--;

	Z_ChangeTag( buffer, TAG_FILE_FREE );

	// if all of our temp files are free, clear all of our space
	if ( fs_loadStack == 0 ) {
		Z_FreeTags(TAG_FILE_USED, TAG_FILE_FREE);
	}
}

qboolean FS_FileExists(const char *file)
{
	const char *path = FS_BuildOSPath(fs_homedir, NULL, file);

	FILE* fp = Sys_FOpen(path, "r");
	if (!fp) {
		return qfalse;
	}
	fclose(fp);
	return qtrue;
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
	
	f->bff = fs_archives[index];
	f->bffFile = qtrue;
	f->bffIndex = index;
	N_strncpyz(f->name, fs_archives[index]->name, MAX_GDR_PATH);
	f->used = qtrue;
	fs_loadStack++;

	fs_lastBFFIndex = index;

	return fd;
}

char* FS_CopyString( const char *in )
{
	char *out;
	out = (char *)Z_Malloc(strlen( in ) + 1, TAG_STATIC, &out, "fsCopyString");
	strcpy( out, in );
	return out;
}

uint64_t FS_LoadFile(const char *path, void **buffer)
{
	char *ospath = FS_BuildOSPath(fs_homedir, NULL, path);
	FILE* fp = Sys_FOpen(ospath, "r");
	if (!fp) {
		Con_Printf(ERROR, "FS_LoadFile: failed to load file named '%s'", path);
		*buffer = NULL;
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	uint64_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	void *buf = Z_Malloc(fsize, TAG_FILE_USED, &buf, "filebuf");
	fread(buf, fsize, 1, fp);
	fclose(fp);
	*buffer = buf;

#if 0
	file_t fd = FS_FOpenRead(path);
	if (fd == FS_INVALID_HANDLE) {
		Con_Printf(ERROR, "FS_LoadFile: failed to load file named '%s'", path);
		*buffer = NULL;
		return 0;
	}

	uint64_t fsize = FS_FileLength(fd);
	void *buf = Z_Malloc(fsize, TAG_FILE_USED, &buf, "filebuf");
	FS_Read(buf, fsize, fd);
	*buffer = buf;
#endif

	return fsize;
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
	remove(FS_BuildOSPath(fs_homedir, NULL, ospath));
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

	return FS_FileLength(file->data.fp);
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
		return (uint64_t)(file->data.chunk->size - file->data.chunk->bytesRead);
	}
	
	return (uint64_t)ftell(file->data.fp);
}

fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence)
{
	fileHandle_t* file;
	uint32_t fwhence;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileSeek: out of range");
	}
	
	file = &handles[f];

	if (FS_IsChunk(file)) {
		if (whence == FS_SEEK_END && offset) {
			return -1;
		}
		else if (whence == FS_SEEK_CUR
		&& file->data.chunk->bytesRead + offset >= file->data.chunk->size) {
			return -1;
		}
		switch (whence) {
		case FS_SEEK_CUR:
			file->data.chunk->bytesRead += offset;
			break;
		case FS_SEEK_BEGIN:
			file->data.chunk->bytesRead = offset;
			break;
		case FS_SEEK_END:
			file->data.chunk->bytesRead = file->data.chunk->size;
			break;
		};
		return (fileOffset_t)(file->data.chunk->size - file->data.chunk->bytesRead);
	}

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
	return (fileOffset_t)fseek(file->data.fp, (long)offset, (int)fwhence);
}

void GDR_DECL FS_Printf(file_t f, const char *fmt, ...)
{
	va_list argptr;
	char msg[MAX_MSG_SIZE];

	va_start(argptr, fmt);
	stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	FS_Write(msg, strlen(msg), f);
}

void FS_Restart(void)
{
	fs_archives = (bffFile_t *)Hunk_Alloc(sizeof(*fs_archives) * fs_numArchives.i, "BFFarchives", h_low);

	Cmd_AddCommand("listbffs", FS_ListArchives_f);

	fs_totalArchives = fs_numArchives.i;

	fs_gamepath = (searchpath_t *)Hunk_Alloc(sizeof(*fs_gamepath) * fs_gamepath, "gamepath", h_low);
	fs_homepath = (searchpath_t *)
}

void FS_Init(void)
{
	fs_homedir = Z_Strdup(Sys_pwd());

	fs_archives = (bffFile_t *)Hunk_Alloc(sizeof(fs_archives));

	for (uint32_t i = 0; i < MAX_FILE_HANDLES; i++) {
		handles[i].used = qfalse;
		handles[i].bff = NULL;
		handles[i].bffIndex = -1;
		handles[i].bffFile = qfalse;
		handles[i].data.stream = NULL;
	}

	Cmd_AddCommand("listbffs", FS_ListArchives_f);
	Cmd_AddCommand("listregistered", FS_ListRegisteredBFFs_f);

	fs_totalArchives = numArchives;

	fs_lastBFFIndex = -1;
	fs_readCount = 0;
	fs_writeCount = 0;
	fs_numHandles = 0;
	fs_initialized = qtrue;
	fs_numTmpFiles = 0;
	fs_loadStack = 0;

	// init the bffs, this'll use the filesystem
	FS_InitBFFs();
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

static uint64_t FS_ReadFromChunk(void *buffer, uint64_t size, fileHandle_t *file)
{
	if (file->data.chunk->bytesRead + size > file->data.chunk->size) {
		// should (logically) never happen, if it does, probably a code or bff corruption error
		N_Error("FS_Read: overread of %lu bytes", size);
	}
	memcpy(buffer, &file->data.chunk->buffer[file->data.chunk->bytesRead], size);
	file->data.chunk->bytesRead += size;
	return size;
}

uint64_t FS_Read(void *buffer, uint64_t size, file_t fd)
{
	fileHandle_t *f;
	int64_t readCount;
	uint64_t remaining;
	int tries;
	byte *buf;

	if (fd <= FS_INVALID_HANDLE || fd >= MAX_FILE_HANDLES) {
		N_Error("FS_Read: out of range");
	}

	f = &handles[fd];

	if (FS_IsChunk(f)) {
		return FS_ReadFromChunk(buffer, size, f);
	}

	buf = (byte *)buffer;
	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		read = fread( buf, 1, block, f->data.fp );
		if (read == 0) {
			// we might have been trying to read from a CD, which
			// sometimes returns a 0 read on windows
			if (!tries) {
				tries = 1;
			} else {
				return size - remaining;	//Com_Error (ERR_FATAL, "FS_Read: 0 bytes read");
			}
		}

		if (read == -1) {
			N_Error("FS_Read: -1 bytes read");
		}

		remaining -= read;
		buf += read;
	}
	return size;
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

	ospath = FS_BuildOSPath(fs_homedir, NULL, tmpname);
	f->tmpFile = qtrue;
	f->data.fp = Sys_FOpen(ospath, "wb+");
	if (!f->data.fp) {
		return FS_INVALID_HANDLE;
	}
	f->used = qtrue;
	N_strncpyz(f->name, tmpname, MAX_GDR_PATH);
	*name = ospath;

#ifdef _NOMAD_DEBUG
	Con_Printf("DEBUG: created tempfile %s", ospath);
#endif

	fs_numTmpFiles++;
	fs_loadStack++;

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
	fs_loadStack--;
}

const char* FS_GetOSPath(file_t f)
{
	fileHandle_t *file;
	static const char *ospath;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_GetOSPath: out of range");
	}

	file = &handles[f];
	ospath = FS_BuildOSPath(fs_homedir, NULL, file->name);

	return ospath;
}
int FS_FileToFileno(file_t f)
{
	fileHandle_t *file;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileToFileno: out of range");
	}

	file = &handles[f];
	return fileno(file->data.fp);
}

file_t FS_FOpenRW(const char *filepath)
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
		N_Error("FS_FOpenWrite: attempted to create write stream for bff chunk %s", filepath);
	}
	ospath = FS_BuildOSPath(fs_homedir, NULL, filepath);
	N_strncpyz(f->name, filepath, MAX_GDR_PATH);
	f->data.fp = Sys_FOpen(ospath, "wb+");
	if (!f->data.fp) {
		N_Error("FS_FOpenWrite: failed to open file %s", ospath);
		return FS_INVALID_HANDLE;
	}
	
	f->used = qtrue;
	fs_loadStack++;
	return fd;
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
		N_Error("FS_FOpenWrite: attempted to create write stream for bff chunk %s", filepath);
	}
	ospath = FS_BuildOSPath(fs_homedir, NULL, filepath);
	N_strncpyz(f->name, filepath, MAX_GDR_PATH);
	f->data.fp = Sys_FOpen(ospath, "wb");
	if (!f->data.fp) {
		N_Error("FS_FOpenWrite: failed to open file %s", ospath);
		return FS_INVALID_HANDLE;
	}
	
	f->used = qtrue;
	fs_loadStack++;
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
		ospath = FS_BuildOSPath(fs_homedir, NULL, filepath);
		N_strncpyz(f->name, filepath, MAX_GDR_PATH);
		f->data.fp = Sys_FOpen(ospath, "rb");
		if (!f->data.fp) {
			Con_Printf(ERROR, "FS_FOpenRead: failed to open file in path '%s'", ospath);
			return FS_INVALID_HANDLE;
		}
	}
	f->used = qtrue;
	fs_loadStack++;
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

static uint32_t FS_MakeBFFChecksum(const bffFile_t *bff)
{
	uint32_t crc, i;

	for (i = 0; i < bff->numfiles; ++i) {
		(void)crc32_buffer((const byte *)bff->fileList[i].name, MAX_BFF_CHUNKNAME);
		(void)crc32_buffer((const byte *)bff->fileList[i].buffer, bff->fileList[i].size);
	}
	crc = crc32_buffer((const byte *)bff->name, MAX_GDR_PATH);
	return crc;
}

static qboolean FS_BFFIsRegistered(const bffFile_t *bff)
{
	uint32_t crc, i;

	crc = FS_MakeBFFChecksum(bff);
	for (i = 0; i < fs_totalArchives; ++i) {
		if (fs_archives[i]->checksum == crc) {
			// its a registered, officially paid for bff
			return qtrue;
		}
	}

	// its either been cracked, hacked, or isn't a registered bff
	return qfalse;
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
		if (N_stricmpn(e, extlist[n], strlen(extlist[n]))) {
			if (extension) {
				*extension = extlist[n];
			}
			return false;
		}
	}
	return true;
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

/*
FS_TouchFileInBFF: call this when you don't want a file to be purged when clearing out the bffs
*/
void FS_TouchFileInBFF(const char *name)
{
	fileInBFF_t *file;
	bffFile_t *bff;
	uint32_t i, a;

	for (i = 0; i < fs_totalArchives; ++i) {
		bff = fs_archives[i];

		for (a = 0; a < bff->numfiles; a++) {
			file = &bff->fileList[a];

			// found it
			if (FS_FilenameCompare(file->name, name)) {
				file->touched = qtrue;
			}
		}
	}
}

/*
FS_FilenameCompare: case and path separator insensitive filepath comparison
*/
qboolean FS_FilenameCompare( const char *s1, const char *s2 )
{
	int		c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 <= 'Z' && c1 >= 'A' )
			c1 += ('a' - 'A');
		else if ( c1 == '\\' || c1 == ':' )
			c1 = '/';

		if ( c2 <= 'Z' && c2 >= 'A' )
			c2 += ('a' - 'A');
		else if ( c2 == '\\' || c2 == ':' )
			c2 = '/';

		if ( c1 != c2 ) {
			return qfalse; // strings not equal
		}
	} while ( c1 );
	
	return qtrue; // strings are equal
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
		for (uint32_t c = 0; c < fs_archives[i]->numfiles; c++) {
			if (!N_stricmpn(filepath, fs_archives[i]->fileList[c].name, MAX_BFF_CHUNKNAME) == 1) {
				return true;
			}
		}
	}
	return false;
}
static fileInBFF_t* FS_GetBFFChunk(const char *filepath)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		for (uint32_t c = 0; c < fs_archives[i]->numfiles; c++) {
			if (!N_stricmpn(filepath, fs_archives[i]->fileList[c].name, MAX_BFF_CHUNKNAME) == 1) {
				return &fs_archives[i]->fileList[c];
			}
		}
	}
	return NULL;
}
static bffFile_t* FS_GetChunkBFF(fileInBFF_t* chunk)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		for (uint32_t c = 0; c < fs_archives[i]->numfiles; c++) {
			if (chunk == &fs_archives[i]->fileList[c]) {
				return fs_archives[i];
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
	fs_lastBFFIndex = bff->index;
	N_strncpyz(f->name, chunk->name, MAX_BFF_CHUNKNAME);

	bff->handlesUsed++;

	return qtrue;
}

static uint32_t FS_ReturnPath( const char *zname, char *zpath, uint32_t *depth )
{
	uint32_t len, at, newdep;

	newdep = 0;
	zpath[0] = '\0';
	len = 0;
	at = 0;

	while (zname[at] != 0) {
		if (zname[at]=='/' || zname[at]=='\\') {
			len = at;
			newdep++;
		}
		at++;
	}
	strcpy(zpath, zname);
	zpath[len] = '\0';
	*depth = newdep;

	return len;
}

/*
FS_PathCmp: case insensitive file path compare
*/
static int FS_PathCmp(const char *s1, const char *s2)
{
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 >= 'a' && c1 <= 'z')
			c1 -= ('a' - 'A');
		if (c2 >= 'a' && c2 <= 'z')
			c2 -= ('a' - 'A');

		if ( c1 == '\\' || c1 == ':' )
			c1 = '/';
		if ( c2 == '\\' || c2 == ':' )
			c2 = '/';
		
		if (c1 < c2)
			return -1; // strings not equal
		if (c1 > c2)
			return 1;
	} while (c1);
	
	return 0; // strings are equal
}

static void FS_BFFInfo(const bffFile_t *bff, bool showOnlyRegistered = false)
{
	uint32_t i;
	fileInBFF_t *file;

	if (showOnlyRegistered && !FS_BFFIsRegistered(bff))
		return;
	
	Con_Printf("------------------------------");
	
	Con_Printf("Name: %s", bff->name);
	Con_Printf("Number of Files: %i", bff->numfiles);
	Con_Printf("Checksum: %i", bff->checksum);
	Con_Printf("Registered: %s", N_booltostr(FS_BFFIsRegistered(bff)));
	Con_Printf("[Individual Chunk Info]");
	for (i = 0; i < bff->numfiles; ++i) {
		file = &bff->fileList[i];

		Con_Printf("Chunk #%i", i);
		Con_Printf("Name: %s", file->name);
		Con_Printf("Size: %i\n", file->size);
	}
}

static void FS_ListArchives_f(void)
{
	uint32_t i;

	Con_Printf("Total BFF Archives: %i", fs_totalArchives);
	for (i = 0; i < fs_totalArchives; ++i) {
		FS_BFFInfo(fs_archives[i]);
	}
}

static void FS_ListRegisteredBFFs_f(void)
{
	uint32_t i;
	Con_Printf("Total BFF Archives: %i", fs_totalArchives);
	for (i = 0; i < fs_totalArchives; ++i) {
		FS_BFFInfo(fs_archives[i], true);
	}
}

static bffFile_t *fs_archiveCache[MAX_BFF_FILES];

static void FS_AddToCache(bffFile_t *bff)
{
	uint32_t i;

	for (i = 0; i < MAX_BFF_FILES; i++) {
		if (!fs_archiveCache[i]) {
			fs_archiveCache[i] = bff;
			break;
		}
	}
}

static qboolean FS_IsCached(const bffFile_t *bff)
{
	uint32_t i;

	for (i = 0; i < MAX_BFF_FILES; i++) {
		if (fs_archiveCache[i] == bff)
			return qtrue;
	}
	return qfalse;
}

static void FS_RemoveFromCache(bffFile_t *bff)
{
	uint32_t i;

	for (i = 0; i < MAX_BFF_FILES; i++) {
		if (fs_archiveCache[i] == bff) {
			fs_archiveCache[i] = NULL;
			break;
		}
	}
}

/*
FS_FreeUnused: purges all the untouched files in bffs
*/
static void FS_FreeUnused(void)
{
	uint32_t i, c, untouched;

	untouched = 0;
	for (i = 0; i < fs_totalArchives; i++) {
		for (c = 0; c < fs_archives[i]->numfiles; c++) {
			// hasn't been touched, free it back to the zone
			if (!fs_archives[i]->fileList[c].touched) {
				untouched++;
			}
		}
		// if all the files are untouched, free the list
		if (untouched == fs_archives[i]->numfiles) {
			Z_ChangeTag(fs_archives[i]->fileList, TAG_UBFF);
		}
		// if its not in the cached list, purge it
		if (!FS_IsCached(fs_archives[i])) {
			Z_ChangeTag(fs_archives[i], TAG_UBFF);
		}
	}
}

void FS_GetLine(char *buffer, uint64_t bufferLen, file_t f)
{
	fileHandle_t *file;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_GetLine: out of range");
	}

	file = &handles[f];

	if (FS_IsChunk(file)) {
		char *ptr = &file->data.chunk->buffer[file->data.chunk->bytesRead];
		char *it = buffer;
		while (*ptr != '\n' && --bufferLen) {
			*it++ = *ptr++;
		}
		return;
	}
	
	fgets(buffer, bufferLen, file->data.fp);
}

file_t FS_FOpenWithMode(const char *path, const char *mode, bool vm)
{
	file_t f;
	fileHandle_t *h;
	const char *ospath;
	FILE *fp;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path) {
		N_Error("FS_FOpenWithMode: NULL path");
	}
	if (!mode) {
		N_Error("FS_FOpenWithMode: NULL mode");
	}
#ifdef __unix__
	if (!*path) {
		Con_Printf(ERROR, "FS_FOpenWithMode: empty path (this is a *nix system)");
		return FS_INVALID_HANDLE;
	}
#endif

	f = FS_HandleForFile();
	h = &handles[f];
	FS_InitHandle(h);

	if ((*mode == 'r' && vm) || (*mode == 'w' && FS_IsChunk(path))) {
		return FS_INVALID_HANDLE;
	}
	else if (FS_IsChunk(path) && *mode != 'w') {
		bffFile_t *bff = FS_GetChunkBFF(FS_GetBFFChunk(path));
		FS_OpenFileInBFF(path, &f, bff, FS_GetBFFChunk(path), h);
		return f;
	}

	ospath = FS_BuildOSPath(fs_homedir, NULL, path);
	
	fp = Sys_FOpen(ospath, mode);
	if (!fp) {
		return FS_INVALID_HANDLE;
	}

	return f;
}

/*
FS_LoadLibrary: tried to load libraries within known searchpaths
*/
void *FS_LoadLibrary(const char *name)
{
	void *libHandle = NULL;
	char *fn;

#ifdef _NOMAD_DEBUG
	fn = FS_BuildOSPath(Sys_pwd(), name, NULL);
	libHandle = Sys_LoadDLL(fn);
#endif

	if (!libHandle)
		return NULL;
	
	return libHandle;
}

#define BFF_CACHE_MAGIC 0xffa28d

#pragma pack(push, 1)

typedef struct
{
	uint32_t magic;			// should be BFF_CACHE_MAGIC
	uint32_t version;		// should be _NOMAD_VERSION
	uint32_t numFiles;		// number of cached bff files
} bffCacheHeader_t;

typedef struct
{
	char *name;
	uint32_t nameLen;
	uint32_t hash;
	uint32_t numChunks;
	time_t mtime;
	time_t ctime;
	fileOffset_t size;
} bffCacheEntry_t;

#pragma pack(pop)

#define BFF_CACHE_FILE "bffcache.dat"
#define MAX_BFF_CACHE 256
#define FS_HashBFF(x) Com_GenerateHashValue((x),MAX_BFF_CACHE)

static bff_t *bffCacheTable[MAX_BFF_CACHE];

static uint32_t fs_numMods;
static uint32_t fs_cachedBFFs;
static uint32_t fs_activeBFFs;

static bff_t *FS_FindBFFInCache(const char *name)
{
}

static void FS_LoadBFFFromCache(FILE *fp)
{
	bff_t *bff;


}

static void FS_WriteCache(void)
{
	bffCacheHeader_t header;
	bffCacheEntry_t entry;
	const char *ospath;
	FILE *fp;

	header.magic = BFF_CACHE_MAGIC;
	header.version = _NOMAD_VERSION;
	header.numFiles = fs_totalArchives;
	
	{
		ospath = FS_BuildOSPath(fs_homedir, NULL, BFF_CACHE_FILE);
		fp = Sys_FOpen(ospath, "wb");
		if (!fp) {
			N_Error("FS_WriteCache: failed to open cache file %s in write mode", BFF_CACHE_FILE);
		}
	}

	if (fwrite(&header, sizeof(header), 1, fp) != sizeof(header)) {
		N_Error("FS_WriteCache: failed to write header");
	}

	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		entry.hash = FS_HashBFF(fs_archives[i]->name);
		entry.nameLen = strlen(fs_archives[i]->name);
		entry.numChunks = fs_archives[i]->numfiles;
		entry.mtime = fs_archives[i]->stats.mtime;
		entry.ctime = fs_archives[i]->stats.ctime;

		if (fwrite(&entry, sizeof(entry), 1, fp) != sizeof(entry)) {
			N_Error("FS_WriteCache: failed to write entry %i into the cache", i);
		}
	}
	fclose(fp);
}


static char **fs_modList;
static uint64_t fs_modCount;

static void FS_GetModList(void)
{
	
}