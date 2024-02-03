#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "win_local.h"
#include "sys_win32.h"
#include <errhandlingapi.h>
#include <processthreadsapi.h>

uint64_t Sys_Milliseconds( void )
{
    static qboolean initialized;
    static DWORD sys_timeBase;
    uint64_t sys_curtime;

    if (!initialized) {
        sys_timeBase = timeGetTime();
        initialized = qtrue;
    }

    sys_curtime = timeGetTime() - sys_timeBase;

    return sys_curtime;
}

void Sys_Sleep( double msec ) {
	Sleep( msec );
}

uint64_t Sys_StackMemoryRemaining( void )
{
	// FIXME: implement
	return (1*1024*1024); // return a guess of 1 MiB
}

/*
================
Sys_RandomBytes
================
*/
qboolean Sys_RandomBytes( byte *string, uint64_t len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return qfalse;
	}

	if( !CryptGenRandom( prov, len, (BYTE *)string ) )  {
		CryptReleaseContext( prov, 0 );
		return qfalse;
	}
	CryptReleaseContext( prov, 0 );
	return qtrue;
}


#ifdef UNICODE
LPWSTR AtoW( const char *s ) 
{
	static WCHAR buffer[MAXPRINTMSG*2];
	MultiByteToWideChar( CP_ACP, 0, s, strlen( s ) + 1, (LPWSTR) buffer, ARRAYSIZE( buffer ) );
	return buffer;
}

const char *WtoA( const LPWSTR s ) 
{
	static char buffer[MAXPRINTMSG*2];
	WideCharToMultiByte( CP_ACP, 0, s, -1, buffer, ARRAYSIZE( buffer ), NULL, NULL );
	return buffer;
}
#endif

/*
================
Sys_DefaultHomePath
================
*/
const char *Sys_DefaultHomePath( void ) 
{
#ifdef USE_PROFILES
	TCHAR szPath[MAX_PATH];
	static char path[MAX_OSPATH];
	FARPROC qSHGetFolderPath;
	HMODULE shfolder = LoadLibrary("shfolder.dll");
	
	if (shfolder == NULL) {
		Con_Printf("Unable to load SHFolder.dll\n");
		return NULL;
	}

	qSHGetFolderPath = GetProcAddress(shfolder, "SHGetFolderPathA");
	if (qSHGetFolderPath == NULL) {
		Con_Printf("Unable to find SHGetFolderPath in SHFolder.dll\n");
		FreeLibrary(shfolder);
		return NULL;
	}

	if( !SUCCEEDED( qSHGetFolderPath( NULL, CSIDL_APPDATA,
		NULL, 0, szPath ) ) )
	{
		Con_Printf("Unable to detect CSIDL_APPDATA\n");
		FreeLibrary(shfolder);
		return NULL;
	}
	N_strncpyz( path, szPath, sizeof(path) );
	N_strcat( path, sizeof(path), "\\Quake3" );
	FreeLibrary(shfolder);
	if( !CreateDirectory( path, NULL ) )
	{
		if( GetLastError() != ERROR_ALREADY_EXISTS ) {
			Con_Printf("Unable to create directory \"%s\"\n", path);
			return NULL;
		}
	}
	return path;
#else
    return NULL;
#endif
}



/*
================
Sys_SetAffinityMask
================
*/
#ifdef USE_AFFINITY_MASK
static HANDLE hCurrentProcess = 0;

uint64_t Sys_GetAffinityMask( void )
{
	DWORD_PTR dwProcessAffinityMask;
	DWORD_PTR dwSystemAffinityMask;

	if ( hCurrentProcess == 0 )	{
		hCurrentProcess = GetCurrentProcess();
	}

	if ( GetProcessAffinityMask( hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ) )	{
		return (uint64_t)dwProcessAffinityMask;
	}

	return 0;
}


qboolean Sys_SetAffinityMask( const uint64_t mask )
{
	DWORD_PTR dwProcessAffinityMask = (DWORD_PTR)mask;

	if ( hCurrentProcess == 0 ) {
		hCurrentProcess = GetCurrentProcess();
	}

	if ( SetProcessAffinityMask( hCurrentProcess, dwProcessAffinityMask ) )	{
		//Sleep( 0 );
		return qtrue;
	}

	return qfalse;
}
#endif // USE_AFFINITY_MASK


 

qboolean Sys_GetFileStats( fileStats_t *stats, const char *filename )
{
    struct _stat fdata;

    if (_stat(filename, &fdata) == 0) {
        stats->mtime = fdata.st_mtime;
        stats->ctime = fdata.st_ctime;
        stats->exists = qtrue;
        stats->size = fdata.st_size;

        return qtrue;
    }

    stats->mtime = 0;
    stats->ctime = 0;
    stats->exists = qfalse;
    stats->size = 0;

    return qfalse;
}


/*
==============
Sys_Mkdir
==============
*/
qboolean Sys_mkdir( const char *path )
{
	if ( _mkdir( path ) == 0 ) {
		return qtrue;
	} else {
		if ( errno == EEXIST ) {
			return qtrue;
		} else {
			return qfalse;
		}
	}
}


/*
==============
Sys_FOpen
==============
*/
FILE *Sys_FOpen( const char *ospath, const char *mode )
{
	size_t length;

	// Windows API ignores all trailing spaces and periods which can get around Quake 3 file system restrictions.
	length = strlen( ospath );
	if ( length == 0 || ospath[length-1] == ' ' || ospath[length-1] == '.' ) {
		return NULL;
	}

	return fopen( ospath, mode );
}


/*
==============
Sys_ResetReadOnlyAttribute
==============
*/
qboolean Sys_ResetReadOnlyAttribute( const char *ospath )
{
    DWORD dwAttr;

	dwAttr = GetFileAttributesA( ospath );
	if ( dwAttr & FILE_ATTRIBUTE_READONLY ) {
		dwAttr &= ~FILE_ATTRIBUTE_READONLY;
		if ( SetFileAttributesA( ospath, dwAttr ) ) {
			return qtrue;
		} else {
			return qfalse;
		}
	} else {
		return qfalse;
	}
}


/*
==============
Sys_Pwd
==============
*/
const char *Sys_pwd( void )
{
	static char pwd[ MAX_OSPATH ];
	TCHAR	buffer[ MAX_OSPATH ];
	char *s;

	if ( pwd[0] )
		return pwd;

	GetModuleFileName( NULL, buffer, arraylen( buffer ) );
	buffer[ arraylen( buffer ) - 1 ] = '\0';

	N_strncpyz( pwd, WtoA( buffer ), sizeof( pwd ) );

	s = strrchr( pwd, PATH_SEP );
	if ( s ) 
		*s = '\0';
	else // bogus case?
	{
		_getcwd( pwd, sizeof( pwd ) - 1 );
		pwd[ sizeof( pwd ) - 1 ] = '\0';
	}

	return pwd;
}


/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath( void ) {
	return Sys_pwd();
}

int dll_err_count = 0;

int Sys_GetDLLErrorCount( void ) {
    return dll_err_count;
}

const char *Sys_GetDLLError( void ) {
    if (dll_err_count) {
        dll_err_count--;
        return Sys_GetError();
    }
    return "no error";
}

void Sys_ClearDLLError( void ) {
    if (dll_err_count) {
        Con_DPrintf( COLOR_YELLOW "WARNING: clearing dll_err_count, but there's errors, listing them:\n" );
        for (int i = 0; i < dll_err_count; i++) {
            Con_DPrintf( COLOR_YELLOW "dll_error[%i]: %s\n", i, Sys_GetError() );
        }
    }
    dll_err_count = 0;
}

void *Sys_LoadDLL( const char *name )
{
    const char *ext;
    void *handle;

    if (!name || !*name) {
        return NULL;
    }
    
    if (FS_AllowedExtension( name, qfalse, &ext )) {
        N_Error(ERR_FATAL, "Sys_LoadDLL: unable to load library with '%s' extension", ext);
    }

    handle = (void *)LoadLibrary( AtoW( name ) );
    if (!handle) {
        dll_err_count++;
    }

    return handle;
}

void *Sys_GetProcAddress( void *handle, const char *name )
{
    void *symbol;

    if (!handle || !name || *name == '\0') {
        dll_err_count++;
        return NULL;
    }

    symbol = (void *)GetProcAddress( (HMODULE)handle, name );
    if (!symbol) {
        dll_err_count++;
    }

    return symbol;
}

void Sys_CloseDLL( void *handle ) {
    if (handle) {
        FreeLibrary( (HMODULE)handle );
    }
}


void UpdateMonitorInfo( const RECT *target ) {
}

#ifdef USE_AFFINITY_MASK
static HANDLE hCurrentProcess = 0;

uint64_t Sys_GetAffinityMask( void )
{
	DWORD_PTR dwProcessAffinityMask;
	DWORD_PTR dwSystemAffinityMask;

	if ( hCurrentProcess == 0 )	{
		hCurrentProcess = GetCurrentProcess();
	}

	if ( GetProcessAffinityMask( hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ) )	{
		return (uint64_t)dwProcessAffinityMask;
	}

	return 0;
}


qboolean Sys_SetAffinityMask( const uint64_t mask )
{
	DWORD_PTR dwProcessAffinityMask = (DWORD_PTR)mask;

	if ( hCurrentProcess == 0 ) {
		hCurrentProcess = GetCurrentProcess();
	}

	if ( SetProcessAffinityMask( hCurrentProcess, dwProcessAffinityMask ) )	{
		//Sleep( 0 );
		return qtrue;
	}

	return qfalse;
}
#endif // USE_AFFINITY_MASK
