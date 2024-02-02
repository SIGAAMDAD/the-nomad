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
ADDRGP4 sg+72
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
ADDRGP4 sg+4268392
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
ADDRGP4 sg+4268392+23624
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
;98:    G_Printf( "Starting up level %s\n", info->name );
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
;100:    // clear the old level
;101:    memset( &level, 0, sizeof(level) );
ADDRGP4 level
ARGP4
CNSTI4 0
ARGI4
CNSTU4 28
ARGU4
ADDRGP4 memset
CALLP4
pop
line 103
;102:
;103:    G_SetActiveMap( info->maphandle, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
ADDRLP4 0
INDIRP4
CNSTI4 64
ADDP4
INDIRI4
ARGI4
ADDRGP4 sg+4268392
ARGP4
ADDRGP4 sg+74080
ARGP4
ADDRGP4 sg+74040
ARGP4
ADDRGP4 G_SetActiveMap
CALLV
pop
line 105
;104:
;105:    SG_InitPlayer();
ADDRGP4 SG_InitPlayer
CALLV
pop
line 108
;106:
;107:    // spawn everything
;108:    SG_SpawnLevelEntities();
ADDRGP4 SG_SpawnLevelEntities
CALLV
pop
line 110
;109:
;110:    sg.state = SG_IN_LEVEL;
ADDRGP4 sg+64
CNSTI4 3
ASGNI4
line 112
;111:
;112:    if ( sg_printLevelStats.i ) {
ADDRGP4 sg_printLevelStats+260
INDIRI4
CNSTI4 0
EQI4 $119
line 113
;113:        G_Printf( "\n---------- Level Info ----------\n" );
ADDRGP4 $122
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 114
;114:        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
ADDRGP4 $123
ARGP4
ADDRGP4 sg+4268392+23552
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 115
;115:        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
ADDRGP4 $126
ARGP4
ADDRGP4 sg+4268392+23628
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 116
;116:        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
ADDRGP4 $129
ARGP4
ADDRGP4 sg+4268392+23624
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 117
;117:        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
ADDRGP4 $132
ARGP4
ADDRGP4 sg+4268392+23616
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 118
;118:        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
ADDRGP4 $135
ARGP4
ADDRGP4 sg+4268392+23620
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 119
;119:    }
LABELV $119
line 121
;120:
;121:    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );
ADDRGP4 $138
ARGP4
ADDRGP4 sg+4268392+23552
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
line 123
;122:
;123:    Cvar_Set( "sg_levelIndex", va( "%i", (uintptr_t)( info - sg_levelInfoData ) ) );
ADDRGP4 $142
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
ARGU4
ADDRLP4 88
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $141
ARGP4
ADDRLP4 88
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 125
;124:
;125:    level.stats.timeStart = trap_Milliseconds();
ADDRLP4 92
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4
ADDRLP4 92
INDIRI4
ASGNI4
line 127
;126:
;127:    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
ADDRLP4 96
ADDRGP4 sg+4268392
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
ADDRGP4 sg+4268392+4
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
line 128
;128:    zoom = 1.6f;
ADDRLP4 76
CNSTF4 1070386381
ASGNF4
line 130
;129:
;130:    G_Printf( "Done." );
ADDRGP4 $148
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 134
;131:
;132://    G_SetCameraData( cameraPos, zoom, 0.0f );
;133:
;134:    return qtrue;
CNSTI4 1
RETI4
LABELV $109
endproc SG_StartLevel 104 16
export SG_SaveLevelData
proc SG_SaveLevelData 0 4
line 138
;135:}
;136:
;137:void SG_SaveLevelData( void )
;138:{
line 139
;139:    trap_BeginSaveSection( "level" );
ADDRGP4 $150
ARGP4
ADDRGP4 trap_BeginSaveSection
CALLV
pop
line 140
;140:}
LABELV $149
endproc SG_SaveLevelData 0 4
export SG_LoadLevelData
proc SG_LoadLevelData 16 16
line 143
;141:
;142:void SG_LoadLevelData( void )
;143:{
line 146
;144:    nhandle_t section;
;145:
;146:    section = trap_GetSaveSection( "level" );
ADDRGP4 $150
ARGP4
ADDRLP4 4
ADDRGP4 trap_GetSaveSection
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRI4
ASGNI4
line 147
;147:    if ( section == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $152
line 148
;148:        trap_Error( "SG_LoadLevelData: failed to fetch \"level\" section from save file!" );
ADDRGP4 $154
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 149
;149:    }
LABELV $152
line 151
;150:
;151:    level.checkpointIndex = trap_LoadInt( "checkpoint", section );
ADDRGP4 $156
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 8
ADDRGP4 trap_LoadInt
CALLI4
ASGNI4
ADDRGP4 level+24
ADDRLP4 8
INDIRI4
ASGNI4
line 152
;152:    level.index = trap_LoadInt( "index", section );
ADDRGP4 $157
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 12
ADDRGP4 trap_LoadInt
CALLI4
ASGNI4
ADDRGP4 level
ADDRLP4 12
INDIRI4
ASGNI4
line 154
;153:
;154:    G_SetActiveMap( level.index, &sg.mapInfo, sg.soundBits, &sg.activeEnts );
ADDRGP4 level
INDIRI4
ARGI4
ADDRGP4 sg+4268392
ARGP4
ADDRGP4 sg+74080
ARGP4
ADDRGP4 sg+74040
ARGP4
ADDRGP4 G_SetActiveMap
CALLV
pop
line 155
;155:}
LABELV $151
endproc SG_LoadLevelData 16 16
export SG_DrawLevelStats
proc SG_DrawLevelStats 20 12
line 158
;156:
;157:void SG_DrawLevelStats( void )
;158:{
line 162
;159:    float font_scale;
;160:    vec2_t cursorPos;
;161:
;162:    font_scale = ImGui_GetFontScale();
ADDRLP4 12
ADDRGP4 ImGui_GetFontScale
CALLF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ASGNF4
line 164
;163:
;164:    if ( ImGui_BeginWindow( "EndLevel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) ) {
ADDRGP4 $164
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
EQI4 $162
line 165
;165:        ImGui_SetWindowFontScale(font_scale * 6);
ADDRLP4 0
INDIRF4
CNSTF4 1086324736
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 166
;166:        ImGui_TextUnformatted("Level Statistics");
ADDRGP4 $165
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 167
;167:        ImGui_SetWindowFontScale(font_scale * 3.5f);
ADDRLP4 0
INDIRF4
CNSTF4 1080033280
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 168
;168:        ImGui_NewLine();
ADDRGP4 ImGui_NewLine
CALLV
pop
line 170
;169:
;170:        ImGui_GetCursorScreenPos( &cursorPos.x, &cursorPos.y );
ADDRLP4 4
ARGP4
ADDRLP4 4+4
ARGP4
ADDRGP4 ImGui_GetCursorScreenPos
CALLV
pop
line 172
;171:
;172:        ImGui_SetCursorScreenPos( cursorPos.x, cursorPos.y + 20);
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
line 173
;173:    }
LABELV $162
line 174
;174:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 175
;175:}
LABELV $161
endproc SG_DrawLevelStats 20 12
export SG_EndLevel
proc SG_EndLevel 4 0
line 178
;176:
;177:int SG_EndLevel( void )
;178:{
line 179
;179:    level.stats.timeEnd = trap_Milliseconds();
ADDRLP4 0
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4+4
ADDRLP4 0
INDIRI4
ASGNI4
line 181
;180:
;181:    sg.state = SG_SHOW_LEVEL_STATS;
ADDRGP4 sg+64
CNSTI4 4
ASGNI4
line 183
;182:
;183:    return 1;
CNSTI4 1
RETI4
LABELV $168
endproc SG_EndLevel 4 0
export SG_ParseInfos
proc SG_ParseInfos 2084 16
line 192
;184:}
;185:
;186:#define MAX_LEVELINFO_LEN 8192
;187:#define MAX_LEVELS 1024
;188:
;189:static char *sg_levelInfos[MAX_LEVELS];
;190:
;191:int SG_ParseInfos( char *buf, int max, char **infos )
;192:{
line 198
;193:    const char *token, **text;
;194:    int count;
;195:    char key[MAX_TOKEN_CHARS];
;196:    char info[MAX_INFO_STRING];
;197:
;198:    text = (const char **)&buf;
ADDRLP4 1028
ADDRFP4 0
ASGNP4
line 199
;199:    count = 0;
ADDRLP4 2056
CNSTI4 0
ASGNI4
ADDRGP4 $174
JUMPV
LABELV $173
line 201
;200:
;201:    while (1) {
line 202
;202:        token = COM_Parse( text );
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
line 203
;203:        if (!token[0]) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $176
line 204
;204:            break;
ADDRGP4 $175
JUMPV
LABELV $176
line 206
;205:        }
;206:        if (token[0] != '{') {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 123
EQI4 $178
line 207
;207:            Con_Printf( "missing '{' in info file\n" );
ADDRGP4 $180
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 208
;208:            break;
ADDRGP4 $175
JUMPV
LABELV $178
line 211
;209:        }
;210:
;211:        if (count == max) {
ADDRLP4 2056
INDIRI4
ADDRFP4 4
INDIRI4
NEI4 $181
line 212
;212:            Con_Printf( "max infos exceeded\n" );
ADDRGP4 $183
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 213
;213:            break;
ADDRGP4 $175
JUMPV
LABELV $181
line 216
;214:        }
;215:
;216:        info[0] = '\0';
ADDRLP4 1032
CNSTI1 0
ASGNI1
ADDRGP4 $185
JUMPV
LABELV $184
line 217
;217:        while (1) {
line 218
;218:            token = COM_ParseExt( text, qtrue );
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
line 219
;219:            if (!token[0]) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $187
line 220
;220:                Con_Printf( "unexpected end of info file\n" );
ADDRGP4 $189
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 221
;221:                break;
ADDRGP4 $186
JUMPV
LABELV $187
line 223
;222:            }
;223:            if (token[0] == '}') {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 125
NEI4 $190
line 224
;224:                break;
ADDRGP4 $186
JUMPV
LABELV $190
line 226
;225:            }
;226:            N_strncpyz( key, token, sizeof(key) );
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
line 228
;227:
;228:            token = COM_ParseExt( text, qfalse );
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
line 229
;229:            if (!token[0]) {
ADDRLP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $192
line 230
;230:                token = "<NULL>";
ADDRLP4 0
ADDRGP4 $194
ASGNP4
line 231
;231:            }
LABELV $192
line 232
;232:            Info_SetValueForKey( info, key, token );
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
line 233
;233:        }
LABELV $185
line 217
ADDRGP4 $184
JUMPV
LABELV $186
line 235
;234:        // NOTE: extra space for level index
;235:        infos[count] = SG_MemAlloc( strlen(info) + strlen("\\num\\") + strlen(va("%i", MAX_LEVELS)) + 1 );
ADDRLP4 1032
ARGP4
ADDRLP4 2064
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRGP4 $195
ARGP4
ADDRLP4 2068
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRGP4 $142
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
CALLU4
ASGNU4
ADDRLP4 2064
INDIRU4
ADDRLP4 2068
INDIRU4
ADDU4
ADDRLP4 2076
INDIRU4
ADDU4
CNSTU4 1
ADDU4
CVUI4 4
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
line 236
;236:        if (infos[count]) {
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
EQU4 $196
line 237
;237:            strcpy(infos[count], info);
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
CALLP4
pop
line 238
;238:            count++;
ADDRLP4 2056
ADDRLP4 2056
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 239
;239:        }
LABELV $196
line 240
;240:    }
LABELV $174
line 201
ADDRGP4 $173
JUMPV
LABELV $175
line 242
;241:
;242:    return count;
ADDRLP4 2056
INDIRI4
RETI4
LABELV $172
endproc SG_ParseInfos 2084 16
proc SG_LoadLevelInfoFromFile 8212 16
line 246
;243:}
;244:
;245:static void SG_LoadLevelInfoFromFile( const char *filename )
;246:{
line 251
;247:    int len;
;248:    file_t f;
;249:    char buf[MAX_LEVELINFO_LEN];
;250:
;251:    len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
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
line 252
;252:    if (f == FS_INVALID_HANDLE) {
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $199
line 253
;253:        G_Printf( COLOR_RED "file not found: %s\n", filename );
ADDRGP4 $201
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 254
;254:        return;
ADDRGP4 $198
JUMPV
LABELV $199
line 256
;255:    }
;256:    if (len >= MAX_LEVELINFO_LEN) {
ADDRLP4 0
INDIRI4
CNSTI4 8192
LTI4 $202
line 257
;257:        G_Printf( COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_LEVELINFO_LEN );
ADDRGP4 $204
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
CNSTI4 8192
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 258
;258:        trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 259
;259:        return;
ADDRGP4 $198
JUMPV
LABELV $202
line 262
;260:    }
;261:
;262:    trap_FS_Read( buf, len, f );
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
line 263
;263:    buf[len] = 0;
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 0
ASGNI1
line 264
;264:    trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 266
;265:
;266:    sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
ADDRLP4 8
ARGP4
CNSTI4 1024
ADDRGP4 sg+72
INDIRI4
SUBI4
ARGI4
ADDRGP4 sg+72
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
ADDRGP4 sg+72
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
line 267
;267:}
LABELV $198
endproc SG_LoadLevelInfoFromFile 8212 16
proc SG_LoadLevelInfos 1456 16
line 270
;268:
;269:static void SG_LoadLevelInfos( void )
;270:{
line 279
;271:    int numdirs;
;272:    char filename[128];
;273:    char dirlist[1024];
;274:    char *dirptr;
;275:    int i, dirlen;
;276:    int n;
;277:    vmCvar_t levelInfoFile;
;278:
;279:    sg.numLevels = 0;
ADDRGP4 sg+72
CNSTI4 0
ASGNI4
line 281
;280:
;281:    Cvar_Register( &levelInfoFile, "sg_levelInfoFile", "", CVAR_INIT | CVAR_ROM );
ADDRLP4 148
ARGP4
ADDRGP4 $210
ARGP4
ADDRGP4 $211
ARGP4
CNSTI4 80
ARGI4
ADDRGP4 Cvar_Register
CALLV
pop
line 282
;282:    if ( *levelInfoFile.s ) {
ADDRLP4 148
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $212
line 283
;283:        SG_LoadLevelInfoFromFile( levelInfoFile.s );
ADDRLP4 148
ARGP4
ADDRGP4 SG_LoadLevelInfoFromFile
CALLV
pop
line 284
;284:    } else {
ADDRGP4 $213
JUMPV
LABELV $212
line 285
;285:        SG_LoadLevelInfoFromFile( "scripts/levels.txt" );
ADDRGP4 $214
ARGP4
ADDRGP4 SG_LoadLevelInfoFromFile
CALLV
pop
line 286
;286:    }
LABELV $213
line 289
;287:
;288:    // get all arenas from .lvl files
;289:    numdirs = trap_FS_GetFileList( "scripts", ".lvl", dirlist, 1024 );
ADDRGP4 $215
ARGP4
ADDRGP4 $216
ARGP4
ADDRLP4 424
ARGP4
CNSTI4 1024
ARGI4
ADDRLP4 1448
ADDRGP4 trap_FS_GetFileList
CALLI4
ASGNI4
ADDRLP4 144
ADDRLP4 1448
INDIRI4
ASGNI4
line 290
;290:    dirptr = dirlist;
ADDRLP4 4
ADDRLP4 424
ASGNP4
line 291
;291:    for (i = 0; i < numdirs; i++, dirptr += dirlen + 1) {
ADDRLP4 136
CNSTI4 0
ASGNI4
ADDRGP4 $220
JUMPV
LABELV $217
line 292
;292:        dirlen = strlen( dirptr );
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 1452
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 140
ADDRLP4 1452
INDIRU4
CVUI4 4
ASGNI4
line 296
;293:
;294:        // FIXME: possibly use Com_snprintf?
;295:
;296:        strcpy( filename, "scripts/" );
ADDRLP4 8
ARGP4
ADDRGP4 $221
ARGP4
ADDRGP4 strcpy
CALLP4
pop
line 297
;297:        strcat( filename, dirptr );
ADDRLP4 8
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 strcat
CALLP4
pop
line 298
;298:        SG_LoadLevelInfoFromFile( filename );
ADDRLP4 8
ARGP4
ADDRGP4 SG_LoadLevelInfoFromFile
CALLV
pop
line 299
;299:    }
LABELV $218
line 291
ADDRLP4 136
ADDRLP4 136
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 4
ADDRLP4 140
INDIRI4
CNSTI4 1
ADDI4
ADDRLP4 4
INDIRP4
ADDP4
ASGNP4
LABELV $220
ADDRLP4 136
INDIRI4
ADDRLP4 144
INDIRI4
LTI4 $217
line 300
;300:    G_Printf( "%i levels parsed.\n", sg.numLevels );
ADDRGP4 $222
ARGP4
ADDRGP4 sg+72
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 303
;301:
;302:    // set initial numbers
;303:    for ( n = 0; n < sg.numLevels; n++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $227
JUMPV
LABELV $224
line 304
;304:        Info_SetValueForKey( sg_levelInfos[n], "num", va( "%i", n ) );
ADDRGP4 $142
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 1452
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
ADDRGP4 $229
ARGP4
ADDRLP4 1452
INDIRP4
ARGP4
ADDRGP4 Info_SetValueForKey_s
CALLI4
pop
line 305
;305:    }
LABELV $225
line 303
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $227
ADDRLP4 0
INDIRI4
ADDRGP4 sg+72
INDIRI4
LTI4 $224
line 306
;306:}
LABELV $208
endproc SG_LoadLevelInfos 1456 16
proc SG_LoadLevelsFromFile 8212 16
line 308
;307:
;308:static void SG_LoadLevelsFromFile( const char *filename ) {
line 313
;309:	int				len;
;310:	file_t	        f;
;311:	char			buf[MAX_LEVELINFO_LEN];
;312:
;313:	len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
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
line 314
;314:	if ( !f ) {
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $231
line 315
;315:		trap_Print( va( COLOR_RED "ERROR: file not found: %s\n", filename ) );
ADDRGP4 $233
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
line 316
;316:		return;
ADDRGP4 $230
JUMPV
LABELV $231
line 318
;317:	}
;318:	if ( len >= MAX_LEVELINFO_LEN ) {
ADDRLP4 0
INDIRI4
CNSTI4 8192
LTI4 $234
line 319
;319:		trap_Print( va( COLOR_RED "ERROR: file too large: %s is %i, max allowed is %i", filename, len, MAX_LEVELINFO_LEN ) );
ADDRGP4 $236
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
line 320
;320:		trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 321
;321:		return;
ADDRGP4 $230
JUMPV
LABELV $234
line 324
;322:	}
;323:
;324:	trap_FS_Read( buf, len, f );
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
line 325
;325:	buf[len] = 0;
ADDRLP4 0
INDIRI4
ADDRLP4 8
ADDP4
CNSTI1 0
ASGNI1
line 326
;326:	trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 328
;327:
;328:	sg.numLevels += SG_ParseInfos( buf, MAX_LEVELS - sg.numLevels, &sg_levelInfos[sg.numLevels] );
ADDRLP4 8
ARGP4
CNSTI4 1024
ADDRGP4 sg+72
INDIRI4
SUBI4
ARGI4
ADDRGP4 sg+72
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
ADDRGP4 sg+72
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
line 329
;329:}
LABELV $230
endproc SG_LoadLevelsFromFile 8212 16
export SG_LoadLevels
proc SG_LoadLevels 2616 16
line 332
;330:
;331:void SG_LoadLevels( void )
;332:{
line 343
;333:    int numdirs;
;334:    vmCvar_t levelsFile;
;335:    char filename[1024];
;336:    char dirlist[1024];
;337:    char *dirptr;
;338:    int i;
;339:    int dirlen;
;340:    const char *mapname;
;341:    levelInfo_t *info;
;342:
;343:    sg.numLevels = 0;
ADDRGP4 sg+72
CNSTI4 0
ASGNI4
line 345
;344:
;345:    G_Printf( "Loading level configs...\n" );
ADDRGP4 $242
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 347
;346:
;347:    Cvar_Register( &levelsFile, "sg_levelsFile", "", CVAR_INIT | CVAR_ROM );
ADDRLP4 2068
ARGP4
ADDRGP4 $243
ARGP4
ADDRGP4 $211
ARGP4
CNSTI4 80
ARGI4
ADDRGP4 Cvar_Register
CALLV
pop
line 348
;348:    if ( *levelsFile.s ) {
ADDRLP4 2068
INDIRI1
CVII4 1
CNSTI4 0
EQI4 $244
line 349
;349:        SG_LoadLevelsFromFile( levelsFile.s );
ADDRLP4 2068
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 350
;350:    } else {
ADDRGP4 $245
JUMPV
LABELV $244
line 351
;351:        SG_LoadLevelsFromFile( "scripts/levels.txt" );
ADDRGP4 $214
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 352
;352:    }
LABELV $245
line 355
;353:
;354:    // get all levels from .level files
;355:    numdirs = trap_FS_GetFileList( "scripts", ".level", dirlist, sizeof(dirlist) );
ADDRGP4 $215
ARGP4
ADDRGP4 $246
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
line 356
;356:	dirptr  = dirlist;
ADDRLP4 4
ADDRLP4 1044
ASGNP4
line 357
;357:	for ( i = 0; i < numdirs; i++, dirptr += dirlen + 1 ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $250
JUMPV
LABELV $247
line 358
;358:		dirlen = strlen( dirptr );
ADDRLP4 4
INDIRP4
ARGP4
ADDRLP4 2352
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 1036
ADDRLP4 2352
INDIRU4
CVUI4 4
ASGNI4
line 359
;359:		strcpy( filename, "scripts/" );
ADDRLP4 8
ARGP4
ADDRGP4 $221
ARGP4
ADDRGP4 strcpy
CALLP4
pop
line 360
;360:		strcat( filename, dirptr );
ADDRLP4 8
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 strcat
CALLP4
pop
line 361
;361:		SG_LoadLevelsFromFile( filename );
ADDRLP4 8
ARGP4
ADDRGP4 SG_LoadLevelsFromFile
CALLV
pop
line 362
;362:	}
LABELV $248
line 357
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
LABELV $250
ADDRLP4 0
INDIRI4
ADDRLP4 1040
INDIRI4
LTI4 $247
line 363
;363:	trap_Print( va( "%i levels parsed\n", sg.numLevels ) );
ADDRGP4 $251
ARGP4
ADDRGP4 sg+72
INDIRI4
ARGI4
ADDRLP4 2352
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 2352
INDIRP4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 364
;364:	if ( SG_OutOfMemory() ) {
ADDRLP4 2356
ADDRGP4 SG_OutOfMemory
CALLI4
ASGNI4
ADDRLP4 2356
INDIRI4
CNSTI4 0
EQI4 $253
line 365
;365:        trap_Error( COLOR_RED "ERROR: not anough memory in pool to load all levels" );
ADDRGP4 $255
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 366
;366:    }
LABELV $253
line 369
;367:
;368:	// set initial numbers
;369:	for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $259
JUMPV
LABELV $256
line 370
;370:		Info_SetValueForKey( sg_levelInfos[i], "num", va( "%i", i ) );
ADDRGP4 $142
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
ADDRGP4 $229
ARGP4
ADDRLP4 2360
INDIRP4
ARGP4
ADDRGP4 Info_SetValueForKey_s
CALLI4
pop
line 371
;371:    }
LABELV $257
line 369
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $259
ADDRLP4 0
INDIRI4
ADDRGP4 sg+72
INDIRI4
LTI4 $256
line 373
;372:
;373:    sg_levelInfoData = SG_MemAlloc( sizeof(*sg_levelInfoData) * sg.numLevels );
ADDRGP4 sg+72
INDIRI4
CVIU4 4
CNSTU4 172
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
line 376
;374:
;375:    // load the level information (difficulty, map, etc.)
;376:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $265
JUMPV
LABELV $262
line 377
;377:        N_strncpyz( sg_levelInfoData[i].name, Info_ValueForKey( sg_levelInfos[i], "name" ), MAX_NPATH );
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
ADDRLP4 2364
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
ADDRLP4 2364
INDIRP4
ARGP4
CNSTU4 64
ARGU4
ADDRGP4 N_strncpyz
CALLV
pop
line 379
;378:
;379:        mapname = Info_ValueForKey( sg_levelInfos[i], "mapname" );
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
ADDRLP4 2368
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 1032
ADDRLP4 2368
INDIRP4
ASGNP4
line 380
;380:        sg_levelInfoData[i].maphandle = G_LoadMap( mapname );
ADDRLP4 1032
INDIRP4
ARGP4
ADDRLP4 2372
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
ADDRLP4 2372
INDIRI4
ASGNI4
line 381
;381:        if ( sg_levelInfoData[i].maphandle == FS_INVALID_HANDLE ) {
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
NEI4 $268
line 382
;382:            G_Printf( COLOR_YELLOW "WARNING: failed to load map '%s' for level '%s'\n", mapname, sg_levelInfoData[i].name );
ADDRGP4 $270
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
line 383
;383:            continue;
ADDRGP4 $263
JUMPV
LABELV $268
line 386
;384:        }
;385:        
;386:        sg_levelInfoData[i].a.rank = LEVEL_RANK_A;
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
line 387
;387:        sg_levelInfoData[i].a.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $271
ARGP4
ADDRLP4 2380
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2380
INDIRP4
ARGP4
ADDRLP4 2384
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
ADDRLP4 2384
INDIRI4
ASGNI4
line 388
;388:        sg_levelInfoData[i].a.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $272
ARGP4
ADDRLP4 2392
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2392
INDIRP4
ARGP4
ADDRLP4 2396
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
ADDRLP4 2396
INDIRI4
ASGNI4
line 389
;389:        sg_levelInfoData[i].a.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankA_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $273
ARGP4
ADDRLP4 2404
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2404
INDIRP4
ARGP4
ADDRLP4 2408
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
ADDRLP4 2408
INDIRI4
ASGNI4
line 390
;390:        sg_levelInfoData[i].a.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $274
ARGP4
ADDRLP4 2416
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2416
INDIRP4
ARGP4
ADDRLP4 2420
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
ADDRLP4 2420
INDIRI4
ASGNI4
line 392
;391:
;392:        sg_levelInfoData[i].b.rank = LEVEL_RANK_B;
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
line 393
;393:        sg_levelInfoData[i].b.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $275
ARGP4
ADDRLP4 2428
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2428
INDIRP4
ARGP4
ADDRLP4 2432
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
ADDRLP4 2432
INDIRI4
ASGNI4
line 394
;394:        sg_levelInfoData[i].b.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $276
ARGP4
ADDRLP4 2440
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2440
INDIRP4
ARGP4
ADDRLP4 2444
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
ADDRLP4 2444
INDIRI4
ASGNI4
line 395
;395:        sg_levelInfoData[i].b.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankB_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $277
ARGP4
ADDRLP4 2452
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2452
INDIRP4
ARGP4
ADDRLP4 2456
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
ADDRLP4 2456
INDIRI4
ASGNI4
line 396
;396:        sg_levelInfoData[i].b.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $274
ARGP4
ADDRLP4 2464
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2464
INDIRP4
ARGP4
ADDRLP4 2468
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
ADDRLP4 2468
INDIRI4
ASGNI4
line 398
;397:
;398:        sg_levelInfoData[i].c.rank = LEVEL_RANK_C;
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
line 399
;399:        sg_levelInfoData[i].c.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $278
ARGP4
ADDRLP4 2476
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2476
INDIRP4
ARGP4
ADDRLP4 2480
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
ADDRLP4 2480
INDIRI4
ASGNI4
line 400
;400:        sg_levelInfoData[i].c.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $279
ARGP4
ADDRLP4 2488
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2488
INDIRP4
ARGP4
ADDRLP4 2492
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
ADDRLP4 2492
INDIRI4
ASGNI4
line 401
;401:        sg_levelInfoData[i].c.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankC_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $280
ARGP4
ADDRLP4 2500
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2500
INDIRP4
ARGP4
ADDRLP4 2504
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
ADDRLP4 2504
INDIRI4
ASGNI4
line 402
;402:        sg_levelInfoData[i].c.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $274
ARGP4
ADDRLP4 2512
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2512
INDIRP4
ARGP4
ADDRLP4 2516
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
ADDRLP4 2516
INDIRI4
ASGNI4
line 404
;403:
;404:        sg_levelInfoData[i].d.rank = LEVEL_RANK_D;
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
line 405
;405:        sg_levelInfoData[i].d.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $281
ARGP4
ADDRLP4 2524
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2524
INDIRP4
ARGP4
ADDRLP4 2528
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
ADDRLP4 2528
INDIRI4
ASGNI4
line 406
;406:        sg_levelInfoData[i].d.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $282
ARGP4
ADDRLP4 2536
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2536
INDIRP4
ARGP4
ADDRLP4 2540
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
ADDRLP4 2540
INDIRI4
ASGNI4
line 407
;407:        sg_levelInfoData[i].d.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankD_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $283
ARGP4
ADDRLP4 2548
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2548
INDIRP4
ARGP4
ADDRLP4 2552
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
ADDRLP4 2552
INDIRI4
ASGNI4
line 408
;408:        sg_levelInfoData[i].d.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $274
ARGP4
ADDRLP4 2560
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2560
INDIRP4
ARGP4
ADDRLP4 2564
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
ADDRLP4 2564
INDIRI4
ASGNI4
line 410
;409:
;410:        sg_levelInfoData[i].f.rank = LEVEL_RANK_WERE_U_BOTTING;
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
line 411
;411:        sg_levelInfoData[i].f.minKills = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minKills" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $284
ARGP4
ADDRLP4 2572
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2572
INDIRP4
ARGP4
ADDRLP4 2576
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
ADDRLP4 2576
INDIRI4
ASGNI4
line 412
;412:        sg_levelInfoData[i].f.minStyle = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minStyle" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $285
ARGP4
ADDRLP4 2584
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2584
INDIRP4
ARGP4
ADDRLP4 2588
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
ADDRLP4 2588
INDIRI4
ASGNI4
line 413
;413:        sg_levelInfoData[i].f.minTime = atoi( Info_ValueForKey( sg_levelInfos[i], "rankF_minTime" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $286
ARGP4
ADDRLP4 2596
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2596
INDIRP4
ARGP4
ADDRLP4 2600
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
ADDRLP4 2600
INDIRI4
ASGNI4
line 414
;414:        sg_levelInfoData[i].f.requireClean = atoi( Info_ValueForKey( sg_levelInfos[i], "cleanRunRequired" ) );
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg_levelInfos
ADDP4
INDIRP4
ARGP4
ADDRGP4 $274
ARGP4
ADDRLP4 2608
ADDRGP4 Info_ValueForKey
CALLP4
ASGNP4
ADDRLP4 2608
INDIRP4
ARGP4
ADDRLP4 2612
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
ADDRLP4 2612
INDIRI4
ASGNI4
line 415
;415:    }
LABELV $263
line 376
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $265
ADDRLP4 0
INDIRI4
ADDRGP4 sg+72
INDIRI4
LTI4 $262
line 416
;416:}
LABELV $240
endproc SG_LoadLevels 2616 16
import atoi
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
import RE_AddSpriteToScene
import RE_AddPolyToScene
import RE_RenderScene
import RE_ClearScene
import RE_LoadWorldMap
import RE_RegisterSprite
import RE_RegisterSpriteSheet
import RE_RegisterShader
import trap_Snd_ClearLoopingTrack
import trap_Snd_SetLoopingTrack
import trap_Snd_StopSfx
import trap_Snd_PlaySfx
import trap_Snd_QueueTrack
import trap_Snd_RegisterTrack
import trap_Snd_RegisterSfx
import trap_Key_ClearStates
import trap_Key_GetKey
import trap_Key_GetCatcher
import trap_Key_SetCatcher
import trap_Milliseconds
import trap_CheckWallHit
import G_SoundRecursive
import G_CastRay
import G_SetActiveMap
import G_LoadMap
import G_SetCameraData
import trap_MemoryRemaining
import trap_RemoveCommand
import trap_AddCommand
import trap_SendConsoleCommand
import trap_LoadVec4
import trap_LoadVec3
import trap_LoadVec2
import trap_LoadString
import trap_LoadFloat
import trap_LoadInt
import trap_LoadUInt
import trap_GetSaveSection
import trap_WriteVec4
import trap_WriteVec3
import trap_WriteVec2
import trap_WriteFloat
import trap_WriteString
import trap_WriteUInt
import trap_WriteInt
import trap_WriteChar
import trap_EndSaveSection
import trap_BeginSaveSection
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
import I_GetParm
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
import ClearBounds
import RadiusFromBounds
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
import acos
import fabs
import abs
import tan
import atan2
import cos
import sin
import sqrt
import floor
import ceil
import sscanf
import vsprintf
import rand
import srand
import qsort
import toupper
import tolower
import strncmp
import strcmp
import strstr
import strchr
import strlen
import strcat
import strcpy
import memmove
import memset
import memchr
import memcpy
lit
align 1
LABELV $286
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
LABELV $285
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
LABELV $284
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
LABELV $283
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
LABELV $282
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
LABELV $281
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
LABELV $280
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
LABELV $279
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
LABELV $278
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
LABELV $277
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
LABELV $276
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
LABELV $275
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
LABELV $274
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
LABELV $273
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
LABELV $272
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
LABELV $271
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
LABELV $270
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
LABELV $267
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $255
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
LABELV $251
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
byte 1 10
byte 1 0
align 1
LABELV $246
byte 1 46
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $243
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
LABELV $242
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 99
byte 1 111
byte 1 110
byte 1 102
byte 1 105
byte 1 103
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
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
LABELV $233
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
LABELV $229
byte 1 110
byte 1 117
byte 1 109
byte 1 0
align 1
LABELV $222
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
LABELV $221
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
LABELV $216
byte 1 46
byte 1 108
byte 1 118
byte 1 108
byte 1 0
align 1
LABELV $215
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $214
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
LABELV $211
byte 1 0
align 1
LABELV $210
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
byte 1 102
byte 1 111
byte 1 70
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $204
byte 1 94
byte 1 49
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
byte 1 10
byte 1 0
align 1
LABELV $201
byte 1 94
byte 1 49
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
LABELV $195
byte 1 92
byte 1 110
byte 1 117
byte 1 109
byte 1 92
byte 1 0
align 1
LABELV $194
byte 1 60
byte 1 78
byte 1 85
byte 1 76
byte 1 76
byte 1 62
byte 1 0
align 1
LABELV $189
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
LABELV $183
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
LABELV $180
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
LABELV $165
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
LABELV $164
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
LABELV $157
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $156
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
byte 1 0
align 1
LABELV $154
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
LABELV $150
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $148
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 46
byte 1 0
align 1
LABELV $142
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $141
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
LABELV $138
byte 1 109
byte 1 97
byte 1 112
byte 1 115
byte 1 47
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $135
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
LABELV $132
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
LABELV $129
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
LABELV $126
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
LABELV $123
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
LABELV $122
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
