#include "sg_local.h"

#define MAX_LEVELINFO_LEN 8192
#define MAX_LEVELS 1024

static char *sg_levelInfos[MAX_LEVELS];

int SG_ParseInfos( char *buf, int max, char **infos )
{
    const char *token, **text;
    int count;
    char key[MAX_TOKEN_CHARS];
    char info[MAX_INFO_STRING];

    text = (const char **)&buf;
    count = 0;

    while (1) {
        token = COM_Parse( text );
        if (!token[0]) {
            break;
        }
        if (token[0] != '{') {
            Con_Printf( "missing '{' in info file\n" );
            break;
        }

        if (count == max) {
            Con_Printf( "max infos exceeded\n" );
            break;
        }

        info[0] = '\0';
        while (1) {
            token = COM_ParseExt( text, qtrue );
            if (!token[0]) {
                Con_Printf( "unexpected end of info file\n" );
                break;
            }
            if (token[0] == '}') {
                break;
            }
            N_strncpyz( key, token, sizeof(key) );

            token = COM_ParseExt( text, qfalse );
            if (!token[0]) {
                token = "<NULL>";
            }
            Info_SetValueForKey( info, key, token );
        }
        // NOTE: extra space for level index
        infos[count] = SG_MemAlloc( strlen(info) + strlen("\\num\\") + strlen(va("%i", MAX_LEVELS)) + 1 );
        if (infos[count]) {
            strcpy(infos[count], info);
            count++;
        }
    }

    return count;
}

static void SG_LoadLevelInfoFromFile( const char *filename )
{
    int len;
    file_t f;
    char buf[MAX_LEVELINFO_LEN];

    len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
    if (f == FS_INVALID_HANDLE) {
        G_Printf( COLOR_RED "file not found: %s\n", filename );
        return;
    }
    if (len >= MAX_LEVELINFO_LEN) {
        G_Printf( COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_LEVELINFO_LEN );
        trap_FS_FClose( f );
        return;
    }

    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FClose( f );

    sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
}

void SG_LoadLevelInfos( void )
{
    int numdirs;
    char filename[128];
    char dirlist[1024];
    char *dirptr;
    int i, dirlen;
    int n;

    sg.numLevels = 0;

    if (*sg_levelInfoFile.s) {
        SG_LoadLevelInfoFromFile( sg_levelInfoFile.s );
    }
    else {
        SG_LoadLevelInfoFromFile( "scripts/levels.txt" );
    }

    // get all arenas from .lvl files
    numdirs = trap_FS_GetFileList( "scripts", ".lvl", dirlist, 1024 );
    dirptr = dirlist;
    for (i = 0; i < numdirs; i++, dirptr += dirlen + 1) {
        dirlen = strlen( dirptr );

        // FIXME: possibly use Com_snprintf?

        strcpy( filename, "scripts/" );
        strcat( filename, dirptr );
        SG_LoadLevelInfoFromFile( filename );
    }
    G_Printf( "%i levels parsed\n", sg.numLevels );

    // set initial numbers
    for (n = 0; n < sg.numLevels; n++) {
        Info_SetValueForKey( sg_levelInfos[n], "num", va( "%i", n ) );
    }
}

const char *SG_GetLevelInfoByIndex( int index )
{
    int n;
    const char *value;

    if ( index < 0 || index >= sg.numLevels ) {
        G_Printf( COLOR_RED "Invalid level index: %i\n", index );
        return NULL;
    }

    for ( n = 0; n < sg.numLevels; n++ ) {
        value = Info_ValueForKey( sg_levelInfos[n], "num" );
        if ( *value && atoi(value) == index ) {
            return sg_levelInfos[n];
        }
    }
    
    return NULL;
}

const char *SG_GetLevelInfoByMap( const char *mapname )
{
    int n;

    for ( n = 0; n < sg.numLevels; n++ ) {
        if ( N_stricmp( Info_ValueForKey( sg_levelInfos[n], "map" ), mapname ) == 0 ) {
            return sg_levelInfos[n];
        }
    }
    
    return NULL;
}
