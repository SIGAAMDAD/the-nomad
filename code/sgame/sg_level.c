#include "sg_local.h"
#include "sg_imgui.h"

typedef struct
{
    int timeStart;
    int timeEnd;

    int stylePoints;

    int numDeaths;
    int numKills;
} levelstats_t;

typedef struct
{
    int index;

    // save data
    levelstats_t stats;
    int checkpointIndex;
} level_t;

typedef enum {
    LEVEL_RANK_A,
    LEVEL_RANK_B,
    LEVEL_RANK_C,
    LEVEL_RANK_D,
    LEVEL_RANK_WERE_U_BOTTING
} rank_t;

typedef struct {
    rank_t rank;
    int minStyle;
    int minKills;
    int minTime;

    qboolean requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
} levelRank_t;

typedef struct levelInfo_s
{
    char name[MAX_NPATH];
    nhandle_t maphandle;
    gamedif_t difficulty;

    // ranking info
    levelRank_t a;
    levelRank_t b;
    levelRank_t c;
    levelRank_t d;
    levelRank_t f;
} levelInfo_t;

static levelInfo_t *sg_levelInfoData;
static level_t level;

static levelInfo_t *SG_GetLevelInfoByMapName( const char *mapname )
{
    int i;

    for ( i = 0; i < sg.numLevels; i++ ) {
        if ( !N_stricmp( sg_levelInfoData[i].name, mapname ) )  {
            return &sg_levelInfoData[i];
        }
    }
    return NULL;
}

//
// SG_SpawnLevelEntities
//
static void SG_SpawnLevelEntities( void )
{
    int i;
    const mapspawn_t *spawn;

    spawn = sg.mapInfo.spawns;
    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
        SG_Spawn( spawn->entityid, spawn->entitytype, &spawn->xyz );
    }
}

qboolean SG_StartLevel( void )
{
    vec2_t cameraPos;
    float zoom;
    char mapname[MAX_NPATH];
    levelInfo_t *info;

    Cvar_VariableStringBuffer( "mapname", mapname, sizeof(mapname) );

    info = SG_GetLevelInfoByMapName( mapname );
    if ( !info ) {
        G_Error( "Couldn't find level info for map '%s'", mapname );
    }

    G_Printf( "Starting up level %s...\n", info->name );

    // clear the old level data
    memset( &level, 0, sizeof(level) );
    memset( &sg.mapInfo, 0, sizeof(sg.mapInfo) );

    G_SetActiveMap( info->maphandle, &sg.mapInfo, sg.soundBits, &sg.activeEnts );

    SG_InitPlayer();

    // spawn everything
    SG_SpawnLevelEntities();

    sg.state = SG_IN_LEVEL;

    if ( sg_printLevelStats.i ) {
        G_Printf( "\n---------- Level Info ----------\n" );
        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
    }

    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );

    Cvar_Set( "sg_levelIndex", va( "%i", (int)(uintptr_t)( info - sg_levelInfoData ) ) );

    level.stats.timeStart = trap_Milliseconds();

    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
    zoom = 1.6f;

    G_Printf( "Done." );

//    G_SetCameraData( cameraPos, zoom, 0.0f );

    return qtrue;
}

void SG_SaveLevelData( void )
{
    int i;

    trap_BeginSaveSection( "level" );

    trap_SaveInt( "checkpoint_index", level.checkpointIndex );
    trap_SaveInt( "level_index", level.index );

    trap_SaveVec2( "sg_camera_position", &sg.cameraPos );

    //
    // archive entity data
    //
    for ( i = 0; i < sg.numEntities; i++ ) {

    }

    trap_EndSaveSection();
}

void SG_LoadLevelData( void )
{
    nhandle_t section;

    section = trap_GetSaveSection( "level" );
    if ( section == FS_INVALID_HANDLE ) {
        trap_Error( "SG_LoadLevelData: failed to fetch \"level\" section from save file!" );
    }

    level.checkpointIndex = trap_LoadInt( "checkpoint_index", section );
    level.index = trap_LoadInt( "level_index", section );

    G_SetActiveMap( level.index, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
}

void SG_DrawLevelStats( void )
{
    float font_scale;
    vec2_t cursorPos;

    font_scale = ImGui_GetFontScale();

    if ( ImGui_BeginWindow( "EndLevel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) ) {
        ImGui_SetWindowFontScale(font_scale * 6);
        ImGui_TextUnformatted("Level Statistics");
        ImGui_SetWindowFontScale(font_scale * 3.5f);
        ImGui_NewLine();

        ImGui_GetCursorScreenPos( &cursorPos.x, &cursorPos.y );

        ImGui_SetCursorScreenPos( cursorPos.x, cursorPos.y + 20);
    }
    ImGui_EndWindow();
}

int SG_EndLevel( void )
{
    level.stats.timeEnd = trap_Milliseconds();

    sg.state = SG_SHOW_LEVEL_STATS;

    return 1;
}

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

static void SG_LoadLevelInfos( void )
{
    int numdirs;
    char filename[128];
    char dirlist[1024];
    char *dirptr;
    int i, dirlen;
    int n;
    vmCvar_t levelInfoFile;

    sg.numLevels = 0;

    Cvar_Register( &levelInfoFile, "sg_levelInfoFile", "", CVAR_INIT | CVAR_ROM );
    if ( *levelInfoFile.s ) {
        SG_LoadLevelInfoFromFile( levelInfoFile.s );
    } else {
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
    G_Printf( "%i levels parsed.\n", sg.numLevels );

    // set initial numbers
    for ( n = 0; n < sg.numLevels; n++ ) {
        Info_SetValueForKey( sg_levelInfos[n], "num", va( "%i", n ) );
    }
}

static void SG_LoadLevelsFromFile( const char *filename ) {
	int				len;
	file_t	        f;
	char			buf[MAX_LEVELINFO_LEN];

	len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
	if ( !f ) {
		trap_Print( va( COLOR_RED "ERROR: file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_LEVELINFO_LEN ) {
		trap_Print( va( COLOR_RED "ERROR: file too large: %s is %i, max allowed is %i", filename, len, MAX_LEVELINFO_LEN ) );
		trap_FS_FClose( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FClose( f );

	sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
}

void SG_LoadLevels( void )
{
    int numdirs;
    vmCvar_t levelsFile;
    char filename[1024];
    char dirlist[1024];
    char *dirptr;
    int i;
    int dirlen;
    const char *mapname;
    levelInfo_t *info;

    sg.numLevels = 0;

    G_Printf( "Loading level configs...\n" );

    Cvar_Register( &levelsFile, "sg_levelsFile", "", CVAR_INIT | CVAR_ROM );
    if ( *levelsFile.s ) {
        SG_LoadLevelsFromFile( levelsFile.s );
    } else {
        SG_LoadLevelsFromFile( "scripts/levels.txt" );
    }

    // get all levels from .level files
    numdirs = trap_FS_GetFileList( "scripts", ".level", dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
		dirlen = strlen( dirptr );
		strcpy( filename, "scripts/" );
		strcat( filename, dirptr );
		SG_LoadLevelsFromFile( filename );
	}
	trap_Print( va( "%i levels parsed\n", sg.numLevels ) );
	if ( SG_OutOfMemory() ) {
        trap_Error( COLOR_RED "ERROR: not anough memory in pool to load all levels" );
    }

	// set initial numbers
	for ( i = 0; i < sg.numLevels; i++ ) {
		Info_SetValueForKey( sg_levelInfos[i], "num", va( "%i", i ) );
    }

    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );

    // load the level information (difficulty, map, etc.)
    for ( i = 0; i < sg.numLevels; i++ ) {
        N_strncpyz( sg_levelInfoData[i].name, Info_ValueForKey( sg_levelInfos[i], "name" ), MAX_NPATH );

        mapname = Info_ValueForKey( sg_levelInfos[i], "mapname" );
        sg_levelInfoData[i].maphandle = G_LoadMap( mapname );
        if ( sg_levelInfoData[i].maphandle == FS_INVALID_HANDLE ) {
            G_Printf( COLOR_YELLOW "WARNING: failed to load map '%s' for level '%s'\n", mapname, sg_levelInfoData[i].name );
            continue;
        }
        
        sg_levelInfoData[i].a.rank = LEVEL_RANK_A;
        sg_levelInfoData[i].a.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minKills" ) );
        sg_levelInfoData[i].a.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minStyle" ) );
        sg_levelInfoData[i].a.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minTime" ) );
        sg_levelInfoData[i].a.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );

        sg_levelInfoData[i].b.rank = LEVEL_RANK_B;
        sg_levelInfoData[i].b.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minKills" ) );
        sg_levelInfoData[i].b.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minStyle" ) );
        sg_levelInfoData[i].b.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minTime" ) );
        sg_levelInfoData[i].b.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );

        sg_levelInfoData[i].c.rank = LEVEL_RANK_C;
        sg_levelInfoData[i].c.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minKills" ) );
        sg_levelInfoData[i].c.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minStyle" ) );
        sg_levelInfoData[i].c.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minTime" ) );
        sg_levelInfoData[i].c.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );

        sg_levelInfoData[i].d.rank = LEVEL_RANK_D;
        sg_levelInfoData[i].d.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minKills" ) );
        sg_levelInfoData[i].d.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minStyle" ) );
        sg_levelInfoData[i].d.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minTime" ) );
        sg_levelInfoData[i].d.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );

        sg_levelInfoData[i].f.rank = LEVEL_RANK_WERE_U_BOTTING;
        sg_levelInfoData[i].f.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minKills" ) );
        sg_levelInfoData[i].f.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minStyle" ) );
        sg_levelInfoData[i].f.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minTime" ) );
        sg_levelInfoData[i].f.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
    }
}
