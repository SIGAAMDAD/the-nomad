#include "n_shared.h"

/*
======================================================================================================

THE NOMAD FILESYSTEM

All of glnomad's resources camn be accesse through a series of searchpaths that are structured hierarichally.

A "npath" is a system independent reference to a file. All npaths are terminated by a zero, and any path
separators are illegal within one. This is done so that no file outside of the dedicated system can be accessed
accidentally.

the basepath is the path to the user's current working directory.

======================================================================================================
*/

#define MAX_FILE_HANDLES 1024
#define MAX_FILEHASH_SIZE 256

#define BASEGAME_DIR "gamedata"

// tunables
#define USE_BFF_CACHE_FILE // cache the bff archives into a file for faster load times

typedef struct fileInBFF_s
{
	bff_chunk_t *handle;
    uint64_t bytesRead;
	struct fileInBFF_s *next;
} fileInBFF_t;

typedef struct
{
	char *bffBasename;
	char *bffFilename;
	char *bffGamename;

	uint64_t numfiles;
	uint64_t hashSize;
	int64_t index;

	fileInBFF_t **hashTable;
	fileInBFF_t *fileList;
	bff_chunk_t *chunkList;
	memoryMap_t *mapping;
	FILE *file;

	uint32_t checksum;
} bffFile_t;

typedef union
{
	FILE *fp;
	fileInBFF_t *chunk;
	void *stream;
} fileData;

typedef struct
{
	char name[MAX_GDR_PATH];

	fileData data;
	memoryMap_t *mapping; // a file mapping is a fake handle to a file

	int64_t bffIndex;
	bffFile_t *bff;
	qboolean bffFile;
	qboolean tmp;
	qboolean used;
	qboolean mapped;
} fileHandle_t;

typedef enum : uint64_t
{
	DIR_STATIC = 0, // always allow access
	DIR_CONST,		// read-only access
} diraccess_t;

typedef struct
{
	char *path;
	char *gamedir;
} directory_t;

typedef struct searchpath_s
{
	directory_t *dir;
	bffFile_t *bff;
	struct searchpath_s *next;
	diraccess_t access;
} searchpath_t;

#define FS_HashFileName(name,len) Com_GenerateHashValue((name),(len))

static bffFile_t	**fs_archives;

static searchpath_t *fs_searchpaths;
static cvar_t		*fs_homepath;
static cvar_t		*fs_basepath;
static cvar_t		*fs_steampath;
static cvar_t		*fs_basegame;
static cvar_t		*fs_gamedirvar;
static char			fs_gamedir[MAX_OSPATH]; // this will be a single file name with no separators

static uint64_t		fs_loadStack;
static uint64_t		fs_readCount;
static uint64_t		fs_writeCount;

static uint64_t		fs_bffCount;
static uint64_t		fs_bffChunks;
static uint64_t		fs_dirCount;

static int64_t		fs_lastBFFIndex;
static qboolean		fs_initialized;

static fileHandle_t handles[MAX_FILE_HANDLES];

static void FS_ReplaceSeparators(char *path)
{
	char *s;

	for (s = path; *s; s++) {
		if (*s == PATH_SEP_FOREIGN)
			*s = PATH_SEP;
	}
}


static FILE *FS_FileForHandle(file_t f)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileForHandle: out of range");
	}
	if (!handles[f].used) {
		N_Error("FS_FileForHandle: invalid handle");
	}

	return handles[f].data.fp;
}

static void FS_InitHandle(fileHandle_t *f)
{
	f->used = qfalse;
	f->bffFile = qfalse;
	f->bff = NULL;
	f->bffIndex = -1;
	f->mapped = qfalse;
	f->tmp = qfalse;
}

static file_t FS_HandleForFile(void)
{
	for (uint64_t i = 0; i < MAX_FILE_HANDLES; i++) {
		if (!handles[i].used) {
			return (file_t)i;
		}
	}
	Con_Error(false, "FS_HandleForFile: not enough handles");
	return FS_INVALID_HANDLE;
}

/*
FS_BuildOSPath: npath may have either forward or backwards slashes
*/
char *FS_BuildOSPath(const char *base, const char *game, const char *npath)
{
	char temp[MAX_OSPATH*2+1];
	static char ospath[2][sizeof(temp)+MAX_OSPATH];
	static int toggle;

	toggle ^= 1; // flip-flop to allow two returns without clash

	if (!game || !game[0]) {
		game = fs_gamedir;
	}

	if (npath)
		snprintf(temp, sizeof(temp), "%c%s%c%s", PATH_SEP, game, PATH_SEP, npath);
	else
		snprintf(temp, sizeof(temp), "%c%s", PATH_SEP, game);
	
	FS_ReplaceSeparators(temp);
	snprintf(ospath[toggle], sizeof(*ospath), "%s%s", base, temp);

	return ospath[toggle];
}

/*
===========
FS_PathCmp

Ignore case and separator char distinctions
===========
*/
static int FS_PathCmp( const char *s1, const char *s2 )
{
	int c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

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

/*
FS_AllowedExtension: returns qfalse if the extension is allowed
*/
qboolean FS_AllowedExtension( const char *fileName, qboolean allowBFFs, const char **ext ) 
{
	static const char *extlist[] =	{ "dll", "exe", "so", "dylib", "qvm", "bff" };
	const char *e;
	uint64_t i, n;

	e = (const char *)strrchr( fileName, '.' );

	// check for unix '.so.[0-9]' pattern
	if ( e >= (fileName + 3) && *(e+1) >= '0' && *(e+1) <= '9' && *(e+2) == '\0' )  {
		if ( *(e-3) == '.' && (*(e-2) == 's' || *(e-2) == 'S') && (*(e-1) == 'o' || *(e-1) == 'O') ) {
			if ( ext ) {
				*ext = (e-2);
			}
			return qfalse;
		}
	}
	if ( !e )
		return qtrue;

	e++; // skip '.'

	if ( allowBFFs )
		n = arraylen( extlist ) - 1;
	else
		n = arraylen( extlist );
	
	for ( i = 0; i < n; i++ )  {
		if ( N_stricmp( e, extlist[i] ) == 0 ) {
			if ( ext )
				*ext = e;
			return qfalse;
		}
	}

	return qtrue;
}

/*
FS_CheckFilenameIsNotAllowed: error if trying to manipulate a file with the platform library extension
 */
static void FS_CheckFilenameIsNotAllowed( const char *filename, const char *function, qboolean allowBFFs )
{
	const char *extension;

	// Check if the filename ends with the library extension
	if ( FS_AllowedExtension( filename, allowBFFs, &extension ) == qfalse )  {
		N_Error( "%s: Not allowed to manipulate '%s' due "
			"to %s extension", function, filename, extension );
	}
}


void FS_Remove(const char *path)
{
	FS_CheckFilenameIsNotAllowed(path, __func__, qtrue);

	remove(path);
}

/*
FS_FileExists

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
*/
qboolean FS_FileExists(const char *filename)
{
	FILE *fp;
	char *testpath;

	testpath = FS_BuildOSPath(fs_homepath->s, fs_gamedir, filename);

	fp = Sys_FOpen(testpath, "rb");
	if (fp) {
		fclose(fp);
		return qtrue;
	}
	return qfalse;
}


/*
FS_CheckDirTraversal:

Check whether the string contains stuff like "../" to prevent directory traversal bugs
and return qtrue if it does.
*/
static qboolean FS_CheckDirTraversal( const char *checkdir )
{
	if ( strstr( checkdir, "../" ) || strstr( checkdir, "..\\" ) )
		return qtrue;

	if ( strstr( checkdir, "::" ) )
		return qtrue;
	
	return qfalse;
}

qboolean FS_FileIsInBFF(const char *filename)
{
	const searchpath_t *sp;
	const fileInBFF_t *chunk;
	uint64_t hash, fullHash;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}

	// npaths are not supposed to have a leading slash
	while (filename[0] == '/' || filename[0] == '\\')
		filename++;
	
	// make absolutely sure that it can't back up the path
	// The searchpaths do guarentee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo"
	if (FS_CheckDirTraversal(filename))
		return qfalse;
	
	fullHash = FS_HashFileName(filename, 0U);

	// search through the paths, one element at a time
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
			// look through all the bff chunks
			chunk = sp->bff->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if (FS_FilenameCompare(chunk->handle->chunkName, filename)) {
					return qtrue;
				}
				chunk = chunk->next;
			} while (chunk != NULL);
		}
	}
	return qfalse;
}

static void FS_SortFileList( char **list, uint64_t n )
{
	const char *m;
	char *temp;
	uint64_t i, j;
	i = 0;
	j = n;
	m = list[ n >> 1 ];
	do {
		while ( FS_PathCmp( list[i], m ) < 0 ) i++;
		while ( FS_PathCmp( list[j], m ) > 0 ) j--;
		if ( i <= j ) {
			temp = list[i];
			list[i] = list[j];
			list[j] = temp;
			i++; 
			j--;
		}
	} while ( i <= j );
	if ( j > 0 ) FS_SortFileList( list, j );
	if ( n > i ) FS_SortFileList( list+i, n-i );
}

static fileInBFF_t *FS_GetChunkHandle(const char *path, int64_t *bffIndex)
{
	searchpath_t *sp;
	fileInBFF_t *chunk;
	uint64_t hash, fullHash;

	// npaths are not supposed to have a leading slash
	while (path[0] == '/' || path[0] == '\\')
		path++;
	
	fullHash = FS_HashFileName(path, 0U);

	// search through the paths, one element at a time
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
			// look through all the bff chunks
			chunk = sp->bff->hashTable[hash];
			do {
				if (FS_FilenameCompare(path, chunk->handle->chunkName)) {
					// found it!
					return chunk;
				}
				chunk = chunk->next;
			} while (chunk != NULL);
		}
	}

	return NULL;
}

file_t FS_OpenFileMapping(const char *path, qboolean temp)
{
	file_t fd;
	fileHandle_t *f;
	const char *ospath;
	searchpath_t *sp;
	FILE *fp;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_OpenFileMapping: NULL or empty path");
	}

	fd = FS_HandleForFile();
	if (fd == FS_INVALID_HANDLE)
		return fd;
	
	f = &handles[fd];
	FS_InitHandle(f);

	// the path isn't a unique file, don't map it
	if (FS_FileIsInBFF(path)) {
		f->data.chunk = FS_GetChunkHandle(path, &f->bffIndex);
		if (!f->data.chunk) {
			return FS_INVALID_HANDLE;
		}
		N_strncpyz(f->name, f->data.chunk->handle->chunkName, MAX_BFF_CHUNKNAME);
		f->mapped = qfalse;
		f->bff = fs_archives[f->bffIndex];
		f->bffFile = qtrue;
		f->used = qtrue;
		return fd;
	}

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->access != DIR_STATIC)
			continue;
		
		ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
		fp = Sys_FOpen(ospath, "wb+");
		if (fp) {
			f->data.fp = fp;
			break;
		}
		return FS_INVALID_HANDLE;
	}

	f->mapping = Sys_MapMemory(f->data.fp, temp, fd);
	if (!f->mapping) {
		// cleanup, file mapping failed
		f->data.stream = NULL;
		f->used = qfalse;
		fclose(fp);

		return FS_INVALID_HANDLE;
	}

	f->mapped = qtrue;
	N_strncpyz(f->name, path, MAX_GDR_PATH);
	f->used = qtrue;

	return fd;
}

bff_chunk_t *FS_GetCurrentChunkList(uint64_t *numchunks)
{
	bff_chunk_t *chunkList;
	fileInBFF_t *file;

	if (fs_lastBFFIndex == -1) {
		return NULL;
	}

	*numchunks = fs_archives[fs_lastBFFIndex]->numfiles;

	chunkList = (bff_chunk_t *)Z_Malloc(sizeof(*chunkList) * *numchunks, TAG_STATIC, &chunkList, "chunkList");
	file = fs_archives[fs_lastBFFIndex]->fileList;
	for (uint64_t i = 0; i < *numchunks; i++) {
		N_strncpyz(chunkList[i].chunkName, file->handle->chunkName, MAX_BFF_CHUNKNAME);
		chunkList[i].chunkSize = file->handle->chunkSize;
		chunkList[i].chunkType = file->handle->chunkType;
		chunkList[i].chunkBuffer = file->handle->chunkBuffer;
	}

	return chunkList;
}

static file_t FS_OpenChunk(const char *name, fileInBFF_t *chunk, fileHandle_t *f, bffFile_t *bff)
{
	f->bff = bff;
	f->bffIndex = bff->index;
	f->bffFile = qtrue;
	f->data.chunk = chunk;
	f->used = qtrue;
	fs_lastBFFIndex = bff->index;
	N_strncpyz(f->name, name, MAX_BFF_CHUNKNAME);

	return (file_t)(handles - f);
}


/*
FS_IsExt: Return qtrue if ext matches file extension filename
*/
static qboolean FS_IsExt( const char *filename, const char *ext, size_t namelen )
{
	size_t extlen;

	extlen = strlen( ext );

	if ( extlen > namelen )
		return qfalse;

	filename += namelen - extlen;

	return (qboolean)!N_stricmp( filename, ext );
}


int64_t FS_LastBFFIndex(void)
{
	return fs_lastBFFIndex;
}

/*
==========================================================

BFF FILE LOADING

==========================================================
*/

// map a bff archive file if its >= to this size
#define BFF_MAPFILE_SIZE (900*1024*1024)
#define BFF_STREAM_MAPPED 0
#define BFF_STREAM_FILE 1

#define BFF_CACHE_FILENAME "bffcache.dat"

static uint64_t fs_bffsRead;
static uint64_t fs_bffsReleased;
static uint64_t fs_bffsCached;
static uint64_t fs_bffsSkipped;
static qboolean fs_cacheLoaded;

static uint64_t FS_BFFHashSize( const uint64_t filecount )
{
	uint64_t hashSize;

	for ( hashSize = 2; hashSize < MAX_FILEHASH_SIZE; hashSize <<= 1 ) {
		if ( hashSize >= filecount ) {
			break;
		}
	}

	return hashSize;
}

static uint64_t FS_ReadFromBFF(uint32_t stream, void *fp, void *buffer, uint64_t size)
{
	if (stream == BFF_STREAM_MAPPED) {
		return Sys_ReadMappedFile(buffer, size, (memoryMap_t *)fp);
	}
	return fread(buffer, 1, size, (FILE *)fp);
}

static fileOffset_t FS_SeekBFF(uint32_t stream, void *fp, fileOffset_t offset, uint32_t whence)
{
	if (stream == BFF_STREAM_MAPPED) {
		return Sys_SeekMappedFile(offset, whence, (memoryMap_t *)fp);
	}
	return fseek((FILE *)fp, offset, whence);
}

static fileOffset_t FS_TellBFF(uint32_t stream, void *fp)
{
	if (stream == BFF_STREAM_MAPPED) {
		return Sys_TellMappedFile((memoryMap_t *)fp);
	}
	return ftell((FILE *)fp);
}

static void FS_CloseBFFStream(FILE *fp, memoryMap_t *file)
{
	if (file)
		Sys_UnmapFile(file);
	else
		fclose(fp);
}

#if 0
static void FS_LoadCache(void)
{
	const char *filename = BFF_CACHE_FILENAME;
	const char *ospath;
	FILE *fp;

	fs_bffsRead = 0;
	fs_bffsReleased = 0;
	if (fs_cacheLoaded)
		return;
	
	fs_bffsCached = 0;
	fs_bffsSkipped = 0;

	ospath = FS_BuildOSPath(fs_homepath->s, filename, NULL);
	fp = Sys_FOpen(ospath, "rb");
	if (!fp)
		return;
	
	if (!FS_ValidateCacheHeader(fp)) {
		fclose(fp);
		return;
	}
}
#endif

/*
FS_LoadBFF: creates a new bffFile_t in the search chain for the contents of a bff archive file
*/
static bffFile_t *FS_LoadBFF(const char *bffpath)
{
	fileInBFF_t *curFile;
	bffFile_t *bff;
	bffheader_t header;
	bff_chunk_t *curChunk;
	uint64_t i, hashSize, size;
	uint64_t hash;
	uint64_t chunkCount;
	const char *basename;
	char *gameName, *namePtr;
	uint64_t gameNameLen, gameNameLenReal;
	uint64_t chunkSize;
	uint64_t baseNameLen, fileNameLen;
	uint32_t stream;
	void *streamPtr;
	memoryMap_t *file;
	FILE *fp;
	fileStats_t stats;

	// extract the basename from bffpath
	basename = strrchr(bffpath, PATH_SEP);
	if (basename == NULL)
		basename = bffpath;
	else
		basename++;
	
	fileNameLen = strlen(bffpath) + 1;
	baseNameLen = strlen(basename) + 1;

	if (!Sys_GetFileStats(&stats, bffpath)) {
		Con_Printf(ERROR, "FS_LoadBFF: failed to load bff %s", bffpath);
		return NULL;
	}

	chunkCount = 0;
	fp = NULL;
	file = NULL;

	// if the file is really heavy, map it, otherwise, open it normally
	if (stats.size >= BFF_MAPFILE_SIZE) {
		file = Sys_MapFile(bffpath, qtrue); // create a temporary file mapping, don't save it to disk
		if (!file) {
			Con_Printf(ERROR, "FS_LoadBFF: failed to create a memory mapping of bff %s", bffpath);
			return NULL;
		}
		stream = BFF_STREAM_MAPPED;
		streamPtr = (void *)file;
	}
	else {
		fp = Sys_FOpen(bffpath, "rb");
		if (!fp) {
			Con_Printf(ERROR, "FS_LoadBFF: failed to open bff %s in readonly mode", bffpath);
			return NULL;
		}
		stream = BFF_STREAM_FILE;
		streamPtr = (void *)fp;
	}

	if (FS_ReadFromBFF(stream, streamPtr, &header, sizeof(header)) != sizeof(header)) {
		FS_CloseBFFStream(fp, file);
		N_Error("FS_LoadBFF: failed to read header");
	}

	if (header.ident != BFF_IDENT) {
		FS_CloseBFFStream(fp, file);
		return NULL;
	}
	if (header.magic != BFF_VERSION) {
		FS_CloseBFFStream(fp, file);
		return NULL;
	}
	if (!header.numChunks) {
		FS_CloseBFFStream(fp, file);
		return NULL;
	}

	// technically not an error, but could cause some severe issues if the
	// version gap is large enough
	if (header.version != BFF_VERSION) {
		Con_Printf(
			C_YELLOW "==== WARNING: bff version found in header isn't the same as this program's ====\n" C_RESET
			"\tHeader Version: %hi\n\tProgram BFF Version: %hi", header.version, BFF_VERSION);
	}
	
	if (FS_ReadFromBFF(stream, streamPtr, &gameNameLen, sizeof(uint64_t)) != sizeof(uint64_t)) {
		FS_CloseBFFStream(fp, file);
		N_Error("FS_LoadBFF: failed to read gameNameLen");
	}

	gameNameLenReal = gameNameLen + 1;

	// read the name
	gameName = (char *)alloca(PAD(gameNameLen, sizeof(uintptr_t)));
	if (FS_ReadFromBFF(stream, streamPtr, gameName, gameNameLen + 1) != gameNameLen + 1) {
		FS_CloseBFFStream(fp, file);
		N_Error("FS_LoadBFF: failed to read gameName");
	}

	// get the hash table size from the number of files in the bff
	hashSize = FS_BFFHashSize(chunkCount);

	size = 0;
	gameNameLen++;
	gameNameLen = PAD(gameNameLen, sizeof(uintptr_t));
	size += sizeof(*bff) + hashSize * sizeof(*bff->hashTable) + chunkCount * sizeof(*bff->fileList) + gameNameLen
		+ sizeof(*bff->chunkList) * chunkCount;
	size += PAD(fileNameLen, sizeof(uintptr_t));
	size += PAD(baseNameLen, sizeof(uintptr_t));

	bff = (bffFile_t *)Z_Malloc(size, TAG_BFF, &bff, "BFFfile");
	memset(bff, 0, size);

	bff->file = fp;
	bff->mapping = file;
	bff->numfiles = chunkCount;
	bff->hashSize = hashSize;
	bff->hashTable = (fileInBFF_t **)(bff + 1);

	bff->fileList = (fileInBFF_t *)(bff->hashTable + bff->hashSize);
	bff->chunkList = (bff_chunk_t *)(bff->fileList + bff->numfiles);

	namePtr = (char *)(bff->chunkList + chunkCount);

	bff->bffGamename = namePtr;
	bff->bffFilename = (char *)(bff->bffGamename + gameNameLen);
	bff->bffBasename = (char *)(bff->bffFilename + PAD(fileNameLen, sizeof(uintptr_t)));

	memcpy(bff->bffGamename, gameName, gameNameLen);
	memcpy(bff->bffFilename, bffpath, fileNameLen);
	memcpy(bff->bffBasename, basename, baseNameLen);

	// strip the .bff if needed
	FS_StripExt(bff->bffBasename, ".bff");

	// set the correct filepos
	FS_SeekBFF(stream, streamPtr, sizeof(bffheader_t) + sizeof(uint64_t) + gameNameLenReal, SEEK_SET);
	
	curFile = bff->fileList;
	curChunk = bff->chunkList;
	constexpr uint64_t readSize = sizeof(*curChunk) - sizeof(char *);
	for (i = 0; i < chunkCount; i++) {
		if (FS_ReadFromBFF(stream, streamPtr, curChunk, readSize) != readSize) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: failed to read chunk info at %lu", i);
		}
		if (!curChunk->chunkSize) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: bad chunk size at %lu", i);
		}

		// update the hash table
		hash = FS_HashFileName(curChunk->chunkName, bff->hashSize);

		curFile->next = bff->hashTable[hash];
		bff->hashTable[hash] = curFile;
		curFile->handle = curChunk;

		// read the chunk data
		curChunk->chunkBuffer = (char *)Hunk_Alloc(curChunk->chunkSize, "chunkBuffer", h_low);
		if (FS_ReadFromBFF(stream, streamPtr, curChunk->chunkBuffer, curChunk->chunkSize) != curChunk->chunkSize) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: failed to read chunk buffer at %lu", i);
		}

		curFile++;
		curChunk++;
	}

	bff->checksum = Com_BlockChecksum(bff, size);
	bff->checksum = LittleInt(bff->checksum);

	FS_CloseBFFStream(fp, file);

	return bff;
}


fileOffset_t FS_FileTell(file_t f)
{
	fileHandle_t *p;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileTell: out of range");
	}

	p = &handles[f];

	// not a unique file
	if (p->bffFile)
		return (fileOffset_t)(p->data.chunk->handle->chunkSize - p->data.chunk->bytesRead);
	
	// normal file pointer
	return (fileOffset_t)ftell(p->data.fp);
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

	if (FS_FileIsInBFF(file->name)) {
		if (whence == FS_SEEK_END && offset) {
			return -1;
		}
		else if (whence == FS_SEEK_CUR
		&& file->data.chunk->bytesRead + offset >= file->data.chunk->handle->chunkSize) {
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
			file->data.chunk->bytesRead = file->data.chunk->handle->chunkSize;
			break;
		default:
			N_Error("FS_FileSeek: invalid seek");
		};
		return (fileOffset_t)(file->data.chunk->handle->chunkSize - file->data.chunk->bytesRead);
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
		N_Error("FS_FileSeek: invalid seek");
	};
	return (fileOffset_t)fseek(file->data.fp, (long)offset, (int)fwhence);
}

uint64_t FS_FileLength(file_t f)
{
	uint64_t curPos, length;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileLength: out of range");
	}

	curPos = FS_FileTell(f);
	FS_FileSeek(f, 0, FS_SEEK_END);
	length = FS_FileTell(f);
	FS_FileSeek(f, curPos, FS_SEEK_BEGIN);
	return length;
}

/*
FS_Write: properly handles partial writes
*/
uint64_t FS_Write(const void *buffer, uint64_t size, file_t f)
{
	int64_t writeCount, remaining, block;
	const byte *buf;
	int tries;
	FILE *fp;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}

	fp = FS_FileForHandle(f);

	buf = (const byte *)buffer;
	remaining = size;
	tries = 0;

	while (remaining) {
		block = remaining;
		writeCount = fwrite(buf, 1, block, fp);
		if (writeCount == 0) {
			if (!tries) {
				tries = 1;
			}
			else {
				Con_Printf("FS_Write: 0 bytes written");
				return 0;
			}
		}

		if (writeCount == -1) {
			Con_Printf("FS_Write: -1 bytes written");
			return 0;
		}

		buf += writeCount;
		remaining -= writeCount;
		fs_writeCount += writeCount;
	}
	// fflush(fp);

	return size;
}

static uint64_t FS_ReadFromChunk(void *buffer, uint64_t size, file_t f)
{
	fileHandle_t *handle = &handles[f];

	// this should never happen, if it does, no questioning, just crash
	if (handle->data.chunk->bytesRead + size > handle->data.chunk->handle->chunkSize) {
		N_Error("FS_ReadFromChunk: overread");
	}

	memcpy(buffer, handle->data.chunk->handle->chunkBuffer + handle->data.chunk->bytesRead, size);
	handle->data.chunk->bytesRead += size;
	return size;
}

/*
FS_Read: properly handles partial reads
*/
uint64_t FS_Read(void *buffer, uint64_t size, file_t f)
{
	int64_t readCount, remaining, block;
	byte *buf;
	int tries;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}

	buf = (byte *)buffer;
	fs_readCount += size;

	if (!handles[f].bffFile) {
		remaining = size;
		tries = 0;

		while (remaining) {
			block = remaining;
			readCount = fread(buf, 1, block, handles[f].data.fp);
			if (readCount == 0) {
				// we might have been trying to read from a CD, which
				// sometimes returns a 0 read on windows
				if (!tries) {
					tries = 1;
				}
				else {
					return size - remaining;
				}
			}

			if (readCount == -1) {
				N_Error("FS_Read: -1 bytes read");
			}

			remaining -= readCount;
			buf += readCount;
		}
		return size;
	}
	else {
		return FS_ReadFromChunk(buffer, size, f);
	}
}

void GDR_DECL FS_Printf(file_t f, const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	N_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	FS_Write(msg, strlen(msg), f);
}


file_t FS_FOpenWrite(const char *path)
{
	file_t fd;
	fileHandle_t *f;
	FILE *fp;
	const char *ospath;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_FOpenWrite: NULL or empty path");
	}

	// write streams aren't allowed for chunks
	if (FS_FileIsInBFF(path)) {
		return FS_INVALID_HANDLE;
	}

	// validate the file is actually write-enabled
	FS_CheckFilenameIsNotAllowed(path, __func__, qfalse);

	fd = FS_HandleForFile();
	if (fd == FS_INVALID_HANDLE)
		return fd;
	
	f = &handles[fd];
	FS_InitHandle(f);
	ospath = FS_BuildOSPath(fs_homepath->s, NULL, path);

	fp = Sys_FOpen(ospath, "wb");
	if (!fp) {
		N_Error("FS_FOpenWrite: failed to create write-only stream for %s", path);
	}

	f->used = qtrue;
	f->data.fp = fp;

	return fd;
}

file_t FS_FOpenRead(const char *path)
{
	file_t fd;
	fileHandle_t *f;
	fileInBFF_t *chunk;
	uint64_t fullHash, hash;
	searchpath_t *sp;
	FILE *fp;
	const char *ospath;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_FOpenRead: NULL or empty path");
	}

	fd = FS_HandleForFile();
	if (fd == FS_INVALID_HANDLE)
		return fd;
	
	f = &handles[fd];
	FS_InitHandle(f);

	// we will calculate the full hash only once then just mask it by current bff->hashSize
	// we can do that as long as we know properties of our hash function
	fullHash = FS_HashFileName(path, 0U);
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
			// look through all the chunks
			chunk = sp->bff->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if (FS_FilenameCompare(chunk->handle->chunkName, path)) {
					// found it!
					return FS_OpenChunk(path, chunk, f, sp->bff);
				}
				chunk = chunk->next;
			} while (chunk != NULL);
		}
		else if (sp->dir && sp->access != DIR_CONST) {
			ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
			fp = Sys_FOpen(ospath, "rb");
			if (!fp) {
				continue;
			}

			f->data.fp = fp;
			N_strncpyz(f->name, path, sizeof(f->name));
			f->bffFile = qfalse;

			return fd;
		}
	}

	return FS_INVALID_HANDLE;
}

uint64_t FS_FOpenFileRead(const char *path, file_t *fd)
{
	fileHandle_t *f;
	const char *ospath;
	FILE *fp;
	searchpath_t *sp;
	fileInBFF_t *chunk;
	uint64_t fullHash, hash;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_FOpenFileRead: NULL or empty path");
	}

	// we will calculate the full hash only once then just mask it by current bff->hashSize
	// we can do that as long as we know properties of our hash function
	fullHash = FS_HashFileName(path, 0U);

	// we just want the file's size
	if (fd == NULL) {
		for (sp = fs_searchpaths; sp; sp = sp->next) {
			// is the element a bff file?
			if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
				// look through all the chunks
				chunk = sp->bff->hashTable[hash];
				do {
					if (FS_FilenameCompare(chunk->handle->chunkName, path)) {
						// found it!
						return chunk->handle->chunkSize;
					}
					chunk = chunk->next;
				} while (chunk != NULL);
			}
			else if (sp->dir && sp->access != DIR_CONST) {
				ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
				fp = Sys_FOpen(ospath, "rb");
				if (fp) {
					fseek(fp, 0L, SEEK_END);
					uint64_t size = ftell(fp);
					fclose(fp);
					return size;
				}
			}
		}
	}
	else {
		*fd = FS_HandleForFile();
		if (*fd == FS_INVALID_HANDLE)
			return 0;

		f = &handles[*fd];
		FS_InitHandle(f);
		for (sp = fs_searchpaths; sp; sp = sp->next) {
			// is the element a bff file?
			if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
				// look through all the chunks
				chunk = sp->bff->hashTable[hash];
				do {
					if (FS_FilenameCompare(chunk->handle->chunkName, path)) {
						// found it!
						return FS_OpenChunk(path, chunk, f, sp->bff);
					}
					chunk = chunk->next;
				} while (chunk != NULL);
			}
			else if (sp->dir && sp->access != DIR_CONST) {
				ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
				fp = Sys_FOpen(ospath, "rb");
				if (!fp) {
					continue;
				}

				f->data.fp = fp;
				f->used = qtrue;
				N_strncpyz(f->name, path, sizeof(f->name));
				return FS_FileLength(*fd);
			}
		}
	}

	*fd = FS_INVALID_HANDLE;
	return 0;
}

/*
FS_LoadFile: filename are relative to the glnomad search path.
A null buffer will just return the file length without loading
*/
uint64_t FS_LoadFile(const char *npath, void **buffer)
{
	file_t fd;
	fileHandle_t *f;
	byte *buf;
	uint64_t size;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!npath || !*npath) {
		N_Error("FS_LoadFile: NULL or empty path");
	}

	buf = NULL; // go away warnings

	// look for it in the filesystem or archives
	size = FS_FOpenFileRead(npath, &fd);
	if (fd == FS_INVALID_HANDLE) {
		if (buffer)
			*buffer = NULL;
	}
	if (!buffer) {
		FS_FClose(fd);
		return size;
	}

	buf = (byte *)Hunk_AllocateTempMemory(size + 1);
	*buffer = buf;

	FS_Read(buf, size, fd);

	fs_loadStack++;

	// guarentee that it will have a trialing 0 for string operations
	buf[size] = '\0';
	FS_FClose(fd);

	return size;
}

void FS_FreeFile(void *buffer)
{
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!buffer) {
		N_Error("FS_FreeFile(NULL)");
	}
	fs_loadStack--;

	Hunk_FreeTempMemory(buffer);

	// if all our temp files are free, clear all of our space
	if (fs_loadStack == 0) {
		Hunk_ClearTempMemory();
	}
}

void FS_FClose(file_t f)
{
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FClose: out of range");
	}
	fileHandle_t *p = &handles[f];
	
	// unique file
	if (!FS_FileIsInBFF(handles[f].name)) {
		// has it been mapped?
		if (p->mapped && p->mapping) {
			Sys_UnmapMemory(p->mapping);
			Z_Free(p->mapping);
		}
		fclose(p->data.fp);
	}

	// reinitialize it all back to 0
	FS_InitHandle(p);
}

qboolean FS_StripExt( char *filename, const char *ext )
{
	uint64_t extlen, namelen;

	extlen = strlen( ext );
	namelen = strlen( filename );

	if ( extlen > namelen )
		return qfalse;

	filename += namelen - extlen;

	if ( !N_stricmp( filename, ext ) )  {
		filename[0] = '\0';
		return qtrue;
	}

	return qfalse;
}

uint64_t FS_LoadStack(void)
{
	return fs_loadStack;
}


static void FS_AddGameDirectory(const char *path, const char *dir)
{
	const searchpath_t *sp;
	uint64_t size;
	searchpath_t *search;
	bffFile_t *bff;
	const char *gamedir;
	char curpath[MAX_OSPATH*2+1];
	char *bffpath;
	uint64_t numfiles, numdirs;
	char **bffFiles;
	uint64_t bffFilesI;
	char **bffDirs;
	uint64_t bffDirsI;
	int bffWhich;
	uint64_t path_len;
	uint64_t dir_len;

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->dir && !N_stricmp(sp->dir->path, path) && !N_stricmp(sp->dir->gamedir, dir)) {
			return; // we've already added this one
		}
	}

	N_strncpyz(fs_gamedir, dir, sizeof(fs_gamedir));

	// add the directory to the search path
	path_len = strlen(path) + 1;
	path_len = PAD(path_len, sizeof(uintptr_t));
	dir_len = strlen(dir) + 1;
	dir_len = PAD(dir_len, sizeof(uintptr_t));
	size = sizeof(*search) + sizeof(*search->dir) + path_len + dir_len;

	search = (searchpath_t *)Z_Malloc(size, TAG_SEARCH_PATH, &search, "searchpath");
	memset(search, 0, size);

	search->dir = (directory_t *)(search + 1);
	search->dir->path = (char *)(search->dir + 1);
	search->dir->gamedir = (char *)(search->dir->path + path_len);
	
	strcpy(search->dir->path, path);
	strcpy(search->dir->gamedir, dir);
	gamedir = (const char *)search->dir->gamedir;

	search->next = fs_searchpaths;
	fs_searchpaths = search;
	fs_dirCount++;

	// find all the bffs in this directory
	N_strncpyz(curpath, FS_BuildOSPath(path, dir, NULL), sizeof(curpath));

	// get .bff files
	bffFiles = Sys_ListFiles(curpath, ".bff", NULL, &numfiles, qfalse);

	if (numfiles >= 2)
		FS_SortFileList(bffFiles, numfiles - 1);
	
	bffFilesI = 0;
	bffDirsI = 0;
	
	// get top level directories (we'll filter them later since the Sys_ListFiles filtering is terrible)
	bffDirs = Sys_ListFiles(curpath, "/", NULL, &numdirs, qfalse);
	if (numdirs >= 2)
		FS_SortFileList(bffDirs, numdirs - 1);
	
	while ((bffFilesI < numfiles) || (bffDirsI < numdirs)) {
		// check if a bffFIle or bffDir comes next
		if (bffFilesI >= numfiles) {
			// we've used all the bffFiles, it must be a bffDir
			bffWhich = 0;
		}
		else if (bffDirsI >= numdirs) {
			// we've used all the bffDirs, it must be a bffFIle
			bffWhich = 1;
		}
		else {
			// could be either, compare to see which name comes first
			bffWhich = (FS_PathCmp(bffFiles[bffFilesI], bffDirs[bffDirsI]) < 0);
		}

		if (bffWhich) {
			size = strlen(bffFiles[bffFilesI]);
			if (!FS_IsExt(bffFiles[bffFilesI], ".bff", size)) {
				// not a bff file
				bffFilesI++;
				continue;
			}

			// the next .bff file is before the next .bffdir
			bffpath = FS_BuildOSPath(path, dir, bffFiles[bffFilesI]);
			if ((bff = FS_LoadBFF(bffpath)) == NULL) {
				// this isn't a .bff! NeXT!
				bffFilesI++;
				continue;
			}

			bff->index = fs_bffCount;

			fs_bffChunks += bff->numfiles;
			fs_bffCount++;

			search = (searchpath_t *)Z_Malloc(sizeof(*search), TAG_SEARCH_PATH, &search, "searchpath");
			memset(search, 0, sizeof(*search));
			search->bff = bff;

			search->next = fs_searchpaths;
			fs_searchpaths = search;

			bffFilesI++;
		}
		else {
			N_Error("THIS SHOULDN'T HAPPEN!!!!!");
		}
	}

	Sys_FreeFileList(bffDirs);
	Sys_FreeFileList(bffFiles);
}

static void FS_ReorderSearchPaths(void)
{
	searchpath_t **list, **bffs, **dirs;
	searchpath_t *path;
	uint64_t i, ndirs, nbffs, cnt;

	cnt = fs_bffCount + fs_dirCount;
	if (cnt == 0)
		return;
	
	// relink path chains in the following order:
	// 1. files
	// 2. directories
	list = (searchpath_t **)Z_Malloc(cnt * sizeof(*list), TAG_SEARCH_PATH, &list, "searchpaths");
	bffs = list;
	dirs = list + fs_dirCount + fs_bffCount;

	nbffs = ndirs = 0;
	path = fs_searchpaths;
	while (path) {
		if (path->bff || path->access != DIR_STATIC)
			bffs[nbffs++] = path;
		else
			dirs[ndirs++] = path;
		
		path = path->next;
	}

	fs_searchpaths = list[0];
	for (i = 0; i < cnt - 1; i++) {
		list[i]->next = list[i+1];
	}
	list[cnt - 1]->next = NULL;

	Z_Free(list);
}

void FS_InitFilesystem(void)
{
	const char *homepath;
	uint64_t start, end;

	fs_bffCount = 0;
	fs_loadStack = 0;
	fs_lastBFFIndex = -1;
	fs_bffChunks = 0;
	fs_readCount = 0;
	fs_writeCount = 0;
	fs_cacheLoaded = qfalse;

	Con_Printf("============ FS_InitFilesystem ============");

	fs_basepath = Cvar_Get("fs_basepath", Sys_DefaultBasePath(), CVAR_PRIVATE);
	Cvar_SetDescription(fs_basepath, "Write-protected CVar specifying the path to the installation folder of the game.");
	fs_basegame = Cvar_Get("fs_basegame", BASEGAME_DIR, CVAR_PRIVATE);
	Cvar_SetDescription(fs_basegame, "Cvar specifying the path to the base game folder.");
	fs_steampath = Cvar_Get("fs_steampath", Sys_GetSteamPath(), CVAR_PRIVATE | CVAR_ROM);

	if (!fs_basegame->s[0])
		N_Error("* fs_basegame not set *");
	
	homepath = Sys_DefaultHomePath();
	if (!homepath || !homepath[0]) {
		homepath = fs_basepath->s;
	}

	fs_homepath = Cvar_Get("fs_homepath", homepath, CVAR_PRIVATE | CVAR_ROM);

	fs_gamedirvar = Cvar_Get("fs_gamedir", "", CVAR_PRIVATE);
	Cvar_SetDescription(fs_gamedirvar, "Specify an alternate mod directory and run the game with this mod.");

	if (!N_stricmp(fs_basegame->s, fs_gamedirvar->s)) {
		Cvar_Set("fs_gamedir", fs_basegame->s);
	}

	start = Sys_Milliseconds();

	// add search path elements in reverse priority order
	if (fs_steampath->s[0]) {
		FS_AddGameDirectory(fs_steampath->s, fs_basegame->s);
	}
	
	if (fs_basepath->s[0]) {
		FS_AddGameDirectory(fs_basepath->s, fs_basegame->s);
	}

	// fs_homepath is somewhat particular to *nix systems, only add if relevant
	if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basepath->s)) {
		FS_AddGameDirectory(fs_homepath->s, fs_basepath->s);
	}

	// check for additional game folder for mods
	if (fs_gamedirvar->s[0] && N_stricmp(fs_gamedirvar->s, fs_basegame->s)) {
		if (fs_steampath->s[0]) {
			FS_AddGameDirectory(fs_steampath->s, fs_gamedirvar->s);
		}
		if (fs_basepath->s[0]) {
			FS_AddGameDirectory(fs_basepath->s, fs_gamedirvar->s);
		}
		if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basepath->s)) {
			FS_AddGameDirectory(fs_homepath->s, fs_gamedirvar->s);
		}
	}

	end = Sys_Milliseconds();

	Con_Printf( "...loaded in %lu milliseconds\n", end - start );

	Con_Printf( "----------------------\n" );
	Con_Printf( "%lu chunks in %lu bff files\n", fs_bffChunks, fs_bffCount );
}



/*
===========
FS_FilenameCompare

Ignore case and separator char distinctions
===========
*/
qboolean FS_FilenameCompare( const char *s1, const char *s2 )
{
	int c1, c2;
	
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

char *FS_CopyString(const char *s)
{
	char *out;
	out = (char *)Z_Malloc(strlen(s) + 1, TAG_STATIC, &out, "fsCopyString");
	strcpy( out, s );
	return out;
}

static void FS_GetModDescription(const char *modDir, char *description, uint64_t descriptionLen)
{
	file_t descHandle;
	char descPath[MAX_GDR_PATH];
	uint64_t nDescLen;

	snprintf(descPath, sizeof(descPath), "%s%cdescription.txt", modDir, PATH_SEP);
	FS_ReplaceSeparators(descPath);
	nDescLen = FS_FOpenFileRead(descPath, &descHandle);

	if (descHandle != FS_INVALID_HANDLE) {
		if (nDescLen) {
			if (nDescLen > descriptionLen - 1)
				nDescLen = descriptionLen - 1;
			
			nDescLen = FS_Read(description, nDescLen, descHandle);
			if (nDescLen >= 0) {
				description[nDescLen] = '\0';
			}
		}
		else {
			N_strncpyz(description, modDir, descriptionLen);
		}
		FS_FClose(descHandle);
	}
	else {
		N_strncpyz(description, modDir, descriptionLen);
	}
}

/*
=======================
Sys_ConcatenateFileLists

mkv: Naive implementation. Concatenates three lists into a
     new list, and frees the old lists from the heap.
bk001129 - from cvs1.17 (mkv)

FIXME TTimo those two should move to common.c next to Sys_ListFiles
=======================
 */
static uint64_t Sys_CountFileList( char **list )
{
	uint64_t i = 0;

	if ( list ) {
		while ( *list ) {
			list++;
			i++;
		}
	}

	return i;
}


static char** Sys_ConcatenateFileLists( char **list0, char **list1 )
{
	uint64_t totalLength;
	char **src, **dst, **cat;

	totalLength = Sys_CountFileList( list0 );
	totalLength += Sys_CountFileList( list1 );

	/* Create new list. */
	dst = cat = (char **)Z_Malloc( ( totalLength + 1 ) * sizeof( char* ), TAG_STATIC, NULL, "fileList");

	/* Copy over lists. */
	if ( list0 ) {
		for (src = list0; *src; src++, dst++)
			*dst = *src;
	}

	if ( list1 ) {
		for ( src = list1; *src; src++, dst++ )
			*dst = *src;
	}

	// Terminate the list
	*dst = NULL;

	// Free our old lists.
	// NOTE: not freeing their content, it's been merged in dst and still being used
	if ( list0 ) Z_Free( list0 );
	if ( list1 ) Z_Free( list1 );

	return cat;
}

/*
FS_GetModList: returns a list of mod directory names. A mod directory is
a peer to gamedata/ with a .bff in it
*/
static uint64_t FS_GetModList(char *listbuf, uint64_t bufSize)
{
	uint64_t i, j, k;
	uint64_t nMods, nTotal, nLen, nBFFs, nPotential, nDescLen;
	uint64_t nDirs, nBFFDirs;
	char **pFiles = NULL;
	char **pBFFs = NULL;
	char **pDirs = NULL;
	const char *name, *path;
	char description[MAX_OSPATH];

	uint64_t dummy;
	char **pFiles0 = NULL;
	qboolean bDrop = qfalse;

	// paths to search for mods
	cvar_t *const *paths[] = { &fs_basepath, &fs_homepath, &fs_steampath };

	*listbuf = '\0';
	nMods = nTotal = 0;

	// iterate through the paths and get the list of potential mods
	for (i = 0; i < arraylen(paths); i++) {
		if (!*paths[i] || !(*paths[i])->s[0])
			continue;
		
		pFiles0 = Sys_ListFiles((*paths[i])->s, NULL, NULL, &dummy, qtrue);
		// Sys_ConcatenateFileLists frees the lists so Sys_FreeFileList isn't required
		pFiles = Sys_ConcatenateFileLists(pFiles, pFiles0);
	}

	nPotential = Sys_CountFileList(pFiles);

	for (i = 0; i < nPotential; i++) {
		name = pFiles[i];

		if (i != 0) {
			bDrop = qfalse;
			for (j = 0; j < i; j++) {
				if (N_stricmp(pFiles[j], name) == 0) {
					// this one can be dropped
					bDrop = qtrue;
					break;
				}
			}
		}
		
		// we also drop BASEGAME, "." and ".."
		if (bDrop || N_stricmp(name, fs_basegame->s) == 0) {
			continue;
		}
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
			continue;
		}

		nBFFs = nBFFDirs = 0;
		for (j = 0; j < arraylen(paths); j++) {
			if (!*paths[j] || !(*paths[j])->s[0])
				continue;
			
			path = FS_BuildOSPath((*paths[j])->s, name, NULL);

			nBFFs = nDirs = nBFFDirs = 0;
			pBFFs = Sys_ListFiles(path, ".bff", NULL, &nBFFs, qfalse);
			pDirs = Sys_ListFiles(path, "/", NULL, &nDirs, qfalse);
			for (k = 0; k < nDirs; k++) {
				// we only want to count directories ending with ".bffdir"
				if (FS_IsExt(pDirs[k], ".bffdir", strlen(pDirs[k]))) {
					nBFFDirs++;
				}
			}

			// we only use Sys_ListFiles to check whether files are present
			Sys_FreeFileList(pBFFs);
			Sys_FreeFileList(pDirs);
			if (nBFFs > 0 || nBFFDirs > 0) {
				break;
			}
		}
		if (nBFFs > 0 || nBFFDirs > 0) {
			nLen = strlen(name) + 1;
			// nLen is the length of the mod path
			// we need to see if there is a description available
			FS_GetModDescription(name, description, sizeof(description));
			nDescLen = strlen(description) + 1;

			if (nTotal + nLen + 1 + nDescLen + 1 < bufSize) {
				strcpy(listbuf, name);
				listbuf += nLen;
				strcpy(listbuf, description);
				listbuf += nDescLen;
				nTotal += nLen + nDescLen;
				nMods++;
			}
			else {
				break;
			}
		}
	}
	Sys_FreeFileList(pFiles);

	return nMods;
}

static uint64_t FS_AddFileToList( const char *name, char **list, uint64_t nfiles )
{
	uint64_t i;

	if ( nfiles == MAX_FOUND_FILES - 1 ) {
		return nfiles;
	}
	for ( i = 0 ; i < nfiles ; i++ ) {
		if ( !N_stricmp( name, list[i] ) ) {
			return nfiles; // already in list
		}
	}
	list[ nfiles ] = FS_CopyString( name );
	nfiles++;

	return nfiles;
}

/*
FS_ListFilteredFiles: returns a unique list of files that match the given criteria
from all search paths
*/
static char **FS_ListFilteredFiles(const char *path, const char *extension, const char *filter, uint64_t *numfiles, uint32_t flags)
{
	uint64_t nfiles;
	char **listCopy;
	char *list[MAX_FOUND_FILES];
	uint64_t i;
	uint64_t pathLength;
	uint64_t extLen;
	uint64_t length, pathDepth, temp;
	const bffFile_t *bff;
	const fileInBFF_t *fileList;
	qboolean hasPatterns;
	const char *x;
	const searchpath_t *sp;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}

	if (!path) {
		*numfiles = 0;
		return NULL;
	}

	if (!extension) {
		extension = "";
	}

	extLen = strlen(extension);
	hasPatterns = Com_HasPatterns(extension);
	if (hasPatterns && extension[0] == '.' && extension[1] != '\0') {
		extension++;
	}

	pathLength = strlen(path);
	if (pathLength > 0 && (path[pathLength - 1] == '\\' || path[pathLength - 1] == '/')) {
		pathLength--;
	}
	nfiles = 0;
//	FS_ReturnPath(path, );

	// search through the path, one element at a time, adding to the list
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff archive?
		if (sp->bff && (flags & FS_MATCH_BFFs)) {
			
			// look through all the bff chunks
			bff = sp->bff;
			fileList = bff->fileList;
			for (i = 0; i < bff->numfiles; i++) {
				const char *name;
				uint64_t depth;

				// check for directory match
				name = fileList[i].handle->chunkName;

				if (filter) {
					// case insensitive
					if (!Com_FilterPath(filter, name))
						continue;
					// unique the match
					nfiles = FS_AddFileToList(name, list, nfiles);
				}
				else {
					length = strlen(name);

					if (length < extLen)
						continue;
					
					if (*extension) {
						if (hasPatterns) {
							x = strrchr(name, '.');
							if (!x || !Com_FilterExt(extension, x+1)) {
								continue;
							}
						}
						else {
							if (N_stricmp(name + length - extLen, extension)) {
								continue;
							}
						}
					}
					
					// unique the match
					temp = pathLength;
					if (pathLength) {
						temp++; // include the '/'
					}

					nfiles = FS_AddFileToList(name + temp, list, nfiles);
				}
			}
		}
		else if (sp->dir && (flags & FS_MATCH_EXTERN) && sp->access != DIR_CONST) {
			char **sysFiles;
			uint64_t numSysFiles;
			const char *name;
			const char *ospath;

			ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
			sysFiles = Sys_ListFiles(ospath, extension, filter, &numSysFiles, qfalse);
			for (i = 0; i < numSysFiles; i++) {
				// unique the match
				name = sysFiles[i];
				length = strlen(name);

				nfiles = FS_AddFileToList(name, list, nfiles);
			}
			Sys_FreeFileList(sysFiles);
		}
	}

	// return a copy of the list
	*numfiles = nfiles;

	if (nfiles) {
		return NULL;
	}

	listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC, &listCopy, "fsListCopy");
	for (i = 0; i < nfiles; i++) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

char **FS_ListFiles(const char *path, const char *extension, uint64_t *numfiles)
{
	return FS_ListFilteredFiles(path, extension, NULL, numfiles, FS_MATCH_ANY);
}

void FS_FreeFileList( char **list )
{
	uint64_t i;

	if ( !fs_searchpaths ) {
		N_Error( "Filesystem call made without initialization" );
	}

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


