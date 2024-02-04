code
proc SG_GetLevelInfoByMapName 8 8
file "../sg_level.c"
line 74
;1:#include "sg_local.h"
;2:#include "sg_imgui.h"
;3:
;4:typedef struct
;5:{
;6:    int timeStart;
;7:    int timeEnd;
;8:
;9:    int stylePoints;
;10:
;11:    int numDeaths;
;12:    int numKills;
;13:
;14:    int timeTotal;
;15:
;16:    qboolean isCleanRun;
;17:} levelstats_t;
;18:
;19:typedef struct
;20:{
;21:    int index;
;22:
;23:    // save data
;24:    levelstats_t stats;
;25:    int checkpointIndex;
;26:} level_t;
;27:
;28:typedef enum {
;29:    LEVEL_RANK_A,
;30:    LEVEL_RANK_B,
;31:    LEVEL_RANK_C,
;32:    LEVEL_RANK_D,
;33:    LEVEL_RANK_WERE_U_BOTTING,
;34:
;35:    NUMRANKS
;36:} rank_t;
;37:
;38:typedef struct {
;39:    rank_t rank;
;40:    int minStyle;
;41:    int minKills;
;42:    int maxTime;
;43:    int maxDeaths;
;44:
;45:    qboolean requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
;46:} levelRank_t;
;47:
;48:typedef struct {
;49:    levelstats_t stats;
;50:
;51:    char name[MAX_NPATH];
;52:    gamedif_t difficulty;
;53:    nhandle_t handle;
;54:} levelMap_t;
;55:
;56:typedef struct levelInfo_s
;57:{
;58:    levelMap_t maphandles[NUMDIFS];
;59:    char name[MAX_NPATH];
;60:    int index;
;61:
;62:    // ranking info
;63:    levelRank_t a;
;64:    levelRank_t b;
;65:    levelRank_t c;
;66:    levelRank_t d;
;67:    levelRank_t f;
;68:} levelInfo_t;
;69:
;70:static levelInfo_t *sg_levelInfoData;
;71:static level_t level;
;72:
;73:static levelInfo_t *SG_GetLevelInfoByMapName( const char *mapname )
;74:{
line 77
;75:    int i;
;76:
;77:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $98
JUMPV
LABELV $95
line 78
;78:        if ( !N_stricmp( sg_levelInfoData[i].name, mapname ) )  {
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $100
line 79
;79:            return &sg_levelInfoData[i];
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
RETP4
ADDRGP4 $94
JUMPV
LABELV $100
line 81
;80:        }
;81:    }
LABELV $96
line 77
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $98
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $95
line 82
;82:    return NULL;
CNSTP4 0
RETP4
LABELV $94
endproc SG_GetLevelInfoByMapName 8 8
proc SG_SpawnLevelEntities 12 12
line 89
;83:}
;84:
;85://
;86:// SG_SpawnLevelEntities
;87://
;88:static void SG_SpawnLevelEntities( void )
;89:{
line 93
;90:    int i;
;91:    const mapspawn_t *spawn;
;92:
;93:    spawn = sg.mapInfo.spawns;
ADDRLP4 0
ADDRGP4 sg+4270540
ASGNP4
line 94
;94:    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $107
JUMPV
LABELV $104
line 95
;95:        SG_Spawn( spawn->entityid, spawn->entitytype, &spawn->xyz );
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
INDIRU4
ARGU4
ADDRLP4 0
INDIRP4
CNSTI4 12
ADDP4
INDIRU4
ARGU4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 SG_Spawn
CALLV
pop
line 96
;96:    }
LABELV $105
line 94
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
ASGNP4
LABELV $107
ADDRLP4 4
INDIRI4
ADDRGP4 sg+4270540+23624
INDIRI4
LTI4 $104
line 97
;97:}
LABELV $102
endproc SG_SpawnLevelEntities 12 12
export SG_StartLevel
proc SG_StartLevel 104 16
line 100
;98:
;99:qboolean SG_StartLevel( void )
;100:{
line 106
;101:    vec2_t cameraPos;
;102:    float zoom;
;103:    char mapname[MAX_NPATH];
;104:    levelInfo_t *info;
;105:
;106:    Cvar_VariableStringBuffer( "mapname", mapname, sizeof(mapname) );
ADDRGP4 $111
ARGP4
ADDRLP4 4
ARGP4
CNSTI4 64
ARGI4
ADDRGP4 Cvar_VariableStringBuffer
CALLV
pop
line 108
;107:
;108:    info = SG_GetLevelInfoByMapName( mapname );
ADDRLP4 4
ARGP4
ADDRLP4 80
ADDRGP4 SG_GetLevelInfoByMapName
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 80
INDIRP4
ASGNP4
line 109
;109:    if ( !info ) {
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $112
line 110
;110:        G_Error( "Couldn't find level info for map '%s'", mapname );
ADDRGP4 $114
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 G_Error
CALLV
pop
line 111
;111:    }
LABELV $112
line 113
;112:
;113:    G_Printf( "Starting up level %s...\n", info->name );
ADDRGP4 $115
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 600
ADDP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 118
;114:
;115:    // check if there's an existing save for this level
;116:
;117:    // clear the old level data
;118:    memset( &level, 0, sizeof(level) );
ADDRGP4 level
ARGP4
CNSTI4 0
ARGI4
CNSTU4 36
ARGU4
ADDRGP4 memset
CALLI4
pop
line 119
;119:    memset( &sg.mapInfo, 0, sizeof(sg.mapInfo) );
ADDRGP4 sg+4270540
ARGP4
CNSTI4 0
ARGI4
CNSTU4 23632
ARGU4
ADDRGP4 memset
CALLI4
pop
line 121
;120:
;121:    G_SetActiveMap( info->maphandles[ sg_gameDifficulty.i ].handle, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
ADDRGP4 sg_gameDifficulty+260
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 0
INDIRP4
ADDP4
CNSTI4 96
ADDP4
INDIRI4
ARGI4
ADDRGP4 sg+4270540
ARGP4
ADDRGP4 sg+76228
ARGP4
ADDRGP4 sg+76188
ARGP4
ADDRGP4 G_SetActiveMap
CALLV
pop
line 123
;122:
;123:    SG_InitPlayer();
ADDRGP4 SG_InitPlayer
CALLV
pop
line 126
;124:
;125:    // spawn everything
;126:    SG_SpawnLevelEntities();
ADDRGP4 SG_SpawnLevelEntities
CALLV
pop
line 128
;127:
;128:    level.index = info->index;
ADDRGP4 level
ADDRLP4 0
INDIRP4
CNSTI4 664
ADDP4
INDIRI4
ASGNI4
line 129
;129:    sg.state = SG_IN_LEVEL;
ADDRGP4 sg+2212
CNSTI4 3
ASGNI4
line 131
;130:
;131:    if ( sg_printLevelStats.i ) {
ADDRGP4 sg_printLevelStats+260
INDIRI4
CNSTI4 0
EQI4 $123
line 132
;132:        G_Printf( "\n---------- Level Info ----------\n" );
ADDRGP4 $126
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 133
;133:        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
ADDRGP4 $127
ARGP4
ADDRGP4 sg+4270540+23552
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 134
;134:        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
ADDRGP4 $130
ARGP4
ADDRGP4 sg+4270540+23628
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 135
;135:        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
ADDRGP4 $133
ARGP4
ADDRGP4 sg+4270540+23624
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 136
;136:        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
ADDRGP4 $136
ARGP4
ADDRGP4 sg+4270540+23616
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 137
;137:        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
ADDRGP4 $139
ARGP4
ADDRGP4 sg+4270540+23620
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 138
;138:    }
LABELV $123
line 140
;139:
;140:    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );
ADDRGP4 $142
ARGP4
ADDRGP4 sg+4270540+23552
ARGP4
ADDRLP4 84
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 84
INDIRP4
ARGP4
ADDRGP4 RE_LoadWorldMap
CALLV
pop
line 142
;141:
;142:    Cvar_Set( "sg_levelIndex", va( "%i", level.index ) );
ADDRGP4 $146
ARGP4
ADDRGP4 level
INDIRI4
ARGI4
ADDRLP4 88
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $145
ARGP4
ADDRLP4 88
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 144
;143:
;144:    level.stats.timeStart = Sys_Milliseconds();
ADDRLP4 92
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4
ADDRLP4 92
INDIRI4
ASGNI4
line 146
;145:
;146:    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
ADDRLP4 96
ADDRGP4 sg+4270540
INDIRU4
ASGNU4
ADDRLP4 68
ADDRLP4 96
INDIRU4
CNSTI4 1
RSHU4
CVUI4 4
CVIF4 4
CNSTF4 1073741824
MULF4
ADDRLP4 96
INDIRU4
CNSTU4 1
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
ADDRLP4 100
ADDRGP4 sg+4270540+4
INDIRU4
ASGNU4
ADDRLP4 68+4
ADDRLP4 100
INDIRU4
CNSTI4 1
RSHU4
CVUI4 4
CVIF4 4
CNSTF4 1073741824
MULF4
ADDRLP4 100
INDIRU4
CNSTU4 1
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
line 147
;147:    zoom = 1.6f;
ADDRLP4 76
CNSTF4 1070386381
ASGNF4
line 149
;148:
;149:    G_Printf( "Done." );
ADDRGP4 $152
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 153
;150:
;151://    G_SetCameraData( cameraPos, zoom, 0.0f );
;152:
;153:    return qtrue;
CNSTI4 1
RETI4
LABELV $110
endproc SG_StartLevel 104 16
export SG_SaveLevelData
proc SG_SaveLevelData 24 12
line 160
;154:}
;155:
;156://
;157:// SG_SaveLevelData: archives all level data currently loaded
;158://
;159:void SG_SaveLevelData( void )
;160:{
line 168
;161:    int i, n;
;162:    const levelMap_t *m;
;163:    const levelRank_t *rank;
;164:    
;165:    //
;166:    // save current level data
;167:    //
;168:    G_BeginSaveSection( "levelData" );
ADDRGP4 $154
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 169
;169:    {
line 170
;170:        G_SaveInt( "checkpointIndex", level.checkpointIndex );
ADDRGP4 $155
ARGP4
ADDRGP4 level+32
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 171
;171:        G_SaveInt( "index", level.index );
ADDRGP4 $157
ARGP4
ADDRGP4 level
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 172
;172:        G_BeginSaveSection( "levelStats" );
ADDRGP4 $158
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 173
;173:        {
line 174
;174:            G_SaveInt( "numKills", level.stats.numKills );
ADDRGP4 $159
ARGP4
ADDRGP4 level+4+16
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 175
;175:            G_SaveInt( "numDeaths", level.stats.numDeaths );
ADDRGP4 $162
ARGP4
ADDRGP4 level+4+12
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 176
;176:            G_SaveInt( "stylePoints", level.stats.stylePoints );
ADDRGP4 $165
ARGP4
ADDRGP4 level+4+8
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 177
;177:            G_SaveInt( "timeStart", level.stats.timeStart );
ADDRGP4 $168
ARGP4
ADDRGP4 level+4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 178
;178:            G_SaveInt( "isCleanRun", level.stats.isCleanRun );
ADDRGP4 $170
ARGP4
ADDRGP4 level+4+24
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 179
;179:        }
line 180
;180:        G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 181
;181:    }
line 182
;182:    G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 187
;183:
;184:    //
;185:    // save all current ranking data
;186:    //
;187:    G_BeginSaveSection( "rankData" );
ADDRGP4 $173
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 188
;188:    G_SaveInt( "numLevels", sg.numLevels );
ADDRGP4 $174
ARGP4
ADDRGP4 sg+2220
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 190
;189:
;190:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 12
CNSTI4 0
ASGNI4
ADDRGP4 $179
JUMPV
LABELV $176
line 191
;191:        G_BeginSaveSection( va( "level_%i", i ) );
ADDRGP4 $181
ARGP4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 16
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 16
INDIRP4
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 192
;192:        G_SaveString( "name", sg_levelInfoData[i].name );
ADDRGP4 $182
ARGP4
ADDRLP4 12
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
ADDRGP4 G_SaveString
CALLV
pop
line 195
;193:
;194:        // save static ranking data
;195:        rank = &sg_levelInfoData[i].a;
ADDRLP4 8
ADDRLP4 12
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 668
ADDP4
ASGNP4
line 196
;196:        for ( n = 0; n < NUMRANKS; n++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $183
line 197
;197:            G_BeginSaveSection( va( "rank%i_index_%i", n, i ) );
ADDRGP4 $187
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 12
INDIRI4
ARGI4
ADDRLP4 20
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 20
INDIRP4
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 198
;198:            {
line 199
;199:                G_SaveInt( "minKills", rank->minKills );
ADDRGP4 $188
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 8
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 200
;200:                G_SaveInt( "minStyle", rank->minStyle );
ADDRGP4 $189
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 4
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 201
;201:                G_SaveInt( "maxTime", rank->maxTime );
ADDRGP4 $190
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 12
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 202
;202:                G_SaveInt( "maxDeaths", rank->maxDeaths );
ADDRGP4 $191
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 16
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 203
;203:                G_SaveInt( "requiresCleanRun", rank->requireClean );
ADDRGP4 $192
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 20
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 204
;204:            }
line 205
;205:            G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 207
;206:
;207:            rank++;
ADDRLP4 8
ADDRLP4 8
INDIRP4
CNSTI4 24
ADDP4
ASGNP4
line 208
;208:        }
LABELV $184
line 196
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 5
LTI4 $183
line 211
;209:
;210:        // save map and active ranking data for each difficulty
;211:        for ( n = 0; n < NUMDIFS; n++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $193
line 212
;212:            m = &sg_levelInfoData[i].maphandles[n];
ADDRLP4 0
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 12
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
ASGNP4
line 214
;213:
;214:            G_BeginSaveSection( va( "map_difficulty_%i", m->difficulty ) );
ADDRGP4 $197
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRI4
ARGI4
ADDRLP4 20
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 20
INDIRP4
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 215
;215:            {
line 216
;216:                G_SaveString( "name", m->name );
ADDRGP4 $182
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 28
ADDP4
ARGP4
ADDRGP4 G_SaveString
CALLV
pop
line 217
;217:                G_SaveInt( "numKills", m->stats.numKills );
ADDRGP4 $159
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 218
;218:                G_SaveInt( "numDeaths", m->stats.numDeaths );
ADDRGP4 $162
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 12
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 219
;219:                G_SaveInt( "stylePoints", m->stats.stylePoints );
ADDRGP4 $165
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 220
;220:                G_SaveInt( "timeTotal", m->stats.timeTotal );
ADDRGP4 $198
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 221
;221:                G_SaveInt( "isCleanRun", m->stats.isCleanRun );
ADDRGP4 $170
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 24
ADDP4
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 222
;222:            }
line 223
;223:            G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 224
;224:        }
LABELV $194
line 211
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 6
LTI4 $193
line 226
;225:
;226:        G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 227
;227:    }
LABELV $177
line 190
ADDRLP4 12
ADDRLP4 12
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $179
ADDRLP4 12
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $176
line 228
;228:    G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 229
;229:}
LABELV $153
endproc SG_SaveLevelData 24 12
export SG_LoadLevelData
proc SG_LoadLevelData 104 16
line 237
;230:
;231://
;232:// SG_LoadLevelData: loads level data from save file
;233:// NOTE: sgame's level resources MUST be cleared before calling this
;234:// to ensure we aren't fucking stuff up
;235://
;236:void SG_LoadLevelData( void )
;237:{
line 246
;238:    nhandle_t hSection;
;239:    int i, n;
;240:    levelMap_t *m;
;241:    levelRank_t *rank;
;242:
;243:    //
;244:    // load current level data
;245:    //
;246:    hSection = G_GetSaveSection( "levelData" );
ADDRGP4 $154
ARGP4
ADDRLP4 20
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 20
INDIRI4
ASGNI4
line 247
;247:    if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $200
line 248
;248:        trap_Error( "SG_LoadLevelData: failed to get section \"levelData\" from save file" );
ADDRGP4 $202
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 249
;249:    }
LABELV $200
line 250
;250:    level.checkpointIndex = G_LoadInt( "checkpointIndex", hSection );
ADDRGP4 $155
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 24
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+32
ADDRLP4 24
INDIRI4
ASGNI4
line 251
;251:    level.index = G_LoadInt( "index", hSection );
ADDRGP4 $157
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 28
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level
ADDRLP4 28
INDIRI4
ASGNI4
line 253
;252:
;253:    hSection = G_GetSaveSection( "levelStats" );
ADDRGP4 $158
ARGP4
ADDRLP4 32
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 32
INDIRI4
ASGNI4
line 254
;254:    if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $204
line 255
;255:        trap_Error( "SG_LoadLevelData: failed to get section \"levelStats\" from save file" );
ADDRGP4 $206
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 256
;256:    }
LABELV $204
line 257
;257:    level.stats.numKills = G_LoadInt( "numKills", hSection );
ADDRGP4 $159
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 36
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+4+16
ADDRLP4 36
INDIRI4
ASGNI4
line 258
;258:    level.stats.numDeaths = G_LoadInt( "numDeaths", hSection );
ADDRGP4 $162
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 40
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+4+12
ADDRLP4 40
INDIRI4
ASGNI4
line 259
;259:    level.stats.stylePoints = G_LoadInt( "stylePoints", hSection );
ADDRGP4 $165
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 44
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+4+8
ADDRLP4 44
INDIRI4
ASGNI4
line 260
;260:    level.stats.timeStart = G_LoadInt( "timeStart", hSection );
ADDRGP4 $168
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 48
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+4
ADDRLP4 48
INDIRI4
ASGNI4
line 261
;261:    level.stats.isCleanRun = G_LoadInt( "isCleanRun", hSection );
ADDRGP4 $170
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 52
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+4+24
ADDRLP4 52
INDIRI4
ASGNI4
line 266
;262:
;263:    //
;264:    // load all current ranking data
;265:    //
;266:    hSection = G_GetSaveSection( "rankData" );
ADDRGP4 $173
ARGP4
ADDRLP4 56
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 56
INDIRI4
ASGNI4
line 267
;267:    if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $216
line 268
;268:        trap_Error( "SG_LoadLevelData: failed to get section \"rankData\" from save file" );
ADDRGP4 $218
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 269
;269:    }
LABELV $216
line 270
;270:    sg.numLevels = G_LoadInt( "numLevels", hSection );
ADDRGP4 $174
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 60
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 sg+2220
ADDRLP4 60
INDIRI4
ASGNI4
line 271
;271:    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );
ADDRGP4 sg+2220
INDIRI4
CVIU4 4
CNSTU4 788
MULU4
CVUI4 4
ARGI4
ADDRLP4 64
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRGP4 sg_levelInfoData
ADDRLP4 64
INDIRP4
ASGNP4
line 273
;272:
;273:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 16
CNSTI4 0
ASGNI4
ADDRGP4 $224
JUMPV
LABELV $221
line 274
;274:        hSection = G_GetSaveSection( va( "level_%i", i ) );
ADDRGP4 $181
ARGP4
ADDRLP4 16
INDIRI4
ARGI4
ADDRLP4 68
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 68
INDIRP4
ARGP4
ADDRLP4 72
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 72
INDIRI4
ASGNI4
line 275
;275:        if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $226
line 276
;276:            SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod imcompatibility", va( "level_%i", i ) );
ADDRGP4 $181
ARGP4
ADDRLP4 16
INDIRI4
ARGI4
ADDRLP4 76
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $228
ARGP4
ADDRLP4 76
INDIRP4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 277
;277:        }
LABELV $226
line 278
;278:        G_LoadString( "name", sg_levelInfoData[i].name, sizeof(sg_levelInfoData[i].name), hSection );
ADDRGP4 $182
ARGP4
ADDRLP4 16
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
CNSTI4 64
ARGI4
ADDRLP4 0
INDIRI4
ARGI4
ADDRGP4 G_LoadString
CALLV
pop
line 280
;279:        
;280:        rank = &sg_levelInfoData[i].a;
ADDRLP4 12
ADDRLP4 16
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 668
ADDP4
ASGNP4
line 281
;281:        for ( n = 0; n < NUMRANKS; n++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $229
line 282
;282:            hSection = G_GetSaveSection( va( "rank%i_index_%i", n, i ) );
ADDRGP4 $187
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRLP4 76
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 76
INDIRP4
ARGP4
ADDRLP4 80
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 80
INDIRI4
ASGNI4
line 283
;283:            if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $233
line 284
;284:                SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod incompatibility",
ADDRGP4 $187
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 16
INDIRI4
ARGI4
ADDRLP4 84
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $235
ARGP4
ADDRLP4 84
INDIRP4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 286
;285:                    va( "rank%i_index_%i", n, i ) );
;286:            }
LABELV $233
line 287
;287:            rank->minKills = G_LoadInt( "minKills", hSection );
ADDRGP4 $188
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 84
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 12
INDIRP4
CNSTI4 8
ADDP4
ADDRLP4 84
INDIRI4
ASGNI4
line 288
;288:            rank->minStyle = G_LoadInt( "minStyle", hSection );
ADDRGP4 $189
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 88
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 12
INDIRP4
CNSTI4 4
ADDP4
ADDRLP4 88
INDIRI4
ASGNI4
line 289
;289:            rank->maxDeaths = G_LoadInt( "maxDeaths", hSection );
ADDRGP4 $191
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 92
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 12
INDIRP4
CNSTI4 16
ADDP4
ADDRLP4 92
INDIRI4
ASGNI4
line 290
;290:            rank->maxTime = G_LoadInt( "maxTime", hSection );
ADDRGP4 $190
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 96
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 12
INDIRP4
CNSTI4 12
ADDP4
ADDRLP4 96
INDIRI4
ASGNI4
line 291
;291:            rank->requireClean = G_LoadInt( "requiresCleanRun", hSection );
ADDRGP4 $192
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 100
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 12
INDIRP4
CNSTI4 20
ADDP4
ADDRLP4 100
INDIRI4
ASGNI4
line 293
;292:
;293:            rank++;
ADDRLP4 12
ADDRLP4 12
INDIRP4
CNSTI4 24
ADDP4
ASGNP4
line 294
;294:        }
LABELV $230
line 281
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 5
LTI4 $229
line 296
;295:
;296:        for ( n = 0; n < NUMDIFS; n++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $236
line 297
;297:            m = &sg_levelInfoData[i].maphandles[n];
ADDRLP4 8
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 16
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
ASGNP4
line 299
;298:
;299:            hSection = G_GetSaveSection( va( "map_difficulty_%i", n ) );
ADDRGP4 $197
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 76
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 76
INDIRP4
ARGP4
ADDRLP4 80
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 80
INDIRI4
ASGNI4
line 300
;300:            if ( hSection == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $240
line 301
;301:                SG_Error( "SG_LoadLevelData: failed to get section \"%s\" from save file, possible mod incompatibility",
ADDRGP4 $197
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 84
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $235
ARGP4
ADDRLP4 84
INDIRP4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 303
;302:                    va( "map_difficulty_%i", n ) );
;303:            }
LABELV $240
line 304
;304:            m->difficulty = n;
ADDRLP4 8
INDIRP4
CNSTI4 92
ADDP4
ADDRLP4 4
INDIRI4
ASGNI4
line 305
;305:            G_LoadString( "name", m->name, sizeof(m->name), hSection );
ADDRGP4 $182
ARGP4
ADDRLP4 8
INDIRP4
CNSTI4 28
ADDP4
ARGP4
CNSTI4 64
ARGI4
ADDRLP4 0
INDIRI4
ARGI4
ADDRGP4 G_LoadString
CALLV
pop
line 306
;306:            m->stats.numKills = G_LoadInt( "numKills", hSection );
ADDRGP4 $159
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 84
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 8
INDIRP4
CNSTI4 16
ADDP4
ADDRLP4 84
INDIRI4
ASGNI4
line 307
;307:            m->stats.numDeaths = G_LoadInt( "numDeaths", hSection );
ADDRGP4 $162
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 88
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 8
INDIRP4
CNSTI4 12
ADDP4
ADDRLP4 88
INDIRI4
ASGNI4
line 308
;308:            m->stats.stylePoints = G_LoadInt( "stylePoints", hSection );
ADDRGP4 $165
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 92
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 8
INDIRP4
CNSTI4 8
ADDP4
ADDRLP4 92
INDIRI4
ASGNI4
line 309
;309:            m->stats.timeTotal = G_LoadInt( "timeTotal", hSection );
ADDRGP4 $198
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 96
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 8
INDIRP4
CNSTI4 20
ADDP4
ADDRLP4 96
INDIRI4
ASGNI4
line 310
;310:            m->stats.isCleanRun = G_LoadInt( "isCleanRun", hSection );
ADDRGP4 $170
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 100
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRLP4 8
INDIRP4
CNSTI4 24
ADDP4
ADDRLP4 100
INDIRI4
ASGNI4
line 311
;311:        }
LABELV $237
line 296
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 6
LTI4 $236
line 312
;312:    }
LABELV $222
line 273
ADDRLP4 16
ADDRLP4 16
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $224
ADDRLP4 16
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $221
line 313
;313:}
LABELV $199
endproc SG_LoadLevelData 104 16
export SG_DrawLevelStats
proc SG_DrawLevelStats 20 12
line 316
;314:
;315:void SG_DrawLevelStats( void )
;316:{
line 320
;317:    float font_scale;
;318:    vec2_t cursorPos;
;319:
;320:    font_scale = ImGui_GetFontScale();
ADDRLP4 12
ADDRGP4 ImGui_GetFontScale
CALLF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ASGNF4
line 322
;321:
;322:    if ( ImGui_BeginWindow( "EndLevel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) ) {
ADDRGP4 $245
ARGP4
CNSTP4 0
ARGP4
CNSTI4 7
ARGI4
ADDRLP4 16
ADDRGP4 ImGui_BeginWindow
CALLI4
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 0
EQI4 $243
line 323
;323:        ImGui_SetWindowFontScale(font_scale * 6);
ADDRLP4 0
INDIRF4
CNSTF4 1086324736
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 324
;324:        ImGui_TextUnformatted("Level Statistics");
ADDRGP4 $246
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 325
;325:        ImGui_SetWindowFontScale(font_scale * 3.5f);
ADDRLP4 0
INDIRF4
CNSTF4 1080033280
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 326
;326:        ImGui_NewLine();
ADDRGP4 ImGui_NewLine
CALLV
pop
line 328
;327:
;328:        ImGui_GetCursorScreenPos( &cursorPos.x, &cursorPos.y );
ADDRLP4 4
ARGP4
ADDRLP4 4+4
ARGP4
ADDRGP4 ImGui_GetCursorScreenPos
CALLV
pop
line 330
;329:
;330:        ImGui_SetCursorScreenPos( cursorPos.x, cursorPos.y + 20);
ADDRLP4 4
INDIRF4
ARGF4
ADDRLP4 4+4
INDIRF4
CNSTF4 1101004800
ADDF4
ARGF4
ADDRGP4 ImGui_SetCursorScreenPos
CALLV
pop
line 331
;331:    }
LABELV $243
line 332
;332:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 333
;333:}
LABELV $242
endproc SG_DrawLevelStats 20 12
export SG_EndLevel
proc SG_EndLevel 4 0
line 336
;334:
;335:int SG_EndLevel( void )
;336:{
line 337
;337:    level.stats.timeEnd = Sys_Milliseconds();
ADDRLP4 0
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4+4
ADDRLP4 0
INDIRI4
ASGNI4
line 339
;338:
;339:    sg.state = SG_SHOW_LEVEL_STATS;
ADDRGP4 sg+2212
CNSTI4 4
ASGNI4
line 341
;340:
;341:    return 1;
CNSTI4 1
RETI4
LABELV $249
endproc SG_EndLevel 4 0
export SG_ParseInfos
proc SG_ParseInfos 2084 16
line 350
;342:}
;343:
;344:#define MAX_LEVELINFO_LEN 8192
;345:#define MAX_LEVELS 1024
;346:
;347:static char *sg_levelInfos[MAX_LEVELS];
;348:
;349:int SG_ParseInfos( char *buf, int max, char **infos )
;350:{
line 356
;351:    const char *token, **text;
;352:    int count;
;353:    char key[MAX_TOKEN_CHARS];
;354:    char info[MAX_INFO_STRING];
;355:
;356:    text = (const char **)&buf;
ADDRLP4 1028
ADDRFP4 0
ASGNP4
line 357
;357:    count = 0;
ADDRLP4 2056
CNSTI4 0
ASGNI4
ADDRGP4 $255
JUMPV
LABELV $254
line 359
;358:
;359:    while ( 1 ) {
line 360
;360:        token = COM_ParseExt( text, qtrue );
ADDRLP4 1028
INDIRP4
ARGP4
CNSTI4 1
ARGI4
ADDRLP4 2060
ADDRGP4 COM_ParseExt
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 2060
INDIRP4
ASGNP4
line 361
;361:        if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $257
line 362
;362:            break;
ADDRGP4 $256
JUMPV
LABELV $257
line 364
;363:        }
;364:        if ( token[0] == ' ' ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 32
NEI4 $259
line 365
;365:            continue;
ADDRGP4 $255
JUMPV
LABELV $259
line 367
;366:        }
;367:        if ( token[0] != '{' ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 123
EQI4 $261
line 368
;368:            G_Printf( "missing '{' in info file\n" );
ADDRGP4 $263
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 369
;369:            break;
ADDRGP4 $256
JUMPV
LABELV $261
line 372
;370:        }
;371:
;372:        if ( count == max ) {
ADDRLP4 2056
INDIRI4
ADDRFP4 4
INDIRI4
NEI4 $264
line 373
;373:            G_Printf( "max infos exceeded\n" );
ADDRGP4 $266
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 374
;374:            break;
ADDRGP4 $256
JUMPV
LABELV $264
line 377
;375:        }
;376:
;377:        info[0] = '\0';
ADDRLP4 1032
CNSTI1 0
ASGNI1
ADDRGP4 $268
JUMPV
LABELV $267
line 378
;378:        while ( 1 ) {
line 379
;379:            token = COM_ParseExt( text, qtrue );
ADDRLP4 1028
INDIRP4
ARGP4
CNSTI4 1
ARGI4
ADDRLP4 2064
ADDRGP4 COM_ParseExt
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 2064
INDIRP4
ASGNP4
line 380
;380:            if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $270
line 381
;381:                G_Printf( "unexpected end of info file\n" );
ADDRGP4 $272
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 382
;382:                break;
ADDRGP4 $269
JUMPV
LABELV $270
line 384
;383:            }
;384:            if ( token[0] == '}' ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 125
NEI4 $273
line 385
;385:                break;
ADDRGP4 $269
JUMPV
LABELV $273
line 387
;386:            }
;387:            N_strncpyz( key, token, sizeof(key) );
ADDRLP4 4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
CNSTU4 1024
ARGU4
ADDRGP4 N_strncpyz
CALLV
pop
line 389
;388:
;389:            token = COM_ParseExt( text, qfalse );
ADDRLP4 1028
INDIRP4
ARGP4
CNSTI4 0
ARGI4
ADDRLP4 2068
ADDRGP4 COM_ParseExt
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 2068
INDIRP4
ASGNP4
line 390
;390:            if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $275
line 391
;391:                token = "<NULL>";
ADDRLP4 0
ADDRGP4 $277
ASGNP4
line 392
;392:            }
LABELV $275
line 393
;393:            Info_SetValueForKey( info, key, token );
ADDRLP4 1032
ARGP4
CNSTU4 1024
ARGU4
ADDRLP4 4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Info_SetValueForKey_s
CALLI4
pop
line 394
;394:        }
LABELV $268
line 378
ADDRGP4 $267
JUMPV
LABELV $269
line 396
;395:        // NOTE: extra space for level index
;396:        infos[count] = SG_MemAlloc( strlen( info ) + strlen( "\\num\\" ) + strlen( va( "%i", MAX_LEVELS ) ) + 1 );
ADDRLP4 1032
ARGP4
ADDRLP4 2064
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRGP4 $278
ARGP4
ADDRLP4 2068
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRGP4 $146
ARGP4
CNSTI4 1024
ARGI4
ADDRLP4 2072
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 2072
INDIRP4
ARGP4
ADDRLP4 2076
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 2064
INDIRI4
ADDRLP4 2068
INDIRI4
ADDI4
ADDRLP4 2076
INDIRI4
ADDI4
CNSTI4 1
ADDI4
ARGI4
ADDRLP4 2080
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRLP4 2056
INDIRI4
CNSTI4 2
LSHI4
ADDRFP4 8
INDIRP4
ADDP4
ADDRLP4 2080
INDIRP4
ASGNP4
line 397
;397:        if ( infos[count] ) {
ADDRLP4 2056
INDIRI4
CNSTI4 2
LSHI4
ADDRFP4 8
INDIRP4
ADDP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $279
line 398
;398:            strcpy( infos[count], info );
ADDRLP4 2056
INDIRI4
CNSTI4 2
LSHI4
ADDRFP4 8
INDIRP4
ADDP4
INDIRP4
ARGP4
ADDRLP4 1032
ARGP4
ADDRGP4 strcpy
CALLI4
pop
line 399
;399:            count++;
ADDRLP4 2056
ADDRLP4 2056
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 400
;400:        }
LABELV $279
line 401
;401:    }
LABELV $255
line 359
ADDRGP4 $254
JUMPV
LABELV $256
line 403
;402:
;403:    return count;
ADDRLP4 2056
INDIRI4
RETI4
LABELV $253
endproc SG_ParseInfos 2084 16
proc SG_LoadLevelsFromFile 8212 16
line 406
;404:}
;405:
;406:static void SG_LoadLevelsFromFile( const char *filename ) {
line 411
;407:	int				len;
;408:	fileHandle_t    f;
;409:	char			buf[MAX_LEVELINFO_LEN];
;410:
;411:	len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ARGP4
CNSTI4 0
ARGI4
ADDRLP4 8200
ADDRGP4 trap_FS_FOpenFile
CALLU4
ASGNU4
ADDRLP4 0
ADDRLP4 8200
INDIRU4
CVUI4 4
ASGNI4
line 412
;412:	if ( !f ) {
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $282
line 413
;413:		trap_Print( va( COLOR_RED "ERROR: file not found: %s\n", filename ) );
ADDRGP4 $284
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 8204
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 8204
INDIRP4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 414
;414:		return;
ADDRGP4 $281
JUMPV
LABELV $282
line 416
;415:	}
;416:	if ( len >= MAX_LEVELINFO_LEN ) {
ADDRLP4 0
INDIRI4
CNSTI4 8192
LTI4 $285
line 417
;417:		trap_Print( va( COLOR_RED "ERROR: file too large: %s is %i, max allowed is %i", filename, len, MAX_LEVELINFO_LEN ) );
ADDRGP4 $287
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
CNSTI4 8192
ARGI4
ADDRLP4 8204
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 8204
INDIRP4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 418
;418:		trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 419
;419:		return;
ADDRGP4 $281
JUMPV
LABELV $285
line 422
;420:	}
;421:
;422:	trap_FS_Read( buf, len, f );
ADDRLP4 8
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_Read
CALLI4
pop
line 423
;423:	buf[len] = 0;
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 0
ASGNI1
line 424
;424:	trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 426
;425:
;426:	sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
ADDRLP4 8
ARGP4
CNSTI4 1024
ADDRGP4 sg+2220
INDIRI4
SUBI4
ARGI4
ADDRGP4 sg+2220
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
ARGP4
ADDRLP4 8204
ADDRGP4 SG_ParseInfos
CALLI4
ASGNI4
ADDRLP4 8208
ADDRGP4 sg+2220
ASGNP4
ADDRLP4 8208
INDIRP4
ADDRLP4 8208
INDIRP4
INDIRI4
ADDRLP4 8204
INDIRI4
ADDI4
ASGNI4
line 427
;427:}
LABELV $281
endproc SG_LoadLevelsFromFile 8212 16
export SG_LoadLevels
proc SG_LoadLevels 2672 16
line 430
;428:
;429:void SG_LoadLevels( void )
;430:{
line 441
;431:    int numdirs;
;432:    vmCvar_t levelsFile;
;433:    char filename[1024];
;434:    char dirlist[1024];
;435:    char *dirptr;
;436:    int i, j;
;437:    int dirlen;
;438:    const char *mapname;
;439:    levelInfo_t *info;
;440:
;441:    sg.numLevels = 0;
ADDRGP4 sg+2220
CNSTI4 0
ASGNI4
line 443
;442:
;443:    Cvar_Register( &levelsFile, "sg_levelsFile", "", CVAR_INIT | CVAR_ROM );
ADDRLP4 2072
ARGP4
ADDRGP4 $293
ARGP4
ADDRGP4 $294
ARGP4
CNSTI4 80
ARGI4
ADDRGP4 Cvar_Register
CALLV
pop
line 444
;444:    if ( *levelsFile.s ) {
ADDRLP4 2072
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $295
line 445
;445:        SG_LoadLevelsFromFile( levelsFile.s );
ADDRLP4 2072
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 446
;446:    } else {
ADDRGP4 $296
JUMPV
LABELV $295
line 447
;447:        SG_LoadLevelsFromFile( "scripts/levels.txt" );
ADDRGP4 $297
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 448
;448:    }
LABELV $296
line 451
;449:
;450:    // get all levels from .level files
;451:    numdirs = trap_FS_GetFileList( "scripts", ".level", dirlist, sizeof(dirlist) );
ADDRGP4 $298
ARGP4
ADDRGP4 $299
ARGP4
ADDRLP4 1048
ARGP4
CNSTI4 1024
ARGI4
ADDRLP4 2352
ADDRGP4 trap_FS_GetFileList
CALLI4
ASGNI4
ADDRLP4 1044
ADDRLP4 2352
INDIRI4
ASGNI4
line 452
;452:	dirptr  = dirlist;
ADDRLP4 12
ADDRLP4 1048
ASGNP4
line 453
;453:	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $303
JUMPV
LABELV $300
line 454
;454:		dirlen = strlen( dirptr );
ADDRLP4 12
INDIRP4
ARGP4
ADDRLP4 2356
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 1040
ADDRLP4 2356
INDIRI4
ASGNI4
line 455
;455:		strcpy( filename, "scripts/" );
ADDRLP4 16
ARGP4
ADDRGP4 $304
ARGP4
ADDRGP4 strcpy
CALLI4
pop
line 456
;456:		strcat( filename, dirptr );
ADDRLP4 16
ARGP4
ADDRLP4 12
INDIRP4
ARGP4
ADDRGP4 strcat
CALLI4
pop
line 457
;457:		SG_LoadLevelsFromFile( filename );
ADDRLP4 16
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 458
;458:	}
LABELV $301
line 453
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 12
ADDRLP4 1040
INDIRI4
CNSTI4 1
ADDI4
ADDRLP4 12
INDIRP4
ADDP4
ASGNP4
LABELV $303
ADDRLP4 0
INDIRI4
ADDRLP4 1044
INDIRI4
LTI4 $300
line 460
;459:
;460:	SG_Printf( "%i levels parsed.\n", sg.numLevels);
ADDRGP4 $305
ARGP4
ADDRGP4 sg+2220
INDIRI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 461
;461:	if ( SG_OutOfMemory() ) {
ADDRLP4 2356
ADDRGP4 SG_OutOfMemory
CALLI4
ASGNI4
ADDRLP4 2356
INDIRI4
CNSTI4 0
EQI4 $307
line 462
;462:        trap_Error( COLOR_RED "ERROR: not anough memory in pool to load all levels" );
ADDRGP4 $309
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 463
;463:    }
LABELV $307
line 466
;464:
;465:	// set initial numbers
;466:	for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $313
JUMPV
LABELV $310
line 467
;467:		Info_SetValueForKey( sg_levelInfos[i], "num", va( "%i", i ) );
ADDRGP4 $146
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 2360
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
CNSTU4 1024
ARGU4
ADDRGP4 $315
ARGP4
ADDRLP4 2360
INDIRP4
ARGP4
ADDRGP4 Info_SetValueForKey_s
CALLI4
pop
line 468
;468:    }
LABELV $311
line 466
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $313
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $310
line 471
;469:
;470:    // if there are no levels, let it crash
;471:    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );
ADDRGP4 sg+2220
INDIRI4
CVIU4 4
CNSTU4 788
MULU4
CVUI4 4
ARGI4
ADDRLP4 2360
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRGP4 sg_levelInfoData
ADDRLP4 2360
INDIRP4
ASGNP4
line 474
;472:
;473:    // load the level information (difficulty, map, etc.)
;474:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $320
JUMPV
LABELV $317
line 475
;475:        N_strncpyz( sg_levelInfoData[i].name, Info_ValueForKey( sg_levelInfos[i], "name" ), MAX_NPATH );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $182
ARGP4
ADDRLP4 2364
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
ADDRLP4 2364
INDIRP4
ARGP4
CNSTU4 64
ARGU4
ADDRGP4 N_strncpyz
CALLV
pop
line 476
;476:        G_Printf( "Loaded Level %s...\n", sg_levelInfoData[i].name );
ADDRGP4 $322
ARGP4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 478
;477:
;478:        for ( j = 0; j < NUMDIFS; j++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
LABELV $323
line 479
;479:            mapname = Info_ValueForKey( sg_levelInfos[i], va( "mapname_difficulty_%i", j ) );
ADDRGP4 $327
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 2368
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRLP4 2368
INDIRP4
ARGP4
ADDRLP4 2372
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 8
ADDRLP4 2372
INDIRP4
ASGNP4
line 480
;480:            if ( !N_stricmp( mapname, "none" ) ) {
ADDRLP4 8
INDIRP4
ARGP4
ADDRGP4 $330
ARGP4
ADDRLP4 2376
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 2376
INDIRI4
CNSTI4 0
NEI4 $328
line 482
;481:                // map currently doesn't support this difficulty
;482:                continue;
ADDRGP4 $324
JUMPV
LABELV $328
line 484
;483:            }
;484:            N_strncpyz( sg_levelInfoData[i].maphandles[j].name, mapname, MAX_NPATH );
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
CNSTI4 28
ADDP4
ARGP4
ADDRLP4 8
INDIRP4
ARGP4
CNSTU4 64
ARGU4
ADDRGP4 N_strncpyz
CALLV
pop
line 485
;485:            sg_levelInfoData[i].maphandles[j].handle = G_LoadMap( mapname );
ADDRLP4 8
INDIRP4
ARGP4
ADDRLP4 2380
ADDRGP4 G_LoadMap
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
CNSTI4 96
ADDP4
ADDRLP4 2380
INDIRI4
ASGNI4
line 486
;486:            sg_levelInfoData[i].maphandles[j].difficulty = j;
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
CNSTI4 92
ADDP4
ADDRLP4 4
INDIRI4
ASGNI4
line 487
;487:            if ( sg_levelInfoData[i].maphandles[j].handle == FS_INVALID_HANDLE ) {
ADDRLP4 4
INDIRI4
CNSTI4 100
MULI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ADDP4
CNSTI4 96
ADDP4
INDIRI4
CNSTI4 0
NEI4 $331
line 488
;488:                G_Printf( COLOR_YELLOW "WARNING: failed to load map '%s' for level '%s'\n", mapname, sg_levelInfoData[i].name );
ADDRGP4 $333
ARGP4
ADDRLP4 8
INDIRP4
ARGP4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 600
ADDP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 489
;489:                continue;
LABELV $331
line 491
;490:            }
;491:        }
LABELV $324
line 478
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 6
LTI4 $323
line 493
;492:
;493:        sg_levelInfoData[i].index = i;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 664
ADDP4
ADDRLP4 0
INDIRI4
ASGNI4
line 495
;494:        
;495:        sg_levelInfoData[i].a.rank = LEVEL_RANK_A;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 668
ADDP4
CNSTI4 0
ASGNI4
line 496
;496:        sg_levelInfoData[i].a.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $334
ARGP4
ADDRLP4 2376
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2376
INDIRP4
ARGP4
ADDRLP4 2380
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 676
ADDP4
ADDRLP4 2380
INDIRI4
ASGNI4
line 497
;497:        sg_levelInfoData[i].a.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $335
ARGP4
ADDRLP4 2388
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2388
INDIRP4
ARGP4
ADDRLP4 2392
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 672
ADDP4
ADDRLP4 2392
INDIRI4
ASGNI4
line 498
;498:        sg_levelInfoData[i].a.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_maxTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $336
ARGP4
ADDRLP4 2400
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2400
INDIRP4
ARGP4
ADDRLP4 2404
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 680
ADDP4
ADDRLP4 2404
INDIRI4
ASGNI4
line 499
;499:        sg_levelInfoData[i].a.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_maxDeaths" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $337
ARGP4
ADDRLP4 2412
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2412
INDIRP4
ARGP4
ADDRLP4 2416
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 684
ADDP4
ADDRLP4 2416
INDIRI4
ASGNI4
line 500
;500:        sg_levelInfoData[i].a.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $338
ARGP4
ADDRLP4 2424
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2424
INDIRP4
ARGP4
ADDRLP4 2428
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 688
ADDP4
ADDRLP4 2428
INDIRI4
ASGNI4
line 502
;501:
;502:        sg_levelInfoData[i].b.rank = LEVEL_RANK_B;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 692
ADDP4
CNSTI4 1
ASGNI4
line 503
;503:        sg_levelInfoData[i].b.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $339
ARGP4
ADDRLP4 2436
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2436
INDIRP4
ARGP4
ADDRLP4 2440
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 700
ADDP4
ADDRLP4 2440
INDIRI4
ASGNI4
line 504
;504:        sg_levelInfoData[i].b.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $340
ARGP4
ADDRLP4 2448
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2448
INDIRP4
ARGP4
ADDRLP4 2452
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 696
ADDP4
ADDRLP4 2452
INDIRI4
ASGNI4
line 505
;505:        sg_levelInfoData[i].b.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_maxTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $341
ARGP4
ADDRLP4 2460
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2460
INDIRP4
ARGP4
ADDRLP4 2464
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 704
ADDP4
ADDRLP4 2464
INDIRI4
ASGNI4
line 506
;506:        sg_levelInfoData[i].b.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_maxDeaths" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $342
ARGP4
ADDRLP4 2472
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2472
INDIRP4
ARGP4
ADDRLP4 2476
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 708
ADDP4
ADDRLP4 2476
INDIRI4
ASGNI4
line 507
;507:        sg_levelInfoData[i].b.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $343
ARGP4
ADDRLP4 2484
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2484
INDIRP4
ARGP4
ADDRLP4 2488
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 712
ADDP4
ADDRLP4 2488
INDIRI4
ASGNI4
line 509
;508:
;509:        sg_levelInfoData[i].c.rank = LEVEL_RANK_C;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 716
ADDP4
CNSTI4 2
ASGNI4
line 510
;510:        sg_levelInfoData[i].c.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $344
ARGP4
ADDRLP4 2496
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2496
INDIRP4
ARGP4
ADDRLP4 2500
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 724
ADDP4
ADDRLP4 2500
INDIRI4
ASGNI4
line 511
;511:        sg_levelInfoData[i].c.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $345
ARGP4
ADDRLP4 2508
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2508
INDIRP4
ARGP4
ADDRLP4 2512
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 720
ADDP4
ADDRLP4 2512
INDIRI4
ASGNI4
line 512
;512:        sg_levelInfoData[i].c.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_maxTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $346
ARGP4
ADDRLP4 2520
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2520
INDIRP4
ARGP4
ADDRLP4 2524
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 728
ADDP4
ADDRLP4 2524
INDIRI4
ASGNI4
line 513
;513:        sg_levelInfoData[i].c.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_maxDeaths" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $347
ARGP4
ADDRLP4 2532
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2532
INDIRP4
ARGP4
ADDRLP4 2536
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 732
ADDP4
ADDRLP4 2536
INDIRI4
ASGNI4
line 514
;514:        sg_levelInfoData[i].c.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $348
ARGP4
ADDRLP4 2544
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2544
INDIRP4
ARGP4
ADDRLP4 2548
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 736
ADDP4
ADDRLP4 2548
INDIRI4
ASGNI4
line 516
;515:
;516:        sg_levelInfoData[i].d.rank = LEVEL_RANK_D;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 740
ADDP4
CNSTI4 3
ASGNI4
line 517
;517:        sg_levelInfoData[i].d.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $349
ARGP4
ADDRLP4 2556
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2556
INDIRP4
ARGP4
ADDRLP4 2560
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 748
ADDP4
ADDRLP4 2560
INDIRI4
ASGNI4
line 518
;518:        sg_levelInfoData[i].d.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $350
ARGP4
ADDRLP4 2568
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2568
INDIRP4
ARGP4
ADDRLP4 2572
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 744
ADDP4
ADDRLP4 2572
INDIRI4
ASGNI4
line 519
;519:        sg_levelInfoData[i].d.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_maxTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $351
ARGP4
ADDRLP4 2580
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2580
INDIRP4
ARGP4
ADDRLP4 2584
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 752
ADDP4
ADDRLP4 2584
INDIRI4
ASGNI4
line 520
;520:        sg_levelInfoData[i].d.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_maxDeaths" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $352
ARGP4
ADDRLP4 2592
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2592
INDIRP4
ARGP4
ADDRLP4 2596
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 756
ADDP4
ADDRLP4 2596
INDIRI4
ASGNI4
line 521
;521:        sg_levelInfoData[i].d.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $353
ARGP4
ADDRLP4 2604
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2604
INDIRP4
ARGP4
ADDRLP4 2608
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 760
ADDP4
ADDRLP4 2608
INDIRI4
ASGNI4
line 523
;522:
;523:        sg_levelInfoData[i].f.rank = LEVEL_RANK_WERE_U_BOTTING;
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 764
ADDP4
CNSTI4 4
ASGNI4
line 524
;524:        sg_levelInfoData[i].f.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $354
ARGP4
ADDRLP4 2616
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2616
INDIRP4
ARGP4
ADDRLP4 2620
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 772
ADDP4
ADDRLP4 2620
INDIRI4
ASGNI4
line 525
;525:        sg_levelInfoData[i].f.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $355
ARGP4
ADDRLP4 2628
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2628
INDIRP4
ARGP4
ADDRLP4 2632
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 768
ADDP4
ADDRLP4 2632
INDIRI4
ASGNI4
line 526
;526:        sg_levelInfoData[i].f.maxTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_maxTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $356
ARGP4
ADDRLP4 2640
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2640
INDIRP4
ARGP4
ADDRLP4 2644
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 776
ADDP4
ADDRLP4 2644
INDIRI4
ASGNI4
line 527
;527:        sg_levelInfoData[i].f.maxDeaths = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_maxDeaths" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $357
ARGP4
ADDRLP4 2652
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2652
INDIRP4
ARGP4
ADDRLP4 2656
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 780
ADDP4
ADDRLP4 2656
INDIRI4
ASGNI4
line 528
;528:        sg_levelInfoData[i].f.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $358
ARGP4
ADDRLP4 2664
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2664
INDIRP4
ARGP4
ADDRLP4 2668
ADDRGP4 atoi
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 788
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 784
ADDP4
ADDRLP4 2668
INDIRI4
ASGNI4
line 529
;529:    }
LABELV $318
line 474
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $320
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $317
line 530
;530:}
LABELV $291
endproc SG_LoadLevels 2672 16
import atoi
import strcat
import strcpy
import strlen
import memset
bss
align 4
LABELV sg_levelInfos
skip 4096
align 4
LABELV level
skip 36
align 4
LABELV sg_levelInfoData
skip 4
import Cvar_VariableStringBuffer
import Cvar_Set
import Cvar_Update
import Cvar_Register
import trap_FS_Printf
import trap_FS_FileTell
import trap_FS_FileLength
import trap_FS_FileSeek
import trap_FS_GetFileList
import trap_FS_Read
import trap_FS_Write
import trap_FS_FClose
import trap_FS_FOpenRead
import trap_FS_FOpenWrite
import trap_FS_FOpenFile
import Sys_GetGPUConfig
import RE_SetColor
import RE_DrawImage
import RE_AddSpriteToScene
import RE_AddPolyToScene
import RE_RenderScene
import RE_ClearScene
import RE_LoadWorldMap
import RE_RegisterSprite
import RE_RegisterSpriteSheet
import RE_RegisterShader
import Snd_ClearLoopingTrack
import Snd_SetLoopingTrack
import Snd_StopSfx
import Snd_PlaySfx
import Snd_RegisterTrack
import Snd_RegisterSfx
import trap_Key_ClearStates
import trap_Key_GetKey
import trap_Key_GetCatcher
import trap_Key_SetCatcher
import Sys_Milliseconds
import G_CheckWallHit
import G_SoundRecursive
import G_CastRay
import G_SetActiveMap
import G_LoadMap
import trap_GetHashString
import G_SetCameraData
import Sys_MemoryRemaining
import trap_RemoveCommand
import trap_AddCommand
import trap_SendConsoleCommand
import G_LoadVector4
import G_LoadVector3
import G_LoadVector2
import G_LoadString
import LoadFloat
import G_LoadInt
import G_LoadUInt
import G_GetSaveSection
import G_SaveVector4
import G_SaveVector3
import G_SaveVector2
import G_SaveFloat
import G_SaveString
import G_SaveUInt
import G_SaveInt
import G_SaveChar
import G_EndSaveSection
import G_BeginSaveSection
import trap_Args
import trap_Argv
import trap_Argc
import trap_Error
import trap_Print
import P_GiveWeapon
import P_GiveItem
import P_ParryTicker
import P_MeleeTicker
import P_Ticker
import SG_SendUserCmd
import SG_MouseEvent
import SG_KeyEvent
import SG_InitPlayer
import SG_OutOfMemory
import SG_ClearToMemoryMark
import SG_MakeMemoryMark
import SG_MemInit
import SG_MemAlloc
import String_Alloc
import SG_SpawnMob
import SG_Spawn
import Ent_SetState
import SG_InitEntities
import Ent_BuildBounds
import SG_BuildBounds
import SG_FreeEntity
import SG_AllocEntity
import Ent_RunTic
import Ent_CheckEntityCollision
import Ent_CheckWallCollision
import SG_PickupWeapon
import SG_SpawnWeapon
import SG_SpawnItem
import SG_UpdateCvars
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import SG_ShutdownCommands
import SG_InitCommands
import SGameCommand
import SG_DrawFrame
import sgc_godmode
import sg_cheatsOn
import sgc_deafMobs
import sgc_blindMobs
import sgc_infiniteAmmo
import sgc_infiniteRage
import sgc_infiniteHealth
import pm_wallTime
import pm_wallrunAccelMove
import pm_wallrunAccelVertical
import pm_airAccel
import pm_baseSpeed
import pm_baseAccel
import pm_waterAccel
import pm_airFriction
import pm_waterFriction
import pm_groundFriction
import sg_memoryDebug
import sg_numSaves
import sg_gameDifficulty
import sg_savename
import sg_levelIndex
import sg_gibs
import sg_decalDetail
import sg_printLevelStats
import sg_mouseAcceleration
import sg_mouseInvert
import sg_paused
import sg_debugPrint
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
import sg
import sg_entities
import sg_activeEnts
import sg_freeEnts
import inversedirs
import dirvectors
import stateinfo
import ImGui_CloseCurrentPopup
import ImGui_OpenPopup
import ImGui_EndPopup
import ImGui_BeginPopupModal
import ImGui_ColoredText
import ImGui_Text
import ImGui_ColoredTextUnformatted
import ImGui_TextUnformatted
import ImGui_SameLine
import ImGui_ProgressBar
import ImGui_Separator
import ImGui_SeparatorText
import ImGui_NewLine
import ImGui_PopColor
import ImGui_PushColor
import ImGui_GetCursorScreenPos
import ImGui_SetCursorScreenPos
import ImGui_GetCursorPos
import ImGui_SetCursorPos
import ImGui_GetFontScale
import ImGui_Button
import ImGui_Checkbox
import ImGui_ArrowButton
import ImGui_ColorEdit4
import ImGui_ColorEdit3
import ImGui_SliderInt4
import ImGui_SliderInt3
import ImGui_SliderInt2
import ImGui_SliderInt
import ImGui_SliderFloat4
import ImGui_SliderFloat3
import ImGui_SliderFloat2
import ImGui_SliderFloat
import ImGui_InputInt4
import ImGui_InputInt3
import ImGui_InputInt2
import ImGui_InputInt
import ImGui_InputFloat4
import ImGui_InputFloat3
import ImGui_InputFloat2
import ImGui_InputFloat
import ImGui_InputTextWithHint
import ImGui_InputTextMultiline
import ImGui_InputText
import ImGui_EndTable
import ImGui_TableNextColumn
import ImGui_TableNextRow
import ImGui_BeginTable
import ImGui_SetItemTooltip
import ImGui_SetItemTooltipUnformatted
import ImGui_MenuItem
import ImGui_EndMenu
import ImGui_BeginMenu
import ImGui_SetWindowFontScale
import ImGui_SetWindowSize
import ImGui_SetWindowPos
import ImGui_SetWindowCollapsed
import ImGui_IsWindowCollapsed
import ImGui_EndWindow
import ImGui_BeginWindow
import Com_TouchMemory
import Hunk_TempIsClear
import Hunk_Check
import Hunk_Print
import Hunk_SetMark
import Hunk_ClearToMark
import Hunk_CheckMark
import Hunk_SmallLog
import Hunk_Log
import Hunk_MemoryRemaining
import Hunk_ClearTempMemory
import Hunk_FreeTempMemory
import Hunk_AllocateTempMemory
import Hunk_Clear
import Hunk_Alloc
import Hunk_InitMemory
import Z_InitMemory
import Z_InitSmallZoneMemory
import CopyString
import Z_AvailableMemory
import Z_FreeTags
import Z_Free
import S_Malloc
import Z_Malloc
import Z_Realloc
import CPU_flags
import FS_ReadLine
import FS_ListFiles
import FS_FreeFileList
import FS_FreeFile
import FS_SetBFFIndex
import FS_GetCurrentChunkList
import FS_Initialized
import FS_FileIsInBFF
import FS_StripExt
import FS_AllowedExtension
import FS_GetFileList
import FS_LoadLibrary
import FS_CopyString
import FS_BuildOSPath
import FS_FilenameCompare
import FS_FileTell
import FS_FileLength
import FS_FileSeek
import FS_FileExists
import FS_LastBFFIndex
import FS_LoadStack
import FS_Rename
import FS_FOpenFileRead
import FS_FOpenAppend
import FS_FOpenRW
import FS_FOpenWrite
import FS_FOpenRead
import FS_FOpenFileWithMode
import FS_FOpenWithMode
import FS_FileToFileno
import FS_Printf
import FS_GetGamePath
import FS_GetHomePath
import FS_GetBasePath
import FS_GetBaseGameDir
import FS_GetCurrentGameDir
import FS_Flush
import FS_ForceFlush
import FS_FClose
import FS_LoadFile
import FS_WriteFile
import FS_Write
import FS_Read
import FS_Remove
import FS_Restart
import FS_Shutdown
import FS_InitFilesystem
import FS_Startup
import FS_VM_CloseFiles
import FS_VM_FileLength
import FS_VM_Read
import FS_VM_Write
import FS_VM_WriteFile
import FS_VM_FClose
import FS_VM_FOpenFileRead
import FS_VM_FOpenFileWrite
import FS_VM_FOpenFile
import FS_VM_FileTell
import FS_VM_FileSeek
import FS_VM_FOpenRW
import FS_VM_FOpenAppend
import FS_VM_FOpenWrite
import FS_VM_FOpenRead
import com_errorMessage
import com_fullyInitialized
import com_errorEntered
import com_cacheLine
import com_frameTime
import com_fps
import com_frameNumber
import com_maxfps
import sys_cpuString
import com_devmode
import com_version
import com_logfile
import com_journal
import com_demo
import Con_HistoryGetNext
import Con_HistoryGetPrev
import Con_SaveField
import Con_ResetHistory
import Field_CompleteCommand
import Field_CompleteFilename
import Field_CompleteKeyBind
import Field_CompleteKeyname
import Field_AutoComplete
import Field_Clear
import Cbuf_Init
import Cbuf_Clear
import Cbuf_AddText
import Cbuf_Execute
import Cbuf_InsertText
import Cbuf_ExecuteText
import va
import Cmd_CompleteArgument
import Cmd_CommandCompletion
import Cmd_Clear
import Cmd_Argv
import Cmd_ArgsFrom
import Cmd_SetCommandCompletionFunc
import Cmd_TokenizeStringIgnoreQuotes
import Cmd_TokenizeString
import Cmd_ArgvBuffer
import Cmd_Argc
import Cmd_ExecuteString
import Cmd_ExecuteText
import Cmd_ArgsBuffer
import Cmd_ExecuteCommand
import Cmd_RemoveCommand
import Cmd_AddCommand
import Cmd_Init
import keys
import Key_WriteBindings
import Key_SetOverstrikeMode
import Key_GetOverstrikeMode
import Key_GetKey
import Key_GetCatcher
import Key_SetCatcher
import Key_ClearStates
import Key_GetBinding
import Key_IsDown
import Key_KeynumToString
import Key_StringToKeynum
import Key_KeynameCompletion
import Com_EventLoop
import Com_KeyEvent
import Com_SendKeyEvents
import Com_QueueEvent
import Com_InitKeyCommands
import Parse3DMatrix
import Parse2DMatrix
import Parse1DMatrix
import ParseHex
import SkipRestOfLine
import SkipBracedSection
import com_tokentype
import COM_ParseComplex
import Com_BlockChecksum
import COM_ParseWarning
import COM_ParseError
import COM_Compress
import COM_ParseExt
import COM_Parse
import COM_GetCurrentParseLine
import COM_BeginParseSession
import COM_StripExtension
import COM_GetExtension
import Com_TruncateLongString
import Com_SortFileList
import Com_Base64Decode
import Com_HasPatterns
import Com_FilterPath
import Com_Filter
import Com_FilterExt
import Com_HexStrToInt
import COM_DefaultExtension
import Com_WriteConfig
import Con_RenderConsole
import Com_GenerateHashValue
import Com_Shutdown
import Com_Init
import Com_StartupVariable
import crc32_buffer
import Com_EarlyParseCmdLine
import Com_Milliseconds
import Com_Frame
import Sys_SnapVector
import I_GetParm
import Con_DPrintf
import Con_Printf
import Con_Shutdown
import Con_Init
import Con_DrawConsole
import Con_AddText
import ColorIndexFromChar
import g_color_table
import Info_RemoveKey
import Info_NextPair
import Info_ValidateKeyValue
import Info_Validate
import Info_SetValueForKey_s
import Info_ValueForKeyToken
import Info_Tokenize
import Info_ValueForKey
import Com_Clamp
import bytedirs
import N_isnan
import N_crandom
import N_random
import N_rand
import N_fabs
import N_acos
import N_log2
import ColorBytes4
import ColorBytes3
import VectorNormalize
import AddPointToBounds
import NormalizeColor
import _VectorMA
import _VectorScale
import _VectorCopy
import _VectorAdd
import _VectorSubtract
import _DotProduct
import ByteToDir
import DirToByte
import CrossProduct
import VectorInverse
import VectorNormalizeFast
import DistanceSquared
import Distance
import VectorLengthSquared
import VectorLength
import VectorCompare
import BoundsIntersectPoint
import BoundsIntersectSphere
import BoundsIntersect
import disBetweenOBJ
import vec3_set
import vec3_get
import ClearBounds
import RadiusFromBounds
import ClampShort
import ClampCharMove
import ClampChar
import N_exp2f
import N_log2f
import Q_rsqrt
import N_Error
import locase
import colorDkGrey
import colorMdGrey
import colorLtGrey
import colorWhite
import colorCyan
import colorMagenta
import colorYellow
import colorBlue
import colorGreen
import colorRed
import colorBlack
import vec2_origin
import vec3_origin
import COM_SkipPath
import Com_Split
import N_replace
import N_memcmp
import N_memchr
import N_memcpy
import N_memset
import N_strncpyz
import N_strncpy
import N_strcpy
import N_stradd
import N_strneq
import N_streq
import N_strlen
import N_atof
import N_atoi
import N_fmaxf
import N_stristr
import N_strcat
import N_strupr
import N_strlwr
import N_stricmpn
import N_stricmp
import N_strncmp
import N_strcmp
import N_isanumber
import N_isintegral
import N_isalpha
import N_isupper
import N_islower
import N_isprint
import Com_SkipCharset
import Com_SkipTokens
import Com_snprintf
lit
align 1
LABELV $358
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $357
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $356
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $355
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $354
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $353
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $352
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $351
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $350
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $349
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $348
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $347
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $346
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $345
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $344
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $343
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $342
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $341
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $340
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $339
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $338
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $337
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $336
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $335
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $334
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $333
byte 1 94
byte 1 51
byte 1 87
byte 1 65
byte 1 82
byte 1 78
byte 1 73
byte 1 78
byte 1 71
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 32
byte 1 102
byte 1 111
byte 1 114
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 10
byte 1 0
align 1
LABELV $330
byte 1 110
byte 1 111
byte 1 110
byte 1 101
byte 1 0
align 1
LABELV $327
byte 1 109
byte 1 97
byte 1 112
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 95
byte 1 100
byte 1 105
byte 1 102
byte 1 102
byte 1 105
byte 1 99
byte 1 117
byte 1 108
byte 1 116
byte 1 121
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $322
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 101
byte 1 100
byte 1 32
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $315
byte 1 110
byte 1 117
byte 1 109
byte 1 0
align 1
LABELV $309
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 97
byte 1 110
byte 1 111
byte 1 117
byte 1 103
byte 1 104
byte 1 32
byte 1 109
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 112
byte 1 111
byte 1 111
byte 1 108
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 32
byte 1 97
byte 1 108
byte 1 108
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $305
byte 1 37
byte 1 105
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 32
byte 1 112
byte 1 97
byte 1 114
byte 1 115
byte 1 101
byte 1 100
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $304
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 47
byte 1 0
align 1
LABELV $299
byte 1 46
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $298
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $297
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 47
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 46
byte 1 116
byte 1 120
byte 1 116
byte 1 0
align 1
LABELV $294
byte 1 0
align 1
LABELV $293
byte 1 115
byte 1 103
byte 1 95
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 70
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $287
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 116
byte 1 111
byte 1 111
byte 1 32
byte 1 108
byte 1 97
byte 1 114
byte 1 103
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 105
byte 1 44
byte 1 32
byte 1 109
byte 1 97
byte 1 120
byte 1 32
byte 1 97
byte 1 108
byte 1 108
byte 1 111
byte 1 119
byte 1 101
byte 1 100
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $284
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 102
byte 1 111
byte 1 117
byte 1 110
byte 1 100
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $278
byte 1 92
byte 1 110
byte 1 117
byte 1 109
byte 1 92
byte 1 0
align 1
LABELV $277
byte 1 60
byte 1 78
byte 1 85
byte 1 76
byte 1 76
byte 1 62
byte 1 0
align 1
LABELV $272
byte 1 117
byte 1 110
byte 1 101
byte 1 120
byte 1 112
byte 1 101
byte 1 99
byte 1 116
byte 1 101
byte 1 100
byte 1 32
byte 1 101
byte 1 110
byte 1 100
byte 1 32
byte 1 111
byte 1 102
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 10
byte 1 0
align 1
LABELV $266
byte 1 109
byte 1 97
byte 1 120
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 115
byte 1 32
byte 1 101
byte 1 120
byte 1 99
byte 1 101
byte 1 101
byte 1 100
byte 1 101
byte 1 100
byte 1 10
byte 1 0
align 1
LABELV $263
byte 1 109
byte 1 105
byte 1 115
byte 1 115
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 39
byte 1 123
byte 1 39
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 10
byte 1 0
align 1
LABELV $246
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 83
byte 1 116
byte 1 97
byte 1 116
byte 1 105
byte 1 115
byte 1 116
byte 1 105
byte 1 99
byte 1 115
byte 1 0
align 1
LABELV $245
byte 1 69
byte 1 110
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $235
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 103
byte 1 101
byte 1 116
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 34
byte 1 37
byte 1 115
byte 1 34
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 44
byte 1 32
byte 1 112
byte 1 111
byte 1 115
byte 1 115
byte 1 105
byte 1 98
byte 1 108
byte 1 101
byte 1 32
byte 1 109
byte 1 111
byte 1 100
byte 1 32
byte 1 105
byte 1 110
byte 1 99
byte 1 111
byte 1 109
byte 1 112
byte 1 97
byte 1 116
byte 1 105
byte 1 98
byte 1 105
byte 1 108
byte 1 105
byte 1 116
byte 1 121
byte 1 0
align 1
LABELV $228
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 103
byte 1 101
byte 1 116
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 34
byte 1 37
byte 1 115
byte 1 34
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 44
byte 1 32
byte 1 112
byte 1 111
byte 1 115
byte 1 115
byte 1 105
byte 1 98
byte 1 108
byte 1 101
byte 1 32
byte 1 109
byte 1 111
byte 1 100
byte 1 32
byte 1 105
byte 1 109
byte 1 99
byte 1 111
byte 1 109
byte 1 112
byte 1 97
byte 1 116
byte 1 105
byte 1 98
byte 1 105
byte 1 108
byte 1 105
byte 1 116
byte 1 121
byte 1 0
align 1
LABELV $218
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 103
byte 1 101
byte 1 116
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 34
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 34
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $206
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 103
byte 1 101
byte 1 116
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 34
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 83
byte 1 116
byte 1 97
byte 1 116
byte 1 115
byte 1 34
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $202
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 103
byte 1 101
byte 1 116
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 34
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 34
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $198
byte 1 116
byte 1 105
byte 1 109
byte 1 101
byte 1 84
byte 1 111
byte 1 116
byte 1 97
byte 1 108
byte 1 0
align 1
LABELV $197
byte 1 109
byte 1 97
byte 1 112
byte 1 95
byte 1 100
byte 1 105
byte 1 102
byte 1 102
byte 1 105
byte 1 99
byte 1 117
byte 1 108
byte 1 116
byte 1 121
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $192
byte 1 114
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 115
byte 1 67
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 0
align 1
LABELV $191
byte 1 109
byte 1 97
byte 1 120
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $190
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $189
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $188
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $187
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 37
byte 1 105
byte 1 95
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $182
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $181
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $174
byte 1 110
byte 1 117
byte 1 109
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $173
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 0
align 1
LABELV $170
byte 1 105
byte 1 115
byte 1 67
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 0
align 1
LABELV $168
byte 1 116
byte 1 105
byte 1 109
byte 1 101
byte 1 83
byte 1 116
byte 1 97
byte 1 114
byte 1 116
byte 1 0
align 1
LABELV $165
byte 1 115
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 80
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $162
byte 1 110
byte 1 117
byte 1 109
byte 1 68
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 115
byte 1 0
align 1
LABELV $159
byte 1 110
byte 1 117
byte 1 109
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $158
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 83
byte 1 116
byte 1 97
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $157
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $155
byte 1 99
byte 1 104
byte 1 101
byte 1 99
byte 1 107
byte 1 112
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 73
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $154
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
byte 1 0
align 1
LABELV $152
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 46
byte 1 0
align 1
LABELV $146
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $145
byte 1 115
byte 1 103
byte 1 95
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 73
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $142
byte 1 109
byte 1 97
byte 1 112
byte 1 115
byte 1 47
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $139
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 72
byte 1 101
byte 1 105
byte 1 103
byte 1 104
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $136
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 87
byte 1 105
byte 1 100
byte 1 116
byte 1 104
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $133
byte 1 83
byte 1 112
byte 1 97
byte 1 119
byte 1 110
byte 1 32
byte 1 67
byte 1 111
byte 1 117
byte 1 110
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $130
byte 1 67
byte 1 104
byte 1 101
byte 1 99
byte 1 107
byte 1 112
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 32
byte 1 67
byte 1 111
byte 1 117
byte 1 110
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $127
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 78
byte 1 97
byte 1 109
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $126
byte 1 10
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 32
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 73
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 10
byte 1 0
align 1
LABELV $115
byte 1 83
byte 1 116
byte 1 97
byte 1 114
byte 1 116
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 117
byte 1 112
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $114
byte 1 67
byte 1 111
byte 1 117
byte 1 108
byte 1 100
byte 1 110
byte 1 39
byte 1 116
byte 1 32
byte 1 102
byte 1 105
byte 1 110
byte 1 100
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 111
byte 1 114
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 0
align 1
LABELV $111
byte 1 109
byte 1 97
byte 1 112
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
