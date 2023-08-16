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

constexpr file_t invalid_handle = FS_INVALID_HANDLE;

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
	char bffGamename[MAX_BFF_PATH];

	uint64_t numfiles;
	int64_t index;
	qboolean touched;

	eastl::unordered_map<const char *, fileInBFF_t> hashTable;
	bff_chunk_t *chunkList;
	memoryMap_t *mapping;
	FILE *file;

	uint32_t checksum;
} bffFile_t;

typedef eastl::unordered_map<const char *, fileInBFF_t>::const_iterator const_chunk_iterator;
typedef eastl::unordered_map<const char *, fileInBFF_t>::iterator chunk_iterator;

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

static boost::recursive_mutex fs_lock;
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

static void FS_FixPath(char *path)
{
	char p;
	
	p = path[strlen(path)];
	if (p == PATH_SEP || p == PATH_SEP_FOREIGN)
		p = '\0';
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
	for (file_t i = 0; i < MAX_FILE_HANDLES; i++) {
		if (!handles[i].used || !handles[i].data.stream) {
			return i;
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
	
	if (!game || !*game) {
		game = fs_basegame->s;
	}

	if (npath)
		snprintf(temp, sizeof(temp), "%c%s%c%s", PATH_SEP, game, PATH_SEP, npath);
	else
		snprintf(temp, sizeof(temp), "%c%s", PATH_SEP, game);

	FS_ReplaceSeparators(temp);
	snprintf(ospath[toggle], sizeof(*ospath), "%s%s", base, temp);

	Con_Printf(DEBUG, "FS_BuildOSPath: %s", ospath[toggle]);

	return ospath[toggle];
}

/*
FS_PathCmp: Ignore case and separator char distinctions
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

void FS_VM_FOpenRead(const char *path, file_t *f)
{
	*f = FS_FOpenRead(path);
}

void FS_VM_FOpenWrite(const char *path, file_t *f)
{
	*f = FS_FOpenWrite(path);
}

void FS_VM_FClose(file_t *f)
{
	if (*f <= FS_INVALID_HANDLE || *f >= MAX_FILE_HANDLES)
		return;
	
	FS_FClose(*f);
	*f = FS_INVALID_HANDLE;
}

qboolean FS_Initialized(void)
{
	return fs_initialized;
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	eastl::unordered_map<const char *, fileInBFF_t>::const_iterator file;
	const searchpath_t *sp;

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->bff) {
			file = sp->bff->hashTable.cbegin();
			while (file != sp->bff->hashTable.cend()) {
				if (FS_FilenameCompare(filename, file->first)) {
					// found it!
					return qtrue;
				}
				++file;
			}
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	searchpath_t *sp;
	eastl::unordered_map<const char *, fileInBFF_t>::iterator file;

	// npaths are not supposed to have a leading slash
	while (path[0] == '/' || path[0] == '\\')
		path++;

	// search through the paths, one element at a time
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff) {
			// look through all the bff chunks
			file = sp->bff->hashTable.begin();
			while (file != sp->bff->hashTable.end()) {
				if (FS_FilenameCompare(path, file->first)) {
					// found it!
					*bffIndex = sp->bff->index;
					return &file->second;
				}
				++file;
			}
		}
	}

	return NULL;
}

file_t FS_OpenFileMapping(const char *path, qboolean temp)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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

void FS_SetBFFIndex(uint64_t index)
{
	fs_lastBFFIndex = index;
}

static void FS_OpenChunk(const char *name, fileInBFF_t *chunk, fileHandle_t *f, bffFile_t *bff)
{
	f->bff = bff;
	f->bffIndex = bff->index;
	f->bffFile = qtrue;
	f->data.chunk = chunk;
	f->used = qtrue;
	fs_lastBFFIndex = bff->index;
	N_strncpyz(f->name, name, MAX_BFF_CHUNKNAME);
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
static qboolean fs_cacheSynced;


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

typedef struct
{
	uint32_t nomadVersion;		// the version of the executable
	uint32_t endianess;			// LE: 0x0, BE: 0x1
	uint16_t bffVersion;		// the version of the bff api
} bffCacheHeader_t;

static const bffCacheHeader_t cacheHeader = {
	NOMAD_VERSION_FULL,
#ifdef GDR_LITTLE_ENDIAN
	0x0,
#else
	0x1,
#endif
	BFF_VERSION
};

#define MAX_BFF_HASH 12
static bffFile_t *bffHashTable[MAX_BFF_HASH];


static void FS_FreeBFF(bffFile_t *bff)
{
	if (!bff) {
		N_Error("FS_FreeBFF(NULL)");
	}

	if (bff->file) {
		fclose(bff->file);
		bff->file = NULL;
	}

	Z_Free(bff);
}

#if 0
static qboolean FS_SaveCache(void)
{
	const char *cachename = fs_cacheName->s;
	const char *ospath;
	FILE *fp;

	if (!fs_searchpaths) {
		return qfalse;
	}
	if (!fs_cacheLoaded) {
		Con_Printf("BFF cache synced on initalization");
		fs_cacheSynced = qfalse;
		fs_cacheLoaded = qtrue;
	}


	if (fs_cacheSynced)
		return qtrue;
	
	ospath = FS_BuildOSPath(fs_basepath->s, filename, NULL);
	fp = Sys_FOpen(ospath, "wb");
	if (!fp)
		return qfalse;
	
	FS_WriteCacheHeader(fp);
}

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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	fileInBFF_t *curFile;
	bffFile_t *bff;
	bffheader_t header;
	bff_chunk_t *curChunk;
	uint64_t i, hashSize, size;
	uint64_t hash;
	uint64_t chunkCount;
	const char *basename;
	char gameName[MAX_BFF_PATH], *namePtr;
	uint64_t chunkSize;
	uint64_t gameNameLen;
	uint64_t baseNameLen, fileNameLen;
	uint32_t stream;
	void *streamPtr;
	memoryMap_t *file;
	FILE *fp;
	fileStats_t stats;
	const zone_allocator_notemplate alloc;

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
	if (header.magic != HEADER_MAGIC) {
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
	if (FS_ReadFromBFF(stream, streamPtr, gameName, MAX_BFF_PATH) != MAX_BFF_PATH) {
		FS_CloseBFFStream(fp, file);
		N_Error("FS_LoadBFF: failed to read gameName");
	}

	size = 0;
	size += sizeof(*bff) + sizeof(*bff->chunkList) * header.numChunks;
	size += PAD(fileNameLen, sizeof(uintptr_t));
	size += PAD(baseNameLen, sizeof(uintptr_t));

	bff = (bffFile_t *)Z_Malloc(size, TAG_BFF, &bff, "BFFfile");
	memset(bff, 0, size);

	bff->file = fp;
	bff->mapping = file;
	bff->numfiles = header.numChunks;
	bff->hashTable.clear();
	bff->hashTable = eastl::unordered_map<const char *, fileInBFF_t>((const eastl::allocator &)alloc);
	bff->hashTable.reserve(bff->numfiles);
	bff->chunkList = (bff_chunk_t *)(bff + 1);

	bff->bffFilename = (char *)(bff->chunkList + header.numChunks);
	bff->bffBasename = (char *)(bff->bffFilename + PAD(fileNameLen, sizeof(uintptr_t)));

	memcpy(bff->bffGamename, gameName, sizeof(bff->bffGamename));
	memcpy(bff->bffFilename, bffpath, fileNameLen);
	memcpy(bff->bffBasename, basename, baseNameLen);

	// strip the .bff if needed
	FS_StripExt(bff->bffBasename, ".bff");
	
	curChunk = bff->chunkList;

	constexpr uint64_t readSize = sizeof(*curChunk) - sizeof(char *);
	for (i = 0; i < bff->numfiles; i++) {
		if (FS_ReadFromBFF(stream, streamPtr, curChunk->chunkName, sizeof(curChunk->chunkName)) != sizeof(curChunk->chunkName)) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: failed to read chunk name at %lu", i);
		}
		if (FS_ReadFromBFF(stream, streamPtr, &curChunk->chunkSize, sizeof(curChunk->chunkSize)) != sizeof(curChunk->chunkSize)) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: failed to read chunk size at %lu", i);
		}

		if (!curChunk->chunkSize) {
			FS_CloseBFFStream(fp, file);
			N_Error("FS_LoadBFF: bad chunk size at %lu", i);
		}

		Con_Printf(DEBUG, "chunk stats: (name) %s, (size) %lu", curChunk->chunkName, curChunk->chunkSize);

		bff->hashTable.try_emplace(curChunk->chunkName);
		curFile = &bff->hashTable.at(curChunk->chunkName);
		curFile->bytesRead = 0;
		curFile->next = NULL;
		curFile->handle = curChunk;

		// read the chunk data
		curChunk->chunkBuffer = (char *)Hunk_AllocateTempMemory(curChunk->chunkSize);
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	fileHandle_t* file;
	uint32_t fwhence;
	
	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error("FS_FileSeek: out of range");
	}
	
	file = &handles[f];

	if (!N_stricmp("backtrace.dat", file->name)) {
		return (fileOffset_t)ftell(file->data.fp);
	}

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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	ospath = FS_BuildOSPath(fs_basepath->s, NULL, path);

	fp = Sys_FOpen(ospath, "wb");
	if (!fp) {
		N_Error("FS_FOpenWrite: failed to create write-only stream for %s", path);
	}

	f->used = qtrue;
	f->data.fp = fp;

	return fd;
}

file_t FS_FOpenRW(const char *path)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	file_t fd;
	fileHandle_t *f;
	FILE *fp;
	const char *ospath;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_FOpenRW: NULL or empty path");
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
	ospath = FS_BuildOSPath(fs_basepath->s, NULL, path);

	fp = Sys_FOpen(ospath, "wb+");
	if (!fp) {
		N_Error("FS_FOpenRW: failed to create read/write stream for %s", path);
	}

	f->used = qtrue;
	f->data.fp = fp;

	return fd;
}

file_t FS_FOpenRead(const char *path)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	file_t fd;
	fileHandle_t *f;
	fileInBFF_t *chunk;
	int64_t unused;
	searchpath_t *sp;
	qboolean inBFF;
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
	inBFF = FS_FileIsInBFF(path);

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff && inBFF) {
			// look through all the chunks
			chunk = FS_GetChunkHandle(path, &unused);
			if (chunk) {
				// found it!
				FS_OpenChunk(path, chunk, f, sp->bff);
				return fd;
			}
		}
		else if (sp->dir && sp->access != DIR_CONST) {
			ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
			Con_Printf(DEBUG, "FS_FOpenRead: %s", ospath);
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	fileHandle_t *f;
	const char *ospath;
	FILE *fp;
	searchpath_t *sp;
	fileInBFF_t *chunk;
	int64_t unused;

	if (!fs_initialized) {
		N_Error("Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error("FS_FOpenFileRead: NULL or empty path");
	}

	// we just want the file's size
	if (fd == NULL) {
		for (sp = fs_searchpaths; sp; sp = sp->next) {
			// is the element a bff file?
			if (sp->bff) {
				// look through all the chunks
				chunk = FS_GetChunkHandle(path, &unused);
				if (chunk) {
					// found it!
					return chunk->handle->chunkSize;
				}
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
			if (sp->bff) {
				// look through all the chunks
				chunk = FS_GetChunkHandle(path, &unused);
				if (chunk) {
					// found it!
					FS_OpenChunk(path, chunk, f, sp->bff);
					return chunk->handle->chunkSize;
				}
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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

int FS_FileToFileno(file_t f)
{
	return fileno(handles[f].data.fp);
}

void FS_FreeFile(void *buffer)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
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
	searchpath_t *search;
	bffFile_t *bff;
	char curpath[MAX_OSPATH*2+1];
	char **bffList;
	const char *bffpath;
	uint64_t numBFFs, i;
	uint64_t path_len, dir_len, size;

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->dir && sp->dir->path && sp->dir->gamedir && !N_stricmp(sp->dir->path, path) && !N_stricmp(sp->dir->gamedir, dir)) {
			return; // we've already got this one
		}
	}

	path_len = PAD(strlen(path) + 1, sizeof(uintptr_t));
	dir_len = PAD(strlen(dir) + 1, sizeof(uintptr_t));
	size = sizeof(*search) + sizeof(*search->dir) + path_len + dir_len;

	search = (searchpath_t *)Z_Malloc(size, TAG_SEARCH_PATH, &search, "searchpath");
	memset(search, 0, size);
	search->dir = (directory_t *)(search + 1);
	search->dir->path = (char *)(search->dir + 1);
	search->dir->gamedir = (char *)(search->dir->path + path_len);
	search->access = DIR_STATIC;

	strcpy(search->dir->path, path);
	strcpy(search->dir->gamedir, dir);

	search->next = fs_searchpaths;
	fs_searchpaths = search;

	N_strncpyz(curpath, FS_BuildOSPath(path, dir, NULL), sizeof(curpath));
	bffList = Sys_ListFiles(curpath, ".bff", NULL, &numBFFs, qfalse);

	for (i = 0; i < numBFFs; i++) {
		size = strlen(bffList[i]) + 1;

		bffpath = FS_BuildOSPath(path, dir, bffList[i]);
		if ((bff = FS_LoadBFF(bffpath)) == NULL) {
			// this isn't a .bff! NeXT!
			continue;
		}

		bff->index = fs_bffCount;

		search = (searchpath_t *)Z_Malloc(sizeof(*search), TAG_SEARCH_PATH, &search, "searchpath");
		memset(search, 0, sizeof(*search));
		search->bff = bff;
		search->access = DIR_STATIC;

		search->next = fs_searchpaths;
		fs_searchpaths = search;

		fs_bffChunks += bff->numfiles;
		fs_bffCount++;
	}
	
	Sys_FreeFileList(bffList);
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

static void FS_ThePurge_f(void)
{

}

static void FS_ListBFF_f(void)
{
	
}

static void FS_ShowDir_f(void)
{

}

static void FS_AddMod_f(void)
{
	
}

static void FS_Touch_f(void)
{

}

void FS_Restart(void)
{
	Cmd_RemoveCommand("dir");
	Cmd_RemoveCommand("ls");
	Cmd_RemoveCommand("list");
	Cmd_RemoveCommand("addmod");
	Cmd_RemoveCommand("touch");
	Cmd_RemoveCommand("purge");
}

void FS_Shutdown(void)
{
	for (uint64_t i = 0; i < MAX_FILE_HANDLES; i++) {
		if (!handles[i].bff && handles[i].data.fp)
			fclose(handles[i].data.fp);
	}
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

	fs_basepath = Cvar_Get("fs_basepath", Sys_DefaultBasePath(), CVAR_PRIVATE | CVAR_ROM);
	Cvar_SetDescription(fs_basepath, "Write-protected CVar specifying the path to the installation folder of the game.");
	fs_basegame = Cvar_Get("fs_basegame", BASEGAME_DIR, CVAR_PRIVATE);
	Cvar_SetDescription(fs_basegame, "Cvar specifying the path to the base game folder.");
//	fs_steampath = Cvar_Get("fs_steampath", Sys_GetSteamPath(), CVAR_PRIVATE | CVAR_ROM);

	if (!fs_basegame->s[0])
		N_Error("* fs_basegame not set *");

#if 0	
	homepath = Sys_DefaultHomePath();
	if (!homepath || !homepath[0]) {
		homepath = fs_basepath->s;
	}

	fs_homepath = Cvar_Get("fs_homepath", homepath, CVAR_PRIVATE | CVAR_ROM);
#endif

	fs_gamedirvar = Cvar_Get("fs_gamedir", "", CVAR_PRIVATE);
	Cvar_SetDescription(fs_gamedirvar, "Specify an alternate mod directory and run the game with this mod.");

	if (!N_stricmp(fs_basegame->s, fs_gamedirvar->s)) {
		Cvar_Set("fs_gamedir", fs_basegame->s);
	}

	start = Sys_Milliseconds();

	// add search path elements in reverse priority order
//	if (fs_steampath->s[0]) {
//		FS_AddGameDirectory(fs_steampath->s, fs_basegame->s);
//	}

	if (fs_basepath->s[0]) {
		FS_AddGameDirectory(fs_basepath->s, fs_basegame->s);
	}

#if 0
	// fs_homepath is somewhat particular to *nix systems, only add if relevant
	if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basegame->s)) {
		FS_AddGameDirectory(fs_homepath->s, fs_basegame->s);
	}
#endif

	// check for additional game folder for mods
	if (fs_gamedirvar->s[0] && N_stricmp(fs_gamedirvar->s, fs_basegame->s)) {
//		if (fs_steampath->s[0]) {
//			FS_AddGameDirectory(fs_steampath->s, fs_gamedirvar->s);
//		}
		if (fs_basepath->s[0]) {
			FS_AddGameDirectory(fs_basepath->s, fs_gamedirvar->s);
		}
#if 0
		if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basepath->s)) {
			FS_AddGameDirectory(fs_homepath->s, fs_gamedirvar->s);
		}
#endif
	}
	fs_initialized = qtrue;

	end = Sys_Milliseconds();

	Con_Printf(
		"fs_gamedir: %s\n"
		"fs_basepath: %s\n"
		"fs_basegame: %s",
	fs_gamedirvar->s, fs_basepath->s, fs_basegame->s);

	Con_Printf( "...loaded in %lu milliseconds", end - start );

	Con_Printf( "----------------------" );
	Con_Printf( "%lu chunks in %lu bff files\n", fs_bffChunks, fs_bffCount );

	for (uint64_t i = 0; i < MAX_FILE_HANDLES; i++) {
		FS_InitHandle(&handles[i]);
	}
	
//	Cmd_AddCommand("dir", FS_ShowDir_f);
//	Cmd_AddCommand("ls", FS_ShowDir_f);
//	Cmd_AddCommand("list", FS_ListBFF_f);
//	Cmd_AddCommand("addmod", FS_AddMod_f);
//	Cmd_AddCommand("touch", FS_Touch_f);
//	Cmd_AddCommand("purge", FS_ThePurge_f);
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
	bff_chunk_t *file;
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
			file = bff->chunkList;
			for (i = 0; i < bff->numfiles; i++) {
				const char *name;
				uint64_t depth;

				// check for directory match
				name = file->chunkName;

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
				file++;
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

char *FS_ReadLine(char *buf, uint64_t size, file_t f)
{
	return fgets(buf, size, handles[f].data.fp);
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


