#include "sg_local.h"
#include "sg_imgui.h"

typedef struct
{
    int timeStart;
    int timeEnd;

    int stylePoints;

    int numDeaths;
    int numKills;

    int timeTotal;

    qboolean isCleanRun;
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
    LEVEL_RANK_WERE_U_BOTTING,

    NUMRANKS
} rank_t;

typedef struct {
    rank_t rank;
    int minStyle;
    int minKills;
    int maxTime;
    int maxDeaths;

    qboolean requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
} levelRank_t;

typedef struct {
    levelstats_t stats;

    char name[MAX_NPATH];
    gamedif_t difficulty;
    nhandle_t handle;
} levelMap_t;

typedef struct levelInfo_s
{
    levelMap_t maphandles[NUMDIFS];
    char name[MAX_NPATH];
    int index;

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

    // check if there's an existing save for this level

    // clear the old level data
    memset( &level, 0, sizeof(level) );
    memset( &sg.mapInfo, 0, sizeof(sg.mapInfo) );

    G_SetActiveMap( info->maphandles[ sg_gameDifficulty.i ].handle, &sg.mapInfo, sg.soundBits, &sg.activeEnts );

    SG_InitPlayer();

    // spawn everything
    SG_SpawnLevelEntities();

    level.index = info->index;
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

    Cvar_Set( "sg_levelIndex", va( "%i", level.index ) );

    level.stats.timeStart = Sys_Milliseconds();

    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
    zoom = 1.6f;

    G_Printf( "Done." );

//    G_SetCameraData( cameraPos, zoom, 0.0f );

    return qtrue;
}

//
// SG_SaveLevelData: archives all level data currently loaded
//
void SG_SaveLevelData( void )
{
    int i, n;
    const levelMap_t *m;
    const levelRank_t *rank;
    
    //
    // save current level data
    //
    G_BeginSaveSection( "levelData" );
    {
        G_SaveInt( "checkpointIndex", level.checkpointIndex );
        G_SaveInt( "index", level.index );
        G_BeginSaveSection( "levelStats" );
        {
            G_SaveInt( "numKills", level.stats.numKills );
            G_SaveInt( "numDeaths", level.stats.numDeaths );
            G_SaveInt( "stylePoints", level.stats.stylePoints );
            G_SaveInt( "timeStart", level.stats.timeStart );
            G_SaveInt( "isCleanRun", level.stats.isCleanRun );
        }
        G_EndSaveSection();
    }
    G_EndSaveSection();

    //
    // save all current ranking data
    //
    G_BeginSaveSection( "rankData" );
    G_SaveInt( "numLevels", sg.numLevels );

    for ( i = 0; i < sg.numLevels; i++ ) {
        G_BeginSaveSection( va( "level_%i", i ) );
        G_SaveString( "name", sg_levelInfoData[i].name );

        // save static ranking data
        rank = &sg_levelInfoData[i].a;
        for ( n = 0; n < NUMRANKS; n++ ) {
            G_BeginSaveSection( va( "rank%i_index_%i", n, i ) );
            {
                G_SaveInt( "minKills", rank->minKills );
                G_SaveInt( "minStyle", rank->minStyle );
                G_SaveInt( "maxTime", rank->maxTime );
                G_SaveInt( "maxDeaths", rank->maxDeaths );
                G_SaveInt( "requiresCleanRun", rank->requireClean );
            }
            G_EndSaveSection();

            rank++;
        }

        // save map and active ranking data for each difficulty
        for ( n = 0; n < NUMDIFS; n++ ) {
            m = &sg_levelInfoData[i].maphandles[n];

            G_BeginSaveSection( va( "map_difficulty_%i", m->difficulty ) );
            {
                G_SaveString( "name", m->name );
                G_SaveInt( "numKills", m->stats.numKills );
                G_SaveInt( "numDeaths", m->stats.numDeaths );
                G_SaveInt( "stylePoints", m->stats.stylePoints );
                G_SaveInt( "timeTotal", m->stats.timeTotal );
                G_SaveInt( "isCleanRun", m->stats.isCleanRun );
            }
            G_EndSaveSection();
        }

        G_EndSaveSection();
    }
    G_EndSaveSection();
}

//
// SG_LoadLevelData: loads level data from save file
// NOTE: sgame's level resources MUST be cleared before calling this
// to ensure we aren't fucking stuff up
//
void SG_LoadLevelData( void )
{
    nhandle_t hSection;
    int i, n;
    levelMap_t *m;
    levelRank_t *rank;

    //
    // load current level data
    //
    hSection = G_GetSaveSection( "levelData" );
    if ( hSection == FS_INVALID_HANDLE ) {
        trap_Error( "SG_LoadLevelData: failed to get section \"levelData\" from save file" );
    }
    level.checkpointIndex = G_LoadInt( "checkpointIndex", hSection );
    level.index = G_LoadInt( "index", hSection );

    hSection = G_GetSaveSection( "levelStats" );
    if ( hSection == FS_INVALID_HANDLE ) {
        trap_Error( "SG_LoadLevelData: failed to get section \"levelStats\" from save file" );
    }
    level.stats.numKills = G_LoadInt( "numKills", hSection );
    level.stats.numDeaths = G_LoadInt( "numDeaths", hSection );
    level.stats.stylePoints = G_LoadInt( "stylePoints", hSection );
    level.stats.timeStart = G_LoadInt( "timeStart", hSection );
    level.stats.isCleanRun = G_LoadInt( "isCleanRun", hSection );

    //
    // load all current ranking data
    //
    hSection = G_GetSaveSection( "rankData" );
    if ( hSection == FS_INVALID_HANDLE ) {
        trap_Error( "SG_LoadLevelData: failed to get section \"rankData\" from save file" );
    }
    sg.numLevels = G_LoadInt( "numLevels", hSection );
    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );

    for ( i = 0; i < sg.numLevels; i++ ) {
        hSection = G_GetSaveSection( va( "level_%i", i ) );
        if ( hSection == FS_INVALID_HANDLE ) {
            SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod imcompatibility", va( "level_%i", i ) );
        }
        G_LoadString( "name", sg_levelInfoData[i].name, sizeof(sg_levelInfoData[i].name), hSection );
        
        rank = &sg_levelInfoData[i].a;
        for ( n = 0; n < NUMRANKS; n++ ) {
            hSection = G_GetSaveSection( va( "rank%i_index_%i", n, i ) );
            if ( hSection == FS_INVALID_HANDLE ) {
                SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod incompatibility",
                    va( "rank%i_index_%i", n, i ) );
            }
            rank->minKills = G_LoadInt( "minKills", hSection );
            rank->minStyle = G_LoadInt( "minStyle", hSection );
            rank->maxDeaths = G_LoadInt( "maxDeaths", hSection );
            rank->maxTime = G_LoadInt( "maxTime", hSection );
            rank->requireClean = G_LoadInt( "requiresCleanRun", hSection );

            rank++;
        }

        for ( n = 0; n < NUMDIFS; n++ ) {
            m = &sg_levelInfoData[i].maphandles[n];

            hSection = G_GetSaveSection( va( "map_difficulty_%i", n ) );
            if ( hSection == FS_INVALID_HANDLE ) {
                SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod incompatibility",
                    va( "map_difficulty_%i", n ) );
            }
            m->difficulty = n;
            G_LoadString( "name", m->name, sizeof(m->name), hSection );
            m->stats.numKills = G_LoadInt( "numKills", hSection );
            m->stats.numDeaths = G_LoadInt( "numDeaths", hSection );
            m->stats.stylePoints = G_LoadInt( "stylePoints", hSection );
            m->stats.timeTotal = G_LoadInt( "timeTotal", hSection );
            m->stats.isCleanRun = G_LoadInt( "isCleanRun", hSection );
        }
    }
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
    level.stats.timeEnd = Sys_Milliseconds();

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

    while ( 1 ) {
        token = COM_Parse( text );
        if ( !token[0] ) {
            break;
        }
        if ( token[0] != '{' ) {
            G_Printf( "missing '{' in info file\n" );
            break;
        }

        if ( count == max ) {
            G_Printf( "max infos exceeded\n" );
            break;
        }

        info[0] = '\0';
        while ( 1 ) {
            token = COM_ParseExt( text, qtrue );
            if ( !token[0] ) {
                G_Printf( "unexpected end of info file\n" );
                break;
            }
            if ( token[0] == '}' ) {
                break;
            }
            N_strncpyz( key, token, sizeof(key) );

            token = COM_ParseExt( text, qfalse );
            if ( !token[0] ) {
                token = "<NULL>";
            }
            Info_SetValueForKey( info, key, token );
        }
        // NOTE: extra space for level index
        infos[count] = SG_MemAlloc( strlen( info ) + strlen( "\\num\\" ) + strlen( va( "%i", MAX_LEVELS ) ) + 1 );
        if ( infos[count] ) {
            strcpy( infos[count], info );
            count++;
        }
    }

    return count;
}

static void SG_LoadLevelsFromFile( const char *filename ) {
	int				len;
	fileHandle_t    f;
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
    int i, j;
    int dirlen;
    const char *mapname;
    levelInfo_t *info;

    sg.numLevels = 0;

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

	SG_Printf( "%i levels parsed.\n", sg.numLevels);
	if ( SG_OutOfMemory() ) {
        trap_Error( COLOR_RED "ERROR: not anough memory in pool to load all levels" );
    }

	// set initial numbers
	for ( i = 0; i < sg.numLevels; i++ ) {
		Info_SetValueForKey( sg_levelInfos[i], "num", va( "%i", i ) );
    }

    // if there are no levels, let it crash
    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );

    // load the level information (difficulty, map, etc.)
    for ( i = 0; i < sg.numLevels; i++ ) {
        N_strncpyz( sg_levelInfoData[i].name, Info_ValueForKey( sg_levelInfos[i], "name" ), MAX_NPATH );
        G_Printf( "Loaded Level %s...\n", sg_levelInfoData[i].name );

        for ( j = 0; j < NUMDIFS; j++ ) {
            mapname = Info_ValueForKey( sg_levelInfos[i], va( "mapname_difficulty_%i", j ) );
            if ( !N_stricmp( mapname, "none" ) ) {
                // map currently doesn't support this difficulty
                continue;
            }
            N_strncpyz( sg_levelInfoData[i].maphandles[j].name, mapname, MAX_NPATH );
            sg_levelInfoData[i].maphandles[j].handle = G_LoadMap( mapname );
            sg_levelInfoData[i].maphandles[j].difficulty = j;
            if ( sg_levelInfoData[i].maphandles[j].handle == FS_INVALID_HANDLE ) {
                G_Printf( COLOR_YELLOW "WARNING: failed to load map '%s' for level '%s'\n", mapname, sg_levelInfoData[i].name );
                continue;
            }
        }

        sg_levelInfoData[i].index = i;
        
        sg_levelInfoData[i].a.rank = LEVEL_RANK_A;
        sg_levelInfoData[i].a.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minKills" ) );
        sg_levelInfoData[i].a.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minStyle" ) );
        sg_levelInfoData[i].a.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_maxTime" ) );
        sg_levelInfoData[i].a.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_maxDeaths" ) );
        sg_levelInfoData[i].a.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_cleanRunRequired" ) );

        sg_levelInfoData[i].b.rank = LEVEL_RANK_B;
        sg_levelInfoData[i].b.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minKills" ) );
        sg_levelInfoData[i].b.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minStyle" ) );
        sg_levelInfoData[i].b.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_maxTime" ) );
        sg_levelInfoData[i].b.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_maxDeaths" ) );
        sg_levelInfoData[i].b.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_cleanRunRequired" ) );

        sg_levelInfoData[i].c.rank = LEVEL_RANK_C;
        sg_levelInfoData[i].c.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minKills" ) );
        sg_levelInfoData[i].c.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minStyle" ) );
        sg_levelInfoData[i].c.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_maxTime" ) );
        sg_levelInfoData[i].c.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_maxDeaths" ) );
        sg_levelInfoData[i].c.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_cleanRunRequired" ) );

        sg_levelInfoData[i].d.rank = LEVEL_RANK_D;
        sg_levelInfoData[i].d.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minKills" ) );
        sg_levelInfoData[i].d.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minStyle" ) );
        sg_levelInfoData[i].d.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_maxTime" ) );
        sg_levelInfoData[i].d.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_maxDeaths" ) );
        sg_levelInfoData[i].d.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_cleanRunRequired" ) );

        sg_levelInfoData[i].f.rank = LEVEL_RANK_WERE_U_BOTTING;
        sg_levelInfoData[i].f.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minKills" ) );
        sg_levelInfoData[i].f.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minStyle" ) );
        sg_levelInfoData[i].f.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_maxTime" ) );
        sg_levelInfoData[i].f.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_maxDeaths" ) );
        sg_levelInfoData[i].f.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_cleanRunRequired" ) );
    }
}
