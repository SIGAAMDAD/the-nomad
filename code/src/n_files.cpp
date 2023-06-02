#include "n_shared.h"

#define MAX_FILE_HANDLES 1024
#define MAX_BFF_FILES 8
#define MAX_FILE_HASH 1024

typedef struct
{
	char name[MAX_GDR_PATH];
	uint32_t size;
	uint32_t type;
	
	char *curPtr; // position in the cache
	void *cache;
	
	uint32_t filePos;
	uint32_t bytesLeft;
} fileInBFF_t;

typedef struct
{
	char bffPathname[MAX_BFF_PATH];
	char bffGamename[MAX_GDR_PATH];
	
	uint16_t checksum;
	uint16_t id[40];
	uint32_t numfiles;
	fileInBFF_t* hashtable;
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
	fileData data;
	char name[MAX_GDR_PATH];
	int32_t bffIndex;
	qboolean bffFile;
	uint32_t size;
	uint32_t filePos;
	bffFile_t* bff;
} fileHandle_t;

static fileHandle_t handles[MAX_FILE_HANDLES];
static bffFile_t* bffs[MAX_BFF_FILES];
static uint32_t fs_totalArchives;
static uint32_t fs_currentArchive;

cvar_t fs_gamedir;
cvar_t fs_numArchives;
static bool fs_initialized;

// c++ overloaders here
static fileInBFF_t* FS_OpenFileInBFF(const char* name, int32_t bffIndex, fileHandle_t* file);
static bool FS_FilenameCompare(const char *s1, const char *s2);
static bool FS_IsBFFRegistered(bffFile_t* bff);
static bool FS_FileIsInBFF(fileHandle_t *f);
static bool FS_FileIsInBFF(const char *path);
static bool FS_FileIsBFF(fileHandle_t *f);
static bool FS_FileIsBFF(const char *path);
static bffFile_t* FS_LoadBFF(const char *path);
static void FS_ReplaceSeparators(char *str);
static bool FS_CheckFileExt(const char *filepath, bool allowBFFs, const char **ext)
{
	static const char *extlist[] = { "dll", "exe", "so", "dylib", "qvm", "bff" };
	char *e;
	int i, n;

	char str[6];
	for (uint32_t i = 0; i < arraylen(extlist); ++i) {
		if (strstr(filepath, extlist[i]) != NULL) {
			str[0] = '.';
			N_strncpy(&str[1], extlist[i], sizeof(str));
			*ext = str;
			return false;
		}
	}
	return true;
}
static void FS_CheckFilenameIsNotAllowed(const char *filename, const char *function, bool allowBFFs)
{
	const char *extension;
	if (!FS_CheckFileExt(filename, allowBFFs, &extension)) {
		N_Error("%s: not allowed to manipulate '%s' due to %s extension",
			function, filename, extension);
	}
}

static int FS_PathCmp(const char *s1, const char *s2)
{
	int c1, c2;
	const char *str1 = s1;
	const char *str2 = s2;
	
	do {
		c1 = *str1++;
		c2 = *str2++;

		if (c1 >= 'a' && c1 <= 'z') {
			c1 -= ('a' - 'A');
		}
		if (c2 >= 'a' && c2 <= 'z') {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}
		
		if (c1 < c2) {
			return -1;		// strings not equal
		}
		if (c1 > c2) {
			return 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

static bool FS_PathEq(const char *s1, const char *s2)
{
	int c1, c2;
	const char *str1 = s1;
	const char *str2 = s2;
	
	do {
		c1 = *str1++;
		c2 = *str2++;

		if (c1 >= 'a' && c1 <= 'z') {
			c1 -= ('a' - 'A');
		}
		if (c2 >= 'a' && c2 <= 'z') {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}
		
		if (c1 < c2 || c1 > c2) {
			return false;
		}
	} while (c1);
	
	return true;
}

static void FS_ReplaceSeparators(char *str);
static bool FS_CheckDirTraversal(const char *checkdir)
{
	if (strstr(checkdir, "../") || strstr(checkdir, "..\\"))
		return true;
	if (strstr(checkdir, "::"))
		return true;
	
	return false;
}
static bool FS_CreatePath(const char *ospath)
{
	char path[MAX_OSPATH*2+1];
	char *ofs;
	
	// make absolutely sure that it can't back up the path
	// FIXME: is c: allowed???
	if (FS_CheckDirTraversal(ospath)) {
		Con_Printf("WARNING: refusing to create relative path \"%s\"", ospath);
		return false;
	}
	
	N_strncpy(path, ospath, sizeof(path));
	// make sure we have os correct slashes
	FS_ReplaceSeparators(path);
	for (ofs = path + 1; *ofs; ofs++) {
		if (*ofs == PATH_SEP) {
			// create the directory
			*ofs = '\0';
			Sys_mkdir(path);
			*ofs = PATH_SEP;
		}
	}
	return false;
}

static void FS_CopyFile(const char *fromospath, const char *toospath)
{
	FILE *f;
	uint32_t len, read;
	byte *buf;
	
	Con_Printf("copying %s to %s", fromospath, toospath);
	
	if (strstr(fromospath, "journal.dat") || strstr(fromospath, "journaldata.dat")) {
		Con_Printf("Ignoring journal files");
		return;
	}
	
	f = Sys_FOpen(fromospath, "rb");
	if (!f) {
		return;
	}
	
	len = FS_FileLength(f);
	
	// we are using direct malloc instead of Z_Malloc here, so it probably won't work on mac... It's only for developers anyway...
	buf = (byte *)Mem_Alloc(len);
	if (!buf) {
		fclose(f);
		N_Error("FS_CopyFile: malloc() failed");
	}
	if ((read = fread(buf, 1, len, f)) != len) {
		Mem_Free(buf);
		fclose(f);
		N_Error("FS_CopyFile: short read of %i bytes, should have read %i bytes", read, len);
	}
	fclose(f);
	
	f = Sys_FOpen(toospath, "wb");
	if (!f) {
		if (!FS_CreatePath(toospath)) {
			Mem_Free(buf);
			return;
		}
		f = Sys_FOpen(toospath, "wb");
		if (!f) {
			Mem_Free(buf);
			return;
		}
	}
	
	if ((read = fwrite(buf, 1, len, f)) != len) {
		Mem_Free(buf);
		fclose(f);
		N_Error("FS_CopyFile: short write of %i bytes, should have written %i bytes", read, len);
	}
	fclose(f);
	Mem_Free(buf);
}

static FILE* FS_FileForHandle(file_t fd)
{
	if (fd <= FS_INVALID_HANDLE || fd >= MAX_FILE_HANDLES) {
		N_Error("FS_FileForHandle: out of range");
	}
	if (handles[fd].bffFile) {
		N_Error("FS_FileForHandle: can't get FILE on bff file");
	}
	if (!handles[fd].data.fp) {
		N_Error("FS_FileForHandle: NULL");
	}
	return handles[fd].data.fp;
}

static file_t FS_HandleForFile(void)
{
	file_t fd;
	
	for (fd = 1; fd < MAX_FILE_HANDLES; fd++) {
		if (handles[fd].data.stream == NULL)
			return fd;
	}
	Con_Printf("WARNING: FS_HandleForFile: none free");
	return FS_INVALID_HANDLE;
}

static char* FS_BuildOSPath(const char *base, const char *game, const char *fname)
{
	char temp[MAX_GDR_PATH*2+1];
	static char ospath[2][sizeof(temp)+MAX_OSPATH];
	static int toggle;
	
	toggle ^= 1;
	
	if (!game || !game[0])
		game = fs_gamedir.value;
	
	if (fname)
		snprintf(temp, sizeof(temp), "%c%s%c%s", PATH_SEP, game, PATH_SEP, fname);
	else
		snprintf(temp, sizeof(temp), "%c%s", PATH_SEP, game);
	
	FS_ReplaceSeparators(temp);
	snprintf(ospath[toggle], sizeof(ospath[0]), "%s%s", base, temp);
	
	return ospath[toggle];
}

static bffFile_t* FS_OpenBFF(const char *path)
{
	char *ospath;
	bffFile_t* file;
	fileHandle_t* handle;
	bffheader_t header;
	file_t fd;
	uint64_t hash, offset;
	
	fd = FS_HashFileName(path, MAX_FILE_HASH);
	handle = &handles[fd];
	ospath = FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, path);
	
	FILE* fp = Sys_FOpen(ospath, "rb");
	if (!fp) {
		N_Error();
	}
	
	fread(&header, sizeof(bffheader_t), 1, fp);
	if (header.ident != BFF_IDENT) {
		N_Error("FS_OpenBFF: header identifier isn't correct");
	}
	if (header.magic != BFF_MAGIC) {
		N_Error("FS_OpenBFF: header magic isn't correct");
	}
	if (!header.numChunks) {
		N_Error("FS_OpenBFF: bad chunk count");
	}
	
	file = (bffFile_t *)Hunk_Alloc(sizeof(bffFile_t), "BFFfile", h_high);
	file->numfiles = header.numChunks;
	file->checksum = 0;
	file->hashtable = (fileInBFF_t *)Z_Malloc(sizeof(fileInBFF_t) * file->numfiles, TAG_STATIC, &file->hashtable, "BFFfiles");
	
	fread(file->bffPathname, sizeof(char), MAX_BFF_PATH, fp);
	fread(file->bffGamename, sizeof(char), MAX_GDR_PATH, fp);
	
	for (uint32_t i = 0; i < file->numfiles; ++i) {
		offset = ftell(fp);
		char chunkname[MAX_BFF_CHUNKNAME];
		
		fread(chunkname, sizeof(char), MAX_BFF_CHUNKNAME, fp);
		hash = FS_HashFileName(chunkname, file->numfiles);
		
		chunk = &file->hashtable[hash];
		
		fread(&chunk->size, sizeof(uint32_t), 1, fp);
		fread(&chunk->type, sizeof(uint32_t), 1, fp);
		
		chunk->cache = Z_Malloc(chunk->size, TAG_STATIC, &chunk->cache, "BFFcache");
		fread(chunk->cache, chunk->size, 1, fp);
	}
	
	fclose(fp);
}

static void FS_InitBFFs(void)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		char name[MAX_GDR_PATH];
		sprintf(name, "bff%i.bff", i);
		const char *path = FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, name);
		
		bffs[i] = FS_OpenBFF(path);
	}
}

void FS_Init(void)
{
	int numArchives = N_atoi(fs_numArchives.value);
	if (numArchives >= MAX_BFF_FILES) {
		N_Error("FS_Init: too many bff files in config, limit is %i files", MAX_BFF_FILES);
	}
	fs_totalArchives = numArchives;
	
	fs_currentArchive = 0;
	memset(bffs, 0, sizeof(bffs));
	
	FS_InitBFFs();
	
	for (uint32_t i = 0; i < MAX_FILE_HANDLES; i++) {
		handles[i].id = FS_INVALID_HANDLE;
	}
	
	fs_initialized = true;
}

void FS_Remove(const char *ospath)
{
	FS_CheckFilenameIsNotAllowed(ospath, __func__, true);
	remove(ospath);
}

void FS_HomeRemove(const char *ospath)
{
	FS_CheckFilenameIsNotAllowed(ospath, __func__, false);
	remove(FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, ospath));
}

void FS_Rename(const char *from, const char *to)
{
	const char *from_ospath, *to_ospath;
	FILE *f;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	
	from_ospath = FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, from);
	to_ospath = FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, to);
	
	Con_Printf(DEBUG, "FS_Rename: %s --> %s", from_ospath, to_ospath);
	
	f = Sys_FOpen(from_ospath, "rb");
	if (f) {
		fclose(f);
		FS_Remove(to_ospath);
	}
	
	if (rename(from_ospath, to_ospath)) {
		// failed, try copying it and deleting the original
		FS_CopyFile(from_ospath, to_ospath);
		FS_Remove(from_ospath);
	}
}

void FS_VM_FOpenRead(const char* filepath, file_t* f)
{
	*f = FS_FOpenRead(filepath);
}

void FS_VM_FOpenWrite(const char* filepath, file_t* f)
{
	*f = FS_FOpenWrite(filepath);
}

file_t FS_FOpenRead(const char* filepath)
{
	char *ospath;
	uint64_t hash;
	file_t f;
	fileHandle_t *handle;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!filepath) {
		N_Error("FS_FOpenRead: null path");
	}
	if (!*filepath) {
		N_Error("FS_FOpenRead: empty path");
	}
	
	hash = FS_HashFileName(filepath, MAX_GDR_PATH);
	handle = &handles[hash];
	if (handle->id != FS_INVALID_HANDLE) {
		N_Error("FS_FOpenRead: %s opened twice", filepath);
	}
	
	f = hash;
	handle->id = f;
	
	// bff chunk
	if (FS_FileIsInBFF(filepath)) {
		ospath = filepath;
	}
	// bff archive file
	else if (FS_FileIsBFF(filepath)) {
		
	}
	// regular file
	else {
		ospath = FS_BuildOSPath(fs_homepath.value, fs_gamedir.value, filepath);
		
		handle->data.fp = Sys_FOpen(ospath, "rb");
		if (!handle->data.fp) {
			Con_Printf("WARNING: failed to create an rb stream for %s", filepath);
			return FS_INVALID_HANDLE;
		}
		
		handle->bffIndex = -1;
		handle->bff = NULL;
		handle->bffFile = qfalse;
		handle->filePos = 0;
		
		fseek(handle->data.fp, 0L, SEEK_END);
		handle->size = ftell(handle->data.fp);
		fseek(handle->data.fp, 0L, SEEK_SET);
	}
	
	return f;
}

void FS_VM_FOpenWrite(const char* filepath, file_t* f)
{

}

file_t FS_FOpenWrite(const char* filepath)
{
	char *ospath;
	uint64_t hash;
	fileHandle_t* handle;
	file_t f;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	
	hash = FS_HashFileName(filepath, MAX_GDR_PATH);
	handle = &handles[hash];
	if (handle->id != FS_INVALID_HANDLE) {
		N_Error("File %s opened twice", filepath);
	}
	
	f = hash;
	handle->id = f;
	
	if (FS_FileIsInBFF(filepath)) {
		Con_Printf("WARNING: refusing to open bff chunk with FS_FOpenWrite");
		return FS_INVALID_HANDLE;
	}
	
	ospath = FS_BuildOSPath(fs_gamedir.value, filepath);
	handle->data.fp = Sys_FOpen(filepath, "wb");
	if (!handle->data.fp) {
		Con_Printf("WARNING: failed to create write-only FILE* for file %s", filepath);
		return FS_INVALID_HANDLE;
	}
	
	return f;
}

int32_t FS_BFFIndex(file_t f)
{
	
}

// compares two bffs, if they aren't exactly equal, then return false
bool FS_CompareBFFs(const bffFile_t* bff1, const bffFile_t* bff2)
{
	// get the easy stuff out of the way
	if (bff1->numfiles != bff2->numfiles) {
		return false;
	}
	if (bff1->checksum != bff2->checksum) {
		return false;
	}
	if (!N_memcmp(bff1->id, bff2->id, sizeof(uint16_t) * 40)) {
		return false;
	}
	
	fileInBFF_t *chunk1, *chunk2;
	
	// compare each chunk
	for (uint32_t i = 0; i < bff1->numfiles; i++) {
		chunk1 = &bff1->hashtable[i];
		chunk2 = &bff2->hashtable[i];
		
		if (!N_strncmp(chunk1->name, chunk2->name, MAX_GDR_PATH)) {
			return false;
		}
		if (chunk1->size != chunk2->size) {
			return false;
		}
		if (chunk1->type != chunk2->type) {
			return false;
		}
		if (!N_memcmp(chunk1->cache, chunk2->cache, chunk1->size)) {
			return false;
		}
	}
	return true;
}

bool FS_InvalidGameDir(const char *gamedir)
{
	if (!strcmp(gamedir, ".") || !strcmp(gamedir, "..")
	|| strchr(gamedir, '/') || strchr(gamedir, '\\')) {
		return true;
	}
	return false;
}

static bool FS_OpenFileInBFF(const char* name, fileHandle_t* file)
{
	bffFile_t* bff;
	fileInBFF_t* chunk;
	uint64_t hash;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!name) {
		N_Error("FS_FOpenFileRead: null name");
	}
	
	while (name[0] == '/' || name[0] == '\\')
		name++;
	
	if (FS_CheckDirTraversal(name))
		return false;
	
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		
		bool found = false;
		hash = FS_HashFileName(name, bffs[i]->numfiles);
		chunk = &bffs[i]->hashtable[hash];
		for (uint32_t c = 0; c < bffs[i]->numfiles; c++) {
			if (N_strncmp(name, chunk->name, MAX_BFF_CHUNKNAME)) {
				found = true;
				break;
			}
			chunk++;
		}
		if (found) {
			file->stream.chunk = chunk;
			file->bff = bffs[i];
			file->bffIndex = i;
			file->filePos = 0;
			file->size = chunk->size;
			return true;
		}
		
	}
	return false;
}

void FS_ClearBFFs(void)
{
	bffFile_t* bff;
	
	for (uint32_t i = 0, bff = bffs[0]; i < fs_totalArchives; i++, bff++) {
		if (!bff->touched) {
			Z_ChangeTag(bff->hashtable, TAG_PURGELEVEL);
			for (uint32_t f = 0; f < bff->numfiles; f++) {
				Z_ChangeTag(bff->hashtable[f].cache, TAG_PURGELEVEL);
			}
			B_CloseArchive(bff);
		}
	}
}

void FS_FClose(file_t *f)
{
	if (!f) {
		N_Error("FS_FClose: null handle");
	}
	if (*f <= FS_INVALID_HANDLE || *f >= MAX_FILE_HANDLES) {
		return;
	}
	
	fileHandle_t* handle = &handles[*f];
	if (FS_FileIsInBFF(handle)) {
		Z_ChangeTag(handle->data.chunk->cache, TAG_CACHE);
		handle->data.chunk->filePos = 0;
		handle->data.chunk->bytesLeft = handle->data.chunk->size;
		
		handle->bffFile = qfalse;
		handle->bff = NULL;
		handle->bffIndex = -1;
	}
	// never free the bff (dont want to load it up and fuck up the zone), just simply kill the ownership of the handle
	else if (FS_FileIsBFF(handle)) {
		handle->bffFile = qfalse;
		handle->bffIndex = -1;
		handle->bff = NULL;
		
		// mark as purgable
		Z_ChangeTag(hande->data.bff->hashtable, TAG_CACHE);
	}
	// flush the buffer and fclose it
	else {
		FS_ForceFlush(*f);
		fclose(handle->data.fp);
	}
	
	// for all streams
	handle->size = 0;
	handle->id = 0;
	handle->data.stream = NULL;
	*f = FS_INVALID_HANDLE;
}

uint32_t FS_Write(const void *data, uint32_t size, file_t f)
{
	uint32_t remaining, block;
	size_t written;
	int tries;
	const byte *buf;
	fileHandle_t* file;
	
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}
	file = &handles[f];
	buf = (const char *)data;
	fs_writeCount += size;
	
	if (!FS_FileIsInBFF(file)) {
		remaining = size;
		tries = 0;
		while (remaining) {
			block = remaining;
			write = fwrite(buf);
			if (write == 0) {
				if (!tries) {
					tries = 1;
				}
				else {
					return size - remaining;
				}
			}
			if (write == -1) {
				N_Error("FS_Write: -1 bytes written");
			}
			
			remaining -= written;
			buf += written;
			handle->filePos += written;
		}
	}
	else {
		Con_Printf("WARNING: attempted to write to a bff chunk (all of which are read-only)");
		return 0;
	}
}

uint32_t FS_ReadFromBFF(void *data, uint32_t size, fileHandle_t* f)
{
	fileInBFF_t* chunk = f->data.chunk;
	char *endPtr = &((char *)chunk->cache)[file->size];
	
	if (chunk->curPtr == endPtr) {
		return 0; // eof
	}
	else if (chunk->bytesRead + size >= chunk->size) {
		uint32_t bytesLeft = chunk->size - chunk->bytesRead + size;
		memcpy(data, chunk->curPtr, bytesLeft);
		chunk->curPtr = endPtr;
		Con_Printf("WARNING: overread in chunk %s of %i bytes", chunk->name, chunk->bytesRead + size);
		return bytesLeft;
	}
	else {
		memcpy(data, chunk->curPtr, size);
		chunk->curPtr += size;
		return size;
	}
}

uint32_t FS_Read(void *data, uint32_t size, file_t f)
{
	uint32_t block, remaining;
	size_t read;
	int tries;
	char *buf;
	
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}
	
	fileHandle_t* file = &handles[f];
	buf = (char *)data;
	fs_readCount += size;
	
	// simple fread
	if (!FS_FileIsInBFF(file)) {
		remaining = size;
		tries = 0;
		while (remaining) {
			block = remaining;
			read = fread(buf, 1, block. file->data.fp);
			if (!read) {
				if (!tries) {
					tries = 1;
				}
				else {
					return size - remaining;
				}
			}
			
			if (read == -1) {
				N_Error("FS_Read: -1 bytes read");
			}
			
			remaining -= read;
			buf += read;
		}
		return read;
	}
	// more complex memory management when reading from a chunk
	else {
		return FS_ReadFromBFF(data, size, file);
	}
}

uint32_t FS_FileLength(file_t f)
{
	FILE *fp;
	bff_chunk_t *chunk;
	uint32_t pos, end;
	
	if (!FS_FileIsInBFF(&handles[f])) {
		fp = handles[f].data.fp;
		pos = ftell(fp);
		fseek(fp, 0L, SEEK_END);
		end = ftell(fp);
		fseek(fp, pos, SEEK_SET);
		return end;
	}
	else if (handles[f].data.chunk) {
		return handles[f].data.chunk->chunkSize;
	}
}

uint32_t FS_FileTell(file_t f)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}
	fileHandle_t* handle = &handles[f];
	if (!FS_FileIsInBFF(handle)) {
		return ftell(handle->data.fp);
	}
	else if (FS_FileIsInBFF(handle)) {
		return handle->data.chunk->filePos;
	}
	return 0;
}

/**********************
*  Utility Functions  *
***********************/

/*
FS_ReplaceSeparators: replaces all foreign filepath separators if any are found
*/
static void FS_ReplaceSeparators(char *str)
{
	char *s;
	
	for (s = str; *s; s++) {
		if (*s == PATH_SEP_FOREIGN) {
			*s = PATH_SEP;
		}
	}
}

/*
FS_IsBFFRegistered: checks if the bff file in question is an officially registered one via crc
*/
static bool FS_IsBFFRegistered(bffFile_t* bff)
{
	byte *p;
	const uint32_t crc_size = sizeof(uint32_t) * 2 + MAX_BFF_CHUNKNAME;
	uint16_t crc = 0;
	for (uint32_t i = 0; i < bff->numfiles; i++) {
		p = (byte *)&bff->hashtable[i];
		for (uint32_t c = 0; c < crc_size; c++) {
			CRC_ProcessByte(&crc, *p);
			p++;
		}
	}
	return crc == bff->checksum;
}


/*
FS_FilenameCompare: ignore case and separator char distinctions
*/
static bool FS_FilenameCompare( const char *s1, const char *s2 )
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
			return false;		// strings not equal
		}
	} while ( c1 );
	
	return true;		// strings are equal
}

/*
FS_IsExt: returns true if ext matches file extension filename
*/
static bool FS_IsExt(const char *filename, const char *ext, size_t namelen)
{
	size_t extlen;
	
	extlen = strlen(ext);
	
	if (extlen > namelen)
		return false;
	
	filename += namelen - extlen;
	
	return N_strcasecmp(filename, ext);
}

/*
FS_StripExt:
*/
bool FS_StripExt(char *filename, const char *ext)
{
	int extlen, namelen;
	
	extlen = strlen(ext);
	namelen = strlen(filename);
	
	if (extlen > namelen)
		return false;
	
	filename += namelen - extlen;
	
	if (N_strcasecmp(filename, ext)) {
		filename[0] = '\0';
		return true;
	}
	return false;
}

/*
FS_HasExt
*/
static const char* FS_HasExt(const char *filename, const char **extlist, int extCount)
{
	const char *e;
	int i;
	
	e = strrchr(filename, '.');
	
	if (!e)
		return NULL;
	
	for (i = 0, e++; i < extCount; i++) {
		if (N_strcasecmp(e, extList[i]))
			return e;
	}
	return NULL;
}

/*
FS_FileIsBFF: if the handle is a bff archive
*/
static bool FS_FileIsBFF(const char *path)
{
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
	}
}
static bool FS_FileIsBFF(fileHandle_t *f)
{
	if (!f->data.chunk && f->bff && f->bffFile && f->bffIndex)
		return true;
	else
		return false;
}
/*
FS_FileIsInBFF:
if the handle is a file within a bff
*/
static bool FS_FileIsInBFF(const char *path)
{
	uint64_t hash = FS_HashFileName(path, MAX_FILE_HASH);
	
	for (uint32_t i = 0; i < fs_totalArchives; i++) {
		if (N_strcmp(bffs[i]->hashtable[hash], path)) {
			return true;
		}
	}
	return false;
}
static bool FS_FileIsInBFF(fileHandle_t *f)
{
	if (f->data.chunk && f->bff && f->bffFile && f->bffIndex)
		return true;
	else
		return false;
}


uint32_t FS_LoadFile(const char *filepath, void **buffer)
{
	file_t f = FS_FOpenRead(filepath);
	if (f == FS_INVALID_HANDLE) {
		N_Error("FS_LoadFile: failed to open file %s", filepath);
	}
	
	uint32_t fsize = FS_FileLength(f);
	void *buf = Hunk_TempAlloc(fsize);
	if (FS_Read(buf, fsize, f) != fsize) {
		N_Error("FS_LoadFile: short read");
	}
	FS_FClose(&f);
	
	*buffer = buf;
	return fsize;
}

uint32_t FS_ReadFile(const char *filepath, void *buffer)
{
	file_t f = FS_FOpenRead(filepath);
	if (f == FS_INVALID_HANDLE) {
		N_Error("FS_ReadFile: failed to open file %s", filepath);
	}
	
	uint32_t fsize = FS_FileLength(f);
}

