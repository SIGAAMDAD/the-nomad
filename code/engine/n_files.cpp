#include "n_shared.h"
#include "n_common.h"
#include "n_cvar.h"
#include "../system/sys_timer.h"
#include "../system/sys_thread.h"
#include "gln_files.h"

// every time a new demo bff file is built, this checksum must be updated.
// the easiest way to get it is to just run the game and see what it spits out
#define	DEMO_BFF0_CHECKSUM	0
static const uint32_t bff_checksums[] = {
	3156427443u, // bff0
};

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <io.h>
#endif

//#define USE_ZIP

#ifdef USE_ZIP
#include <zip.h>
#endif

#define MAX_FILE_HANDLES 1024
#define MAX_FILEHASH_SIZE 256
#define BASEGAME_DIR "gamedata"
#define BASEDEMO_DIR "gamedata/demos"

#define HEADER_MAGIC 0x5f3759df
#define BFF_IDENT (('B'<<24)+('F'<<16)+('F'<<8)+'I')

typedef struct bffheader_s {
	int64_t ident = BFF_IDENT;
	int64_t magic = HEADER_MAGIC;
	int64_t numChunks;
	int64_t compression;
	int16_t version;
} bffheader_t;

// tunables
//#define USE_HANDLE_CACHE
#define USE_BFF_CACHE
#define USE_BFF_CACHE_FILE // cache the bff archives into a file for faster load times

//#define USE_HANDLE_CACHE
#define MAX_CACHED_HANDLES 528

typedef struct fileInBFF_s {
#ifdef USE_ZIP
	char					*name;		// name of the file
	zip_uint64_t			pos;		// file info position in zip
	zip_uint64_t			size;		// file size
	zip_file_t				*file;		// libzip file source
	struct	fileInBFF_s*	next;		// next file in the hash
#else
	char *name;
	char *buf;
	int64_t nameLen;
	int64_t size;
	int64_t compressedSize;
    int64_t bytesRead;
	int64_t pos;
	int64_t compression;
	struct fileInBFF_s *next;
#endif
} fileInBFF_t;

typedef struct bffFile_s
{
	char *bffBasename;					// c:\The Nomad\gamedata\bff0.bff
	char *bffFilename;					// bff0
//	char *bffGamename;
	char bffGamename[MAX_BFF_PATH];		// gamedata
#ifdef USE_ZIP
	zip_t *handle;						// handle to zip file
#else
	FILE *handle;						// handle to os file
#endif
	uint32_t referenced;				// referenced file flags
	uint32_t checksum;					// regular checksum
	uint32_t pure_checksum;				// checksum for pure
	qboolean exclude;					// found in \fs_excludeReference list
	uint64_t numfiles;					// number of files in bff
	uint64_t hashSize;					// hash table size (power of 2)
	fileInBFF_t **hashTable;			// hash table
	fileInBFF_t *buildBuffer;			// buffer with the filenames etc.
	int64_t index;
	uint32_t handlesUsed;

#ifdef USE_HANDLE_CACHE
	struct bffFile_s	*next_h;		// double-linked list of unreferenced bffs with open file handles
	struct bffFile_s	*prev_h;
#endif

	// caching subsystem
#ifdef USE_BFF_CACHE
	uint32_t namehash;
	fileOffset_t size;
	fileTime_t mtime;
	fileTime_t ctime;
	qboolean touched;
	struct bffFile_s *next;
	struct bffFile_s *prev;
	uint32_t checksumFeed;
	uint32_t *headerLongs;
	uint32_t numHeaderLongs;
#endif
} bffFile_t;

typedef union {
	FILE *fp;
	fileInBFF_t *chunk;
	void *stream;
} fileData;

typedef struct {
	char name[MAX_NPATH];

	fileData data;

	int64_t bffIndex;
	bffFile_t *bff;
	qboolean bffFile;
	qboolean handleSync;
#ifdef USE_ZIP
	int zipFilePos;
	int zipFileLen;
#endif
	qboolean tmp;
	qboolean used;
	qboolean mapped;
	handleOwner_t owner;
} fileHandleData_t;

typedef enum : uint64_t
{
	DIR_STATIC = 0, // always allow access
	DIR_CONST,		// read-only access
} diraccess_t;

typedef struct {
	char *path;
	char *gamedir;
} directory_t;

typedef struct searchpath_s {
	directory_t *dir;
	bffFile_t *bff;
	struct searchpath_s *next;
	diraccess_t access;
} searchpath_t;

#define FS_HashFileName(name,len) Com_GenerateHashValue((name),(len))

static CThreadMutex	fs_mutex;
static bffFile_t	**fs_archives;
#define MAX_BASEGAMES 4
static  char		basegame_str[MAX_OSPATH], *basegames[MAX_BASEGAMES];
static  int			basegame_cnt;
static  const char  *basegame = ""; /* last value in array */

static uint32_t		fs_checksumFeed;
static searchpath_t *fs_searchpaths;
static cvar_t		*fs_excludeReference;
static cvar_t		*fs_homepath;
static cvar_t		*fs_locked;
#ifdef __APPLE__
// Also search the .app bundle for .bff files
static cvar_t		*fs_apppath;
#endif
#ifdef NOMAD_STEAM_APP
static cvar_t		*fs_steampath;
#endif
static cvar_t		*fs_debug;
static cvar_t		*fs_restrict;
static cvar_t		*fs_basepath;
static cvar_t		*fs_basegame;
static cvar_t		*fs_gamedirvar;
static char			fs_gamedir[MAX_OSPATH]; // this will be a single file name with no separators

static uint64_t		fs_loadStack;
static uint64_t		fs_readCount;
static uint64_t		fs_writeCount;

static uint64_t		fs_bffCount;
static uint64_t		fs_bffChunks;
static uint64_t		fs_dirCount;

int64_t		fs_lastBFFIndex;

void Com_AppendCDKey( const char *filename );
void Com_ReadCDKey( const char *filename );

static fileHandleData_t handles[MAX_FILE_HANDLES];

static void FS_ReplaceSeparators( char *path )
{
	char *s;

	for ( s = path; *s; s++ ) {
		if ( *s == PATH_SEP_FOREIGN ) {
			*s = PATH_SEP;
		}
	}
}

static void FS_FixPath( char *path )
{
	char p;
	
	p = path[strlen( path )];
	if ( p == PATH_SEP || p == PATH_SEP_FOREIGN ) {
		p = '\0';
	}
}


static FILE *FS_FileForHandle( fileHandle_t f )
{
	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		N_Error( ERR_DROP, "FS_FileForHandle: out of range" );
	}
	if ( handles[f].bffFile ) {
		N_Error( ERR_DROP, "FS_FileForHandle: can't get FILE on bff file" );
	}
	if ( !handles[f].data.fp ) {
		N_Error( ERR_DROP, "FS_FileForHandle: invalid handle" );
	}

	return handles[f].data.fp;
}

static uint64_t FS_FileLength( FILE *fp )
{
	uint64_t pos, end;

	pos = ftell( fp );
	fseek( fp, 0L, SEEK_END );
	end = ftell( fp );
	fseek( fp, pos, SEEK_SET );

	return end;
}

static void FS_InitHandle( fileHandleData_t *f )
{
	f->used = qfalse;
	f->bffFile = qfalse;
	f->bff = NULL;
	f->bffIndex = -1;
	f->mapped = qfalse;
	f->tmp = qfalse;
}

static fileHandle_t FS_HandleForFile( void )
{
	for ( fileHandle_t i = 1; i < MAX_FILE_HANDLES; i++ ) {
		if ( !handles[i].used || !handles[i].data.stream ) {
			return i;
		}
	}
	N_Error( ERR_DROP, "FS_HandleForFile: not enough handles" );
	return FS_INVALID_HANDLE;
}

/*
* FS_BuildOSPath: npath may have either forward or backwards slashes
*/
char *FS_BuildOSPath( const char *base, const char *game, const char *npath )
{
	char temp[MAX_OSPATH*2+1];
	static char ospath[2][sizeof(temp)+MAX_OSPATH];
	static int toggle;

	toggle ^= 1; // flip-flop to allow two returns without clash
	
	if ( !game || !*game ) {
		game = fs_basegame->s;
	}

	if ( npath ) {
		snprintf( temp, sizeof(temp), "%c%s%c%s", PATH_SEP, game, PATH_SEP, npath );
	} else {
		snprintf( temp, sizeof(temp), "%c%s", PATH_SEP, game );
	}

	FS_ReplaceSeparators( temp );
	snprintf( ospath[toggle], sizeof(*ospath), "%s%s", base, temp );

	return ospath[toggle];
}

/*
* FS_PathCmp: Ignore case and separator char distinctions
*/
static int FS_PathCmp( const char *s1, const char *s2 )
{
	int c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( c1 >= 'a' && c1 <= 'z' ) {
			c1 -= ('a' - 'A');
		}
		if ( c2 >= 'a' && c2 <= 'z' ) {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}
		
		if ( c1 < c2 ) {
			return -1;		// strings not equal
		}
		if ( c1 > c2 ) {
			return 1;
		}
	} while ( c1 );
	
	return 0;		// strings are equal
}

static const char *FS_OwnerName( handleOwner_t owner )
{
	switch ( owner ) {
	case H_SGAME: return "SGAME";
	case H_SCRIPT: return "SCRIPT";
	case H_UI: return "UI";
	};
	return "UNKOWN";
}

static inline qboolean FS_VM_ValidateParms( const char *func, fileHandle_t file, handleOwner_t owner )
{
	if ( file <= FS_INVALID_HANDLE || file >= MAX_FILE_HANDLES ) {
		Con_Printf( COLOR_RED "%s: invalid handle\n", func );
		return qfalse;
	}
	else if ( handles[file].owner != owner ) {
		Con_Printf( COLOR_RED "%s: owner isn't correct (should be '%s', is '%s')\n", func, FS_OwnerName( owner ), FS_OwnerName( handles[file].owner ) );
		return qfalse;
	}
	else if ( !handles[file].data.fp ) {
		Con_Printf( COLOR_RED "%s: engine file isn't open yet!\n", func );
		return qfalse;
	}

	return qtrue;
}

static inline qboolean FS_VM_ValidateParms( const char *func, const char *npath, fileHandle_t *file, handleOwner_t owner )
{
	if ( !npath || !*npath ) {
		Con_Printf( COLOR_RED "%s: empty path\n", func );
		return qfalse;
	}
	else if ( !file ) {
		Con_Printf( COLOR_RED "%s: NULL file handle\n", func );
		return qfalse;
	}

	return qtrue;
}

static inline qboolean FS_VM_ValidateParms( const char *func, const void *buffer, fileHandle_t file, handleOwner_t owner )
{
	if ( !buffer ) {
		Con_Printf( COLOR_RED "%s: NULL buffer\n", func );
		return qfalse;
	}
	else if ( file <= FS_INVALID_HANDLE || file >= MAX_FILE_HANDLES ) {
		Con_Printf( COLOR_RED "%s: invalid handle\n", func );
		return qfalse;
	}
	else if ( handles[file].owner != owner ) {
		Con_Printf( COLOR_RED "%s: owner isn't correct (should be '%s', is '%s')\n", func, FS_OwnerName( owner ), FS_OwnerName( handles[file].owner ) );
		return qfalse;
	}
	else if ( !handles[file].data.fp ) {
		Con_Printf( COLOR_RED "%s: engine file isn't open yet!\n", func );
		return qfalse;
	}

	return qtrue;
}

uint64_t FS_VM_WriteFile( const void *buffer, uint64_t len, fileHandle_t file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, buffer, file, owner ) ) {
		return 0;
	}

	return FS_Write( buffer, len, file );
}

fileOffset_t FS_VM_FileSeek( fileHandle_t file, fileOffset_t offset, uint32_t whence, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, file, owner ) ) {
		return 0;
	}
	
	return FS_FileSeek( file, offset, whence );
}

fileOffset_t FS_VM_FileTell( fileHandle_t file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, file, owner ) ) {
		return 0;
	}

	return FS_FileTell( file );
}

uint64_t FS_VM_Write( const void *buffer, uint64_t len, fileHandle_t file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, buffer, file, owner ) ) {
		return 0;
	}

	return FS_Write( buffer, len, file );
}

uint64_t FS_VM_Read( void *buffer, uint64_t len, fileHandle_t file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, buffer, file, owner ) ) {
		return 0;
	}

	return FS_Read( buffer, len, file );
}

uint64_t FS_VM_FOpenFile( const char *npath, fileHandle_t *f, fileMode_t mode, handleOwner_t owner )
{
	uint64_t len;

	if ( !FS_VM_ValidateParms( __func__, npath, f, owner ) ) {
		return 0;
	}

	len = FS_FOpenFileWithMode( npath, f, mode );

	if ( *f != FS_INVALID_HANDLE ) {
		handles[*f].owner = owner;
	}

	return len;
}

fileHandle_t FS_VM_FOpenAppend( const char *path, handleOwner_t owner )
{
	fileHandle_t f;

	if ( !path || !*path ) {
		Con_Printf( COLOR_RED "%s: empty path\n", __func__ );
		return FS_INVALID_HANDLE;
	}

	f = FS_FOpenAppend( path );
	if ( f != FS_INVALID_HANDLE ) {
		handles[f].owner = owner;
	}

	return f;
}

fileHandle_t FS_VM_FOpenRW( const char *path, handleOwner_t owner )
{
	fileHandle_t f;

	if ( !path || !*path ) {
		Con_Printf( COLOR_RED "%s: empty path\n", __func__ );
		return FS_INVALID_HANDLE;
	}

	f = FS_FOpenRW( path );
	if ( f != FS_INVALID_HANDLE ) {
		handles[f].owner = owner;
	}

	return f;
}

fileHandle_t FS_VM_FOpenRead( const char *path, handleOwner_t owner )
{
	fileHandle_t f;

	if ( !path || !*path ) {
		Con_Printf( COLOR_RED "%s: empty path\n", __func__ );
		return FS_INVALID_HANDLE;
	}

	f = FS_FOpenRead( path );
	if ( f != FS_INVALID_HANDLE ) {
		handles[f].owner = owner;
	}
	
	return f;
}

fileHandle_t FS_VM_FOpenWrite( const char *path, handleOwner_t owner )
{
	fileHandle_t f;

	if ( !path || !*path ) {
		Con_Printf( COLOR_RED "%s: empty path\n", __func__ );
		return FS_INVALID_HANDLE;
	}

	f = FS_FOpenWrite( path );
	if ( f != FS_INVALID_HANDLE ) {
		handles[f].owner = owner;
	}

	return f;
}

uint64_t FS_VM_FileLength( fileHandle_t file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, file, owner ) ) {
		return 0;
	}

	return FS_FileLength( file );
}

fileHandle_t FS_VM_FOpenFileWrite( const char *npath, fileHandle_t *file, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, npath, file, owner ) ) {
		return 0;
	}

	*file = FS_FOpenWrite( npath );
	if (*file != FS_INVALID_HANDLE) {
		handles[*file].owner = owner;
	}

	return *file;
}

uint64_t FS_VM_FOpenFileRead( const char *npath, fileHandle_t *file, handleOwner_t owner )
{
	uint64_t len;

	if ( !FS_VM_ValidateParms( __func__, npath, file, owner ) ) {
		return 0;
	}

	len = FS_FOpenFileRead( npath, file );
	if ( len ) {
		handles[*file].owner = owner;
	}

	return len;
}

void FS_VM_CloseFiles( handleOwner_t owner )
{
	uint64_t i;

	for ( i = 0; i < MAX_FILE_HANDLES; i++ ) {
		if ( handles[i].owner != owner ) {
			continue;
		}
		
		Con_Printf( COLOR_YELLOW "%s:%lu:%s leaked filehandle\n", FS_OwnerName( owner ), i, handles[i].name );
		FS_FClose( i );
	}
}

void FS_VM_FClose( fileHandle_t f, handleOwner_t owner )
{
	if ( !FS_VM_ValidateParms( __func__, f, owner ) ) {
		return;
	}
	
	FS_FClose(f);
}

static fnamecallback_f fnamecallback = NULL;

void FS_SetFilenameCallback( fnamecallback_f func )  {
	fnamecallback = func;
}

qboolean FS_Initialized( void ) {
	return fs_searchpaths != NULL;
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
	if ( e >= ( fileName + 3 ) && *( e + 1 ) >= '0' && *(e+1) <= '9' && *(e+2) == '\0' )  {
		if ( *( e - 3 ) == '.' && ( *( e - 2 ) == 's' || *( e - 2 ) == 'S' ) && ( *( e - 1 ) == 'o' || *( e - 1 ) == 'O' ) ) {
			if ( ext ) {
				*ext = (e-2);
			}
			return qfalse;
		}
	}
	if ( !e ) {
		return qtrue;
	}

	e++; // skip '.'

	if ( allowBFFs ) {
		n = arraylen( extlist ) - 1;
	} else {
		n = arraylen( extlist );
	}
	
	for ( i = 0; i < n; i++ )  {
		if ( N_stricmp( e, extlist[i] ) == 0 ) {
			if ( ext ) {
				*ext = e;
			}
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

FILE *FS_Handle( fileHandle_t f )
{
	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		Con_Printf( COLOR_RED "FS_Handle: out of range\n" );
		return NULL;
	}
	if ( handles[f].bffFile ) {
		Con_Printf( COLOR_RED "FS_Handle: can't get FILE on bff file\n" );
		return NULL;
	}
	if ( !handles[f].data.fp ) {
		Con_Printf( COLOR_RED "FS_Handle: invalid handle\n" );
		return NULL;
	}

	return handles[f].data.fp;
}

/*
===========
FS_Remove
===========
*/
void FS_Remove( const char *osPath ) 
{
	FS_CheckFilenameIsNotAllowed( osPath, __func__, qtrue );

	remove( osPath );
}


/*
===========
FS_HomeRemove
===========
*/
void FS_HomeRemove( const char *osPath ) 
{
	FS_CheckFilenameIsNotAllowed( osPath, __func__, qfalse );

	remove( FS_BuildOSPath( fs_homepath->s,
			fs_gamedir, osPath ) );
}

/*
FS_FileExists

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
*/
qboolean FS_FileExists( const char *filename )
{
#if 0
	FILE *fp;
	const char *testpath;

	testpath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, filename );

	fp = Sys_FOpen( testpath, "rb" );;
	if ( fp ) {
		fclose( fp );
		return qtrue;
	}
	return qfalse;
#else
	fileStats_t stats;
	const char *testpath;

	testpath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, filename );
	if ( !Sys_GetFileStats( &stats, testpath ) ) {
		return qfalse;
	}
	return qtrue;
#endif
}


/*
FS_CheckDirTraversal:

Check whether the string contains stuff like "../" to prevent directory traversal bugs
and return qtrue if it does.
*/
static qboolean FS_CheckDirTraversal( const char *checkdir )
{
	if ( strstr( checkdir, "../" ) || strstr( checkdir, "..\\" ) ) {
		return qtrue;
	}
	if ( strstr( checkdir, "::" ) ) {
		return qtrue;
	}
	
	return qfalse;
}

/*
============
FS_CreatePath

Creates any directories needed to store the given filename
============
*/
qboolean FS_CreatePath( const char *OSPath ) {
	char	path[MAX_OSPATH*2+1];
	char	*ofs;
	
	// make absolutely sure that it can't back up the path
	// FIXME: is c: allowed???
	if ( FS_CheckDirTraversal( OSPath ) ) {
		Con_Printf( "WARNING: refusing to create relative path \"%s\"\n", OSPath );
		return qtrue;
	}

	N_strncpyz( path, OSPath, sizeof( path ) );
	// Make sure we have OS correct slashes
	FS_ReplaceSeparators( path );
	for ( ofs = path + 1; *ofs; ofs++ ) {
		if ( *ofs == PATH_SEP ) {
			// create the directory
			*ofs = '\0';
			Sys_mkdir( path );
			*ofs = PATH_SEP;
		}
	}
	return qfalse;
}


/*
=================
FS_CopyFile

Copy a fully specified file from one place to another
=================
*/
static void FS_CopyFile( const char *fromOSPath, const char *toOSPath )
{
	FILE	*f;
	size_t	len;
	byte	*buf;

	Con_Printf( "copy %s to %s\n", fromOSPath, toOSPath );

	if ( strstr( fromOSPath, "journal.dat" ) ) {
		Con_Printf( "Ignoring journal files\n");
		return;
	}

	f = Sys_FOpen( fromOSPath, "rb" );
	if ( !f ) {
		return;
	}

	len = FS_FileLength( f );

	// we are using direct malloc instead of Z_Malloc here, so it
	// probably won't work on a mac... It's only for developers anyway...
	buf = (byte *)malloc( len );
	if ( !buf ) {
		fclose( f );
		N_Error( ERR_FATAL, "Memory alloc error in FS_Copyfiles()\n" );
	}

	if ( fread( buf, 1, len, f ) != len ) {
		free( buf );
		fclose( f );
		N_Error( ERR_FATAL, "Short read in FS_Copyfiles()" );
	}
	fclose( f );

	f = Sys_FOpen( toOSPath, "wb" );
	if ( !f ) {
		if ( FS_CreatePath( toOSPath ) ) {
			free( buf );
			return;
		}
		f = Sys_FOpen( toOSPath, "wb" );
		if ( !f ) {
			free( buf );
			return;
		}
	}

	if ( fwrite( buf, 1, len, f ) != len ) {
		free( buf );
		fclose( f );
		N_Error( ERR_FATAL, "Short write in FS_Copyfiles()" );
	}
	fclose( f );
	free( buf );
}



qboolean FS_FileIsInBFF( const char *filename )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	const fileInBFF_t *file;
	const searchpath_t *sp;
	const bffFile_t *bff;
	uint64_t hash;
	uint64_t fullHash;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !filename ) {
		N_Error( ERR_FATAL, "FS_FileIsInBFF: NULL filename" );
	}

	// npaths are not supposed to have a leading slash
	while ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}
	
	if ( FS_CheckDirTraversal( filename ) ) {
		return qfalse;
	}

	fullHash = FS_HashFileName( filename, 0U );

	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		if ( sp->bff && sp->bff->hashTable[( hash = fullHash & ( sp->bff->hashSize - 1 ) )] ) {
			bff = sp->bff;
			file = sp->bff->hashTable[hash];
			do {
				if ( !FS_FilenameCompare( file->name, filename ) ) {
					return qtrue;
				}
				file = file->next;
			} while ( file != NULL );
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
	int i, j;
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

static fileInBFF_t *FS_GetChunkHandle( const char *path, int64_t *bffIndex )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileInBFF_t *file;
	searchpath_t *sp;
	bffFile_t *bff;
	uint64_t i;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !path ) {
		N_Error( ERR_FATAL, "FS_GetChunkHandle: NULL filename" );
	}

	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		if ( sp->bff ) {
			bff = sp->bff;
			
			for ( i = 0; i < bff->numfiles; i++ ) {
				file = &bff->buildBuffer[i];
				if ( !FS_FilenameCompare( file->name, path ) ) {
					// found it!
					return file;
				}
			}
		}
	}
	return NULL;
}

uint64_t FS_FOpenFileWithMode( const char *npath, fileHandle_t *f, fileMode_t mode )
{
	uint64_t len;

	if ( !npath || !f ) {
		N_Error( ERR_FATAL, "FS_FOpenFileWithMode: NULL parameter" );
	}

	switch ( mode ) {
	case FS_OPEN_WRITE:
	{
		*f = FS_FOpenWrite( npath );
		if ( *f == FS_INVALID_HANDLE ) {
			N_Error( ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in write mode", npath );
		}
		len = 0;
		break;
	}
	case FS_OPEN_READ:
	{
		len = FS_FOpenFileRead( npath, f );
		if ( !len || *f == FS_INVALID_HANDLE ) {
			Con_DPrintf( "FS_FOpenFileWithMode(%s): !len || *f == FS_INVALID_HANDLE (r), failure\n", npath );
			len = 0;
		}
		break;
	}
	case FS_OPEN_APPEND:
	{
		*f = FS_FOpenAppend( npath );
		if ( *f == FS_INVALID_HANDLE ) {
			N_Error( ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in append mode", npath );
		}
		len = FS_FileLength( *f );
		break;
	}
	case FS_OPEN_RW:
	{
		*f = FS_FOpenRW( npath );
		if ( *f == FS_INVALID_HANDLE ) {
			N_Error( ERR_DROP, "FS_FOpenFileWithMode: failed to open '%s' in read-write mode", npath );
		}
		len = FS_FileLength( *f );
		break;
	}
	default:
		N_Error( ERR_DROP, "FS_FOpenFileWithMode: bad mode (%i)", mode );
		break;
	};
	return len;
}

fileHandle_t FS_FOpenAppend( const char *path )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	FILE *fp;
	const char *ospath;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !path || !*path ) {
		return FS_INVALID_HANDLE;
	}

	// write streams aren't allowed for chunks
	if ( FS_FileIsInBFF( path ) ) {
		return FS_INVALID_HANDLE;
	}

	ospath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, path );

	if ( fs_debug->i ) {
		Con_Printf( "FS_FOpenAppend: %s\n", ospath );
	}

	// validate the file is actually write-enabled
	FS_CheckFilenameIsNotAllowed( ospath, __func__, qfalse );

	fd = FS_HandleForFile();
	if ( fd == FS_INVALID_HANDLE ) {
		return fd;
	}
	
	f = &handles[fd];
	FS_InitHandle( f );

	fp = Sys_FOpen( ospath, "a" );
	if ( !fp ) {
		if ( FS_CreatePath( ospath ) ) {
			return FS_INVALID_HANDLE;
		}
		fp = Sys_FOpen( ospath, "a" );
		if ( !fp ) {
			return FS_INVALID_HANDLE;
		}
	}

	N_strncpyz( f->name, path, sizeof(f->name) );
	f->used = qtrue;
	f->data.fp = fp;

	return fd;
}

fileHandle_t FS_OpenFileMapping( const char *path, qboolean temp )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	const char *ospath;
	searchpath_t *sp;
	FILE *fp;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !path || !*path ) {
		N_Error( ERR_FATAL, "FS_OpenFileMapping: NULL or empty path" );
	}

	fd = FS_HandleForFile();
	if ( fd == FS_INVALID_HANDLE ) {
		return fd;
	}
	
	f = &handles[fd];
	FS_InitHandle( f );

	// the path isn't a unique file, don't map it
	if ( FS_FileIsInBFF( path ) ) {
		f->data.chunk = FS_GetChunkHandle( path, &f->bffIndex );
		if ( !f->data.chunk ) {
			return FS_INVALID_HANDLE;
		}
		N_strncpyz( f->name, f->data.chunk->name, MAX_NPATH );
		f->mapped = qfalse;
		f->bff = fs_archives[f->bffIndex];
		f->bffFile = qtrue;
		f->used = qtrue;
		return fd;
	}

	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		if ( sp->access != DIR_STATIC ) {
			continue;
		}
		
		ospath = FS_BuildOSPath( sp->dir->path, sp->dir->gamedir, path );
		fp = Sys_FOpen( ospath, "wb+" );
		if ( fp ) {
			f->data.fp = fp;
			break;
		}
		return FS_INVALID_HANDLE;
	}

/*
	f->mapping = Sys_MapMemory(f->data.fp, temp, fd);
	if ( !f->mapping ) {
		// cleanup, file mapping failed
		f->data.stream = NULL;
		f->used = qfalse;
		fclose( fp );

		return FS_INVALID_HANDLE;
	}

	f->mapped = qtrue;
	N_strncpyz( f->name, path, MAX_NPATH );
	f->used = qtrue;

	return fd;
*/
	return fd;
}

void FS_SetBFFIndex( uint64_t index )
{
	fs_lastBFFIndex = index;
}


static const char *FS_HasExt( const char *fileName, const char **extList, uint64_t extCount ) 
{
	const char *e;
	uint64_t i;

	e = strrchr( fileName, '.' );

	if ( !e ) {
		return NULL;
	}

	for ( i = 0, e++; i < extCount; i++ )  {
		if ( !N_stricmp( e, extList[i] ) )
			return e;
	}

	return NULL;
}


static qboolean FS_GeneralRef( const char *filename )
{
	// allowed non-ref extensions
	static const char *extList[] = { "config", "shader", "cfg", "txt", "menu" };

	if ( FS_HasExt( filename, extList, arraylen(extList) ) ) {
		return qfalse;
	}
	if ( strstr( filename, "levelshots" ) ) {
		return qfalse;
	}
	
	return qtrue;
}

static void FS_OpenChunk( const char *name, fileInBFF_t *chunk, fileHandleData_t *f, bffFile_t *bff )
{
#ifdef USE_ZIP
	int err;
	zip_error_t error;
	zip_file_t *file;
#endif
	if ( !( bff->referenced & FS_GENERAL_REF ) && FS_GeneralRef( chunk->name ) ) {
		bff->referenced |= FS_GENERAL_REF;
	}
	if ( !( bff->referenced & FS_SGAME_REF ) && !strcmp( chunk->name, "vm/sgame.qvm" ) ) {
		bff->referenced |= FS_SGAME_REF;
	}
	if ( !( bff->referenced & FS_UI_REF ) && !strcmp( chunk->name, "vm/ui.qvm" ) ) {
		bff->referenced |= FS_UI_REF;
	}


	if ( !bff->handle ) {
#ifdef USE_ZIP
		bff->handle = zip_open( bff->bffFilename, 0, &err );
#else
		bff->handle = Sys_FOpen( bff->bffFilename, "rb" );
#endif
		if ( !bff->handle ) {
			Con_Printf( COLOR_RED "Error opening %s@%s\n", bff->bffBasename, name );
			memset( f, 0, sizeof( *f ) );
			return;
		}
	}

#ifdef USE_ZIP
	chunk->file = zip_fopen_index( bff->handle, chunk->pos, 0 );
	if ( !chunk->file ) {
		Con_Printf( COLOR_RED "Error opening %s@%s\n", bff->bffBasename, name );
		memset( f, 0, sizeof( *f ) );
		return;
	}
	f->zipFilePos = chunk->pos;
	f->zipFileLen = chunk->size;
#endif

	f->bff = bff;
	f->bffIndex = bff->index;
	f->bffFile = qtrue;
	f->data.chunk = chunk;
	f->used = qtrue;
	fs_lastBFFIndex = bff->index;
	N_strncpyz( f->name, chunk->name, sizeof( f->name ) );

	bff->handlesUsed++;
}


/*
FS_IsExt: Return qtrue if ext matches file extension filename
*/
static qboolean FS_IsExt( const char *filename, const char *ext, size_t namelen )
{
	size_t extlen;

	extlen = strlen( ext );

	if ( extlen > namelen ) {
		return qfalse;
	}

	filename += namelen - extlen;

	return (qboolean)!N_stricmp( filename, ext );
}


int64_t FS_LastBFFIndex( void )
{
	return fs_lastBFFIndex;
}

static uint64_t FS_ReturnPath( const char *bffname, char *bffpath, uint64_t *depth ) {
	uint64_t len, at, newdep;

	newdep = 0;
	bffpath[0] = '\0';
	len = 0;
	at = 0;

	while ( bffname[at] != 0 ) {
		if ( bffname[at] == '/' || bffname[at] == '\\' ) {
			len = at;
			newdep++;
		}
		at++;
	}
	strcpy(bffpath, bffname);
	bffpath[len] = '\0';
	*depth = newdep;

	return len;
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

/*
* FS_ListFilteredFiles: returns a unique list of files that match the given criteria
* from all search paths
*/
static char **FS_ListFilteredFiles( const char *path, const char *extension, const char *filter, uint64_t *numfiles, uint32_t flags )
{
	uint64_t nfiles;
	char **listCopy;
	char *list[MAX_FOUND_FILES];
	uint64_t i;
	uint64_t pathLength;
	uint64_t extLen;
	char bffpath[MAX_NPATH];
	uint64_t length, pathDepth, temp;
	const bffFile_t *bff;
	qboolean hasPatterns;
	const char *x;
	const searchpath_t *sp;

	PROFILE_FUNCTION();

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !path ) {
		*numfiles = 0;
		return NULL;
	}

	if ( !extension ) {
		extension = "";
	}

	extLen = strlen( extension );
	hasPatterns = Com_HasPatterns( extension );
	if ( hasPatterns && extension[0] == '.' && extension[1] != '\0' ) {
		extension++;
	}

	pathLength = strlen( path );
	if ( pathLength > 0 && ( path[pathLength - 1] == '\\' || path[pathLength - 1] == '/' ) ) {
		pathLength--;
	}
	nfiles = 0;
	FS_ReturnPath( path, bffpath, &pathDepth );

	// search through the path, one element at a time, adding to the list
	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		// is the element a bff archive?
		if ( sp->bff ) {
			
			// look through all the bff chunks
			bff = sp->bff;
			for ( i = 0; i < bff->numfiles; i++ ) {
				const char *name;
				uint64_t depth, bffPathLen;

				// check for directory match
				name = bff->buildBuffer[i].name;

				if ( filter ) {
					// case insensitive
					if ( !Com_FilterPath( filter, name ) ) {
						continue;
					}
					
					// unique the match
					nfiles = FS_AddFileToList( name, list, nfiles );
				}
				else {
					bffPathLen = FS_ReturnPath( name, bffpath, &depth );

					if ( ( depth - pathDepth ) > 2 || pathLength > bffPathLen || N_stricmpn( name, path, pathLength ) ) {
						continue;
					}

					length = strlen( name );

					if ( fnamecallback ) {
						// use custom filter
						if ( !fnamecallback( name, length ) ) {
							continue;
						}
					} else {
						if ( length < extLen ) {
							continue;
						}

						if ( *extension ) {
							if ( hasPatterns ) {
								x = strrchr( name, '.' );
								if ( !x || !Com_FilterExt( extension, x + 1 ) ) {
									continue;
								}
							}
							else {
								if ( N_stricmp( name + length - extLen, extension ) ) {
									continue;
								}
							}
						}
					}

					// unique the match
					temp = pathLength;
					if ( pathLength ) {
						temp++; // include the '/'
					}

					nfiles = FS_AddFileToList(name + temp, list, nfiles);
				}
			}
		}
		else if (sp->dir) {
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

	if (!nfiles) {
		return NULL;
	}

	listCopy = (char **)Z_Malloc( (nfiles + 1) * sizeof(*listCopy), TAG_STATIC );
	for (i = 0; i < nfiles; i++) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

char *FS_ReadLine(char *buf, uint64_t size, fileHandle_t f)
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
================
FS_IsBaseGame
================
*/
static qboolean FS_IsBaseGame( const char *game )
{
	int i;

	if ( game == NULL || *game == '\0' ) {
		return qtrue;
	}

	for ( i = 0; i < basegame_cnt; i++ ) {
		if ( N_stricmp( basegames[i], game ) == 0 ) {
			return qtrue;
		}
	}

	return qfalse;
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

uint64_t FS_BFFHashSize( uint32_t fileCount )
{
	uint64_t hashSize;

	for ( hashSize = 2; hashSize < MAX_FILEHASH_SIZE; hashSize <<= 1 ) {
		if ( hashSize >= fileCount ) {
			break;
		}
	}

	return hashSize;
}


#ifdef USE_BFF_CACHE

#define BFF_HASH_SIZE 512

static void FS_FreeBFF( bffFile_t *bff );
static bffFile_t *bffHashTable[ BFF_HASH_SIZE ];

#ifdef USE_BFF_CACHE_FILE

#define CACHE_FILE_NAME CACHE_DIR "/bffcache.dat"
#define CACHE_SYNC_CONDITION ( fs_bffsReaded + fs_bffsSkipped + fs_bffsReaded >= NUM_GDR_BFFS )

static uint64_t fs_bffsCached;		// read from cache file
static uint64_t fs_bffsSkipped;		// outdated/non-existent cache file bff entries

static uint64_t fs_bffsReaded;		// actually read from the disk
static uint64_t fs_bffsReleased;	// unreferenced bffs since last FS restart

static qboolean fs_cacheLoaded = qfalse;
static qboolean fs_cacheSynced = qtrue;

#pragma pack( push, 1 )

// platform-specific 4-byte signature:
// 0: [version] anything following depends from it
// 1: [endianess] 0 - LSB, 1 - MSB
// 2: [path separation] '/' or '\\'
// 3: [size of file offset and file time]
// non-matching header will cause whole file being ignored
static const byte cache_header[ 4 ] = {
	0, //version
#ifdef GDR_LITTLE_ENDIAN
	0x0,
#else
	0x1,
#endif
	PATH_SEP,
	( ( sizeof( fileOffset_t ) - 1 ) << 4 ) | ( sizeof( fileTime_t ) - 1 )
};

typedef struct {
	uint64_t bffNameLen;
	uint64_t numFiles;
	uint64_t contentLen;
	uint64_t numHeaderLongs;
	uint64_t namesLen;
	fileTime_t ctime;	// creation/status change time
	fileTime_t mtime;	// modification time
	fileOffset_t size;	// bff file size
} bffCacheHeader_t;

typedef struct {
	uint64_t name;
	int64_t compressedSize;
	int64_t size;
	int64_t pos;
} bffCacheFileItem_t;

#pragma pack(pop)

#endif // USE_BFF_CACHE_FILE

#ifdef USE_HANDLE_CACHE

static uint64_t	hbffsCount;
static bffFile_t *hhead;

static void FS_RemoveFromHandleList( bffFile_t *bff )
{
	if ( bff->next_h != bff ) {
		// cut bff from list
		bff->next_h->prev_h = bff->prev_h;
		bff->prev_h->next_h = bff->next_h;
		if ( hhead == bff ) {
			hhead = bff->next_h;
		}
	} else {
#if defined(_DEBUG) || !defined(NDEBUG)
		if ( hhead != bff ) {
			N_Error( ERR_DROP, "%s(): invalid head pointer", __func__ );
		}
#endif
		hhead = NULL;
	}

	bff->next_h = NULL;
	bff->prev_h = NULL;
	
	hbffsCount--;

#if defined(_DEBUG) || !defined(NDEBUG)
	if ( hbffsCount < 0 ) {
		N_Error( ERR_DROP, "%s(): negative bffs count", __func__ );
	}

	if ( hbffsCount == 0 && hhead != NULL ) {
		N_Error( ERR_DROP, "%s(): non-null head with zero bffs count", __func__ );
	}
#endif
}


static void FS_AddToHandleList( bffFile_t *bff )
{
#if defined(_DEBUG) || !defined(NDEBUG)
	if ( !bff->handle ) {
		N_Error( ERR_DROP, "%s(): invalid bff handle", __func__ );
	}
	if ( bff->next_h || bff->prev_h ) {
		N_Error( ERR_DROP, "%s(): invalid bff pointers", __func__ );
	}
#endif
	while ( hbffsCount >= MAX_CACHED_HANDLES ) {
		bffFile_t *file = hhead->prev_h; // tail item
#if defined(_DEBUG) || !defined(NDEBUG)
		if ( file->handle == NULL || file->handlesUsed != 0 ) {
			N_Error( ERR_DROP, "%s(): invalid bff handle", __func__ );
		}
#endif
		fclose( file->handle );
		file->handle = NULL;
		FS_RemoveFromHandleList( file );
	} 

	if ( hhead == NULL ) {
		bff->next_h = bff;
		bff->prev_h = bff;
	} else {
		hhead->prev_h->next_h = bff;
		bff->prev_h = hhead->prev_h;
		hhead->prev_h = bff;
		bff->next_h = hhead;
	}

	hhead = bff;
	hbffsCount++;
}
#endif

static uint32_t FS_HashBFF( const char *name )
{
	uint32_t c, hash = 0;
	while ( (c = *name++) != '\0' ) {
		hash = hash * 101 + c;
	}
	hash = hash ^ ( hash >> 16 );
	return hash & ( BFF_HASH_SIZE-1 );
}

static bffFile_t *FS_FindInCache( const char *zipfile )
{
	bffFile_t *bff;
	uint32_t hash;

	hash = FS_HashBFF( zipfile );
	bff = bffHashTable[ hash ];
	while ( bff ) {
		if ( !strcmp( zipfile, bff->bffFilename ) ) {
			return bff;
		}
		bff = bff->next;
	}

	return NULL;
}

static void FS_AddToCache( bffFile_t *bff )
{
	bff->namehash = FS_HashBFF( bff->bffGamename );
	bff->next = bffHashTable[ bff->namehash ];
	bff->prev = NULL;
	if ( bffHashTable[ bff->namehash ] ) {
		bffHashTable[ bff->namehash ] = bff;
	}
	bffHashTable[ bff->namehash ] = bff;
}

static void FS_RemoveFromCache( bffFile_t *bff )
{
	if ( !bff->next && !bff->prev && bffHashTable[bff->namehash] != bff ) {
		N_Error( ERR_FATAL, "Invalid BFF Link" );
	}

	if ( bff->prev ) {
		bff->prev->next = bff->next;
	} else {
		bffHashTable[bff->namehash] = bff->next;
	}
	
	if ( bff->next ) {
		bff->next->prev = bff->prev;
	}
}


static bffFile_t *FS_LoadCachedBFF( const char *bffpath )
{
	fileStats_t stats;
	bffFile_t *bff;

	bff = FS_FindInCache( bffpath );
	if ( !bff ) {
		return NULL;
	}

	if ( !Sys_GetFileStats( &stats, bffpath ) ) {
		FS_RemoveFromCache( bff );
		FS_FreeBFF( bff );
		return NULL;
	}

	if ( bff->size != stats.size || bff->mtime != stats.mtime || bff->ctime != stats.ctime ) {
		// release outdated information
		FS_RemoveFromCache( bff );
		FS_FreeBFF( bff );
		return NULL;
	}

	return bff;
}

static void FS_InsertBFFToCache( bffFile_t *bff )
{
	fileStats_t stats;

	if ( Sys_GetFileStats( &stats, bff->bffFilename ) ) {
		bff->mtime = stats.mtime;
		bff->ctime = stats.ctime;
		FS_AddToCache( bff );
		bff->touched = qtrue;
	}
}

static void FS_ResetCacheReferences( void )
{
	bffFile_t *bff;
	uint64_t i;

	for ( i = 0; i < arraylen( bffHashTable ); i++ ) {
		bff = bffHashTable[i];
		while ( bff ) {
			bff->touched = qfalse;
			bff->referenced = 0;
			bff = bff->next;
		}
	}
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
#ifdef USE_BFF_CACHE_FILE
				fs_bffsReleased++;
#endif
			}
			bff = next;
		}
	}
}

#ifdef USE_BFF_CACHE_FILE

static void FS_WriteCacheHeader( FILE *f )
{
	fwrite( cache_header, sizeof( cache_header ), 1, f );
}

static qboolean FS_ValidateCacheHeader( FILE *f )
{
	byte buf[ sizeof(cache_header) ];

	if ( fread( buf, sizeof( buf ), 1, f ) != 1 ) {
		return qfalse;
	}

	if ( memcmp( buf, cache_header, sizeof( buf ) ) != 0 ) {
		return qfalse;
	}
	
	return qtrue;
}

static void FS_WriteCachedItems( const bffFile_t *bff, char *namePtr, FILE *f )
{
	// had to separate this because g++ refused to compile an if-statement in FS_SaveBFFToFile
	uint64_t i;
	bffCacheFileItem_t it;

	for ( i = 0; i < bff->numfiles; i++ ) {
		it.name = (uint64_t)( bff->buildBuffer[i].name - namePtr );
		it.size = bff->buildBuffer[i].size;
		it.pos = bff->buildBuffer[i].pos;
		fwrite( &it, sizeof( it ), 1, f );
	}
}

static qboolean FS_SaveBFFToFile( const bffFile_t *bff, FILE *f )
{
	uint64_t bffNameLen;
	const char *bffName;
	bffCacheHeader_t header;
	uint64_t namesLen, contentLen;
	char *namePtr;
	uint64_t i;
	bffCacheFileItem_t it;

	namePtr = (char *)( bff->buildBuffer + bff->numfiles );

	bffName = bff->bffFilename;
	bffNameLen = strlen( bffName ) + 1;
	bffNameLen = PAD( bffNameLen, sizeof( uintptr_t ) );

	namesLen = bffName - namePtr;

	// file content length
	contentLen = 0;

	memset( &header, 0, sizeof( header ) );

	// bff filename length
	header.bffNameLen = bffNameLen;
	// filenames length
	header.namesLen = namesLen;
	// number of files
	header.numFiles = bff->numfiles;
	// number of checksums
	header.numHeaderLongs = bff->numHeaderLongs;
	// creation/status change time
	header.ctime = bff->ctime;
	// modification time
	header.mtime = bff->mtime;
	// bff file size
	header.size = bff->size;

	// dump header
	fwrite( &header, sizeof( header ), 1, f );

	// bff filename
	fwrite( bffName, bffNameLen, 1, f );

	// filenames
	fwrite( namePtr, namesLen, 1, f );

	// file entries
	for ( i = 0; i < bff->numfiles; i++ ) {
		it.name = (uint64_t)( bff->buildBuffer[i].name - namePtr );
		it.size = bff->buildBuffer[i].size;
		it.compressedSize = bff->buildBuffer[i].compressedSize;
		it.pos = bff->buildBuffer[i].pos;
		fwrite( &it, sizeof( it ), 1, f );
	}

	// pure checksums, excluding first uninitialized
	fwrite( bff->headerLongs + 1, ( bff->numHeaderLongs - 1 ) * sizeof( bff->headerLongs[0] ), 1, f );

	return qtrue;
}

static qboolean FS_LoadBffFromFile( FILE *f )
{
	fileStats_t stats;
	fileInBFF_t *curFile;
	char bffName[ PAD( MAX_OSPATH*3+1, sizeof( int ) ) ];
	char bffBase[ PAD( MAX_OSPATH, sizeof( int ) ) ], *basename;
	char *filename_inbff;
	bffCacheHeader_t header;
	bffCacheFileItem_t it;
	bffFile_t *bff;
	char *namePtr;
	int size, i;
	int bffBaseLen;
	int hashSize;
	long hash;

	if ( !fread( &header, sizeof( header ), 1, f ) ) {
		return qfalse; // probably EOF
	}

	// validate header data

	if ( header.bffNameLen > sizeof( bffName ) || header.bffNameLen & 3 || header.bffNameLen == 0 ) {
		Con_Printf( "bad bffNameLen: %08lx\n", header.bffNameLen );
		return qfalse;
	}

	if ( header.namesLen & 3 || header.namesLen < header.numFiles ) {
		Con_Printf( "bad namesLen: %lu\n", header.namesLen );
		return qfalse;
	}

	if ( header.numHeaderLongs == 0 || header.numHeaderLongs > header.numFiles + 1 ) {
		Con_Printf( "bad numHeaderLongs: %lu\n", header.numHeaderLongs );
		return qfalse;
	}

	if ( header.contentLen & 3 || header.contentLen < 0 ) {
		Con_Printf( "bad contentLen: %lu\n", header.contentLen );
		return qfalse;
	}

	// load filename
	if ( !fread( bffName, header.bffNameLen, 1, f ) ) {
		Con_Printf( "error reading bffname\n" );
		return qfalse;
	}

	// bffName must be zero-terminated
	if ( bffName[ header.bffNameLen - 1 ] != '\0' ) {
		Con_Printf( "bffname is not zero-terminated!\n" );
		return qfalse;
	}

	if ( !Sys_GetFileStats( &stats, bffName ) || stats.mtime != header.mtime || stats.ctime != header.ctime ) {
		const int seek_len = header.namesLen + header.numFiles * sizeof( it ) + (header.numHeaderLongs-1) * sizeof( bff->headerLongs[0] ) + header.contentLen;
//		const int seek_len = header.namesLen + header.numFiles * sizeof( it ) + header.contentLen;
		if ( fseek( f, seek_len, SEEK_CUR ) != 0 ) {
			return qfalse;
		}
		else {
			fs_bffsSkipped++;
			return qtrue; // just outdated info, we can continue
		}
	}

	// extract basename from zip path
	basename = strrchr( bffName, PATH_SEP );
	if ( basename == NULL ) {
		basename = bffName;
	} else {
		basename++;
	}

	N_strncpyz( bffBase, basename, sizeof( bffBase ) );
	FS_StripExt( bffBase, ".header" );
	bffBaseLen = (int)strlen( bffBase ) + 1;
	bffBaseLen = PAD( bffBaseLen, sizeof( int ) );

	hashSize = FS_BFFHashSize( header.numFiles );

	size = sizeof( *bff ) + sizeof( *bff->buildBuffer ) * header.numFiles + hashSize * sizeof( *bff->hashTable ) + header.namesLen;
	size += header.bffNameLen;
	size += bffBaseLen;
	size += header.numHeaderLongs * sizeof( bff->headerLongs[0] );

	bff = (bffFile_t *)Z_Malloc( size, TAG_BFF );
	memset( bff, 0, size );

	bff->mtime = header.mtime;
	bff->ctime = header.ctime;
	bff->size = header.size;

//	bff->handle = f;
	bff->numfiles = header.numFiles;
	bff->numHeaderLongs = header.numHeaderLongs;

	// setup memory layout
	bff->hashSize = hashSize;
	bff->hashTable = (fileInBFF_t **)( bff + 1 );

	bff->buildBuffer = (fileInBFF_t *)( bff->hashTable + bff->hashSize );

	namePtr = (char *)( bff->buildBuffer + bff->numfiles );

	bff->bffFilename = (char *)( namePtr );
	bff->bffBasename = (char *)( bff->bffFilename + header.bffNameLen );
	bff->headerLongs = (uint32_t *)( bff->bffBasename + bffBaseLen );

	strcpy( bff->bffFilename, bffName );
	strcpy( bff->bffBasename, bffBase );
	
	if ( !fread( namePtr, header.namesLen, 1, f ) ) {
		Con_Printf( "error reading bff filenames\n" );
		goto __error;
	}

	// filenames buffer must be zero-terminated
	if ( namePtr[ header.namesLen - 1 ] != '\0' ) {
		Con_Printf( "not zero terminated filenames\n" );
		goto __error;
	}

	curFile = bff->buildBuffer;
	for ( i = 0; i < header.numFiles; i++ ) {
		if ( !fread( &it, sizeof( it ), 1, f ) ) {
			Con_Printf( "error reading file item[%i]\n", i );
			goto __error;
		}
		if ( it.name >= header.namesLen ) {
			Con_Printf( "bad name offset: %lu (expecting less than %lu)\n", it.name, header.namesLen );
			goto __error;
		}

		filename_inbff = namePtr + it.name;
		FS_ConvertFilename( filename_inbff );
//		if ( !FS_BannedbffFile( filename_inzip ) ) {
			// store the file position in the zip
			curFile->name = filename_inbff;
			curFile->size = it.size;
			curFile->pos = it.pos;
			curFile->compressedSize = it.compressedSize;

			// update hash table
			hash = FS_HashFileName( filename_inbff, bff->hashSize );
			curFile->next = bff->hashTable[ hash ];
			bff->hashTable[ hash ] = curFile;
			curFile++;
//		} else {
//			bff->numfiles--;
//		}
	}

	if ( !fread( bff->headerLongs + 1, ( bff->numHeaderLongs - 1 ) * sizeof( bff->headerLongs[0] ), 1, f ) ) {
		Con_Printf( COLOR_YELLOW "WARNING: couldn't read headerLongs\n" );
//		goto __error;
	}

	bff->checksumFeed = fs_checksumFeed;
	bff->headerLongs[ 0 ] = LittleInt( fs_checksumFeed );

	bff->checksum = Com_BlockChecksum( bff->headerLongs + 1, sizeof( bff->headerLongs[0] ) * ( bff->numHeaderLongs - 1 ) );
	bff->checksum = LittleInt( bff->checksum );

	bff->pure_checksum = Com_BlockChecksum( bff->headerLongs, sizeof( bff->headerLongs[0] ) * bff->numHeaderLongs );
	bff->pure_checksum = LittleInt( bff->pure_checksum );

	// seek through unused content
	if ( header.contentLen > 0 ) {
		if ( fseek( f, header.contentLen, SEEK_CUR ) != 0 ) {
			goto __error;
		}
	}
	else if ( header.contentLen < 0 ) {
		goto __error;
	}

	fs_bffsCached++;

	FS_InsertBFFToCache( bff );

	return qtrue;

__error:
	FS_FreeBFF( bff );
	return qfalse;
}

/*
============
FS_SaveCache

Called at the end of FS_Startup() after releasing unused bffs
============
*/
static qboolean FS_SaveCache( void )
{
	const char *filename = CACHE_FILE_NAME;
	const char *ospath;
	const searchpath_t *sp;
	FILE *fp;

	if ( !fs_searchpaths ) {
		return qfalse;
	}

	if ( !fs_cacheLoaded ) {
		Con_DPrintf( "synced FS cache on startup\n" );
		fs_cacheSynced = qfalse;
		fs_cacheLoaded = qtrue;
	}
	else if ( CACHE_SYNC_CONDITION ) {
		Con_DPrintf( "synced FS cache on readed=%lu, release=%lu, skipped=%lu\n",
			fs_bffsReaded, fs_bffsReleased, fs_bffsSkipped );
		fs_cacheSynced = qfalse;
	}

	if ( fs_cacheSynced ) {
		return qtrue;
	}

	sp = fs_searchpaths;

	ospath = FS_BuildOSPath( fs_homepath->s, NULL, filename );

	fp = Sys_FOpen( ospath, "wb" );
	if ( fp == NULL ) {
		return qfalse;
	}

	FS_WriteCacheHeader( fp );

	while ( sp != NULL ) {
		if ( sp->bff ) {
			FS_SaveBFFToFile( sp->bff, fp );
		}
		sp = sp->next;
	}

	fclose( fp );

	fs_bffsReleased = 0;
	fs_bffsSkipped = 0;
	fs_bffsReaded = 0;

	fs_cacheSynced = qtrue;

	return qtrue;
}

/*
============
FS_LoadCache

Called at FS_Startup() before loading any header3 file
============
*/
static void FS_LoadCache( void )
{
	const char *filename = CACHE_FILE_NAME;
	const char *ospath;
	FILE *fp;

	fs_bffsReaded = 0;
	fs_bffsReleased = 0;

	if ( fs_cacheLoaded ) {
		return;
	}

	fs_bffsCached = 0;
	fs_bffsSkipped = 0;

	ospath = FS_BuildOSPath( fs_homepath->s, NULL, filename );

	Con_Printf( "Loading cached bffs from '%s'...\n", ospath );
	fp = Sys_FOpen( ospath, "rb" );
	if ( fp == NULL ) {
		Con_DPrintf( "Couldn't open cache file '%s'\n", ospath );
		return;
	}

	if ( !FS_ValidateCacheHeader( fp ) ) {
		fclose( fp );
		Con_DPrintf( "Invalid heaer in cache file '%s'\n", ospath );
		return;
	}

	while ( FS_LoadBffFromFile( fp ) )
		;
	fclose( fp );

	fs_cacheLoaded = qtrue;

	Con_Printf( "...found %lu cached bffs\n", fs_bffsCached );
}

#endif // USE_BFF_CACHE_FILE

#endif // USE_BFF_CACHE

/*
==========================================================

BFF FILE LOADING

==========================================================
*/

/*
* FS_LoadBFF: creates a new bffFile_t in the search chain for the contents of a bff archive file
*/
static bffFile_t *FS_LoadBFF( const char *bffpath )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileInBFF_t *curFile;
	bffFile_t *bff;
	bffheader_t header;
	fileInBFF_t *curChunk;
	uint64_t i, hashSize, size;
	uint64_t hash;
	uint64_t chunkCount;
	const char *basename;
	char gameName[MAX_BFF_PATH], *namePtr;
#ifdef USE_ZIP
	char filename_inzip[256];
#else
	char filename_inbff[MAX_STRING_CHARS];
#endif
	uint64_t chunkSize;
	uint64_t gameNameLen;
	uint64_t baseNameLen, fileNameLen;
	uint64_t compressedSize;
	FILE *fp;
	fileStats_t stats;
	char *tempBuf;
	uint64_t outSize, pos;
	int64_t tmp;
	uint64_t namelen, filecount;
	uint32_t fs_numHeaderLongs;
	uint32_t *fs_headerLongs;
#ifdef USE_ZIP
	zip_t *zip;
	zip_source_t *source;
	zip_stat_t *fdata;
	zip_error_t error;
	int err;
#endif

	PROFILE_FUNCTION();

#ifdef USE_BFF_CACHE
	bff = FS_LoadCachedBFF( bffpath );
	if ( bff ) {
		// update pure checksum
		if ( bff->checksumFeed != fs_checksumFeed ) {
			bff->headerLongs[ 0 ] = LittleLong( fs_checksumFeed );
			bff->pure_checksum = Com_BlockChecksum( bff->headerLongs, sizeof( bff->headerLongs[0] ) * bff->numHeaderLongs );
			bff->pure_checksum = LittleLong( bff->pure_checksum );
			bff->checksumFeed = fs_checksumFeed;
		}

		bff->touched = qtrue;
		return bff; // loaded from cache
	}
#endif

	// extract the basename from bffpath
	basename = strrchr( bffpath, PATH_SEP );
	if ( basename == NULL ) {
		basename = bffpath;
	} else {
		basename++;
	}
	
	fileNameLen = strlen( bffpath ) + 1;
	baseNameLen = strlen( basename ) + 1;

#ifdef USE_ZIP
	fdata = (zip_stat_t *)alloca( sizeof( *fdata ) );

	zip = zip_open( bffpath, ZIP_RDONLY, &err );
	if ( !zip ) {
		zip_error_init_with_code( &error, err );
		Con_Printf( COLOR_RED "FS_LoadBFF: failed to open bff %s (zip_open): %s\n", bffpath, error.str );
		zip_error_fini( &error );
		return NULL;
	}
#endif

	if ( !Sys_GetFileStats( &stats, bffpath ) ) {
		N_Error( ERR_DROP, "FS_LoadBFF: failed to load bff %s", bffpath );
	}

	chunkCount = 0;
	fp = NULL;

	// if the file is really heavy, map it, otherwise, open it normally
/*
	if ( stats.size >= BFF_MAPFILE_SIZE ) {
		file = Sys_MapFile( bffpath, qtrue ); // create a temporary file mapping, don't save it to disk
		if ( !file ) {
			N_Error( ERR_FATAL, "FS_LoadBFF: failed to create a memory mapping of bff %s", bffpath );
			return NULL;
		}
		stream = BFF_STREAM_MAPPED;
		streamPtr = (void *)file;
	}
	else
*/
#ifndef USE_ZIP
	{
		fp = Sys_FOpen( bffpath, "rb" );
		if ( !fp ) {
			N_Error( ERR_FATAL, "FS_LoadBFF: failed to open bff %s in readonly mode", bffpath );
			return NULL;
		}
	}

	if ( !fread( &header, sizeof( header ), 1, fp ) ) {
		fclose( fp );
		N_Error( ERR_FATAL, "FS_LoadBFF: failed to read header for '%s'", bffpath );
	}

	if ( header.ident != BFF_IDENT ) {
		Con_DPrintf( "FS_LoadBFF: bad identifier, '%s'\n", bffpath );
		fclose( fp );
		return NULL;
	}
	if ( header.magic != HEADER_MAGIC ) {
		Con_DPrintf( "FS_LoadBFF: bad header magic, '%s'\n", bffpath );
		fclose( fp );
		return NULL;
	}
	if ( !header.numChunks ) {
		Con_DPrintf( "FS_LoadBFF: funny chunk count, '%s'\n", bffpath );
		fclose( fp );
		return NULL;
	}

	// technically not an error, but could cause some severe issues if the
	// version gap is large enough
	if ( header.version != BFF_VERSION ) {
		Con_Printf(
			COLOR_YELLOW "==== WARNING: bff version found in header isn't the same as this program's ====\n" COLOR_RESET
			"\tHeader Version: %hi\n\tProgram BFF Version: %hi\n", header.version, BFF_VERSION );
	}
	if ( !fread( gameName, sizeof( gameName ), 1, fp ) ) {
		fclose( fp );
		N_Error( ERR_FATAL, "FS_LoadBFF: failed to read gameName" );
	}
#endif

	namelen = 0;
	filecount = 0;
#ifdef USE_ZIP
	header.numChunks = 0;
	zip_stat_init( fdata );
	while ( ( zip_stat_index( zip, header.numChunks, 0, fdata ) ) == 0 ) {
		strcpy( filename_inzip, fdata->name );
		filename_inzip[ sizeof( filename_inzip ) - 1 ] = '\0';
		if ( fdata->comp_method != 0 && fdata->comp_method != 8 /*Z_DEFLATED*/ ) {
			Con_Printf( COLOR_YELLOW "%s|%s: unsupported compression method %i\n", basename, filename_inzip, (int)fdata->comp_method );
			continue;
		}
		namelen += strlen( filename_inzip ) + 1;
		filecount++;
		header.numChunks++;
	}
#else
	pos = ftell( fp );
	for ( i = 0; i < header.numChunks; i++ ) {
		if ( !fread( &tmp, sizeof( tmp ), 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk nameLen at %lu\n", i );
			return NULL;
		}
		tmp = LittleLong( tmp );
		Assert( tmp < sizeof( filename_inbff ) );
		if ( !fread( filename_inbff, tmp, 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk name at %lu\n", i );
			return NULL;
		}
		if ( !fread( &tmp, sizeof( tmp ), 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk size at %lu\n", i );
			return NULL;
		}
		if ( header.compression != COMPRESS_NONE ) {
			if ( !fread( &compressedSize, sizeof( compressedSize ), 1, fp ) ) {
				fclose( fp );
				Con_Printf( COLOR_RED "ERROR: failed reading chunk compressedSize size at %lu\n", i );
				return NULL;
			}
			tmp = compressedSize;
		}
		fseek( fp, tmp, SEEK_CUR );

		filename_inbff[ sizeof( filename_inbff ) - 1 ] = '\0';
		namelen += strlen( filename_inbff ) + 1;
		filecount++;
	}
#endif
	if ( filecount == 0 ) {
#ifdef USE_ZIP
		zip_close( zip );
#else
		fclose( fp );
#endif
		return NULL;
	}

	hashSize = FS_BFFHashSize( filecount );

	namelen = PAD( namelen, sizeof( uintptr_t ) );
	size = sizeof( *bff ) + sizeof( *bff->buildBuffer ) * header.numChunks + hashSize * sizeof( *bff->hashTable ) + namelen;
	size += PAD( fileNameLen, sizeof( uintptr_t ) );
	size += PAD( baseNameLen, sizeof( uintptr_t ) );
#ifdef USE_BFF_CACHE
	size += ( filecount + 1 ) * sizeof( fs_headerLongs[0] );
#endif
	bff = (bffFile_t *)Z_Malloc( size, TAG_BFF );
	memset( bff, 0, size );

#ifdef USE_ZIP
	bff->handle = zip;
#else
	bff->handle = fp;
#endif
	bff->hashSize = hashSize;
	bff->numfiles = filecount;
	bff->handlesUsed = 0;
	bff->hashTable = (fileInBFF_t **)( bff + 1 );

	// setup memory layout
	bff->buildBuffer = (fileInBFF_t *)( bff->hashTable + bff->hashSize );
	namePtr = (char *)( bff->buildBuffer + filecount );

	bff->bffFilename = (char *)( namePtr + namelen );
	bff->bffBasename = (char *)( bff->bffFilename + PAD( fileNameLen, sizeof( uintptr_t ) ) );

#ifdef USE_BFF_CACHE
	fs_headerLongs = (uint32_t *)( bff->bffBasename + PAD( baseNameLen, sizeof( uintptr_t ) ) );
#else
	fs_headerLongs = (uint32_t *)Z_Malloc( ( filecount + 1 ) * sizeof( fs_headerLongs[0] ), TAG_BFF );
#endif

	fs_numHeaderLongs = 0;
	fs_headerLongs[ fs_numHeaderLongs++ ] = LittleInt( fs_checksumFeed );

	memcpy( bff->bffGamename, gameName, sizeof( bff->bffGamename ) );
	memcpy( bff->bffFilename, bffpath, fileNameLen );
	memcpy( bff->bffBasename, basename, baseNameLen );

	// strip the .bff if needed
	FS_StripExt( bff->bffBasename, ".bff" );

#ifdef USE_ZIP
#else
	fseek( fp, pos, SEEK_SET );
#endif

	curFile = bff->buildBuffer;
#ifdef USE_ZIP
	tmp = 0;
	while ( ( zip_stat_index( zip, tmp, 0, fdata ) ) == 0 ) {
		strcpy( filename_inzip, fdata->name );
		filename_inzip[ sizeof( filename_inzip ) - 1 ] = '\0';
		if ( fdata->comp_method != 0 && fdata->comp_method != ZIP_CM_DEFLATE ) {
			continue;
		}
		if ( fdata->size > 0 ) {
			fs_headerLongs[ fs_numHeaderLongs++ ] = LittleInt( fdata->crc );
		}

		FS_ConvertFilename( filename_inzip );

		// store the file position in the zip
		curFile->pos = zip_name_locate( zip, namePtr, ZIP_FL_UNCHANGED );
		curFile->file = NULL;
		curFile->size = fdata->size;
		curFile->name = namePtr;
		strcpy( curFile->name, filename_inzip );
		namePtr += strlen( filename_inzip ) + 1;

		// update has table
		hash = FS_HashFileName( filename_inzip, bff->hashSize );
		curFile->next = bff->hashTable[ hash ];
		bff->hashTable[ hash ] = curFile;
		curFile++;
		tmp++;
	}
#else
	for ( i = 0; i < bff->numfiles; i++ ) {
		if ( !fread( &curFile->nameLen, sizeof( curFile->nameLen ), 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk nameLen at %lu\n", i );
			return NULL;
		}
		curFile->nameLen = LittleLong( curFile->nameLen );
		if ( !fread( filename_inbff, curFile->nameLen, 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk name at %lu\n", i );
			return NULL;
		}
		if ( !fread( &curFile->size, sizeof( curFile->size ), 1, fp ) ) {
			fclose( fp );
			Con_Printf( COLOR_RED "ERROR: failed reading chunk size at %lu\n", i );
			return NULL;
		}
		if ( header.compression != COMPRESS_NONE ) {
			if ( !fread( &compressedSize, sizeof( compressedSize ), 1, fp ) ) {
				fclose( fp );
				Con_Printf( COLOR_RED "ERROR: failed reading chunk size at %lu\n", i );
				return NULL;
			}
			curFile->compressedSize = compressedSize;
		} else {
			curFile->compressedSize = curFile->size;
		}
		curFile->size = LittleLong( curFile->size );

//		Con_DPrintf( "Chunk[%lu]: %s (%lu) %lu\n", i, filename_inbff, curFile->nameLen, curFile->size );

		filename_inbff[ sizeof( filename_inbff ) - 1 ] = '\0';
		FS_ConvertFilename( filename_inbff );

		// store the file position in the bff
		curFile->pos = ftell( fp );
		curFile->name = namePtr;
		curFile->compression = header.compression;
		strcpy( curFile->name, filename_inbff );
		namePtr += strlen( filename_inbff ) + 1;

		fseek( fp, curFile->compressedSize, SEEK_CUR );

		// update hash table
		hash = FS_HashFileName( filename_inbff, bff->hashSize );
		curFile->next = bff->hashTable[ hash ];
		bff->hashTable[ hash ] = curFile;
		curFile++;
	}
#endif

//	if ( fs_numHeaderLongs ) {
//		bff->checksum = Com_BlockChecksum( fs_headerLongs + 1, sizeof( fs_headerLongs[0] ) * ( fs_numHeaderLongs - 1 ) );
//		bff->checksum = LittleInt( bff->checksum );
//
//		bff->pure_checksum = Com_BlockChecksum( fs_headerLongs, sizeof( fs_headerLongs[0] ) * fs_numHeaderLongs );
//		bff->pure_checksum = LittleLong( bff->pure_checksum );
//	}
//	else {
		bff->checksum = crc32_buffer( (const byte *)bff->bffBasename, sizeof( baseNameLen ) );
//	}

#ifdef USE_BFF_CACHE
	bff->headerLongs = fs_headerLongs;
	bff->numHeaderLongs = fs_numHeaderLongs;
	bff->checksumFeed = fs_checksumFeed;
#else
	Z_Free( fs_headerLongs );
#endif

#ifdef USE_HANDLE_CACHE
	FS_AddToHandleList( bff );
#else
	if ( fs_locked->i == 0 ) {
	#ifdef USE_ZIP
		zip_close( bff->handle );
	#else
		fclose( bff->handle );
	#endif
		bff->handle = NULL;
	}
#endif

	Con_Printf( "Loaded bff resource file with crc32 block checksum %u\n", bff->checksum );
	
#ifdef USE_BFF_CACHE
	FS_InsertBFFToCache( bff );
#ifdef USE_BFF_CACHE_FILE
	fs_bffsReaded++;
#endif
#endif

	return bff;
}

/*
=================
FS_FreeBFF

Frees a bff structure and releases all associated resources
=================
*/
static void FS_FreeBFF( bffFile_t *bff )
{
	fileInBFF_t *file;
	uint64_t i;

	if ( !bff ) {
		N_Error( ERR_FATAL, "FS_FreeBFF(NULL)" );
	}

	if ( bff->handle ) {
#ifdef USE_HANDLE_CACHE
		if ( bff->next_h ) {
			FS_RemoveFromHandleList( bff );
		}
#endif
	#ifdef USE_ZIP
		zip_close( bff->handle );
	#else
		fclose( bff->handle );
	#endif
		bff->handle = NULL;

		for ( file = bff->buildBuffer, i = 0; i < bff->numfiles; file = bff->buildBuffer->next, i++ ) {
#ifndef USE_ZIP
			if ( file->buf ) {
				Z_Free( file->buf );
			}
#else
			if ( file->file ) {
				zip_fclose( file->file );
			}
#endif
		}
	}

	Z_Free( bff );
}

//
// FS_WriteFile: filename is relative to glnomad's search paths
//
void FS_WriteFile(const char *npath, const void *data, uint64_t size)
{
	fileHandle_t  f;

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

void FS_ForceFlush(fileHandle_t f)
{
	fileHandleData_t *fh;

	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		N_Error(ERR_FATAL, "FS_ForceFlush: out of range");
	}

	fh = &handles[f];
	if ( fh->data.fp ) {
//		setvbuf( fh->data.fp, NULL, _IONBF, 0 );
		fflush( fh->data.fp ); // better to fflush?
	}
	else {
		Con_DPrintf( "FS_ForceFlush: not open\n" );
	}
}

fileOffset_t FS_FileTell( fileHandle_t f )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandleData_t *p;

	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		N_Error(ERR_FATAL, "FS_FileTell: out of range");
	}

	p = &handles[f];

	// not a unique file
	if ( p->bffFile ) {
#ifdef USE_ZIP
#else
		return (fileOffset_t)( p->data.chunk->size - p->data.chunk->bytesRead );
#endif
	}
	
	// normal file pointer
	return (fileOffset_t)ftell( p->data.fp );
}

#define BFF_SEEK_BUFFER_SIZE 65536

fileOffset_t FS_FileSeek( fileHandle_t f, fileOffset_t offset, uint32_t whence )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandleData_t *file;
	uint32_t fwhence;
	
	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		N_Error( ERR_FATAL, "FS_FileSeek: out of range" );
	}
	
	file = &handles[f];

	// dirty fix for a bug
	if ( !N_stricmp( "backtrace.dat", file->name ) ) {
		return (fileOffset_t)ftell( file->data.fp );
	}

	if ( file->bffFile ) {
	#ifdef USE_ZIP
		switch ( whence ) {
		case FS_SEEK_SET:
			return zip_fseek( file->data.chunk->file, offset, SEEK_SET );
		case FS_SEEK_END:
			return zip_fseek( file->data.chunk->file, offset, SEEK_END );
		case FS_SEEK_CUR:
			return zip_fseek( file->data.chunk->file, offset, SEEK_CUR );
		default:
			N_Error( ERR_FATAL, "Bad origin in FS_Seek" );
			return -1;
		};
	#else
		if ( whence == FS_SEEK_END && offset ) {
			return -1;
		}
		else if ( whence == FS_SEEK_CUR && file->data.chunk->bytesRead + offset >= file->data.chunk->size ) {
			return -1;
		}
		switch ( whence ) {
		case FS_SEEK_CUR:
			file->data.chunk->bytesRead += offset;
			break;
		case FS_SEEK_BEGIN:
			file->data.chunk->bytesRead = offset;
			break;
		case FS_SEEK_END:
			file->data.chunk->bytesRead = file->data.chunk->size - offset;
			break;
		default:
			N_Error( ERR_FATAL, "FS_FileSeek: invalid seek" );
		};
		return (fileOffset_t)( file->data.chunk->size - file->data.chunk->bytesRead );
	#endif
	}

	switch ( whence ) {
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
		N_Error( ERR_FATAL, "FS_FileSeek: invalid seek" );
	};
	return (fileOffset_t)fseek( file->data.fp, (long)offset, (int)fwhence );
}

uint64_t FS_FileLength( fileHandle_t f )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	uint64_t curPos, length;

	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		N_Error( ERR_FATAL, "FS_FileLength: out of range" );
	}

	if ( handles[f].bffFile ) {
		return handles[f].data.chunk->size;
	}

	curPos = FS_FileTell( f );
	FS_FileSeek( f, 0, FS_SEEK_END );
	length = FS_FileTell( f );
	FS_FileSeek( f, curPos, FS_SEEK_BEGIN );
	return length;
}

/*
* FS_Write: properly handles partial writes
*/
uint64_t FS_Write( const void *buffer, uint64_t size, fileHandle_t f )
{
	int64_t writeCount, remaining, block;
	const byte *buf;
	int tries;
	FILE *fp;

	if (!fs_searchpaths) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}

	fp = FS_FileForHandle( f );

	buf = (const byte *)buffer;
	remaining = size;
	tries = 0;

	while (remaining) {
		block = remaining;
		writeCount = fwrite( buf, 1, block, fp );
		if (writeCount == 0) {
			if (!tries) {
				tries = 1;
			}
			else {
				Con_Printf( COLOR_RED "FS_Write: 0 bytes written\n" );
				return 0;
			}
		}

		if (writeCount == -1) {
			Con_Printf( COLOR_RED "FS_Write: -1 bytes written\n" );
			return 0;
		}

		buf += writeCount;
		remaining -= writeCount;
		fs_writeCount += writeCount;
	}
	if ( handles[f].handleSync ) {
		fflush( fp );
	}

	return size;
}

#ifndef USE_ZIP
static uint64_t FS_ReadFromChunk( void *buffer, uint64_t size, fileHandle_t f )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandleData_t *handle = &handles[f];

	if ( !handle->data.chunk->buf ) {
		if ( handle->data.chunk->compression != COMPRESS_NONE ) {
			uint64_t outLen;
			int64_t compressedSize;
			char *tempBuf;

			outLen = handle->data.chunk->size;
			tempBuf = (char *)Hunk_AllocateTempMemory( handle->data.chunk->compressedSize );

			fseek( handle->bff->handle, handle->data.chunk->pos, SEEK_SET );
			if ( !fread( tempBuf, handle->data.chunk->compressedSize, 1, handle->bff->handle ) ) {
				N_Error( ERR_FATAL, "Error reading chunk buffer at %s", handle->name );
			}
			handle->data.chunk->buf = Decompress( tempBuf, handle->data.chunk->compressedSize, &outLen, handle->data.chunk->compression );
			handle->data.chunk->size = outLen;
			Hunk_FreeTempMemory( tempBuf );
		}
		else {
			// read the chunk data
			handle->data.chunk->buf = (char *)Z_Malloc( handle->data.chunk->size, TAG_BFF );
			fseek( handle->bff->handle, handle->data.chunk->pos, SEEK_SET );
			if ( !fread( handle->data.chunk->buf, handle->data.chunk->size, 1, handle->bff->handle ) ) {
				fclose( handle->bff->handle );
				N_Error( ERR_DROP, "Error reading chunk buffer at %lu\n", (uint64_t)( handle->bff->buildBuffer - handle->data.chunk ) );
			}
			handle->data.chunk->bytesRead = 0;
		}
	}

	if ( handle->data.chunk->bytesRead + size > handle->data.chunk->size ) {
#if 0
		if (size >= handle->data.chunk->size) {
			N_Error( ERR_FATAL, "FS_ReadFromChunk: size >= chunk size" );
		}
		else {
			uint64_t amount = (handle->data.chunk->bytesRead + size) - handle->data.chunk->size;
			Con_DPrintf( "WARNING: chunk overread of %lu bytes\n", amount );
			size = amount;
		}
#else
		N_Error( ERR_FATAL, "FS_ReadFromChunk: overread of %lu bytes\n",
			( handle->data.chunk->bytesRead + size ) - handle->data.chunk->size );
#endif
	}

	fseek( handle->bff->handle, handle->data.chunk->pos + handle->data.chunk->bytesRead, SEEK_SET );
	memcpy( buffer, handle->data.chunk->buf + handle->data.chunk->bytesRead, size );
	handle->data.chunk->bytesRead += size;
	return size;
}
#endif

/*
=================
FS_Read

Properly handles partial reads
=================
*/
uint64_t FS_Read( void *buffer, uint64_t size, fileHandle_t f )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	int64_t readCount, remaining, block;
	byte *buf;
	int tries;

	if (!fs_searchpaths) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES) {
		return 0;
	}

	buf = (byte *)buffer;
	fs_readCount += size;

	if ( !handles[f].bffFile || !N_stricmp( COM_GetExtension( handles[f].name ), "ngd" ) ) {
		remaining = size;
		tries = 0;

		while (remaining) {
			block = remaining;
			readCount = fread( buf, 1, block, handles[f].data.fp );
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

			if ( readCount == -1 ) {
				N_Error( ERR_FATAL, "FS_Read: -1 bytes read" );
			}

			remaining -= readCount;
			buf += readCount;
		}
		return size;
	}
	else {
#ifdef USE_ZIP
		return zip_fread( handles[f].data.chunk->file, buffer, size );
#else
		return FS_ReadFromChunk( buffer, size, f );
#endif
	}
}

static void FS_PrintSearchPaths( void )
{
	const searchpath_t *sp = fs_searchpaths;

	Con_Printf( "\nSearch Paths:\n" );
	while ( sp ) {
		if ( sp->dir && sp->access == DIR_STATIC ) {
			Con_Printf( " * %s\n", sp->dir->path );
		}
		sp = sp->next;
	}
}

/*
===================
FS_CheckGDRBffs

Checks that bff0.bff is present and its checksum is correct
Note: If you're building a game that doesn't depend on the
The Nomad media bff0.bff, you'll want to remove this function
===================
*/
static void FS_CheckGDRBffs( void )
{
	const searchpath_t *path;
	const char *bffBasename;
	qboolean founddemo = qfalse;
	unsigned foundBff = 0;

	for ( path = fs_searchpaths; path; path = path->next ) {
		if ( !path->bff ) {
			continue;
		}

		bffBasename = path->bff->bffBasename;

		if ( !N_stricmpn( path->bff->bffGamename, BASEDEMO_DIR, MAX_OSPATH )
		   && !N_stricmpn( bffBasename, "bff0", MAX_OSPATH ) )
		{
			founddemo = qtrue;

			if ( path->bff->checksum == DEMO_BFF0_CHECKSUM ) {
				Con_Printf( "\n\n"
						"**************************************************\n"
						"WARNING: It looks like you're using bff0.bff\n"
						"from the demo. This may work fine, but it is not\n"
						"guaranteed or supported.\n"
						"**************************************************\n\n\n" );
			}
		}
		else if ( !N_stricmpn( path->bff->bffGamename, BASEGAME_DIR, MAX_OSPATH )
			&& strlen( bffBasename ) == 4 && !N_stricmpn( bffBasename, "bff", 3 )
			&& bffBasename[3] >= '0' && bffBasename[3] <= '8' )
		{
			if ( path->bff->checksum != bff_checksums[bffBasename[3]-'0'] ) {
				FS_PrintSearchPaths();

				if ( bffBasename[3] == '0' ) {
					Con_Printf("\n\n"
						"**************************************************\n"
						"ERROR: bff0.bff is present but its checksum (%u)\n"
						"is not correct. Please re-copy bff0.bff from your\n"
						"legitimate installation.\n"
						"**************************************************\n\n\n",
						path->bff->checksum );
				}
				else {
					Con_Printf( "\n\n"
						"**************************************************\n"
						"ERROR: bff%d.bff is present but its checksum (%u)\n"
						"is not correct. Please re-install " GLN_VERSION " \n"
						"bff files\n"
						"**************************************************\n\n\n",
						bffBasename[3]-'0', path->bff->checksum );
				}
				N_Error( ERR_FATAL, "\n* You need to install correct The Nomad files in order to play *" );
			}

			foundBff |= 1 << ( bffBasename[3] - '0' );
		}
	}

	if ( !founddemo && ( foundBff & 0x1ff ) != 0x1ff ) {
		FS_PrintSearchPaths();

		if ( ( foundBff & 1 ) != 1 ) {
			Con_Printf( "\n\n"
			"bff0.bff is missing. Please copy it\n"
			"from your legitimate installation.\n");
		}

		if ( ( foundBff & 0x1fe ) != 0x1fe ) {
			Con_Printf( "\n\n"
			GLN_VERSION " files are missing. Please\n"
			"re-install the game.\n");
		}

		Con_Printf( "\n\n"
			"Also check that your TheNomad.x64 executable is in\n"
			"the correct place and that every file\n"
			"in the %s directory is present and readable.\n", BASEGAME_DIR );

		if ( !fs_gamedirvar->s[0] || !N_stricmp( fs_gamedirvar->s, BASEGAME_DIR ) ) {
			N_Error( ERR_FATAL, "\n*** you need to install The Nomad in order to play ***" );
		}
	}
}


/*
=====================
FS_LoadedBFFChecksums

Returns a space separated string containing the checksums of all loaded bff files.
=====================
*/
const char *FS_LoadedBFFChecksums( qboolean *overflowed ) {
	static char	info[BIG_INFO_STRING];
	const searchpath_t *search;
	char buf[ 32 ];
	char *s, *max;
	size_t len;

	s = info;
	info[0] = '\0';
	max = &info[sizeof( info ) - 1];
	*overflowed = qfalse;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a bff file?
		if ( !search->bff ) {
			continue;
		}

		if ( search->bff->exclude ) {
			continue;
		}

		if ( info[0] ) {
			len = sprintf( buf, " %i", search->bff->checksum );
		} else {
			len = sprintf( buf, "%i", search->bff->checksum );
		}

		if ( s + len > max ) {
			*overflowed = qtrue;
			break;
		}

		s = N_stradd( s, buf );
	}

	return info;
}


/*
=====================
FS_LoadedBFFNames

Returns a space separated string containing the names of all loaded bff files.
=====================
*/
const char *FS_LoadedBFFNames( void ) {
	static char	info[BIG_INFO_STRING];
	const searchpath_t *search;
	char *s, *max;
	size_t len;

	s = info;
	info[0] = '\0';
	max = &info[sizeof( info ) - 1];

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a bff file?
		if ( !search->bff )
			continue;

		if ( search->bff->exclude )
			continue;

		len = strlen( search->bff->bffBasename );
		if ( info[0] ) {
			len++;
		}

		if ( s + len > max )
			break;

		if ( info[0] ) {
			s = N_stradd( s, " " );
		}

		s = N_stradd( s, search->bff->bffBasename );
	}

	return info;
}


/*
=====================
FS_ReferencedBFFChecksums

Returns a space separated string containing the checksums of all referenced bff files.
=====================
*/
const char *FS_ReferencedBFFChecksums( void ) {
	static char	info[BIG_INFO_STRING];
	const searchpath_t *search;

	info[0] = '\0';

	for ( search = fs_searchpaths; search; search = search->next ) {
		// is the element a bff file?
		if ( search->bff ) {
			if ( search->bff->exclude ) {
				continue;
			}
			if ( search->bff->referenced || !FS_IsBaseGame( search->bff->bffGamename ) ) {
				N_strcat( info, sizeof( info ), va( "%i ", search->bff->checksum ) );
			}
		}
	}

	return info;
}


/*
=====================
FS_ReferencedBFFPureChecksums

Returns a space separated string containing the pure checksums of all referenced bff files.
If g_validate_purity is enabled, we'll use this for validation 

The string has a specific order, "sgame ui @ ref1 ref2 ref3 ..."
=====================
*/
const char *FS_ReferencedBFFPureChecksums( uint64_t maxlen ) {
	static char	info[ MAX_STRING_CHARS*2 ];
	char *s, *max;
	const searchpath_t	*search;
	int nFlags, numbffs, checksum;

	max = info + maxlen; // maxlen is always smaller than MAX_STRING_CHARS so we can overflow a bit
	s = info;
	*s = '\0';

	checksum = fs_checksumFeed;
	numbffs = 0;
	for ( nFlags = FS_SGAME_REF; nFlags; nFlags = nFlags >> 1 ) {
		if ( nFlags & FS_GENERAL_REF ) {
			// add a delimiter between must haves and general refs
			s = N_stradd( s, "@ " );
			if ( s > max ) // client-side overflow
				break;
		}
		for ( search = fs_searchpaths ; search ; search = search->next ) {
			// is the element a bff file and has it been referenced based on flag?
			if ( search->bff && ( search->bff->referenced & nFlags ) ) {
				s = N_stradd( s, va( "%i ", search->bff->pure_checksum ) );
				if ( s > max ) // client-side overflow
					break;
				if ( nFlags & (FS_SGAME_REF | FS_UI_REF) ) {
					break;
				}
				checksum ^= search->bff->pure_checksum;
				numbffs++;
			}
		}
	}

	// last checksum is the encoded number of referenced header3s
	checksum ^= numbffs;
	s = N_stradd( s, va( "%i ", checksum ) );
	if ( s > max ) { 
		// client-side overflow
		Con_Printf( COLOR_YELLOW "WARNING: pure checksum list is too long (%lu), you might be not able to play!\n", (uintptr_t)( s - info ) );
		*max = '\0';
	}
	
	return info;
}


/*
=====================
FS_ExcludeReference
=====================
*/
qboolean FS_ExcludeReference( void ) {
	const searchpath_t *search;
	const char *bffName;
	int i, nargs;
	qboolean x;

	if ( fs_excludeReference->s[0] == '\0' ) {
		return qfalse;
	}

	Cmd_TokenizeStringIgnoreQuotes( fs_excludeReference->s );
	nargs = Cmd_Argc();
	x = qfalse;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->bff ) {
			if ( !search->bff->referenced ) {
				continue;
			}
			bffName = va( "%s/%s", search->bff->bffGamename, search->bff->bffBasename );
			for ( i = 0; i < nargs; i++ ) {
				if ( N_stricmp( Cmd_Argv( i ), bffName ) == 0 ) {
					search->bff->exclude = qtrue;
					x = qtrue;
					break;
				}
			}
		}
	}

	return x;
}


/*
=====================
FS_ReferencedBFFNames

Returns a space separated string containing the names of all referenced bff files.
=====================
*/
const char *FS_ReferencedBFFNames( void ) {
	static char	info[BIG_INFO_STRING];
	const searchpath_t *search;
	const char *bffName;
	info[0] = '\0';

	// we want to return ALL bff's from the fs_game path
	// and referenced one's from nomadmain
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a bff file?
		if ( search->bff ) {
			if ( search->bff->exclude ) {
				continue;
			}
			if ( search->bff->referenced || !FS_IsBaseGame( search->bff->bffGamename ) ) {
				bffName = va( "%s/%s", search->bff->bffGamename, search->bff->bffBasename );
				if ( *info != '\0' ) {
					N_strcat( info, sizeof( info ), " " );
				}
				N_strcat( info, sizeof( info ), bffName );
			}
		}
	}

	return info;
}


/*
=====================
FS_ClearBFFReferences
=====================
*/
void FS_ClearBFFReferences( uint32_t flags ) {
	const searchpath_t *search;

	if ( !flags ) {
		flags = -1;
	}
	for ( search = fs_searchpaths; search; search = search->next ) {
		// is the element a bff file and has it been referenced?
		if ( search->bff ) {
			search->bff->referenced &= ~flags;
		}
	}
}


/*
=====================
FS_ApplyDirPolicy

Set access rights for directories
=====================
*/
static void FS_SetDirPolicy( diraccess_t policy ) {
	searchpath_t *search;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->dir && search->access != DIR_STATIC ) {
			search->access = policy;
		}
	}
}

/*
================
FS_InvalidGameDir
return true if path is a reference to current directory or directory traversal
or a sub-directory
================
*/
qboolean FS_InvalidGameDir( const char *gamedir ) 
{
	if ( !strcmp( gamedir, "." ) || !strcmp( gamedir, ".." )
		|| strchr( gamedir, '/' ) || strchr( gamedir, '\\' ) ) {
		return qtrue;
	}

	return qfalse;
}


void FS_ClearBFFReferences( int32_t flags )
{
	const searchpath_t *sp;

	if ( !flags ) {
		flags = -1;
	}
	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		// is the element a bff file and it been referenced?
		if ( sp->bff ) {
			sp->bff->referenced &= ~flags;
		}
	}
}

void GDR_DECL FS_Printf(fileHandle_t f, const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	N_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	FS_Write(msg, strlen(msg), f);
}


fileHandle_t FS_FOpenWrite(const char *path)
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	FILE *fp;
	const char *ospath;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !path || !*path ) {
		return FS_INVALID_HANDLE;
	}

	ospath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, path );

	if ( fs_debug->i ) {
		Con_Printf( "FS_FOpenWrite: %s\n", ospath );
	}

	// validate the file is actually write-enabled
	FS_CheckFilenameIsNotAllowed(ospath, __func__, qfalse);

	fd = FS_HandleForFile();
	if ( fd == FS_INVALID_HANDLE ) {
		return fd;
	}
	
	f = &handles[fd];
	FS_InitHandle(f);

	fp = Sys_FOpen(ospath, "wb");
	if (!fp) {
		if (FS_CreatePath(ospath)) {
			return FS_INVALID_HANDLE;
		}
		fp = Sys_FOpen(ospath, "wb");
		if (!fp) {
			return FS_INVALID_HANDLE;
		}
	}

	N_strncpyz(f->name, path, sizeof(f->name));
	f->used = qtrue;
	f->data.fp = fp;

	return fd;
}

fileHandle_t FS_FOpenRW(const char *path)
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	FILE *fp;
	const char *ospath;

	if (!fs_searchpaths) {
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

fileHandle_t FS_FOpenRead( const char *path )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	fileInBFF_t *chunk;
	int64_t unused;
	searchpath_t *sp;
	FILE *fp;
	const char *ospath;
	uint64_t fullHash, hash;

	if (!fs_searchpaths) {
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

	// we will calculate full hash only once then just mask it by current bff->hashSize
	// we can do that as long as we know properties of our hash function
	fullHash = FS_HashFileName( path, 0U );

	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff) {
			// look through all the chunks
			chunk = FS_GetChunkHandle(path, &unused);
			if (chunk) {
				// found it!
				FS_OpenChunk(path, chunk, f, sp->bff);
				return fd;
			}
		}
		else if (sp->dir) {
			ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
			Con_DPrintf("FS_FOpenRead: %s\n", ospath);
			fp = Sys_FOpen(ospath, "rb");
			if (!fp) {
				continue;
			}

			f->data.fp = fp;
			N_strncpyz(f->name, path, sizeof(f->name));
			f->bffFile = qfalse;
			f->used = qtrue;

			return fd;
		}
	}

	return FS_INVALID_HANDLE;
}

uint64_t FS_FOpenFileRead(const char *path, fileHandle_t *fd)
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandleData_t *f;
	const char *ospath;
	FILE *fp;
	searchpath_t *sp;
	fileInBFF_t *chunk;
	int64_t index;
	uint64_t length;
	uint64_t fullHash, hash;

	if (!fs_searchpaths) {
		N_Error(ERR_FATAL, "Filesystem call made without initialization");
	}
	if (!path || !*path) {
		N_Error(ERR_FATAL, "FS_FOpenFileRead: NULL or empty path");
	}

	// npaths are not supposed to have a leading slash
	if ( path[0] == '/' || path[0] == '\\' ) {
		path++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo"
	if ( FS_CheckDirTraversal( path ) ) {
		*fd = FS_INVALID_HANDLE;
		return -1;
	}

	// we will calculate full hash only once then just mask it by current bff->hashSize
	// we can do that as long as we know properties of our hash function
	fullHash = FS_HashFileName( path, 0U );

	// we just want the file's size
	if (fd == NULL) {
		for (sp = fs_searchpaths; sp; sp = sp->next) {
			// is the element a bff file?
			if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize-1))]) {
				// look through all the bff chunks
				chunk = sp->bff->hashTable[hash];
				do {
					// case and separator insensitive comparisons
					if (FS_FilenameCompare(chunk->name, path)) {
						// found it!
						return chunk->size;
					}
					chunk = chunk->next;
				} while (chunk != NULL);
			}
			else if (sp->dir) {
				ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
				fp = Sys_FOpen(ospath, "rb");
				if (fp) {
					length = FS_FileLength(fp);
					fclose(fp);
					return length;
				}
			}
		}
		return 0;
	}

	//
	// search through the paths, one element at a time
	//
	for (sp = fs_searchpaths; sp; sp = sp->next) {
		// is the element a bff file?
		if (sp->bff && sp->bff->hashTable[(hash = fullHash & (sp->bff->hashSize-1))]) {
			// look through all the bff chunks
			chunk = sp->bff->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if (FS_FilenameCompare(chunk->name, path)) {
					// found it!
					*fd = FS_HandleForFile();
					if (*fd == FS_INVALID_HANDLE) {
						return 0;
					}
					f = &handles[*fd];
					FS_InitHandle(f);

					FS_OpenChunk(path, chunk, f, sp->bff);
					return chunk->size;
				}
				chunk = chunk->next;
			} while (chunk != NULL);
		}
		else if (sp->dir) {
			// check a file in the directory tree
			ospath = FS_BuildOSPath(sp->dir->path, sp->dir->gamedir, path);
			fp = Sys_FOpen(ospath, "rb");
			if (!fp) {
				continue;
			}

			*fd = FS_HandleForFile();
			if (*fd == FS_INVALID_HANDLE) {
				return 0;
			}

			f = &handles[*fd];
			FS_InitHandle(f);

			f->data.fp = fp;
			N_strncpyz(f->name, path, sizeof(f->name));
			f->bffFile = qfalse;

			return FS_FileLength(fp);
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
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandle_t fd;
	fileHandleData_t *f;
	byte *buf;
	uint64_t size;

	if (!fs_searchpaths) {
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
		
		return 0;
	}
	if (!buffer) {
		return size;
	}

	buf = (byte *)Hunk_AllocateTempMemory(size + 1);
	*buffer = buf;

	FS_Read( buf, size, fd );

	fs_loadStack++;

	// guarentee that it will have a trialing 0 for string operations
	buf[size] = '\0';
	FS_FClose( fd );

	return size;
}

FILE *FS_GetStream( fileHandle_t fh )
{
	return handles[fh].data.fp;
}

int FS_FileToFileno( fileHandle_t f )
{
#ifdef _WIN32
	return _fileno( handles[f].data.fp );
#else
	return fileno( handles[f].data.fp );
#endif
}

void FS_FreeFile( void *buffer )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !buffer ) {
		N_Error( ERR_FATAL, "FS_FreeFile(NULL)" );
	}
	fs_loadStack--;

	Hunk_FreeTempMemory( buffer );

	// if all our temp files are free, clear all of our space
	if ( fs_loadStack == 0 ) {
		Hunk_ClearTempMemory();
	}
}

void FS_FClose( fileHandle_t f )
{
	CThreadAutoLock<CThreadMutex> lock( fs_mutex );
	fileHandleData_t *p;
//	fileStats_t stats;

	if ( f <= FS_INVALID_HANDLE || f >= MAX_FILE_HANDLES ) {
		N_Error( ERR_FATAL, "FS_FClose: out of range" );
	}

	p = &handles[f];

	if ( p->bff && p->bffFile ) {
#ifdef USE_ZIP
		zip_fclose( p->data.chunk->file );
#else
		p->data.chunk->bytesRead = 0;
		if ( p->data.chunk->buf ) {
			Z_Free( p->data.chunk->buf );
			p->data.chunk->buf = NULL;
		}
#endif
		p->data.stream = NULL;
		p->bffFile = qfalse;
		p->bff->handlesUsed--;
#ifdef USE_HANDLE_CACHE
		if ( p->bff->handlesUsed == 0 ) {
			FS_AddToHandleList( p->bff );
		}
#endif
#ifdef USE_BFF_CACHE_FILE
		if ( !fs_locked->i ) {
			if ( p->bff->handle && !p->bff->handlesUsed ) {
#ifdef USE_ZIP
				zip_close( p->bff->handle );
#else
				fclose( p->bff->handle );
#endif
				p->bff->handle = NULL;
			}
		}
#endif
	}
	else {
		if ( p->data.fp ) {
			fclose( p->data.fp );
			p->data.fp = NULL;
		}
	}

	memset( p, 0, sizeof( *p ) );
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

uint64_t FS_LoadStack( void ) {
	return fs_loadStack;
}


/*
================
FS_NewDir_f
================
*/
static void FS_NewDir_f( void ) {
	const char *filter;
	char	**dirnames;
	char	dirname[ MAX_STRING_CHARS ];
	uint64_t ndirs;
	int		i;

	if ( Cmd_Argc() < 2 ) {
		Con_Printf( "usage: fdir <filter>\n" );
		Con_Printf( "example: fdir *darkofthenight*.map\n");
		return;
	}

	filter = Cmd_Argv( 1 );

	Con_Printf( "---------------\n" );

	dirnames = FS_ListFilteredFiles( "", "", filter, &ndirs, FS_MATCH_ANY );

	if ( ndirs >= 2 )  {
		FS_SortFileList( dirnames, ndirs - 1 );
	}

	for ( i = 0; i < ndirs; i++ ) {
		N_strncpyz( dirname, dirnames[i], sizeof( dirname ) );
		FS_ConvertPath( dirname );
		Con_Printf( "%s\n", dirname );
	}

	Con_Printf( "%lu files listed\n", ndirs );
	FS_FreeFileList( dirnames );
}

/*
============
FS_Path_f
============
*/
static void FS_Path_f( void ) {
	const searchpath_t *s;
	int i;

	Con_Printf( "Current search path:\n" );
	for ( s = fs_searchpaths; s; s = s->next ) {
		if ( s->bff ) {
			Con_Printf( "%s (%lu files)\n", s->bff->bffFilename, s->bff->numfiles );
/*			if ( fs_numServerbffs ) {
				if ( !FS_bffIsPure( s->bff ) ) {
					Con_Printf( S_COLOR_YELLOW "    not on the pure list\n" );
				} else {
					Con_Printf( "    on the pure list\n" );
				}
			}*/
		} else {
			Con_Printf( "%s%c%s\n", s->dir->path, PATH_SEP, s->dir->gamedir );
		}
	}

	Con_Printf( "\n" );
	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( handles[i].data.fp ) {
			Con_Printf( "handle %i: %s\n", i, handles[i].name );
		}
	}
}


/*
============
FS_TouchFile_f

The only purpose of this function is to allow game script files to copy
arbitrary files furing an "fs_copyfiles 1" run.
============
*/
static void FS_TouchFile_f( void ) {
	fileHandle_t	f;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "Usage: touchFile <file>\n" );
		return;
	}

	FS_FOpenFileRead( Cmd_Argv( 1 ), &f );
	if ( f != FS_INVALID_HANDLE ) {
		FS_FClose( f );
	}
}


/*
============
FS_CompleteFileName
============
*/
static void FS_CompleteFileName( const char *args, uint32_t argNum ) {
	if ( argNum == 2 ) {
		Field_CompleteFilename( "", "", qfalse, FS_MATCH_ANY );
	}
}


/*
============
FS_Which_f
============
*/
static void FS_Which_f( void ) {
	const searchpath_t *search;
	char			*netpath;
	bffFile_t		*bff;
	fileInBFF_t		*bffFile;
	directory_t		*dir;
	long			hash;
	FILE			*temp;
	const char		*filename;
	char			buf[ MAX_OSPATH*2 + 1 ];
	int				numfound;

	hash = 0;
	numfound = 0;
	filename = Cmd_Argv(1);

	if ( !filename[0] ) {
		Con_Printf( "Usage: which <file>\n" );
		return;
	}

	// qpaths are not supposed to have a leading slash
	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	// just wants to see if file is there
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		if ( search->bff ) {
			hash = FS_HashFileName( filename, search->bff->hashSize );
		}
		// is the element a bff file?
		if ( search->bff && search->bff->hashTable[hash] ) {
			// look through all the bff file elements
			bff = search->bff;
			bffFile = bff->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if ( !FS_FilenameCompare( bffFile->name, filename ) ) {
					// found it!
					Con_Printf( "File \"%s\" found in \"%s\"\n", filename, bff->bffFilename );
					if ( ++numfound >= 32 ) {
						return;
					}
				}
				bffFile = bffFile->next;
			} while( bffFile != NULL );
		} else if ( search->dir ) {
			dir = search->dir;

			netpath = FS_BuildOSPath( dir->path, dir->gamedir, filename );
			temp = Sys_FOpen( netpath, "rb" );
			if ( !temp ) {
				continue;
			}

			fclose( temp );
			Com_snprintf( buf, sizeof( buf ), "%s%c%s", dir->path, PATH_SEP, dir->gamedir );
			FS_ReplaceSeparators( buf );
			Con_Printf( "File \"%s\" found at \"%s\"\n", filename, buf );
			if ( ++numfound >= 32 ) {
				return;
			}
		}
	}

	if ( !numfound ) {
		Con_Printf( "File not found: \"%s\"\n", filename );
	}
}


/*
===========
FS_Rename
===========
*/
void FS_Rename( const char *from, const char *to ) {
	const char *from_ospath, *to_ospath;
	FILE *f;

	if ( !fs_searchpaths ) {
		N_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	// don't let sound stutter
	// S_ClearSoundBuffer();

	from_ospath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, from );
	to_ospath = FS_BuildOSPath( fs_homepath->s, fs_gamedir, to );

	if ( fs_debug->i ) {
		Con_Printf( "FS_Rename: %s --> %s\n", from_ospath, to_ospath );
	}

	f = Sys_FOpen( from_ospath, "rb" );
	if ( f ) {
		fclose( f );
		FS_Remove( to_ospath );
	}

	if ( rename( from_ospath, to_ospath ) ) {
		// Failed, try copying it and deleting the original
		FS_CopyFile( from_ospath, to_ospath );
		FS_Remove( from_ospath );
	}
}

static void FS_AddGameDirectory( const char *path, const char *dir )
{
	const searchpath_t *sp;
	searchpath_t *search;
	bffFile_t *bff;
	char curpath[MAX_OSPATH*2+1];
	char **bffDirs, **bffFiles;
	const char *bffpath;
	const char *gamedir;
	char *bffFile;
	uint64_t numBFFs, i;
	uint64_t path_len, dir_len, size;
	uint64_t numdirs, numfiles;
	uint64_t bffDirsI, bffFilesI;
	uint64_t len;
	int bffWhich;

	for ( sp = fs_searchpaths; sp; sp = sp->next ) {
		if ( sp->dir && sp->dir->path && sp->dir->gamedir && !N_stricmp( sp->dir->path, path ) && !N_stricmp( sp->dir->gamedir, dir ) ) {
			return; // we've already got this one
		}
	}

	path_len = PAD( strlen( path ) + 1, sizeof( uintptr_t ) );
	dir_len = PAD( strlen( dir ) + 1, sizeof( uintptr_t ) );
	size = sizeof( *search ) + sizeof( *search->dir ) + path_len + dir_len;

	search = (searchpath_t *)Z_Malloc( size, TAG_SEARCH_DIR );
	memset(search, 0, size);
	search->dir = (directory_t *)( search + 1 );
	search->dir->path = (char *)( search->dir + 1 );
	search->dir->gamedir = (char *)( search->dir->path + path_len );
	search->access = DIR_STATIC;

	strcpy( search->dir->path, path );
	strcpy( search->dir->gamedir, dir );
	gamedir = search->dir->gamedir;

	search->next = fs_searchpaths;
	fs_searchpaths = search;

	bffFilesI = 0;
	bffDirsI = 0;

	N_strncpyz( curpath, FS_BuildOSPath( path, dir, NULL ), sizeof( curpath ) );

	// get .bff files
	bffFiles = Sys_ListFiles( curpath, ".bff", NULL, &numfiles, qfalse );
	if ( numfiles >= 2 ) {
		FS_SortFileList( bffFiles, numfiles - 1 );
	}

	bffDirs = Sys_ListFiles( curpath, "/", NULL, &numdirs, qfalse );
	if ( numdirs >= 2 ) {
		FS_SortFileList( bffDirs, numdirs - 1 );
	}

	Con_DPrintf( "Adding game directory '%s'...\n", curpath );

	while (( bffFilesI < numfiles ) || ( bffDirsI < numdirs )) {
		// check if a bff or directory comes next
		if (bffFilesI >= numfiles) {
			// we've used all the bff files, it must be a directory
			bffWhich = 0;
		}
		else if (bffDirsI >= numdirs) {
			// we've used all the directories, it must be a bff file
			bffWhich = 1;
		}
		else {
			// could be either, compare to see which name comes first
			bffWhich = (FS_PathCmp( bffFiles[bffFilesI], bffDirs[bffDirsI] ) < 0);
		}
		
		if (bffWhich) {
			len = strlen( bffFiles[bffFilesI] );
			if (!FS_IsExt( bffFiles[bffFilesI], ".bff", len )) {
				// not a bff file
				bffFilesI++;
				continue;
			}

			// the next .bff file is before the next directory
			bffFile = FS_BuildOSPath( path, dir, bffFiles[bffFilesI] );
			if ((bff = FS_LoadBFF( bffFile )) == NULL) {
				// this isn't a .bff file! NeXT!
				bffFilesI++;
				continue;
			}

			// store the game name
//			bff->bffGamename = search->dir->gamedir;
			N_strncpyz( bff->bffGamename, gamedir, sizeof( bff->bffGamename ) );

			bff->index = fs_bffCount;
			bff->referenced = 0;

			fs_bffChunks += bff->numfiles;
			fs_bffCount++;

			search = (searchpath_t *)Z_Malloc( sizeof(*search), TAG_SEARCH_PATH );
			memset( search, 0, sizeof(*search) );
			search->bff = bff;

			search->next = fs_searchpaths;
			fs_searchpaths = search;

			bffFilesI++;
		} else {
			len = strlen( bffDirs[bffDirsI] );

			// the next directory is before the next .bff file
			// But wait, this could be any directory, we're filtering to only ending with ".bffdir" here.
//			if (!FS_IsExt(bffDirs[bffDirsI], ".bffdir", len)) {
//				// This isn't a .bffdir! NeXT!
//				bffDirsI++;
//				continue;
//			}

			// add the directory to the search path
			path_len = strlen( curpath ) + 2;
			path_len = PAD(path_len, sizeof(uintptr_t));
			dir_len = PAD(len + 1, sizeof(uintptr_t));
			len = sizeof(*search) + sizeof(*search->dir) + path_len + dir_len;

			search = (searchpath_t *)Z_Malloc( len, TAG_SEARCH_DIR );
			memset( search, 0, sizeof(*search) );
			search->dir = (directory_t *)(search  + 1);
			search->dir->path = (char *)(search->dir + 1);
			search->dir->gamedir = (char *)(search->dir->path + path_len);

			strcpy( search->dir->path, curpath ); // /home/user/glnomad/gamedata
			strcpy( search->dir->gamedir, bffDirs[ bffDirsI ] ); // mydir
			
			search->next = fs_searchpaths;
			fs_searchpaths = search;
			fs_dirCount++;

			bffDirsI++;
		}
	}

	// done
	Sys_FreeFileList(bffDirs);
	Sys_FreeFileList(bffFiles);
}


void FS_Flush( fileHandle_t f )
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

	cnt = fs_bffCount;
	if (cnt == 0)
		return;
	
	// relink path chains in the following order:
	// 1. files
	// 2. directories
	list = (searchpath_t **)Z_Malloc( cnt * sizeof( *list ), TAG_STATIC );
	bffs = list;

	nbffs = ndirs = 0;
	path = fs_searchpaths;
	while (path) {
		if (path->bff || path->access != DIR_STATIC) {
			bffs[nbffs++] = path;
		}
		
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

static void FS_ShowDir_f( void )
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

static void FS_AddMod_f( void )
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

void FS_Shutdown( qboolean closeFiles )
{
	searchpath_t *p, *next;
	uint64_t i;
	extern fileHandle_t logfile;

	if ( closeFiles ) {
		for ( i = 1; i < MAX_FILE_HANDLES; i++ ) {
			if ( !handles[i].data.stream ) {
				continue;
			}
			
			FS_FClose( i );
		}
	}
	memset( handles, 0, sizeof( handles ) );

	logfile = FS_INVALID_HANDLE;
#ifdef USE_BFF_CACHE_FILE
//	FS_ResetCacheReferences();
#endif

	// free everything
	for ( p = fs_searchpaths; p; p = next ) {
		next = p->next;

		if ( p->bff ) {
			FS_FreeBFF( p->bff );
			p->bff = NULL;
		}

		Z_Free( p );
	}
	fs_searchpaths = NULL;

	Z_FreeTags( TAG_SEARCH_PATH );
	Z_FreeTags( TAG_SEARCH_DIR );
	Z_FreeTags( TAG_BFF );

	fs_searchpaths = NULL;
	fs_bffCount = 0;

	fs_bffChunks = 0;
	fs_dirCount = 0;

	Cmd_RemoveCommand( "path" );
	Cmd_RemoveCommand( "dir" );
	Cmd_RemoveCommand( "ls" );
	Cmd_RemoveCommand( "list" );
	Cmd_RemoveCommand( "addmod" );
	Cmd_RemoveCommand( "fs_restart" );
	Cmd_RemoveCommand( "lsof" );
	Cmd_RemoveCommand( "which" );
}

void FS_Restart( void )
{
	PROFILE_FUNCTION();

	// last valid game folder
	static char lastValidBase[MAX_OSPATH];
	static char lastValidGame[MAX_OSPATH];

	static qboolean execConfig = qfalse;

	// free anything we currently have loaded
	FS_Shutdown( qfalse );

	// try to start up normally
	FS_Startup();

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FS_LoadFile( "default.cfg", NULL ) <= 0 ) {
		if ( lastValidBase[0] ) {
			Cvar_Set( "fs_basepath", lastValidBase );
			Cvar_Set( "fs_game", lastValidGame );
			lastValidBase[0] = '\0';
			lastValidGame[0] = '\0';
			Cvar_Set( "fs_restrict", "0" );
			execConfig = qtrue;
			FS_Restart();
			N_Error( ERR_DROP, "Invalid game folder" );
			return;
		}
		N_Error( ERR_FATAL, "Couldn't load default.cfg" );
	}

	// now check before safeMode
	if ( !N_stricmp( fs_gamedirvar->s, lastValidGame ) && execConfig ) {
		// skip the user config if "safe" is on the command line
		if ( !Com_SafeMode() ) {
			Cbuf_AddText( "exec " NOMAD_CONFIG "\n" );
		}
	}

	execConfig = qfalse;

	N_strncpyz( lastValidBase, fs_basepath->s, sizeof( lastValidBase ) );
	N_strncpyz( lastValidGame, fs_gamedirvar->s, sizeof( lastValidGame ) );
}

static void FS_ListOpenFiles_f( void )
{
	uint64_t i;
	fileHandleData_t *f;

	f = handles;
	for ( i = 1; i < MAX_FILE_HANDLES; i++, f++ ) {
		if ( !f->data.stream ) {
			continue;
		}
		
		Con_Printf( "%2lu %2s %s\n", i, FS_OwnerName( f->owner ), f->name );
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
	Com_StartupVariable( "fs_copyfiles" );
	Com_StartupVariable( "fs_restrict" );
#ifndef USE_BFF_CACHE_FILE
	Com_StartupVariable( "fs_locked" );
#endif

#if defined(_WIN32) && defined(_MSC_VER)
 	_setmaxstdio( 2048 );
#endif

	// try to start up normally
	FS_Restart();
}


void FS_Startup( void )
{
	const char *homePath;
	CTimer timer;
	int i;

	fs_bffCount = 0;
	fs_loadStack = 0;
	fs_lastBFFIndex = -1;
	fs_bffChunks = 0;
	fs_readCount = 0;
	fs_writeCount = 0;

	PROFILE_FUNCTION();

	Con_Printf( "\n---------- FS_Startup ----------\n" );

#ifdef _NOMAD_DEBUG
	fs_debug = Cvar_Get( "fs_debug", "1", 0 );
#else
	fs_debug = Cvar_Get( "fs_debug", "0", 0 );
#endif
	Cvar_SetDescription( fs_debug, "Debugging tool for the filesystem. Run the game in debug mode. Prints additional information regarding read files into the console." );
	fs_basepath = Cvar_Get( "fs_basepath", Sys_DefaultBasePath(), CVAR_INIT | CVAR_PROTECTED | CVAR_PRIVATE );
	Cvar_SetDescription( fs_basepath, "Write-protected CVar specifying the path to the installation folder of the game." );
	Cvar_SetGroup( fs_basepath, CVG_FILESYSTEM );

	fs_basegame = Cvar_Get( "fs_basegame", BASEGAME_DIR, CVAR_PRIVATE | CVAR_PROTECTED );
	Cvar_SetDescription( fs_basegame, "CVar specifying the path to the base game folder." );
	Cvar_SetGroup( fs_basegame, CVG_FILESYSTEM );

	fs_excludeReference = Cvar_Get( "fs_excludeReference", "", CVAR_ARCHIVE_ND | CVAR_LATCH );
	Cvar_SetDescription( fs_excludeReference,
		"Exclude specified bff files from download list.\n"
		"Format is <moddir>/<bffname> (without .bff suffix), you may list multiple entries separated by space." );


	/* parse fs_basegame cvar */
	if ( basegame_cnt == 0 || N_stricmp( basegame, fs_basegame->s ) ) {
		N_strncpyz( basegame_str, fs_basegame->s, sizeof( basegame_str ) );
		basegame_cnt = Com_Split( basegame_str, basegames, arraylen( basegames ), '/' );
		/* set up basegame on last item from the list */
		basegame = basegames[0];
		for ( i = 1; i < basegame_cnt; i++ ) {
			if ( *basegames[i] != '\0' ) {
				basegame = basegames[i];
			}
		}
		/* change fs_basegame cvar to last item */
		Cvar_Set( "fs_basegame", basegame );
	}
	if ( !fs_basegame->s[0] || !*basegame || basegame_cnt == 0 ) {
		N_Error( ERR_FATAL, "* fs_basegame not set *" );
	}

#ifndef USE_HANDLE_CACHE
	fs_locked = Cvar_Get( "fs_locked", "0", CVAR_INIT );
	Cvar_SetDescription( fs_locked, "Set file handle policy for bff files:\n"
		" 0 - release after use, unlimited number of bff files can be loaded\n"
		" 1 - keep file handle locked, more consistent, total bff files count limited to ~1k-4k\n" );
#endif

#ifdef NOMAD_STEAM_APP
	fs_steampath = Cvar_Get( "fs_steampath", Sys_GetSteamPath(), CVAR_INIT | CVAR_PROTECTED | CVAR_PRIVATE );
	Cvar_SetDescription( fs_steampath, "CVar specifying the path to the steam data folder." );
	Cvar_SetGroup( fs_steampath, CVG_FILESYSTEM );
#endif

	homePath = Sys_DefaultHomePath();
	if ( !homePath || !homePath[0] ) {
		homePath = fs_basepath->s;
	}

	fs_homepath = Cvar_Get( "fs_homepath", homePath, CVAR_INIT | CVAR_PROTECTED | CVAR_PRIVATE );
	Cvar_SetDescription( fs_homepath, "Directory to store user configuration and downloaded files." );

	fs_gamedirvar = Cvar_Get( "fs_gamedir", "", CVAR_INIT | CVAR_SYSTEMINFO );
	Cvar_CheckRange( fs_gamedirvar, NULL, NULL, CVT_FSPATH );
	Cvar_SetDescription( fs_gamedirvar, "Specify an alternate mod directory and run the game with this mod." );

	if ( !N_stricmp( fs_basegame->s, fs_gamedirvar->s ) ) {
		Cvar_ForceReset( "fs_gamedir" );
	}

	timer.Start();

#ifdef USE_BFF_CACHE
#ifdef USE_BFF_CACHE_FILE
	FS_LoadCache();
#endif
#endif

	// add search path elements in reverse priority order
#ifdef NOMAD_STEAM_APP
	if ( fs_steampath->s[0] ) {
		// handle multiple basegames:
		for ( i = 0; i < basegame_cnt; i++ ) {
			FS_AddGameDirectory( fs_steampath->s, basegames[i] );
		}
		FS_AddGameDirectory( fs_steampath->s, fs_basegame->s );
	}
#endif

#ifdef __APPLE__
	fs_apppath = Cvar_Get( "fs_apppath", Sys_DefaultAppPath(), CVAR_INIT|CVAR_PROTECTED );
	// Make MacOSX also include the base path included with the .app bundle
	if ( fs_apppath->s[0] ) {
		FS_AddGameDirectory( fs_apppath->s, fs_basegame->s );
	}
#endif

	if ( fs_basepath->s[0] ) {
		// handle multiple basegames:
		for ( i = 0; i < basegame_cnt; i++ ) {
			FS_AddGameDirectory( fs_basepath->s, basegames[i] );
		}
		FS_AddGameDirectory( fs_basepath->s, BASEGAME_DIR "/modules" );
	}
	// fs_homepath is somewhat particular to *nix systems, only add if relevant
	// NOTE: same filtering below for mods and basegame
	if ( fs_homepath->s[0] && N_stricmp( fs_homepath->s, fs_basegame->s ) ) {
		// handle multiple basegames:
		for (i = 0; i < basegame_cnt; i++) {
			FS_AddGameDirectory( fs_homepath->s, basegames[i] );
		}
	}

	// check for additional game folder for mods
	if ( fs_gamedirvar->s[0] && !FS_IsBaseGame( fs_gamedirvar->s ) ) {
#ifdef NOMAD_STEAM_APP
		if ( fs_steampath->s[0] ) {
			FS_AddGameDirectory( fs_steampath->s, fs_gamedirvar->s );
		}
#endif
		if ( fs_basepath->s[0] ) {
			FS_AddGameDirectory( fs_basepath->s, fs_gamedirvar->s );
		}
		if ( fs_homepath->s[0] && N_stricmp( fs_homepath->s, fs_basepath->s ) ) {
			FS_AddGameDirectory( fs_homepath->s, fs_gamedirvar->s );
		}
	}
	

	timer.Stop();

	Com_ReadCDKey( basegame );
	if ( !FS_IsBaseGame( fs_gamedirvar->s ) ) {
		Com_AppendCDKey( fs_gamedirvar->s );
	}

	// add our commands
	Cmd_AddCommand( "path", FS_Path_f );
	Cmd_AddCommand( "dir", FS_ShowDir_f );
	Cmd_AddCommand( "ls", FS_ShowDir_f );
	Cmd_AddCommand( "list", FS_ListBFF_f );
	Cmd_AddCommand( "addmod", FS_AddMod_f );
	Cmd_AddCommand( "fs_restart", FS_Restart );
	Cmd_AddCommand( "lsof", FS_ListOpenFiles_f );
	Cmd_AddCommand( "which", FS_Which_f );
	Cmd_SetCommandCompletionFunc( "which", FS_CompleteFileName );

	Con_Printf(
		"fs_gamedir: %s\n"
		"fs_basepath: %s\n"
		"fs_basegame: %s\n"
	, fs_gamedirvar->s, fs_basepath->s, fs_basegame->s );

	Con_Printf( "...loaded in %5.5lf milliseconds\n", (double)timer.Milliseconds() );

	Con_Printf( "----------------------\n" );
	Con_Printf( "%lu chunks in %lu bff files\n", fs_bffChunks, fs_bffCount );

	// we just loaded, it's not modified
	fs_gamedirvar->modified = qfalse;

#ifdef USE_BFF_CACHE
	FS_FreeUnusedCache();
#ifdef USE_BFF_CACHE_FILE
	FS_SaveCache();
#endif
#endif
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

	out = (char *)S_Malloc(strlen(s) + 1);
	strcpy( out, s );
	return out;
}

static void FS_GetModDescription(const char *modDir, char *description, uint64_t descriptionLen)
{
	fileHandle_t descHandle;
	char descPath[MAX_NPATH];
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
	dst = cat = (char **)Z_Malloc( ( totalLength + 1 ) * sizeof( char * ), TAG_STATIC );

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
================
FS_GetFileList
================
*/
uint64_t FS_GetFileList( const char *path, const char *extension, char *listbuf, uint64_t bufsize )
{
	uint64_t nFiles, i, nTotal, nLen;
	char **pFiles = NULL;

	*listbuf = 0;
	nFiles = 0;
	nTotal = 0;

	if ( N_stricmp( path, "$modlist" ) == 0 ) {
		return FS_GetModList( listbuf, bufsize );
	}

	pFiles = FS_ListFiles(path, extension, &nFiles);

	for (i =0; i < nFiles; i++) {
		nLen = strlen(pFiles[i]) + 1;
		if (nTotal + nLen + 1 < bufsize) {
			strcpy(listbuf, pFiles[i]);
			listbuf += nLen;
			nTotal += nLen;
		}
		else {
			nFiles = i;
			break;
		}
	}

	FS_FreeFileList(pFiles);

	return nFiles;
}

/*
FS_GetModList: returns a list of mod directory names. A mod directory is
a peer to gamedata/ with a .bff in it
*/
uint64_t FS_GetModList(char *listbuf, uint64_t bufSize)
{
	uint64_t i, j, k;
	uint64_t nMods, nTotal, nLen, nBFFs, nPotential, nDescLen;
	uint64_t nDirs, nBFFDirs;
	char **pFiles = NULL;
	char **pBFFs = NULL;
	char **pDirs = NULL;
	const char *name, *path;
	char description[MAX_OSPATH];
	cvar_t *fs_modules;

	fs_modules = (cvar_t *)alloca( sizeof( *fs_modules ) );
	fs_modules->s = strdup_a( "modules/" );

	uint64_t dummy;
	char **pFiles0 = NULL;
	qboolean bDrop = qfalse;

	// paths to search for mods
	cvar_t *const *paths[] = { &fs_basepath, &fs_homepath, &fs_modules
#ifdef NOMAD_STEAM_APP
		, &fs_steampath
#endif
	};

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

