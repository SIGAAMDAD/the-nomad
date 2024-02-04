code
proc SG_GetLevelInfoByMapName 8 8
file "../sg_level.c"
line 59
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
;13:} levelstats_t;
;14:
;15:typedef struct
;16:{
;17:    int index;
;18:
;19:    // save data
;20:    levelstats_t stats;
;21:    int checkpointIndex;
;22:} level_t;
;23:
;24:typedef enum {
;25:    LEVEL_RANK_A,
;26:    LEVEL_RANK_B,
;27:    LEVEL_RANK_C,
;28:    LEVEL_RANK_D,
;29:    LEVEL_RANK_WERE_U_BOTTING
;30:} rank_t;
;31:
;32:typedef struct {
;33:    rank_t rank;
;34:    int minStyle;
;35:    int minKills;
;36:    int minTime;
;37:
;38:    qboolean requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
;39:} levelRank_t;
;40:
;41:typedef struct levelInfo_s
;42:{
;43:    char name[MAX_NPATH];
;44:    nhandle_t maphandle;
;45:    gamedif_t difficulty;
;46:
;47:    // ranking info
;48:    levelRank_t a;
;49:    levelRank_t b;
;50:    levelRank_t c;
;51:    levelRank_t d;
;52:    levelRank_t f;
;53:} levelInfo_t;
;54:
;55:static levelInfo_t *sg_levelInfoData;
;56:static level_t level;
;57:
;58:static levelInfo_t *SG_GetLevelInfoByMapName( const char *mapname )
;59:{
line 62
;60:    int i;
;61:
;62:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $97
JUMPV
LABELV $94
line 63
;63:        if ( !N_stricmp( sg_levelInfoData[i].name, mapname ) )  {
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
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
NEI4 $99
line 64
;64:            return &sg_levelInfoData[i];
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
RETP4
ADDRGP4 $93
JUMPV
LABELV $99
line 66
;65:        }
;66:    }
LABELV $95
line 62
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $97
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $94
line 67
;67:    return NULL;
CNSTP4 0
RETP4
LABELV $93
endproc SG_GetLevelInfoByMapName 8 8
proc SG_SpawnLevelEntities 12 12
line 74
;68:}
;69:
;70://
;71:// SG_SpawnLevelEntities
;72://
;73:static void SG_SpawnLevelEntities( void )
;74:{
line 78
;75:    int i;
;76:    const mapspawn_t *spawn;
;77:
;78:    spawn = sg.mapInfo.spawns;
ADDRLP4 0
ADDRGP4 sg+4270540
ASGNP4
line 79
;79:    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $106
JUMPV
LABELV $103
line 80
;80:        SG_Spawn( spawn->entityid, spawn->entitytype, &spawn->xyz );
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
line 81
;81:    }
LABELV $104
line 79
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
LABELV $106
ADDRLP4 4
INDIRI4
ADDRGP4 sg+4270540+23624
INDIRI4
LTI4 $103
line 82
;82:}
LABELV $101
endproc SG_SpawnLevelEntities 12 12
export SG_StartLevel
proc SG_StartLevel 104 16
line 85
;83:
;84:qboolean SG_StartLevel( void )
;85:{
line 91
;86:    vec2_t cameraPos;
;87:    float zoom;
;88:    char mapname[MAX_NPATH];
;89:    levelInfo_t *info;
;90:
;91:    Cvar_VariableStringBuffer( "mapname", mapname, sizeof(mapname) );
ADDRGP4 $110
ARGP4
ADDRLP4 4
ARGP4
CNSTI4 64
ARGI4
ADDRGP4 Cvar_VariableStringBuffer
CALLV
pop
line 93
;92:
;93:    info = SG_GetLevelInfoByMapName( mapname );
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
line 94
;94:    if ( !info ) {
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $111
line 95
;95:        G_Error( "Couldn't find level info for map '%s'", mapname );
ADDRGP4 $113
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 G_Error
CALLV
pop
line 96
;96:    }
LABELV $111
line 98
;97:
;98:    G_Printf( "Starting up level %s...\n", info->name );
ADDRGP4 $114
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 101
;99:
;100:    // clear the old level data
;101:    memset( &level, 0, sizeof(level) );
ADDRGP4 level
ARGP4
CNSTI4 0
ARGI4
CNSTU4 28
ARGU4
ADDRGP4 memset
CALLI4
pop
line 102
;102:    memset( &sg.mapInfo, 0, sizeof(sg.mapInfo) );
ADDRGP4 sg+4270540
ARGP4
CNSTI4 0
ARGI4
CNSTU4 23632
ARGU4
ADDRGP4 memset
CALLI4
pop
line 104
;103:
;104:    G_SetActiveMap( info->maphandle, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
ADDRLP4 0
INDIRP4
CNSTI4 64
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
line 106
;105:
;106:    SG_InitPlayer();
ADDRGP4 SG_InitPlayer
CALLV
pop
line 109
;107:
;108:    // spawn everything
;109:    SG_SpawnLevelEntities();
ADDRGP4 SG_SpawnLevelEntities
CALLV
pop
line 111
;110:
;111:    sg.state = SG_IN_LEVEL;
ADDRGP4 sg+2212
CNSTI4 3
ASGNI4
line 113
;112:
;113:    if ( sg_printLevelStats.i ) {
ADDRGP4 sg_printLevelStats+260
INDIRI4
CNSTI4 0
EQI4 $121
line 114
;114:        G_Printf( "\n---------- Level Info ----------\n" );
ADDRGP4 $124
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 115
;115:        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
ADDRGP4 $125
ARGP4
ADDRGP4 sg+4270540+23552
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 116
;116:        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
ADDRGP4 $128
ARGP4
ADDRGP4 sg+4270540+23628
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 117
;117:        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
ADDRGP4 $131
ARGP4
ADDRGP4 sg+4270540+23624
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 118
;118:        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
ADDRGP4 $134
ARGP4
ADDRGP4 sg+4270540+23616
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 119
;119:        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
ADDRGP4 $137
ARGP4
ADDRGP4 sg+4270540+23620
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 120
;120:    }
LABELV $121
line 122
;121:
;122:    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );
ADDRGP4 $140
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
line 124
;123:
;124:    Cvar_Set( "sg_levelIndex", va( "%i", (int)(uintptr_t)( info - sg_levelInfoData ) ) );
ADDRGP4 $144
ARGP4
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRGP4 sg_levelInfoData
INDIRP4
CVPU4 4
SUBU4
CVUI4 4
CNSTI4 172
DIVI4
CVIU4 4
CVUI4 4
ARGI4
ADDRLP4 88
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $143
ARGP4
ADDRLP4 88
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 126
;125:
;126:    level.stats.timeStart = Sys_Milliseconds();
ADDRLP4 92
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4
ADDRLP4 92
INDIRI4
ASGNI4
line 128
;127:
;128:    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
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
line 129
;129:    zoom = 1.6f;
ADDRLP4 76
CNSTF4 1070386381
ASGNF4
line 131
;130:
;131:    G_Printf( "Done." );
ADDRGP4 $150
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 135
;132:
;133://    G_SetCameraData( cameraPos, zoom, 0.0f );
;134:
;135:    return qtrue;
CNSTI4 1
RETI4
LABELV $109
endproc SG_StartLevel 104 16
export SG_SaveLevelData
proc SG_SaveLevelData 4 8
line 139
;136:}
;137:
;138:void SG_SaveLevelData( void )
;139:{
line 142
;140:    int i;
;141:
;142:    G_BeginSaveSection( "level" );
ADDRGP4 $152
ARGP4
ADDRGP4 G_BeginSaveSection
CALLV
pop
line 144
;143:
;144:    G_SaveInt( "checkpoint_index", level.checkpointIndex );
ADDRGP4 $153
ARGP4
ADDRGP4 level+24
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 145
;145:    G_SaveInt( "level_index", level.index );
ADDRGP4 $155
ARGP4
ADDRGP4 level
INDIRI4
ARGI4
ADDRGP4 G_SaveInt
CALLV
pop
line 147
;146:
;147:    G_SaveVector2( "sg_camera_position", &sg.cameraPos );
ADDRGP4 $156
ARGP4
ADDRGP4 sg+4270532
ARGP4
ADDRGP4 G_SaveVector2
CALLV
pop
line 152
;148:
;149:    //
;150:    // archive entity data
;151:    //
;152:    for ( i = 0; i < sg.numEntities; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $161
JUMPV
LABELV $158
line 154
;153:
;154:    }
LABELV $159
line 152
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $161
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2224
INDIRI4
LTI4 $158
line 156
;155:
;156:    G_EndSaveSection();
ADDRGP4 G_EndSaveSection
CALLV
pop
line 157
;157:}
LABELV $151
endproc SG_SaveLevelData 4 8
export SG_LoadLevelData
proc SG_LoadLevelData 16 16
line 160
;158:
;159:void SG_LoadLevelData( void )
;160:{
line 163
;161:    nhandle_t section;
;162:
;163:    section = G_GetSaveSection( "level" );
ADDRGP4 $152
ARGP4
ADDRLP4 4
ADDRGP4 G_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRI4
ASGNI4
line 164
;164:    if ( section == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $164
line 165
;165:        trap_Error( "SG_LoadLevelData: failed to fetch \"level\" section from save file!" );
ADDRGP4 $166
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 166
;166:    }
LABELV $164
line 168
;167:
;168:    level.checkpointIndex = G_LoadInt( "checkpoint_index", section );
ADDRGP4 $153
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 8
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+24
ADDRLP4 8
INDIRI4
ASGNI4
line 169
;169:    level.index = G_LoadInt( "level_index", section );
ADDRGP4 $155
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 12
ADDRGP4 G_LoadInt
CALLI4
ASGNI4
ADDRGP4 level
ADDRLP4 12
INDIRI4
ASGNI4
line 171
;170:
;171:    G_SetActiveMap( level.index, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
ADDRGP4 level
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
line 172
;172:}
LABELV $163
endproc SG_LoadLevelData 16 16
export SG_DrawLevelStats
proc SG_DrawLevelStats 20 12
line 175
;173:
;174:void SG_DrawLevelStats( void )
;175:{
line 179
;176:    float font_scale;
;177:    vec2_t cursorPos;
;178:
;179:    font_scale = ImGui_GetFontScale();
ADDRLP4 12
ADDRGP4 ImGui_GetFontScale
CALLF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ASGNF4
line 181
;180:
;181:    if ( ImGui_BeginWindow( "EndLevel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) ) {
ADDRGP4 $174
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
EQI4 $172
line 182
;182:        ImGui_SetWindowFontScale(font_scale * 6);
ADDRLP4 0
INDIRF4
CNSTF4 1086324736
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 183
;183:        ImGui_TextUnformatted("Level Statistics");
ADDRGP4 $175
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 184
;184:        ImGui_SetWindowFontScale(font_scale * 3.5f);
ADDRLP4 0
INDIRF4
CNSTF4 1080033280
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 185
;185:        ImGui_NewLine();
ADDRGP4 ImGui_NewLine
CALLV
pop
line 187
;186:
;187:        ImGui_GetCursorScreenPos( &cursorPos.x, &cursorPos.y );
ADDRLP4 4
ARGP4
ADDRLP4 4+4
ARGP4
ADDRGP4 ImGui_GetCursorScreenPos
CALLV
pop
line 189
;188:
;189:        ImGui_SetCursorScreenPos( cursorPos.x, cursorPos.y + 20);
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
line 190
;190:    }
LABELV $172
line 191
;191:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 192
;192:}
LABELV $171
endproc SG_DrawLevelStats 20 12
export SG_EndLevel
proc SG_EndLevel 4 0
line 195
;193:
;194:int SG_EndLevel( void )
;195:{
line 196
;196:    level.stats.timeEnd = Sys_Milliseconds();
ADDRLP4 0
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4+4
ADDRLP4 0
INDIRI4
ASGNI4
line 198
;197:
;198:    sg.state = SG_SHOW_LEVEL_STATS;
ADDRGP4 sg+2212
CNSTI4 4
ASGNI4
line 200
;199:
;200:    return 1;
CNSTI4 1
RETI4
LABELV $178
endproc SG_EndLevel 4 0
export SG_ParseInfos
proc SG_ParseInfos 2084 16
line 209
;201:}
;202:
;203:#define MAX_LEVELINFO_LEN 8192
;204:#define MAX_LEVELS 1024
;205:
;206:static char *sg_levelInfos[MAX_LEVELS];
;207:
;208:int SG_ParseInfos( char *buf, int max, char **infos )
;209:{
line 215
;210:    const char *token, **text;
;211:    int count;
;212:    char key[MAX_TOKEN_CHARS];
;213:    char info[MAX_INFO_STRING];
;214:
;215:    text = (const char **)&buf;
ADDRLP4 1028
ADDRFP4 0
ASGNP4
line 216
;216:    count = 0;
ADDRLP4 2056
CNSTI4 0
ASGNI4
ADDRGP4 $184
JUMPV
LABELV $183
line 218
;217:
;218:    while ( 1 ) {
line 219
;219:        token = COM_Parse( text );
ADDRLP4 1028
INDIRP4
ARGP4
ADDRLP4 2060
ADDRGP4 COM_Parse
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 2060
INDIRP4
ASGNP4
line 220
;220:        if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $186
line 221
;221:            break;
ADDRGP4 $185
JUMPV
LABELV $186
line 223
;222:        }
;223:        if ( token[0] != '{' ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 123
EQI4 $188
line 224
;224:            G_Printf( "missing '{' in info file\n" );
ADDRGP4 $190
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 225
;225:            break;
ADDRGP4 $185
JUMPV
LABELV $188
line 228
;226:        }
;227:
;228:        if ( count == max ) {
ADDRLP4 2056
INDIRI4
ADDRFP4 4
INDIRI4
NEI4 $191
line 229
;229:            G_Printf( "max infos exceeded\n" );
ADDRGP4 $193
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 230
;230:            break;
ADDRGP4 $185
JUMPV
LABELV $191
line 233
;231:        }
;232:
;233:        info[0] = '\0';
ADDRLP4 1032
CNSTI1 0
ASGNI1
ADDRGP4 $195
JUMPV
LABELV $194
line 234
;234:        while ( 1 ) {
line 235
;235:            token = COM_ParseExt( text, qtrue );
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
line 236
;236:            if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $197
line 237
;237:                G_Printf( "unexpected end of info file\n" );
ADDRGP4 $199
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 238
;238:                break;
ADDRGP4 $196
JUMPV
LABELV $197
line 240
;239:            }
;240:            if ( token[0] == '}' ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 125
NEI4 $200
line 241
;241:                break;
ADDRGP4 $196
JUMPV
LABELV $200
line 243
;242:            }
;243:            N_strncpyz( key, token, sizeof(key) );
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
line 245
;244:
;245:            token = COM_ParseExt( text, qfalse );
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
line 246
;246:            if ( !token[0] ) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $202
line 247
;247:                token = "<NULL>";
ADDRLP4 0
ADDRGP4 $204
ASGNP4
line 248
;248:            }
LABELV $202
line 249
;249:            Info_SetValueForKey( info, key, token );
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
line 250
;250:        }
LABELV $195
line 234
ADDRGP4 $194
JUMPV
LABELV $196
line 252
;251:        // NOTE: extra space for level index
;252:        infos[count] = SG_MemAlloc( strlen( info ) + strlen( "\\num\\" ) + strlen( va( "%i", MAX_LEVELS ) ) + 1 );
ADDRLP4 1032
ARGP4
ADDRLP4 2064
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRGP4 $205
ARGP4
ADDRLP4 2068
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRGP4 $144
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
line 253
;253:        if ( infos[count] ) {
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
EQU4 $206
line 254
;254:            strcpy( infos[count], info );
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
line 255
;255:            count++;
ADDRLP4 2056
ADDRLP4 2056
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 256
;256:        }
LABELV $206
line 257
;257:    }
LABELV $184
line 218
ADDRGP4 $183
JUMPV
LABELV $185
line 259
;258:
;259:    return count;
ADDRLP4 2056
INDIRI4
RETI4
LABELV $182
endproc SG_ParseInfos 2084 16
proc SG_LoadLevelsFromFile 8212 16
line 262
;260:}
;261:
;262:static void SG_LoadLevelsFromFile( const char *filename ) {
line 267
;263:	int				len;
;264:	fileHandle_t    f;
;265:	char			buf[MAX_LEVELINFO_LEN];
;266:
;267:	len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
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
line 268
;268:	if ( !f ) {
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $209
line 269
;269:		trap_Print( va( COLOR_RED "ERROR: file not found: %s\n", filename ) );
ADDRGP4 $211
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
line 270
;270:		return;
ADDRGP4 $208
JUMPV
LABELV $209
line 272
;271:	}
;272:	if ( len >= MAX_LEVELINFO_LEN ) {
ADDRLP4 0
INDIRI4
CNSTI4 8192
LTI4 $212
line 273
;273:		trap_Print( va( COLOR_RED "ERROR: file too large: %s is %i, max allowed is %i", filename, len, MAX_LEVELINFO_LEN ) );
ADDRGP4 $214
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
line 274
;274:		trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 275
;275:		return;
ADDRGP4 $208
JUMPV
LABELV $212
line 278
;276:	}
;277:
;278:	trap_FS_Read( buf, len, f );
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
line 279
;279:	buf[len] = 0;
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 0
ASGNI1
line 280
;280:	trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 282
;281:
;282:	sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
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
line 283
;283:}
LABELV $208
endproc SG_LoadLevelsFromFile 8212 16
export SG_LoadLevels
proc SG_LoadLevels 2612 16
line 286
;284:
;285:void SG_LoadLevels( void )
;286:{
line 297
;287:    int numdirs;
;288:    vmCvar_t levelsFile;
;289:    char filename[1024];
;290:    char dirlist[1024];
;291:    char *dirptr;
;292:    int i;
;293:    int dirlen;
;294:    const char *mapname;
;295:    levelInfo_t *info;
;296:
;297:    sg.numLevels = 0;
ADDRGP4 sg+2220
CNSTI4 0
ASGNI4
line 299
;298:
;299:    Cvar_Register( &levelsFile, "sg_levelsFile", "", CVAR_INIT | CVAR_ROM );
ADDRLP4 2068
ARGP4
ADDRGP4 $220
ARGP4
ADDRGP4 $221
ARGP4
CNSTI4 80
ARGI4
ADDRGP4 Cvar_Register
CALLV
pop
line 300
;300:    if ( *levelsFile.s ) {
ADDRLP4 2068
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $222
line 301
;301:        SG_LoadLevelsFromFile( levelsFile.s );
ADDRLP4 2068
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 302
;302:    } else {
ADDRGP4 $223
JUMPV
LABELV $222
line 303
;303:        SG_LoadLevelsFromFile( "scripts/levels.txt" );
ADDRGP4 $224
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 304
;304:    }
LABELV $223
line 307
;305:
;306:    // get all levels from .level files
;307:    numdirs = trap_FS_GetFileList( "scripts", ".level", dirlist, sizeof(dirlist) );
ADDRGP4 $225
ARGP4
ADDRGP4 $226
ARGP4
ADDRLP4 1044
ARGP4
CNSTI4 1024
ARGI4
ADDRLP4 2348
ADDRGP4 trap_FS_GetFileList
CALLI4
ASGNI4
ADDRLP4 1040
ADDRLP4 2348
INDIRI4
ASGNI4
line 308
;308:	dirptr  = dirlist;
ADDRLP4 4
ADDRLP4 1044
ASGNP4
line 309
;309:	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $230
JUMPV
LABELV $227
line 310
;310:		dirlen = strlen( dirptr );
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 2352
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 1036
ADDRLP4 2352
INDIRI4
ASGNI4
line 311
;311:		strcpy( filename, "scripts/" );
ADDRLP4 8
ARGP4
ADDRGP4 $231
ARGP4
ADDRGP4 strcpy
CALLI4
pop
line 312
;312:		strcat( filename, dirptr );
ADDRLP4 8
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 strcat
CALLI4
pop
line 313
;313:		SG_LoadLevelsFromFile( filename );
ADDRLP4 8
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 314
;314:	}
LABELV $228
line 309
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
ADDRLP4 1036
INDIRI4
CNSTI4 1
ADDI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
LABELV $230
ADDRLP4 0
INDIRI4
ADDRLP4 1040
INDIRI4
LTI4 $227
line 316
;315:
;316:	SG_Printf( "%i levels parsed.\n", sg.numLevels);
ADDRGP4 $232
ARGP4
ADDRGP4 sg+2220
INDIRI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 317
;317:	if ( SG_OutOfMemory() ) {
ADDRLP4 2352
ADDRGP4 SG_OutOfMemory
CALLI4
ASGNI4
ADDRLP4 2352
INDIRI4
CNSTI4 0
EQI4 $234
line 318
;318:        trap_Error( COLOR_RED "ERROR: not anough memory in pool to load all levels" );
ADDRGP4 $236
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 319
;319:    }
LABELV $234
line 322
;320:
;321:	// set initial numbers
;322:	for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $240
JUMPV
LABELV $237
line 323
;323:		Info_SetValueForKey( sg_levelInfos[i], "num", va( "%i", i ) );
ADDRGP4 $144
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 2356
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
ADDRGP4 $242
ARGP4
ADDRLP4 2356
INDIRP4
ARGP4
ADDRGP4 Info_SetValueForKey_s
CALLI4
pop
line 324
;324:    }
LABELV $238
line 322
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $240
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $237
line 326
;325:
;326:    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );
ADDRGP4 sg+2220
INDIRI4
CVIU4 4
CNSTU4 172
MULU4
CVUI4 4
ARGI4
ADDRLP4 2356
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRGP4 sg_levelInfoData
ADDRLP4 2356
INDIRP4
ASGNP4
line 329
;327:
;328:    // load the level information (difficulty, map, etc.)
;329:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $247
JUMPV
LABELV $244
line 330
;330:        N_strncpyz( sg_levelInfoData[i].name, Info_ValueForKey( sg_levelInfos[i], "name" ), MAX_NPATH );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $249
ARGP4
ADDRLP4 2360
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ARGP4
ADDRLP4 2360
INDIRP4
ARGP4
CNSTU4 64
ARGU4
ADDRGP4 N_strncpyz
CALLV
pop
line 332
;331:
;332:        mapname = Info_ValueForKey( sg_levelInfos[i], "mapname" );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $110
ARGP4
ADDRLP4 2364
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 1032
ADDRLP4 2364
INDIRP4
ASGNP4
line 333
;333:        sg_levelInfoData[i].maphandle = G_LoadMap( mapname );
ADDRLP4 1032
INDIRP4
ARGP4
ADDRLP4 2368
ADDRGP4 G_LoadMap
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 64
ADDP4
ADDRLP4 2368
INDIRI4
ASGNI4
line 334
;334:        if ( sg_levelInfoData[i].maphandle == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 64
ADDP4
INDIRI4
CNSTI4 0
NEI4 $250
line 335
;335:            G_Printf( COLOR_YELLOW "WARNING: failed to load map '%s' for level '%s'\n", mapname, sg_levelInfoData[i].name );
ADDRGP4 $252
ARGP4
ADDRLP4 1032
INDIRP4
ARGP4
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 336
;336:            continue;
ADDRGP4 $245
JUMPV
LABELV $250
line 339
;337:        }
;338:        
;339:        sg_levelInfoData[i].a.rank = LEVEL_RANK_A;
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 72
ADDP4
CNSTI4 0
ASGNI4
line 340
;340:        sg_levelInfoData[i].a.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $253
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 80
ADDP4
ADDRLP4 2380
INDIRI4
ASGNI4
line 341
;341:        sg_levelInfoData[i].a.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $254
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 76
ADDP4
ADDRLP4 2392
INDIRI4
ASGNI4
line 342
;342:        sg_levelInfoData[i].a.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $255
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 84
ADDP4
ADDRLP4 2404
INDIRI4
ASGNI4
line 343
;343:        sg_levelInfoData[i].a.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $256
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 88
ADDP4
ADDRLP4 2416
INDIRI4
ASGNI4
line 345
;344:
;345:        sg_levelInfoData[i].b.rank = LEVEL_RANK_B;
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 92
ADDP4
CNSTI4 1
ASGNI4
line 346
;346:        sg_levelInfoData[i].b.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $257
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 100
ADDP4
ADDRLP4 2428
INDIRI4
ASGNI4
line 347
;347:        sg_levelInfoData[i].b.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $258
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 96
ADDP4
ADDRLP4 2440
INDIRI4
ASGNI4
line 348
;348:        sg_levelInfoData[i].b.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $259
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 104
ADDP4
ADDRLP4 2452
INDIRI4
ASGNI4
line 349
;349:        sg_levelInfoData[i].b.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $256
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 108
ADDP4
ADDRLP4 2464
INDIRI4
ASGNI4
line 351
;350:
;351:        sg_levelInfoData[i].c.rank = LEVEL_RANK_C;
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 112
ADDP4
CNSTI4 2
ASGNI4
line 352
;352:        sg_levelInfoData[i].c.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $260
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 120
ADDP4
ADDRLP4 2476
INDIRI4
ASGNI4
line 353
;353:        sg_levelInfoData[i].c.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $261
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 116
ADDP4
ADDRLP4 2488
INDIRI4
ASGNI4
line 354
;354:        sg_levelInfoData[i].c.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $262
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 124
ADDP4
ADDRLP4 2500
INDIRI4
ASGNI4
line 355
;355:        sg_levelInfoData[i].c.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $256
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 128
ADDP4
ADDRLP4 2512
INDIRI4
ASGNI4
line 357
;356:
;357:        sg_levelInfoData[i].d.rank = LEVEL_RANK_D;
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 132
ADDP4
CNSTI4 3
ASGNI4
line 358
;358:        sg_levelInfoData[i].d.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $263
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 140
ADDP4
ADDRLP4 2524
INDIRI4
ASGNI4
line 359
;359:        sg_levelInfoData[i].d.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $264
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 136
ADDP4
ADDRLP4 2536
INDIRI4
ASGNI4
line 360
;360:        sg_levelInfoData[i].d.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $265
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 144
ADDP4
ADDRLP4 2548
INDIRI4
ASGNI4
line 361
;361:        sg_levelInfoData[i].d.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $256
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 148
ADDP4
ADDRLP4 2560
INDIRI4
ASGNI4
line 363
;362:
;363:        sg_levelInfoData[i].f.rank = LEVEL_RANK_WERE_U_BOTTING;
ADDRLP4 0
INDIRI4
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 152
ADDP4
CNSTI4 4
ASGNI4
line 364
;364:        sg_levelInfoData[i].f.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $266
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 160
ADDP4
ADDRLP4 2572
INDIRI4
ASGNI4
line 365
;365:        sg_levelInfoData[i].f.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $267
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 156
ADDP4
ADDRLP4 2584
INDIRI4
ASGNI4
line 366
;366:        sg_levelInfoData[i].f.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $268
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 164
ADDP4
ADDRLP4 2596
INDIRI4
ASGNI4
line 367
;367:        sg_levelInfoData[i].f.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $256
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
CNSTI4 172
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 168
ADDP4
ADDRLP4 2608
INDIRI4
ASGNI4
line 368
;368:    }
LABELV $245
line 329
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $247
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $244
line 369
;369:}
LABELV $218
endproc SG_LoadLevels 2612 16
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
skip 28
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
import SG_BuildMoveCommand
import SGameCommand
import SG_DrawFrame
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
LABELV $268
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $267
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
LABELV $266
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
LABELV $265
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $264
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
LABELV $263
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
LABELV $262
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $261
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
LABELV $260
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
LABELV $259
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $258
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
LABELV $257
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
LABELV $256
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
LABELV $255
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $254
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
LABELV $253
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
LABELV $252
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
LABELV $249
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $242
byte 1 110
byte 1 117
byte 1 109
byte 1 0
align 1
LABELV $236
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
LABELV $232
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
LABELV $231
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
LABELV $226
byte 1 46
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $225
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $224
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
LABELV $221
byte 1 0
align 1
LABELV $220
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
LABELV $214
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
LABELV $211
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
LABELV $205
byte 1 92
byte 1 110
byte 1 117
byte 1 109
byte 1 92
byte 1 0
align 1
LABELV $204
byte 1 60
byte 1 78
byte 1 85
byte 1 76
byte 1 76
byte 1 62
byte 1 0
align 1
LABELV $199
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
LABELV $193
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
LABELV $190
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
LABELV $175
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
LABELV $174
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
LABELV $166
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
byte 1 102
byte 1 101
byte 1 116
byte 1 99
byte 1 104
byte 1 32
byte 1 34
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 34
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
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
byte 1 33
byte 1 0
align 1
LABELV $156
byte 1 115
byte 1 103
byte 1 95
byte 1 99
byte 1 97
byte 1 109
byte 1 101
byte 1 114
byte 1 97
byte 1 95
byte 1 112
byte 1 111
byte 1 115
byte 1 105
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $155
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 95
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $153
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
byte 1 95
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $152
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $150
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 46
byte 1 0
align 1
LABELV $144
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $143
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
LABELV $140
byte 1 109
byte 1 97
byte 1 112
byte 1 115
byte 1 47
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $137
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
LABELV $134
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
LABELV $131
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
LABELV $128
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
LABELV $125
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
LABELV $124
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
LABELV $114
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
LABELV $113
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
LABELV $110
byte 1 109
byte 1 97
byte 1 112
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
