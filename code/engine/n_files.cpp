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
	char *name;
	char *buf;
	uint64_t nameLen;
	uint64_t size;
    uint64_t bytesRead;
	struct fileInBFF_s *next;
} fileInBFF_t;

typedef struct bffFile_s
{
	char *bffBasename;
	char *bffFilename;
	char bffGamename[MAX_BFF_PATH];

	uint64_t numfiles;
	uint64_t hashSize;
	int64_t index;

	fileInBFF_t **hashTable;
	fileInBFF_t *buildBuffer;

	struct bffFile_s *next;
	struct bffFile_s *prev;

	fileTime_t mtime;
	fileTime_t ctime;
	uint64_t size;

	uint32_t checksum;
	uint32_t nameHash;
	qboolean touched;
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
	handleOwner_t owner;
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
#ifdef USE_BFF_CACHE_FILE
static cvar_t		*fs_locked;
#endif
static cvar_t		*fs_restrict;
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
		N_Error(ERR_DROP, "FS_FileForHandle: out of range");
	}
	if (!handles[f].used) {
		N_Error(ERR_DROP, "FS_FileForHandle: invalid handle");
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
	N_Error(ERR_DROP, "FS_HandleForFile: not enough handles");
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

static const char *FS_OwnerName(handleOwner_t owner)
{
	switch (owner) {
	case H_SGAME: return "SGAME";
	case H_SCRIPT: return "SCRIPT";
	case H_UI: return "UI";
	};
	return "UNKOWN";
}

file_t FS_VM_FOpenRead(const char *path, file_t *f, handleOwner_t owner)
{
	int32_t r;

	r = FS_FOpenRead(path);
	if (f && r != FS_INVALID_HANDLE) {
		handles[*f].owner = owner;
		*f = r;
	}
	return r;
}

file_t FS_VM_FOpenWrite(const char *path, file_t *f, handleOwner_t owner)
{
	int32_t r;
	
	r = FS_FOpenWrite(path);
	if (f && r != FS_INVALID_HANDLE) {
		handles[*f].owner = owner;
		*f = r;
	}
	return r;
}

uint32_t FS_VM_Read(void *buffer, uint32_t len, file_t f, handleOwner_t owner)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES)
		return 0;
	
	if (handles[f].owner != owner || !handles[f].data.fp)
		return 0;
	
	return (uint32_t)FS_Read(buffer, len, f);
}

uint32_t FS_VM_Write(const void *buffer, uint32_t len, file_t f, handleOwner_t owner)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES)
		return 0;
	
	if (handles[f].owner != owner || !handles[f].data.fp)
		return 0;
	
	return (uint32_t)FS_Write(buffer, len, f);
}

void FS_VM_WriteFile(const void *buffer, uint32_t len, file_t f, handleOwner_t owner)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES)
		return;
	
	if (handles[f].owner != owner || !handles[f].data.fp)
		return;
	
	FS_Write(buffer, len, f);
}

void FS_VM_CreateTmp(char *name, const char *ext, file_t *f, handleOwner_t owner);
uint64_t FS_VM_FOpenFileRead(const char *path, file_t *f, handleOwner_t owner);
uint64_t FS_VM_FOpenFileWrite(const char *path, file_t *f, handleOwner_t owner);

fileOffset_t FS_VM_FileSeek(file_t f, fileOffset_t offset, uint32_t whence, handleOwner_t owner)
{
	fileOffset_t r;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES)
		return -1;
	
	if (handles[f].owner != owner || !handles[f].data.fp)
		return -1;
	
	r = FS_FileSeek(f, offset, whence);
	return r;
}

void FS_VM_CloseFiles(handleOwner_t owner)
{
	for (uint32_t i = 0; i < MAX_FILE_HANDLES; i++) {
		if (handles[i].owner != owner)
			continue;
		
		Con_Printf(COLOR_YELLOW "%s:%i:%s leaked filehandle\n", FS_OwnerName(owner), i, handles[i].name);
		FS_FClose(i);
	}
}

void FS_VM_FClose(file_t f)
{
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES)
		return;
	
	FS_FClose(f);
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
		N_Error( ERR_FATAL, "%s: Not allowed to manipulate '%s' due "
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
	const fileInBFF_t *file;
	const searchpath_t *sp;
	const bffFile_t *bff;
	uint64_t hash;
	uint64_t fullHash;

	if (!fs_searchpaths) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!filename) {
		N_Error(ERR_FATAL, "FS_FileIsInBFF: NULL filename");
	}

	// npaths are not supposed to have a leading slash
	while (filename[0] == '/' || filename[0] == '\\')
		filename++;
	
	if (FS_CheckDirTraversal(filename)) {
		return qfalse;
	}

	fullHash = FS_HashFileName(filename, 0U);

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
			bff = sp->bff;
			file = sp->bff->hashTable[hash];
			do {
				if (!FS_FilenameCompare(file->name, filename)) {
					return qtrue;
				}
				file = file->next;
			} while (file != NULL);
		}
	}
	return qfalse;
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
	fileInBFF_t *file;
	searchpath_t *sp;
	bffFile_t *bff;
	uint64_t hash;
	uint64_t fullHash;

	if (!fs_searchpaths) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path) {
		N_Error(ERR_FATAL, "FS_GetChunkHandle: NULL filename");
	}

	// npaths are not supposed to have a leading slash
	while (path[0] == '/' || path[0] == '\\')
		path++;
	
	if (FS_CheckDirTraversal(path)) {
		return qfalse;
	}

	fullHash = FS_HashFileName(path, 0U);

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize - 1))]) {
			bff = sp->bff;
			file = sp->bff->hashTable[hash];
			do {
				if (!FS_FilenameCompare(file->name, path)) {
					// found it!
					*bffIndex = bff->index;
					return file;
				}
				file = file->next;
			} while (file != NULL);
		}
	}
	return NULL;
}

uint64_t FS_FOpenFileWithMode(const char *npath, file_t *f, fileMode_t mode)
{
	uint64_t len;

	if (!npath || !f) {
		N_Error(ERR_FATAL, "FS_FOpenFileWithMode: NULL parameter");
	}

	switch (mode) {
	case FS_OPEN_WRITE:
	{
		*f = FS_FOpenWrite(npath);
		if (*f == FS_INVALID_HANDLE) {
			N_Error(ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in write mode");
		}
		len = 0;
		break;
	}
	case FS_OPEN_READ:
	{
		len = FS_FOpenFileRead(npath, f);
		if (!len || *f == FS_INVALID_HANDLE) {
			Con_DPrintf("FS_FOpenFileWithMode: !len || *f == FS_INVALID_HANDLE (r), failure\n");
			len = 0;
		}
		break;
	}
	case FS_OPEN_APPEND:
	{
		*f = FS_FOpenAppend(npath);
		if (*f == FS_INVALID_HANDLE) {
			N_Error(ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in append mode");
		}
		len = FS_FileLength(*f);
		break;
	}
	case FS_OPEN_RW:
	{
		*f = FS_FOpenRW(npath);
		if (*f == FS_INVALID_HANDLE) {
			N_Error(ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in read-write mode");
		}
		len = FS_FileLength(*f);
		break;
	}
	default:
		N_Error(ERR_DROP, "FS_FOpenFileWithMode: bad mode (%i)", mode);
		break;
	};
	return len;
}

file_t FS_FOpenAppend(const char *path)
{
	file_t f;
	FS_FOpenFileWithMode(path, &f, FS_OPEN_APPEND);
	return f;
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_OpenFileMapping: NULL or empty path");
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
		N_strncpyz(f->name, f->data.chunk->name, MAX_GDR_PATH);
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
	fileInBFF_t *file;
	qboolean hasPatterns;
	const char *x;
	const searchpath_t *sp;

	if (!fs_initialized) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
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
			file = bff->buildBuffer;
			for (i = 0; i < bff->numfiles; i++) {
				const char *name;
				uint64_t depth;

				// check for directory match
				name = file->name;

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

	listCopy = (char **)Z_Malloc((nfiles + 1) * sizeof(*listCopy), TAG_STATIC);
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
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

/*
=================
FS_ConvertFilename

lower case and replace '\\' ':' with '/'
=================
*/
static void FS_ConvertFilename( char *name )
{
	int c;
	while ( (c = *name) != '\0' ) {
		if ( c <= 'Z' && c >= 'A' ) {
			*name = c - 'A' + 'a';
		} else if ( c == '\\' || c == ':' ) {
			*name = '/';
		}
		name++;
	}
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

static uint64_t fs_bffsReaded;
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

#ifdef USE_BFF_CACHE_FILE

#define CACHE_FILE_NAME "bffcache.dat"

#pragma pack(push, 1)

typedef struct {
	uint64_t bffNameLen;
	uint64_t namesLen;
	uint64_t numFiles;
	uint64_t contentLen;
	fileTime_t ctime;	// creation/status change time
	fileTime_t mtime;	// modification time
	fileOffset_t size;	// bff file size
} bffCacheHeader_t;

typedef struct {
	uint64_t name; // offset in the name buffer
	uint64_t size;
	uint64_t pos;
} bffCacheFileItem_t;

// platform-specific 4-byte signature:
// 0: [version] anything following depends from it
// 1: [endianess] 0 - LSB, 1 - MSB
// 2: [path separation] '/' or '\\'
// 3: [size of file offset and file time]
// non-matching header will cause whole file being ignored
static const byte cache_header[ 4 ] = {
	0, //version
#ifdef Q3_LITTLE_ENDIAN
	0x0,
#else
	0x1,
#endif
	PATH_SEP,
	( ( sizeof( fileOffset_t ) - 1 ) << 4 ) | ( sizeof( fileTime_t ) - 1 )
};

#pragma pack(pop)

#endif // USE_BFF_CACHE_FILE

#ifdef USE_BFF_CACHE_FILE
#define MAX_BFF_HASH 64
static bffFile_t *bffHashTable[MAX_BFF_HASH];

static uint32_t FS_HashBFF( const char *name )
{
	uint32_t c, hash = 0;
	while ( (c = *name++) != '\0' ) {
		hash = hash * 101 + c;
	}
	hash = hash ^ (hash >> 16);
	return hash & (MAX_BFF_HASH-1);
}

uint64_t FS_BFFHashSize(uint32_t fileCount)
{
	uint64_t hashSize;

	for (hashSize = 2; hashSize < MAX_FILEHASH_SIZE; hashSize <<= 1) {
		if (hashSize >= fileCount) {
			break;
		}
	}

	return hashSize;
}

static void FS_FreeBFF(bffFile_t *bff)
{
	if (!bff) {
		N_Error(ERR_FATAL, "FS_FreeBFF(NULL)");
	}

	Z_Free(bff);
}

static bffFile_t *FS_FindInCache(const char *bffFile)
{
	bffFile_t *bff;
	uint64_t hash;

	hash = FS_HashBFF(bffFile);
	bff = bffHashTable[hash];
	while (bff) {
		if (!strcmp(bffFile, bff->bffGamename)) {
			return bff;
		}
		bff = bff->next;
	}
	return NULL;
}

static void FS_RemoveFromCache(bffFile_t *bff)
{
	if (!bff->next && !bff->prev && bffHashTable[bff->nameHash] != bff) {
		N_Error(ERR_FATAL, "Invalid BFF Link");
	}

	if (bff->prev)
		bff->prev->next = bff->next;
	else
		bffHashTable[bff->nameHash] = bff->next;
	
	if (bff->next)
		bff->next->prev = bff->prev;
}

static void FS_AddToCache(bffFile_t *bff)
{
	bff->nameHash = FS_HashBFF(bff->bffGamename);
	bff->next = bffHashTable[bff->nameHash];
	bff->prev = NULL;
	if (bffHashTable[bff->nameHash])
		bffHashTable[bff->nameHash]->prev = bff;
	
	bffHashTable[bff->nameHash] = bff;
}

static bffFile_t *FS_LoadCachedBFF(const char *bffpath)
{
	fileStats_t stats;
	bffFile_t *bff;

	bff = FS_FindInCache(bffpath);
	if (!bff)
		return NULL;
	
	if (!Sys_GetFileStats(&stats, bffpath)) {
		FS_RemoveFromCache(bff);
		FS_FreeBFF(bff);
		return NULL;
	}

	if (bff->size != stats.size || bff->mtime != stats.mtime || bff->ctime != stats.ctime) {
		// release outdated information
		FS_RemoveFromCache(bff);
		FS_FreeBFF(bff);
		return NULL;
	}

	return bff;
}

static void FS_InsertBFFToCache(bffFile_t *bff)
{
	fileStats_t stats;
	if (Sys_GetFileStats(&stats, bff->bffFilename)) {
		bff->mtime = stats.mtime;
		bff->ctime = stats.ctime;
		FS_AddToCache(bff);
		bff->touched = qtrue;
	}
}

static void FS_ResetCacheReferences(void)
{
	bffFile_t *bff;

	for (uint64_t i = 0; i < arraylen(bffHashTable); i++) {
		bff = bffHashTable[i];
		while (bff) {
			bff->touched = qfalse;
			bff = bff->next;
		}
	}
}

static qboolean FS_SaveBFFToFile(const bffFile_t *bff, FILE *fp)
{
	const char *namePtr;
	const char *bffName;
	uint64_t i, bffNameLen;
	bffCacheHeader_t file;
	bffCacheFileItem_t item;
	uint64_t namesLen, contentLen;

	namePtr = (const char *)(bff->buildBuffer + bff->numfiles);

	bffName = bff->bffFilename;
	bffNameLen = strlen(bffName) + 1;
	bffNameLen = PAD(bffNameLen, sizeof(uintptr_t));

	namesLen = bffName - namePtr;

	// file content length
	contentLen = 0;

	// bff filename length
	file.bffNameLen = bffNameLen;
	// filenames length
	file.namesLen = namesLen;
	// number of files
	file.numFiles = bff->numfiles;
	// content of some files
	file.contentLen = contentLen;
	// creation/status change time
	file.ctime = bff->ctime;
	// modification time
	file.mtime = bff->mtime;
	// bff file size
	file.size = bff->size;

	// dump header
	fwrite(&file, sizeof(file), 1, fp);

	// bff filename
	fwrite(bffName, bffNameLen, 1, fp);

	// filenames
	fwrite(namePtr, namesLen, 1, fp);

	// file entries
	for (i = 0; i < bff->numfiles; i++) {
		item.name = (uint64_t)(bff->buildBuffer[i].name - namePtr);
		item.size = bff->buildBuffer[i].size;
		item.pos = 0;
		fwrite(&item, sizeof(item), 1, fp);
	}

	return qtrue;
}

static void FS_WriteCacheHeader(FILE *fp)
{
	fwrite(cache_header, sizeof(cache_header), 1, fp);
}

static qboolean FS_ValidateCacheHeader(FILE *fp)
{
	byte buf[sizeof(cache_header)];

	if (fread(buf, sizeof(buf), 1, fp) != 1)
		return qfalse;
	
	if (memcmp(buf, cache_header, sizeof(buf)) != 0)
		return qfalse;
	
	return qtrue;
}

static void FS_FreeUnusedCache( void )
{
	bffFile_t *next, *bff;
	uint64_t i;

	for ( i = 0; i < arraylen( bffHashTable ); i++ ) {
		bff = bffHashTable[ i ];
		while ( bff ) {
			next = bff->next;
			if ( !bff->touched ) {
				FS_RemoveFromCache( bff );
				FS_FreeBFF( bff );
				fs_bffsReleased++;
			}
			bff = next;
		}
	}
}

static qboolean FS_LoadBffFromFile(FILE *fp)
{
	fileTime_t ctime, mtime;
	fileOffset_t fsize;
	fileInBFF_t *curFile;
	char bffName[PAD(MAX_OSPATH*3+1, sizeof(uintptr_t))];
	char bffBase[PAD(MAX_OSPATH, sizeof(uintptr_t))], *basename;
	char *name;
	bffCacheHeader_t header;
	bffCacheFileItem_t file;
	fileStats_t stats;
	bffFile_t *bff;
	char *namePtr;
	uint64_t size;
	uint64_t hashSize, bffBaseLen;
	uint64_t hash;

	if (fread(&header, sizeof(header), 1, fp) != 1)
		return qfalse; // probably EOF
	
	// validate header data

	if (header.bffNameLen > sizeof(bffName) || header.bffNameLen & 3 || header.bffNameLen == 0)
		return qfalse;
	
	if (header.namesLen & 3 || header.namesLen < header.numFiles)
		return qfalse;
	
	if (header.contentLen & 3 || header.contentLen < 0)
		return qfalse;
	
	// load filename
	if (fread(bffName, header.bffNameLen, 1, fp) != 1)
		return qfalse;
	
	// bffName must be zero-terminated
	if (bffName[header.bffNameLen - 1] != '\0')
		return qfalse;
	

	if (!Sys_GetFileStats(&stats, bffName) || stats.size != header.size || stats.mtime != header.mtime || stats.ctime != header.ctime) {
		const uintptr_t seek_len = header.namesLen * sizeof(file) + header.contentLen;
		if (fseek(fp, seek_len, SEEK_CUR) != 0) {
			return qfalse;
		}
		else {
			fs_bffsSkipped++;
			return qtrue; // just outdated info, we can continue
		}
	}

	// extract basename from bffpath
	basename = strrchr(bffName, PATH_SEP);
	if (basename == NULL)
		basename = bffName;
	else
		basename++;
	
	N_strncpyz(bffBase, basename, sizeof(bffBase));
	FS_StripExt(bffBase, ".bff");
	bffBaseLen = strlen(bffBase) + 1;
	bffBaseLen = PAD(bffBaseLen, sizeof(uintptr_t));
	
	hashSize = FS_BFFHashSize(header.numFiles);

	size = sizeof(*bff) + header.namesLen + header.numFiles * sizeof(bff->buildBuffer[0]);
	size += hashSize * sizeof(bff->hashTable[0]);
	size += header.namesLen;
	size += bffBaseLen;

	bff = (bffFile_t *)Z_Malloc(size, TAG_BFF);
	memset(bff, 0, size);

	bff->mtime = header.mtime;
	bff->ctime = header.ctime;
	bff->size = header.size;

	bff->numfiles = header.numFiles;
	
	// setup memory layout
	bff->hashSize = hashSize;
	bff->hashTable = (fileInBFF_t **)(bff + 1);
	bff->buildBuffer = (fileInBFF_t *)(bff->hashTable + bff->hashSize);

	namePtr = (char *)(bff->buildBuffer + bff->numfiles);

	bff->bffFilename = (char *)(namePtr + header.namesLen);
	bff->bffBasename = (char *)(bff->bffFilename + header.bffNameLen);
	
	strcpy(bff->bffFilename, bffName);
	strcpy(bff->bffBasename, bffBase);

	if (fread(namePtr, header.namesLen, 1, fp) != 1) {
		goto __error;
	}
	// filenames buffer must be zero-terminated
	if (namePtr[header.namesLen - 1] != '\0') {
		Con_DPrintf("cached namePtr in '%s' isn't null terminated\n", bffName);
		goto __error;
	}

	curFile = bff->buildBuffer;
	for (uint64_t i = 0; i < header.numFiles; i++) {
		if (fread(&file, sizeof(file), 1, fp) != 1) {
			goto __error;
		}
		if (file.name >= header.namesLen) {
			goto __error;
		}

		name = namePtr + file.name;
		FS_ConvertFilename(name);

		curFile->name = name;
		curFile->size = file.size;

		// update hash table
		hash = FS_HashFileName(name, bff->hashSize);
		curFile->next = bff->hashTable[hash];
		bff->hashTable[hash] = curFile;
		curFile++;
	}

	bff->checksum = Com_BlockChecksum(bff, size);
	bff->checksum = LittleInt(bff->checksum);

	// seek through unused content
	if (header.contentLen > 0) {
		if (fseek(fp, header.contentLen, SEEK_CUR) != 0) {
			goto __error;
		}
	}
	else if (header.contentLen < 0) {
		goto __error;
	}

	fs_bffsCached++;

	FS_InsertBFFToCache(bff);

	return qtrue;

__error:
	Con_DPrintf(COLOR_YELLOW "WARNING: failed to load bff from file\n");
	FS_FreeBFF(bff);
	return qfalse;
}

static qboolean FS_SaveCache(void)
{
	const char *filename = CACHE_FILE_NAME;
	const char *ospath;
	const searchpath_t *sp;
	FILE *fp;

	if (!fs_searchpaths) {
		return qfalse;
	}
	if (!fs_cacheLoaded) {
		Con_DPrintf("synced FS cache on startup\n");
		fs_cacheSynced = qfalse;
		fs_cacheLoaded = qtrue;
	}
	else {
		Con_DPrintf("synced FS cache on readed=%lu, released=%lu, skipped=%lu\n",
			fs_bffsReaded, fs_bffsReleased, fs_bffsSkipped);
		fs_cacheSynced = qfalse;
	}

	if (fs_cacheSynced)
		return qtrue;
	
	sp = fs_searchpaths;

	ospath = FS_BuildOSPath(fs_basepath->s, filename, NULL);
	fp = Sys_FOpen(ospath, "wb");
	if (!fp)
		return qfalse;
	
	FS_WriteCacheHeader(fp);

	while (sp != NULL) {
		if (sp->bff) {
			FS_SaveBFFToFile(sp->bff, fp);
		}
		sp = sp->next;
	}

	fclose(fp);

	fs_bffsReaded = 0;
	fs_bffsReleased = 0;
	fs_bffsSkipped = 0;

	fs_cacheSynced = qtrue;

	return qtrue;
}

static void FS_LoadCache(void)
{
	const char *filename = CACHE_FILE_NAME;
	const char *ospath;
	FILE *fp;
	
	Con_Printf("Loading cached bffs...\n");

	fs_bffsReaded = 0;
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

	while (FS_LoadBffFromFile(fp))
		;
	
	fclose(fp);

	fs_cacheLoaded = qtrue;
	
	Con_Printf("...found %lu cached bffs\n", fs_bffsCached);
}
#endif // USE_BFF_CACHE_FILE

/*
FS_LoadBFF: creates a new bffFile_t in the search chain for the contents of a bff archive file
*/
static bffFile_t *FS_LoadBFF(const char *bffpath)
{
	bffFile_t *bff;
	const char *basename;
	char *namePtr;
	uint64_t hash;
	char name[MAX_OSPATH*2+1];
	uint64_t nameLen, nameSize, hashSize, size;
	uint64_t filecount;
	fileInBFF_t *curFile;
	uint64_t fileNameLen;
	uint64_t baseNameLen;
	bffheader_t header;
	FILE *fp;

#ifdef USE_BFF_CACHE_FILE
	bff = FS_LoadCachedBFF(bffpath);
	if (bff) {
		bff->touched = qtrue;
		return bff; // loaded from cache
	}
#endif

	// extract basename from the ospath
	basename = strrchr(bffpath, PATH_SEP);
	if (basename == NULL) 
		basename = bffpath;
	else
		basename++;
	
	fileNameLen = strlen(bffpath) + 1;
	baseNameLen = strlen(basename) + 1;
	fp = Sys_FOpen(bffpath, "rb");
	if (!fp) {
		return NULL;
	}

	nameLen = 0;
	filecount = 0;

	if (fread(&header, sizeof(header), 1, fp) != sizeof(header)) {
		Con_DPrintf("Failed to read bff header\n");
		fclose(fp);
		return NULL;
	}

	if (header.ident != BFF_IDENT) {
		Con_DPrintf("bff header ident is incorrect\n");
		fclose(fp);
		return NULL;
	}
	if (header.magic != HEADER_MAGIC) {
		Con_DPrintf("bff header magic is incorrect\n");
		fclose(fp);
		return NULL;
	}
	if (!header.numChunks) {
		Con_DPrintf("bff header numChunks funny number\n");
		fclose(fp);
		return NULL;
	}

	for (uint64_t i = 0; i < header.numChunks; i++) {
		uint64_t size, pos;
		if (fread(&nameSize, sizeof(uint64_t), 1, fp) != sizeof(uint64_t)) {
			Con_DPrintf("Error reading nameLen in chunk %lu\n", i);
			fclose(fp);
			return NULL;
		}
		if (fread(name, nameSize, 1, fp) != nameSize) {
			Con_DPrintf("Error reading name in chunk %lu\n", i);
			fclose(fp);
			return NULL;
		}
		if (fread(&size, sizeof(uint64_t), 1, fp) != sizeof(uint64_t)) {
			Con_DPrintf("Error reading size in chunk %lu\n", i);
			fclose(fp);
			return NULL;
		}

		pos = ftell(fp);
		fseek(fp, pos + size, SEEK_SET);
		nameLen += strlen(name) + 1;
		filecount++;
	}

	if (!filecount) {
		return NULL;
	}

	hashSize = FS_BFFHashSize(filecount);

	nameLen = PAD(nameLen, sizeof(uintptr_t));
	size = sizeof(*bff) + hashSize + sizeof(bff->hashTable[0]) + filecount * sizeof(bff->buildBuffer[0]) + nameLen;
	size += PAD(fileNameLen, sizeof(uintptr_t));
	size += PAD(baseNameLen, sizeof(uintptr_t));
#ifdef USE_BFF_CACHE_FILE
	size += filecount + 1;
#endif

	bff = (bffFile_t *)Z_Malloc(size, TAG_BFF);
	memset(bff, 0, size);

	bff->numfiles = filecount;
	bff->hashSize = hashSize;
	bff->hashTable = (fileInBFF_t **)(bff + 1);

	bff->buildBuffer = (fileInBFF_t *)(bff->hashTable + bff->hashSize);
	namePtr = (char *)(bff->buildBuffer + filecount);

	bff->bffFilename = (char *)(namePtr + nameLen);
	bff->bffBasename = (char *)(bff->bffFilename + PAD(fileNameLen, sizeof(uintptr_t)));

	memcpy(bff->bffFilename, bffpath, fileNameLen);
	memcpy(bff->bffBasename, basename, baseNameLen);

	// strip .bff if needed
	FS_StripExt(bff->bffBasename, ".bff");

	fseek(fp, sizeof(header), SEEK_SET);
	curFile = bff->buildBuffer;
	for (uint64_t i = 0; i < header.numChunks; i++) {
		if (fread(&curFile->nameLen, sizeof(uint64_t), 1, fp) != sizeof(uint64_t)) {
			Con_DPrintf("Error reading chunk nameLen\n");
			fclose(fp);
			return NULL;
		}
		if (fread(name, curFile->nameLen, 1, fp) != curFile->nameLen) {
			Con_DPrintf("Error reading chunk name\n");
			fclose(fp);
			return NULL;
		}

		curFile->name = namePtr;
		strcpy(curFile->name, name);
		namePtr += strlen(name) + 1;

		if (fread(&curFile->size, sizeof(uint64_t), 1, fp) != sizeof(uint64_t)) {
			Con_DPrintf("Error reading chunk size\n");
			fclose(fp);
			return NULL;
		}

		curFile->buf = (char *)Hunk_AllocateTempMemory(curFile->size);
		if (fread(curFile->buf, curFile->size, 1, fp) != curFile->size) {
			Con_DPrintf("Error reading chunk buffer\n");
			fclose(fp);
			return NULL;
		}

		// update hash table
		hash = FS_HashFileName(name, bff->hashSize);
		curFile->next = bff->hashTable[hash];
		bff->hashTable[hash] = curFile;
		curFile++;
	}

	bff->checksum = Com_BlockChecksum(bff, size);
	bff->checksum = LittleInt(bff->checksum);

	return bff;
}

//
// FS_WriteFile: filename is relative to glnomad's search paths
//
void FS_WriteFile(const char *npath, const void *data, uint64_t size)
{
	file_t  f;

	if (!npath || !data) {
		N_Error(ERR_FATAL, "FS_WriteFile: NULL parameter");
	}
	Con_DPrintf("FS_WriteFile(%s, %lx, %lu)\n", npath, (uintptr_t)data, size);

	f = FS_FOpenWrite(npath);
	if (f == FS_INVALID_HANDLE) {
		Con_Printf(COLOR_RED "Failed to open %s\n", npath);
		return;
	}
	
	Con_DPrintf("Writing %lu bytes...\n", size);
	FS_Write(data, size, f);
	Con_DPrintf("Success.\n");

	FS_FClose(f);
}

void FS_ForceFlush(file_t f)
{
	fileHandle_t *fh;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_ForceFlush: out of range");
	}

	fh = &handles[f];
	if (fh->data.fp) {
//		setvbuf(fh->data.fp, NULL, _IONBF, 0);
		fflush(fh->data.fp); // better to fflush?
	}
	else {
		Con_DPrintf("FS_ForceFlush: not open\n");
	}
}

fileOffset_t FS_FileTell(file_t f)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	fileHandle_t *p;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_FileTell: out of range");
	}

	p = &handles[f];

	// not a unique file
	if (p->bffFile)
		return (fileOffset_t)(p->data.chunk->size - p->data.chunk->bytesRead);
	
	// normal file pointer
	return (fileOffset_t)ftell(p->data.fp);
}

fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	fileHandle_t* file;
	uint32_t fwhence;
	
	if (!fs_initialized) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_FileSeek: out of range");
	}
	
	file = &handles[f];

	// dirty fix for a bug
	if (!N_stricmp("backtrace.dat", file->name)) {
		return (fileOffset_t)ftell(file->data.fp);
	}

	if (FS_FileIsInBFF(file->name)) {
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
		default:
			N_Error(ERR_FATAL, "FS_FileSeek: invalid seek");
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
		N_Error(ERR_FATAL, "FS_FileSeek: invalid seek");
	};
	return (fileOffset_t)fseek(file->data.fp, (long)offset, (int)fwhence);
}

uint64_t FS_FileLength(file_t f)
{
	boost::lock_guard<boost::recursive_mutex> lock{fs_lock};
	uint64_t curPos, length;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_FileLength: out of range");
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
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
				Con_Printf(COLOR_RED "FS_Write: 0 bytes written\n");
				return 0;
			}
		}

		if (writeCount == -1) {
			Con_Printf(COLOR_RED "FS_Write: -1 bytes written\n");
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
	if (handle->data.chunk->bytesRead + size > handle->data.chunk->size) {
		N_Error(ERR_FATAL, "FS_ReadFromChunk: overread");
	}

	memcpy(buffer, handle->data.chunk->buf + handle->data.chunk->bytesRead, size);
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
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
				N_Error(ERR_FATAL, "FS_Read: -1 bytes read");
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_FOpenWrite: NULL or empty path");
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
		N_Error(ERR_FATAL, "FS_FOpenWrite: failed to create write-only stream for %s", path);
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_FOpenRW: NULL or empty path");
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
		N_Error(ERR_FATAL, "FS_FOpenRW: failed to create read/write stream for %s", path);
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_FOpenRead: NULL or empty path");
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
			Con_DPrintf("FS_FOpenRead: %s\n", ospath);
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_FOpenFileRead: NULL or empty path");
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
					return chunk->size;
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
					return chunk->size;
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!npath || !*npath) {
		N_Error(ERR_FATAL, "FS_LoadFile: NULL or empty path");
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!buffer) {
		N_Error(ERR_FATAL, "FS_FreeFile(NULL)");
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
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_FClose: out of range");
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

	search = (searchpath_t *)Z_Malloc(size, TAG_SEARCH_PATH);
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

		search = (searchpath_t *)Z_Malloc(sizeof(*search), TAG_SEARCH_PATH);
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

/*
===========
FS_ConvertPath
===========
*/
static void FS_ConvertPath( char *s ) {
	while (*s) {
		if ( *s == '\\' || *s == ':' ) {
			*s = '/';
		}
		s++;
	}
}


void FS_Flush(file_t f)
{
	fflush(handles[f].data.fp);
}

void FS_FilenameCompletion( const char *dir, const char *ext, qboolean stripExt, void(*callback)(const char *s), uint32_t flags )
{
	char	filename[ MAX_STRING_CHARS ];
	char	**filenames;
	uint64_t nfiles;
	uint64_t i;

	filenames = FS_ListFilteredFiles( dir, ext, NULL, &nfiles, flags );

	if ( nfiles >= 2 )
		FS_SortFileList( filenames, nfiles-1 );

	for ( i = 0; i < nfiles; i++ ) {
		N_strncpyz( filename, filenames[ i ], sizeof( filename ) );
		FS_ConvertPath( filename );

		if ( stripExt ) {
			COM_StripExtension( filename, filename, sizeof( filename ) );
		}

		callback( filename );
	}
	FS_FreeFileList( filenames );
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
	list = (searchpath_t **)Z_Malloc(cnt * sizeof(*list), TAG_SEARCH_PATH);
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
	Con_Printf("<-------------------- BFF List -------------------->\n");
	Con_Printf("fs_bffCount: %lu\n", fs_bffCount);
	Con_Printf("fs_bffChunks; %lu\n", fs_bffChunks);
	Con_Printf("fs_dirCount: %lu\n", fs_dirCount);
#ifdef USE_BFF_CACHE_FILE
	Con_Printf("(BFF Cache Information)\n");
	Con_Printf("fs_bffsCached: %lu\n", fs_bffsCached);
	Con_Printf("fs_bffsReaded: %lu\n", fs_bffsReaded);
	Con_Printf("fs_bffsSkipped: %lu\n", fs_bffsSkipped);
	Con_Printf("fs_bffsReleased: %lu\n", fs_bffsReleased);
#endif
	Con_Printf("====================================================\n");
	
	for (searchpath_t *sp = fs_searchpaths; sp; sp = sp->next) {
		if (sp->bff) {
			const bffFile_t *bff = sp->bff;

			Con_Printf("[BFF #%lu]\n", bff->index);
			Con_Printf("Chunk count: %lu\n", bff->numfiles);
			Con_Printf("Basename: %s\n", bff->bffBasename);
			Con_Printf("Gamename: %s\n", bff->bffGamename);
			Con_Printf("Filename: %s\n", bff->bffFilename);
			Con_Printf("Checksum: %i\n", bff->checksum);
			Con_Printf("\n");

			for (uint32_t i = 0; i < bff->numfiles; i++) {
				Con_Printf("<Chunk #%i>\n", i);
				Con_Printf("Name: %s\n", bff->buildBuffer[i].name);
				Con_Printf("Size: %lu\n", bff->buildBuffer[i].size);
				Con_Printf("\n");
			}
		}
	}
}

static void FS_ShowDir_f(void)
{
	char **list;
	uint64_t numfiles;

	list = Sys_ListFiles(fs_basegame->s, NULL, NULL, &numfiles, qfalse);

	Con_Printf("<-------------------- Directory %s -------------------->\n", fs_basepath->s);
	for (uint64_t i = 0; i < numfiles; i++) {
		Con_Printf("%s\n", list[i]);
	}

	Sys_FreeFileList(list);
}

static void FS_AddMod_f(void)
{
	const char *moddir, *modname;
	
	if (Cmd_Argc() != 2) {
		Con_Printf("usage: addmod <directory>\n");
		return;
	}

	moddir = Cmd_Argv(1);
	modname = strrchr(moddir, PATH_SEP);
	if (!modname) {
		modname = moddir; 
	}
	else {
		modname++;
	}

	FS_AddGameDirectory(moddir, moddir);
}

static void FS_Touch_f(void)
{

}

void FS_Shutdown(qboolean closeFiles)
{
	searchpath_t *p, *next;

	if (closeFiles) {
		for (uint64_t i = 0; i < MAX_FILE_HANDLES; i++) {
			if (!handles[i].data.stream)
				continue;
			
			FS_FClose(i);
		}
	}

	if (fs_searchpaths) {
	}

#ifdef USE_BFF_CACHE_FILE
	FS_ResetCacheReferences();
#endif

	// free everything
	for (p = fs_searchpaths; p; p = next) {
		next = p->next;

		if (p->bff) {
			FS_FreeBFF(p->bff);
			p->bff = NULL;
		}

		Z_Free(p);
	}

	fs_searchpaths = NULL;
	fs_bffCount = 0;
	fs_bffChunks = 0;
	fs_dirCount = 0;

	Cmd_RemoveCommand("dir");
	Cmd_RemoveCommand("ls");
	Cmd_RemoveCommand("list");
	Cmd_RemoveCommand("fs_restart");
}

void FS_Restart(void)
{
	// last valid game folder
	static char lastValidBase[MAX_OSPATH];
	static char lastValidGame[MAX_OSPATH];

	static qboolean execConfig = qfalse;

	// free anything we currently have loaded
	FS_Shutdown(qfalse);

	// try to start up normally
	FS_Startup();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FS_LoadFile( "default.cfg", NULL ) <= 0 ) {
		if (lastValidBase[0]) {
			Cvar_Set( "fs_basepath", lastValidBase );
			Cvar_Set( "fs_game", lastValidGame );
			lastValidBase[0] = '\0';
			lastValidGame[0] = '\0';
//			Cvar_Set( "fs_restrict", "0" );
			execConfig = qtrue;
			FS_Restart();
			N_Error( ERR_DROP, "Invalid game folder" );
			return;
		}
		N_Error( ERR_FATAL, "Couldn't load default.cfg" );
	}

	if (!N_stricmp(fs_gamedirvar->s, lastValidGame) && execConfig) {
		Cbuf_AddText("exec default.cfg\n");
	}

	execConfig = qfalse;

	N_strncpyz( lastValidBase, fs_basepath->s, sizeof( lastValidBase ) );
	N_strncpyz( lastValidGame, fs_gamedirvar->s, sizeof( lastValidGame ) );
}

static void FS_ListOpenFiles_f(void)
{
	uint64_t i;
	fileHandle_t *f;

	f = handles;
	for (i = 0; i < MAX_FILE_HANDLES; i++, f++) {
		if (!f->data.stream)
			continue;
		
		Con_Printf("%2lu %2s %s\n", i, FS_OwnerName(f->owner), f->name);
	}
}

/*
================
FS_InitFilesystem

Called only at initial startup, not when the filesystem
is resetting due to a game change
================
*/
void FS_InitFilesystem( void )
{
	// allow command line parms to override our defaults
	// we have to specially handle this, because normal command
	// line variable sets don't happen until after the filesystem
	// has already been initialized
	Com_StartupVariable( "fs_basepath" );
	Com_StartupVariable( "fs_homepath" );
	Com_StartupVariable( "fs_game" );
	Com_StartupVariable( "fs_basegame" );
//	Com_StartupVariable( "fs_copyfiles" );
	Com_StartupVariable( "fs_restrict" );
#ifndef USE_BFF_CACHE_FILE
	Com_StartupVariable( "fs_locked" );
#endif

#ifdef _WIN32
 	_setmaxstdio( 2048 );
#endif

	// try to start up normally
	FS_Restart();
}


void FS_Startup(void)
{
	const char *homepath;
	uint64_t start, end;

	fs_bffCount = 0;
	fs_loadStack = 0;
	fs_lastBFFIndex = -1;
	fs_bffChunks = 0;
	fs_readCount = 0;
	fs_writeCount = 0;

	Con_Printf("\n---------- FS_Startup ----------\n");

	fs_basepath = Cvar_Get("fs_basepath", Sys_DefaultBasePath(), CVAR_INIT | CVAR_PROTECTED | CVAR_PRIVATE);
	Cvar_SetDescription(fs_basepath, "Write-protected CVar specifying the path to the installation folder of the game.");
	fs_basegame = Cvar_Get("fs_basegame", BASEGAME_DIR, CVAR_PRIVATE | CVAR_PROTECTED);
	Cvar_SetDescription(fs_basegame, "CVar specifying the path to the base game folder.");

#ifdef USE_BFF_CACHE_FILE
	fs_locked = Cvar_Get("fs_locked", "0", CVAR_INIT);
	Cvar_SetDescription(fs_locked, "Set file handle policy for bff archive files:\n"
		" 0 - release after use, unlimited number of bff files can be loaded\n"
		" 1 - keep file handle locked, more consistent, total bff files count limited to ~1k-4k\n");
#endif

	if (!fs_basegame->s[0])
		N_Error(ERR_FATAL, "* fs_basegame not set *");

	homepath = Sys_DefaultHomePath();
	if (!homepath || !homepath[0]) {
		homepath = fs_basepath->s;
	}

	fs_homepath = Cvar_Get("fs_homepath", homepath, CVAR_INIT | CVAR_PROTECTED | CVAR_PRIVATE);
	Cvar_SetDescription( fs_homepath, "Directory to store user configuration and downloaded files." );

	fs_gamedirvar = Cvar_Get("fs_gamedir", "", CVAR_INIT | CVAR_SYSTEMINFO);
	Cvar_CheckRange(fs_gamedirvar, NULL, NULL, CVT_FSPATH);
	Cvar_SetDescription(fs_gamedirvar, "Specify an alternate mod directory and run the game with this mod.");

	if (!N_stricmp(fs_basegame->s, fs_gamedirvar->s)) {
		Cvar_ForceReset("fs_gamedir");
	}

	start = Sys_Milliseconds();

#ifdef USE_BFF_CACHE_FILE
	FS_LoadCache();
#endif

	// add search path elements in reverse priority order
//	if (fs_steampath->s[0]) {
//		FS_AddGameDirectory(fs_steampath->s, fs_basegame->s);
//	}

	if (fs_basepath->s[0]) {
		FS_AddGameDirectory(fs_basepath->s, fs_basegame->s);
	}
	// fs_homepath is somewhat particular to *nix systems, only add if relevant
	if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basegame->s)) {
		FS_AddGameDirectory(fs_homepath->s, fs_basegame->s);
	}


	// check for additional game folder for mods
	if (fs_gamedirvar->s[0] && N_stricmp(fs_gamedirvar->s, fs_basegame->s)) {
//		if (fs_steampath->s[0]) {
//			FS_AddGameDirectory(fs_steampath->s, fs_gamedirvar->s);
//		}
		if (fs_basepath->s[0]) {
			FS_AddGameDirectory(fs_basepath->s, fs_gamedirvar->s);
		}
		if (fs_homepath->s[0] && N_stricmp(fs_homepath->s, fs_basepath->s)) {
			FS_AddGameDirectory(fs_homepath->s, fs_gamedirvar->s);
		}
	}
	fs_initialized = qtrue;

	FS_ReorderSearchPaths();

	end = Sys_Milliseconds();

	Con_Printf(
		"fs_gamedir: %s\n"
		"fs_basepath: %s\n"
		"fs_basegame: %s\n",
	fs_gamedirvar->s, fs_basepath->s, fs_basegame->s);

	Con_Printf( "...loaded in %lu milliseconds\n", end - start );

	Con_Printf( "----------------------\n" );
	Con_Printf( "%lu chunks in %lu bff files\n", fs_bffChunks, fs_bffCount );

	// we just loaded, it's not modified
	fs_gamedirvar->modified = qfalse;

	for (uint64_t i = 0; i < MAX_FILE_HANDLES; i++) {
		FS_InitHandle(&handles[i]);
	}

#ifdef USE_BFF_CACHE_FILE
	FS_FreeUnusedCache();
	FS_SaveCache();
#endif
	
	Cmd_AddCommand("dir", FS_ShowDir_f);
	Cmd_AddCommand("ls", FS_ShowDir_f);
	Cmd_AddCommand("list", FS_ListBFF_f);
	Cmd_AddCommand("addmod", FS_AddMod_f);
	Cmd_AddCommand("fs_restart", FS_Restart);
	Cmd_AddCommand("lsof", FS_ListOpenFiles_f);
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
	out = (char *)Z_Malloc(strlen(s) + 1, TAG_STATIC);
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
	dst = cat = (char **)Z_Malloc( ( totalLength + 1 ) * sizeof( char* ), TAG_STATIC);

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


const char *FS_GetCurrentGameDir( void )
{
	if ( fs_gamedirvar->s[0] )
		return fs_gamedirvar->s;

	return fs_basegame->s;
}


const char *FS_GetBaseGameDir( void )
{
	return fs_basegame->s;
}


const char *FS_GetBasePath( void )
{
	if ( fs_basepath && fs_basepath->s[0] )
		return fs_basepath->s;
	else
		return "";
}


const char *FS_GetHomePath( void )
{
	if ( fs_homepath && fs_homepath->s[0] )
		return fs_homepath->s;
	else
		return FS_GetBasePath();
}


const char *FS_GetGamePath( void )
{
	static char buffer[ MAX_OSPATH + MAX_CVAR_VALUE + 1 ];
	if ( fs_gamedirvar && fs_gamedirvar->s[0] ) {
		snprintf( buffer, sizeof( buffer ), "%s%c%s", FS_GetHomePath(), 
			PATH_SEP, fs_gamedirvar->s );
		return buffer;
	} else {
		buffer[0] = '\0';
		return buffer;
	}
}

/*
FS_LoadLibrary: tries to load libraries within known searchpaths
*/
void *FS_LoadLibrary(const char *name)
{
	const searchpath_t *sp = fs_searchpaths;
	void *libHandle = NULL;
	char *fn;

#ifdef _NOMAD_DEBUG
	fn = FS_BuildOSPath( Sys_pwd(), name, NULL );
	libHandle = Sys_LoadDLL( fn );
#endif

	while ( !libHandle && sp ) {
		while ( sp && ( sp->access != DIR_STATIC || !sp->dir ) ) {
			sp = sp->next;
		}
		if ( sp ) {
			fn = FS_BuildOSPath( sp->dir->path, sp->dir->gamedir, name );
			libHandle = Sys_LoadDLL( fn );
			sp = sp->next;
		}
	}


	if ( !libHandle ) {
		return NULL;
	}

	return libHandle;
}

