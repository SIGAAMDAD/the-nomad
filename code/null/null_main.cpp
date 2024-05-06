// null_main.cpp -- null system driver to aid porting efforts
// insanely heavy usage of SDL2 here instead of system-specific stuff
#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../game/g_game.h"
#include <SDL2/SDL.h>

uint64_t Sys_Milliseconds( void ) {
    return time( NULL );
}

FILE *Sys_FOpen( const char *filepath, const char *mode ) {
    return fopen( filepath, mode );
}

int Sys_MessageBox( const char *title, const char *text, bool ShowOkAndCancelButton ) {
    return 0;
}

void Sys_GetRAMUsage( uint64_t *curVirt, uint64_t *curPhys, uint64_t *peakVirt, uint64_t *peakPhys ) {
    *curVirt = 0;
    *curPhys = 0;
    *peakVirt = 0;
    *peakPhys = 0;
}

uint64_t Sys_GetCacheLine( void ) {
    return SDL_GetCPUCacheLineSize();
}

uint64_t Sys_GetPageSize( void ) {
    // rough estimate of 4 KiB
    return 4*1024;
}

char *Sys_ConsoleInput( void ) {
    return NULL;
}

uint64_t Sys_EventSubtime( uint64_t time ) {
    return Sys_Milliseconds();
}

uint64_t Sys_StackMemoryRemaining( void ) {
    return 2*1024*1024;
}

qboolean Sys_mkdir( const char *name ) {
    return qtrue;
}

static qboolean printableChar( char c ) {
	if ( ( c >= ' ' && c <= '~' ) || c == '\n' || c == '\r' || c == '\t' ) {
		return qtrue;
	}
	return qfalse;
}

void Sys_Print( const char *msg ) {
    char printmsg[MAXPRINTMSG];
    size_t len;

    memset( printmsg, 0, sizeof( printmsg ) );

    char *out = printmsg;
    while ( *msg != '\0' && out < printmsg + sizeof( printmsg ) ) {
        if ( printableChar( *msg ) ) {
            *out++ = *msg;
		}
        msg++;
    }
    len = out - printmsg;
    
    // don't bother with an fprintf, we've already got it all
    // formatted
    fwrite( msg, len, 1, stdout );
}

void GDR_NORETURN GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Sys_Error( const char *fmt, ... )
{
    va_list argptr;

    fprintf( stderr, "Sys_Error: " );

    va_start( argptr, fmt );
    vfprintf( stderr, fmt, argptr );
    va_end( argptr );

    fprintf( stderr, "\n" );
    fflush( stderr );

    Sys_Exit( -1 );
}

char *Sys_GetClipboardData( void )
{
    char *data = NULL;
	char *cliptext;

	if ( ( cliptext = SDL_GetClipboardText() ) != NULL ) {
		if ( cliptext[0] != '\0' ) {
			size_t bufsize = strlen( cliptext ) + 1;

			data = (char *)Z_Malloc( bufsize, TAG_STATIC );
			N_strncpyz( data, cliptext, bufsize );

			// find first listed char and set to '\0'
			strtok( data, "\n\r\b" );
		}
		SDL_free( cliptext );
	}
	return data;
}

double Sys_CalculateCPUFreq( void ) {
    return 0.0f;
}

void Sys_SendKeyEvents( void ) {
}

const char *Sys_pwd( void ) {
    return NULL;
}

void *Sys_LoadDLL( const char *name ) {
    return NULL;
}

void Sys_CloseDLL( void *handle ) {
}

void *Sys_GetProcAddress( void *handle, const char *name ){
    return NULL;
}

const char *Sys_GetError( void ) {
    return NULL;
}

void GDR_NORETURN Sys_Exit( int code ) {
    if ( code == -1 ) {
        exit( EXIT_FAILURE );
    }
    exit( EXIT_SUCCESS );
}
qboolean Sys_GetFileStats( fileStats_t *stats, const char *path ) {
    return qtrue;
}

void Sys_FreeFileList( char **list ) {
    uint64_t i;

    for ( i = 0; list[i]; i++ ) {
        Z_Free( list[i] );
    }
    Z_Free( list );
}

void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **list, uint64_t *numfiles ) {
}

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, uint64_t *numfiles, qboolean wantsubs ) {
    return NULL;
}

const char *Sys_DefaultHomePath( void ) {
    return SDL_GetPrefPath( "GDRSoftware", "TheNomad" );
}

const char *Sys_DefaultBasePath( void ) {
    return SDL_GetBasePath();
}

qboolean Sys_RandomBytes( byte *s, uint64_t len ) {
    uint64_t i;

    // srand isn't really good at random, but whatevs
    srand( time( NULL ) );

    for ( i = 0; i < len; i++ ) {
        s[i] = rand() % Sys_Milliseconds();
    }

    return qtrue;
}

qboolean Sys_LowPhysicalMemory( void ) {
    return qfalse;
}

void Sys_Sleep( double msec ) {
}

void *Sys_AllocVirtualMemory( uint64_t nBytes ) {
    return NULL;
}

void Sys_ReleaseVirtualMemory( void *pMemory, uint64_t nBytes ) {
}

void *Sys_CommitVirtualMemory( void *pMemory, uint64_t nBytes ) {
    return NULL;
}

void Sys_DecommitVirtualMemory( void *pMemory, uint64_t nBytes ) {
}

void Sys_LockMemory( void *pAddress, uint64_t nBytes ) {
}

void Sys_UnlockMemory( void *pAddress, uint64_t nBytes ) {
}

qboolean Sys_SetAffinityMask( const uint64_t mask ) {
    return qtrue;
}

uint64_t Sys_GetAffinityMask( void ) {
    return 0;
}

void Sys_ClearDLLError( void ) {
}

int Sys_GetDLLErrorCount( void ) {
    return 0;
}

const char *Sys_GetDLLError( void ) {
    return NULL;
}

int main( int argc, char **argv )
{
    char *cmdline;
    int len;
    int i;

    Con_Printf( "WARNING: Sys_Null THIS IS NOT A RELEASE BUILD!!!!!!\n" );

    // merge the command line, this is kinda silly
	for ( len = 1, i = 1; i < argc; i++ ) {
		len += strlen( argv[i] ) + 1;
    }

	cmdline = (char *)malloc( len );
    if ( !cmdline ) {
        Sys_Error( "main(): malloc failed on %i bytes", len );
    }

    Com_Init( cmdline );

    while ( 1 ) {
        IN_Frame();
        Com_Frame( qfalse );
    }

    free( cmdline );
}
