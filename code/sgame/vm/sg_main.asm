export vmMain
code
proc vmMain 16 8
file "../sg_main.c"
line 17
;1:#include "../engine/n_shared.h"
;2:#include "sg_local.h"
;3:
;4:void SG_Init( void );
;5:void SG_Shutdown( void );
;6:int32_t SG_RunLoop( int32_t msec );
;7:int32_t SG_DrawFrame( void );
;8:
;9:/*
;10:vmMain
;11:
;12:this is the only way control passes into the module.
;13:this must be the very first function compiled into the .qvm file
;14:*/
;15:int32_t vmMain( int32_t command, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, int32_t arg6, int32_t arg7,
;16:    int32_t arg8, int32_t arg9, int32_t arg10 )
;17:{
line 18
;18:    switch ( command ) {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 0
LTI4 $84
ADDRLP4 0
INDIRI4
CNSTI4 12
GTI4 $84
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $95
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $95
address $85
address $86
address $92
address $89
address $84
address $93
address $94
address $84
address $84
address $90
address $91
address $91
address $87
code
LABELV $85
line 20
;19:    case SGAME_INIT:
;20:        SG_Init();
ADDRGP4 SG_Init
CALLV
pop
line 21
;21:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $86
line 23
;22:    case SGAME_SHUTDOWN:
;23:        SG_Shutdown();
ADDRGP4 SG_Shutdown
CALLV
pop
line 24
;24:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $87
line 26
;25:    case SGAME_GET_STATE:
;26:        return sg.state;
ADDRGP4 sg+52
INDIRI4
RETI4
ADDRGP4 $82
JUMPV
LABELV $89
line 28
;27:    case SGAME_ENDLEVEL:
;28:        return SG_EndLevel();
ADDRLP4 4
ADDRGP4 SG_EndLevel
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
RETI4
ADDRGP4 $82
JUMPV
LABELV $90
line 31
;29:    case SGAME_SEND_USER_CMD:
;30:
;31:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $91
line 34
;32:    case SGAME_EVENT_HANDLING:
;33:    case SGAME_EVENT_NONE:
;34:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $92
line 36
;35:    case SGAME_LOADLEVEL:
;36:        return SG_InitLevel( arg0 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRLP4 8
ADDRGP4 SG_InitLevel
CALLI4
ASGNI4
ADDRLP4 8
INDIRI4
RETI4
ADDRGP4 $82
JUMPV
LABELV $93
line 38
;37:    case SGAME_CONSOLE_COMMAND:
;38:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $94
line 40
;39:    case SGAME_RUNTIC:
;40:        return SG_RunLoop( arg0 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRLP4 12
ADDRGP4 SG_RunLoop
CALLI4
ASGNI4
ADDRLP4 12
INDIRI4
RETI4
ADDRGP4 $82
JUMPV
line 42
;41:    default:
;42:        break;
LABELV $84
line 43
;43:    };
line 45
;44:
;45:    SG_Error( "vmMain: unrecognized command %i", command );
ADDRGP4 $96
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 46
;46:    return -1;
CNSTI4 -1
RETI4
LABELV $82
endproc vmMain 16 8
data
export dirvectors
align 4
LABELV dirvectors
byte 4 3212836864
byte 4 3212836864
byte 4 0
byte 4 0
byte 4 3212836864
byte 4 0
byte 4 1065353216
byte 4 3212836864
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 1065353216
byte 4 0
byte 4 0
byte 4 1065353216
byte 4 0
byte 4 3212836864
byte 4 1065353216
byte 4 0
byte 4 3212836864
byte 4 0
byte 4 0
export inversedirs
align 4
LABELV inversedirs
byte 4 3
byte 4 4
byte 4 5
byte 4 6
byte 4 7
byte 4 0
byte 4 1
byte 4 2
align 4
LABELV cvarTable
address $98
address $99
address sg_debugPrint
byte 4 32
address $100
address $101
address sg_paused
byte 4 288
address $102
address $103
address pm_airAccel
byte 4 33
address $104
address $105
address pm_waterAccel
byte 4 33
address $106
address $107
address pm_baseAccel
byte 4 33
address $108
address $109
address pm_baseSpeed
byte 4 33
address $110
address $99
address sg_mouseInvert
byte 4 33
address $111
address $99
address sg_mouseAcceleration
byte 4 33
address $112
address $101
address sg_printLevelStats
byte 4 288
address $113
address $114
address sg_decalDetail
byte 4 33
address $115
address $99
address sg_gibs
byte 4 33
address $116
address $117
address sg_levelInfoFile
byte 4 48
address $118
address $119
address sg_savename
byte 4 288
address $120
address $99
address sg_numSaves
byte 4 288
align 4
LABELV cvarTableSize
byte 4 14
code
proc SG_RegisterCvars 12 16
line 124
;47:}
;48:
;49:sgGlobals_t sg;
;50:
;51:spritesheet_t sprites_thenomad;
;52:spritesheet_t sprites_grunt;
;53:spritesheet_t sprites_shotty;
;54:
;55:const vec3_t dirvectors[NUMDIRS] = {
;56:    { -1.0f, -1.0f, 0.0f },
;57:    {  0.0f, -1.0f, 0.0f },
;58:    {  1.0f, -1.0f, 0.0f },
;59:    {  1.0f,  0.0f, 0.0f },
;60:    {  1.0f,  1.0f, 0.0f },
;61:    {  0.0f,  1.0f, 0.0f },
;62:    { -1.0f,  1.0f, 0.0f },
;63:    { -1.0f,  0.0f, 0.0f }
;64:};
;65:
;66:const dirtype_t inversedirs[NUMDIRS] = {
;67:    DIR_SOUTH_EAST,
;68:    DIR_SOUTH,
;69:    DIR_SOUTH_WEST,
;70:    DIR_WEST,
;71:    DIR_NORTH_WEST,
;72:    DIR_NORTH,
;73:    DIR_NORTH_EAST,
;74:    DIR_EAST
;75:};
;76:
;77:vmCvar_t sg_debugPrint;
;78:vmCvar_t sg_paused;
;79:vmCvar_t sg_mouseInvert;
;80:vmCvar_t sg_mouseAcceleration;
;81:vmCvar_t sg_printLevelStats;
;82:vmCvar_t sg_decalDetail;
;83:vmCvar_t sg_gibs;
;84:vmCvar_t sg_levelInfoFile;
;85:vmCvar_t sg_levelIndex;
;86:vmCvar_t sg_levelDataFile;
;87:vmCvar_t sg_savename;
;88:vmCvar_t sg_numSaves;
;89:vmCvar_t pm_waterAccel;
;90:vmCvar_t pm_baseAccel;
;91:vmCvar_t pm_baseSpeed;
;92:vmCvar_t pm_airAccel;
;93:vmCvar_t pm_wallrunAccelVertical;
;94:vmCvar_t pm_wallrunAccelMove;
;95:vmCvar_t pm_wallTime;
;96:
;97:typedef struct {
;98:    const char *name;
;99:    const char *defaultValue;
;100:    vmCvar_t *cvar;
;101:    uint32_t flags;
;102:} cvarTable_t;
;103:
;104:static cvarTable_t cvarTable[] = {
;105:    { "sg_debugPrint",                  "0",            &sg_debugPrint,             CVAR_LATCH },
;106:    { "g_paused",                       "1",            &sg_paused,                 CVAR_LATCH | CVAR_TEMP },
;107:    { "pm_airAccel",                    "1.5",          &pm_airAccel,               CVAR_LATCH | CVAR_SAVE },
;108:    { "pm_waterAccel",                  "0.5",          &pm_waterAccel,             CVAR_LATCH | CVAR_SAVE },
;109:    { "pm_baseAccel",                   "1.2",          &pm_baseAccel,              CVAR_LATCH | CVAR_SAVE },
;110:    { "pm_baseSpeed",                   "1.0",          &pm_baseSpeed,              CVAR_LATCH | CVAR_SAVE },
;111:    { "g_mouseInvert",                  "0",            &sg_mouseInvert,            CVAR_LATCH | CVAR_SAVE },
;112:    { "g_mouseAcceleration",            "0",            &sg_mouseAcceleration,      CVAR_LATCH | CVAR_SAVE },
;113:    { "sg_printLevelStats",             "1",            &sg_printLevelStats,        CVAR_LATCH | CVAR_TEMP },
;114:    { "sg_decalDetail",                 "3",            &sg_decalDetail,            CVAR_LATCH | CVAR_SAVE },
;115:    { "sg_gibs",                        "0",            &sg_gibs,                   CVAR_LATCH | CVAR_SAVE },
;116:    { "sg_levelInfoFile",               "levels.txt",   &sg_levelInfoFile,          CVAR_INIT | CVAR_LATCH },
;117:    { "sg_savename",                    "savedata.ngd", &sg_savename,               CVAR_LATCH | CVAR_TEMP },
;118:    { "sg_numSaves",                    "0",            &sg_numSaves,               CVAR_LATCH | CVAR_TEMP },
;119:};
;120:
;121:static uint32_t cvarTableSize = arraylen(cvarTable);
;122:
;123:static void SG_RegisterCvars( void )
;124:{
line 128
;125:    uint32_t i;
;126:    cvarTable_t *cv;
;127:
;128:    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
ADDRLP4 4
CNSTU4 0
ASGNU4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $125
JUMPV
LABELV $122
line 129
;129:       Cvar_Register( cv->cvar, cv->name, cv->defaultValue, cv->flags );
ADDRLP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 12
ADDP4
INDIRU4
ARGU4
ADDRGP4 Cvar_Register
CALLV
pop
line 130
;130:    }
LABELV $123
line 128
ADDRLP4 4
ADDRLP4 4
INDIRU4
CNSTU4 1
ADDU4
ASGNU4
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
ASGNP4
LABELV $125
ADDRLP4 4
INDIRU4
ADDRGP4 cvarTableSize
INDIRU4
LTU4 $122
line 131
;131:}
LABELV $121
endproc SG_RegisterCvars 12 16
export SG_UpdateCvars
proc SG_UpdateCvars 8 4
line 134
;132:
;133:void SG_UpdateCvars( void )
;134:{
line 138
;135:    uint32_t i;
;136:    cvarTable_t *cv;
;137:
;138:    Cvar_Update( &sg_paused );
ADDRGP4 sg_paused
ARGP4
ADDRGP4 Cvar_Update
CALLV
pop
line 139
;139:}
LABELV $126
endproc SG_UpdateCvars 8 4
export G_Printf
proc G_Printf 4108 12
line 142
;140:
;141:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
;142:{
line 147
;143:    va_list argptr;
;144:    char msg[4096];
;145:    int32_t length;
;146:
;147:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 148
;148:    length = vsprintf(msg, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 149
;149:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 151
;150:
;151:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 152
;152:}
LABELV $127
endproc G_Printf 4108 12
export G_Error
proc G_Error 4108 12
line 155
;153:
;154:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
;155:{
line 160
;156:    va_list argptr;
;157:    char msg[4096];
;158:    int32_t length;
;159:
;160:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 161
;161:    length = vsprintf(msg, err, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 162
;162:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 164
;163:
;164:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 165
;165:}
LABELV $129
endproc G_Error 4108 12
export SG_Printf
proc SG_Printf 4108 12
line 168
;166:
;167:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
;168:{
line 173
;169:    va_list argptr;
;170:    char msg[4096];
;171:    int32_t length;
;172:
;173:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 174
;174:    length = vsprintf(msg, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 175
;175:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 177
;176:
;177:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $133
line 178
;178:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $135
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 179
;179:    }
LABELV $133
line 181
;180:
;181:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 182
;182:}
LABELV $131
endproc SG_Printf 4108 12
export SG_Error
proc SG_Error 4108 12
line 185
;183:
;184:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error(const char *err, ...)
;185:{
line 190
;186:    va_list argptr;
;187:    char msg[4096];
;188:    int32_t length;
;189:
;190:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 191
;191:    length = vsprintf(msg, err, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 192
;192:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 194
;193:
;194:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $138
line 195
;195:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $135
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 196
;196:    }
LABELV $138
line 198
;197:
;198:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 199
;199:}
LABELV $136
endproc SG_Error 4108 12
export N_Error
proc N_Error 4108 12
line 202
;200:
;201:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
;202:{
line 207
;203:    va_list argptr;
;204:    char msg[4096];
;205:    int32_t length;
;206:
;207:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 208
;208:    length = vsprintf(msg, err, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 209
;209:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 211
;210:
;211:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $142
line 212
;212:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $135
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 213
;213:    }
LABELV $142
line 215
;214:
;215:    SG_Error("%s", msg);
ADDRGP4 $144
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 216
;216:}
LABELV $140
endproc N_Error 4108 12
export Con_Printf
proc Con_Printf 4108 12
line 222
;217:
;218://#ifndef SGAME_HARD_LINKED
;219:// this is only here so the functions in n_shared.c and bg_*.c can link
;220:
;221:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
;222:{
line 227
;223:    va_list argptr;
;224:    char msg[4096];
;225:    int32_t length;
;226:
;227:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 228
;228:    length = vsprintf(msg, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 4104
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 4100
ADDRLP4 4104
INDIRI4
ASGNI4
line 229
;229:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 231
;230:
;231:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $147
line 232
;232:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $135
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 233
;233:    }
LABELV $147
line 235
;234:
;235:    SG_Printf("%s", msg);
ADDRGP4 $144
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 236
;236:}
LABELV $145
endproc Con_Printf 4108 12
export SG_RunLoop
proc SG_RunLoop 32 0
line 241
;237:
;238://#endif
;239:
;240:int32_t SG_RunLoop( int32_t levelTime )
;241:{
line 247
;242:    int32_t i;
;243:    int32_t start, end;
;244:    int32_t msec;
;245:    sgentity_t *ent;
;246:
;247:    if ( sg.state == SG_INACTIVE ) {
ADDRGP4 sg+52
INDIRI4
CNSTI4 0
NEI4 $150
line 248
;248:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $149
JUMPV
LABELV $150
line 252
;249:    }
;250:
;251:    // get any cvar changes
;252:    SG_UpdateCvars();
ADDRGP4 SG_UpdateCvars
CALLV
pop
line 255
;253:
;254:    // even if the game is paused, we still render everything in the background
;255:    if ( sg.state == SG_SHOW_LEVEL_STATS ) {
ADDRGP4 sg+52
INDIRI4
CNSTI4 2
NEI4 $153
line 256
;256:        SG_DrawLevelStats();
ADDRGP4 SG_DrawLevelStats
CALLV
pop
line 257
;257:        return 1; // we don't draw the level if we're ending it
CNSTI4 1
RETI4
ADDRGP4 $149
JUMPV
LABELV $153
line 260
;258:    }
;259:
;260:    SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 262
;261:
;262:    if ( sg_paused.i ) {
ADDRGP4 sg_paused+260
INDIRI4
CNSTI4 0
EQI4 $156
line 263
;263:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $149
JUMPV
LABELV $156
line 266
;264:    }
;265:
;266:    sg.framenum++;
ADDRLP4 20
ADDRGP4 sg+76
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 267
;267:    sg.previousTime = sg.framenum;
ADDRGP4 sg+68
ADDRGP4 sg+76
INDIRI4
ASGNI4
line 268
;268:    sg.levelTime = levelTime;
ADDRGP4 sg+72
ADDRFP4 0
INDIRI4
ASGNI4
line 269
;269:    msec = sg.levelTime - sg.previousTime;
ADDRLP4 16
ADDRGP4 sg+72
INDIRI4
ADDRGP4 sg+68
INDIRI4
SUBI4
ASGNI4
line 274
;270:
;271:    //
;272:    // go through all allocated entities
;273:    //
;274:    start = trap_Milliseconds();
ADDRLP4 24
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 24
INDIRI4
ASGNI4
line 275
;275:    ent = &sg_entities[0];
ADDRLP4 4
ADDRGP4 sg_entities
ASGNP4
line 276
;276:    for ( i = 0; i < sg.numEntities; i++) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $168
JUMPV
LABELV $165
line 277
;277:        if ( !ent->health ) {
ADDRLP4 4
INDIRP4
CNSTI4 92
ADDP4
INDIRI4
CNSTI4 0
NEI4 $170
line 278
;278:            continue;
LABELV $170
line 282
;279:        }
;280:
;281:        
;282:    }
LABELV $166
line 276
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $168
ADDRLP4 0
INDIRI4
CVIU4 4
ADDRGP4 sg+60
INDIRU4
LTU4 $165
line 283
;283:    end = trap_Milliseconds();
ADDRLP4 28
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 28
INDIRI4
ASGNI4
line 285
;284:
;285:    return 1;
CNSTI4 1
RETI4
LABELV $149
endproc SG_RunLoop 32 0
proc SG_LoadMedia 40 4
line 290
;286:}
;287:
;288:
;289:static void SG_LoadMedia( void )
;290:{
line 291
;291:    G_Printf( "Loading sgame sfx...\n" );
ADDRGP4 $173
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 293
;292:
;293:    sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" );
ADDRGP4 $175
ARGP4
ADDRLP4 0
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+12
ADDRLP4 0
INDIRI4
ASGNI4
line 294
;294:    sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" );
ADDRGP4 $177
ARGP4
ADDRLP4 4
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+16
ADDRLP4 4
INDIRI4
ASGNI4
line 295
;295:    sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" ); 
ADDRGP4 $179
ARGP4
ADDRLP4 8
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+20
ADDRLP4 8
INDIRI4
ASGNI4
line 296
;296:    sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" );
ADDRGP4 $180
ARGP4
ADDRLP4 12
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg
ADDRLP4 12
INDIRI4
ASGNI4
line 297
;297:    sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" );
ADDRGP4 $182
ARGP4
ADDRLP4 16
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+4
ADDRLP4 16
INDIRI4
ASGNI4
line 298
;298:    sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" );
ADDRGP4 $184
ARGP4
ADDRLP4 20
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+8
ADDRLP4 20
INDIRI4
ASGNI4
line 299
;299:    sg.media.revolver_fire = trap_Snd_RegisterSfx( "sfx/weapons/revolver_fire.wav" );
ADDRGP4 $186
ARGP4
ADDRLP4 24
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+28
ADDRLP4 24
INDIRI4
ASGNI4
line 300
;300:    sg.media.revolver_rld = trap_Snd_RegisterSfx( "sfx/weapons/revolver_rld.wav" );
ADDRGP4 $188
ARGP4
ADDRLP4 28
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+32
ADDRLP4 28
INDIRI4
ASGNI4
line 302
;301:
;302:    G_Printf( "Finished loading sfx.\n" );
ADDRGP4 $189
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 304
;303:
;304:    G_Printf( "Loading sgame sprites...\n" );
ADDRGP4 $190
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 306
;305:
;306:    sg.media.raio_shader = RE_RegisterShader( "textures/sprites/glnomad_raio_base.png" );
ADDRGP4 $192
ARGP4
ADDRLP4 32
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+36
ADDRLP4 32
INDIRI4
ASGNI4
line 307
;307:    sg.media.grunt_shader = RE_RegisterShader( "textures/sprites/glnomad_grunt.png" );
ADDRGP4 $194
ARGP4
ADDRLP4 36
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+40
ADDRLP4 36
INDIRI4
ASGNI4
line 309
;308:
;309:    G_Printf( "Finished loading sprites.\n" );
ADDRGP4 $195
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 310
;310:}
LABELV $172
endproc SG_LoadMedia 40 4
export SG_Init
proc SG_Init 0 12
line 313
;311:
;312:void SG_Init( void )
;313:{
line 315
;314:    // clear sgame state
;315:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4307564
ARGU4
ADDRGP4 memset
CALLP4
pop
line 318
;316:    
;317:    // cache redundant calculations
;318:    Sys_GetGPUConfig( &sg.gpuConfig );
ADDRGP4 sg+73964
ARGP4
ADDRGP4 Sys_GetGPUConfig
CALLV
pop
line 321
;319:
;320:    // for 1024x768 virtualized screen
;321:	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
ADDRGP4 sg+86312
CNSTF4 984263339
ADDRGP4 sg+73964+12312
INDIRI4
CVIF4 4
MULF4
ASGNF4
line 322
;322:	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
CNSTI4 768
ADDRGP4 sg+73964+12308
INDIRI4
MULI4
ADDRGP4 sg+73964+12312
INDIRI4
CNSTI4 10
LSHI4
LEI4 $201
line 324
;323:		// wide screen
;324:		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
ADDRGP4 sg+86316
CNSTF4 1056964608
ADDRGP4 sg+73964+12308
INDIRI4
CVIF4 4
CNSTF4 1068149419
ADDRGP4 sg+73964+12312
INDIRI4
CVIF4 4
MULF4
SUBF4
MULF4
ASGNF4
line 325
;325:	}
ADDRGP4 $202
JUMPV
LABELV $201
line 326
;326:	else {
line 328
;327:		// no wide screen
;328:		sg.bias = 0;
ADDRGP4 sg+86316
CNSTF4 0
ASGNF4
line 329
;329:	}
LABELV $202
line 332
;330:
;331:    // register sgame cvars
;332:    SG_RegisterCvars();
ADDRGP4 SG_RegisterCvars
CALLV
pop
line 335
;333:
;334:    // load assets/resources
;335:    SG_LoadMedia();
ADDRGP4 SG_LoadMedia
CALLV
pop
line 337
;336:
;337:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 339
;338:
;339:    sg.state = SG_INACTIVE;
ADDRGP4 sg+52
CNSTI4 0
ASGNI4
line 340
;340:}
LABELV $196
endproc SG_Init 0 12
export SG_Shutdown
proc SG_Shutdown 4 12
line 343
;341:
;342:void SG_Shutdown( void )
;343:{
line 344
;344:    G_Printf( "Shutting down sgame...\n" );
ADDRGP4 $215
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 346
;345:
;346:    trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_SGAME );
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLU4
ASGNU4
ADDRLP4 0
INDIRU4
CNSTU4 4294959103
BANDU4
ARGU4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 348
;347:
;348:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4307564
ARGU4
ADDRGP4 memset
CALLP4
pop
line 350
;349:
;350:    sg.state = SG_INACTIVE;
ADDRGP4 sg+52
CNSTI4 0
ASGNI4
line 351
;351:}
LABELV $214
endproc SG_Shutdown 4 12
export trap_FS_Printf
proc trap_FS_Printf 8200 12
line 354
;352:
;353:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... )
;354:{
line 358
;355:    va_list argptr;
;356:    char msg[MAXPRINTMSG];
;357:
;358:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 359
;359:    vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 vsprintf
CALLI4
pop
line 360
;360:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 362
;361:
;362:    trap_FS_Write( msg, strlen(msg), f );
ADDRLP4 4
ARGP4
ADDRLP4 8196
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 4
ARGP4
ADDRLP4 8196
INDIRU4
ARGU4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 trap_FS_Write
CALLU4
pop
line 363
;363:}
LABELV $217
endproc trap_FS_Printf 8200 12
import Cvar_VariableStringBuffer
import Cvar_Set
import Cvar_Update
import Cvar_Register
import trap_FS_FileTell
import trap_FS_FileSeek
import trap_FS_GetFileList
import trap_FS_Read
import trap_FS_Write
import trap_FS_FClose
import trap_FS_FOpenWrite
import trap_FS_FOpenFile
import Sys_GetGPUConfig
import RE_AddPolyToScene
import RE_RenderScene
import RE_ClearScene
import RE_LoadWorldMap
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
import G_LoadMap
import trap_MemoryRemaining
import trap_RemoveCommand
import trap_AddCommand
import trap_SendConsoleCommand
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
import SG_MemInit
import SG_MemAlloc
import String_Alloc
import SG_SpawnMobOnMap
import SG_SpawnMob
import SG_AddArchiveHandle
import SG_LoadGame
import SG_SaveGame
import Ent_SetState
import SG_InitEntities
import Ent_BuildBounds
import SG_BuildBounds
import SG_FreeEntity
import SG_AllocEntity
import Ent_RunTic
import Ent_CheckEntityCollision
import Ent_CheckWallCollision
import SG_DrawLevelStats
import SG_DrawAbortMission
import Lvl_AddKillEntity
import SG_EndLevel
import SG_InitLevel
import SG_GenerateSpriteSheetTexCoords
import SG_DrawFrame
bss
export pm_wallTime
align 4
LABELV pm_wallTime
skip 276
export pm_wallrunAccelMove
align 4
LABELV pm_wallrunAccelMove
skip 276
export pm_wallrunAccelVertical
align 4
LABELV pm_wallrunAccelVertical
skip 276
export pm_airAccel
align 4
LABELV pm_airAccel
skip 276
export pm_baseSpeed
align 4
LABELV pm_baseSpeed
skip 276
export pm_baseAccel
align 4
LABELV pm_baseAccel
skip 276
export pm_waterAccel
align 4
LABELV pm_waterAccel
skip 276
export sg_numSaves
align 4
LABELV sg_numSaves
skip 276
export sg_savename
align 4
LABELV sg_savename
skip 276
export sg_levelDataFile
align 4
LABELV sg_levelDataFile
skip 276
export sg_levelIndex
align 4
LABELV sg_levelIndex
skip 276
export sg_levelInfoFile
align 4
LABELV sg_levelInfoFile
skip 276
export sg_gibs
align 4
LABELV sg_gibs
skip 276
export sg_decalDetail
align 4
LABELV sg_decalDetail
skip 276
export sg_printLevelStats
align 4
LABELV sg_printLevelStats
skip 276
export sg_mouseAcceleration
align 4
LABELV sg_mouseAcceleration
skip 276
export sg_mouseInvert
align 4
LABELV sg_mouseInvert
skip 276
export sg_paused
align 4
LABELV sg_paused
skip 276
export sg_debugPrint
align 4
LABELV sg_debugPrint
skip 276
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
export sg
align 4
LABELV sg
skip 4307564
export sprites_shotty
align 4
LABELV sprites_shotty
skip 8
export sprites_grunt
align 4
LABELV sprites_grunt
skip 8
export sprites_thenomad
align 4
LABELV sprites_thenomad
skip 8
import sg_entities
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
import Cmd_CompleteArgument
import Cmd_CommandCompletion
import va
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
import PerpendicularVector
import AngleVectors
import MatrixMultiply
import COM_SkipPath
import MakeNormalVectors
import RotateAroundDirection
import RotatePointAroundVector
import ProjectPointOnPlane
import PlaneFromPoints
import AngleDelta
import AngleNormalize180
import AngleNormalize360
import AnglesSubtract
import AngleSubtract
import LerpAngle
import AngleMod
import BoundsIntersectPoint
import BoundsIntersectSphere
import BoundsIntersect
import disBetweenOBJ
import AxisCopy
import AxisClear
import AnglesToAxis
import vectoangles
import N_crandom
import N_random
import N_rand
import N_fabs
import N_acos
import N_log2
import VectorRotate
import Vector4Scale
import VectorNormalize2
import VectorNormalize
import CrossProduct
import VectorInverse
import VectorNormalizeFast
import DistanceSquared
import Distance
import VectorLengthSquared
import VectorLength
import VectorCompare
import AddPointToBounds
import ClearBounds
import RadiusFromBounds
import NormalizeColor
import ColorBytes4
import ColorBytes3
import _VectorMA
import _VectorScale
import _VectorCopy
import _VectorAdd
import _VectorSubtract
import _DotProduct
import ByteToDir
import DirToByte
import ClampShort
import ClampCharMove
import ClampChar
import N_exp2f
import N_log2f
import Q_rsqrt
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
import mat4_identity
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
LABELV $215
byte 1 83
byte 1 104
byte 1 117
byte 1 116
byte 1 116
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 100
byte 1 111
byte 1 119
byte 1 110
byte 1 32
byte 1 115
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $195
byte 1 70
byte 1 105
byte 1 110
byte 1 105
byte 1 115
byte 1 104
byte 1 101
byte 1 100
byte 1 32
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 115
byte 1 112
byte 1 114
byte 1 105
byte 1 116
byte 1 101
byte 1 115
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $194
byte 1 116
byte 1 101
byte 1 120
byte 1 116
byte 1 117
byte 1 114
byte 1 101
byte 1 115
byte 1 47
byte 1 115
byte 1 112
byte 1 114
byte 1 105
byte 1 116
byte 1 101
byte 1 115
byte 1 47
byte 1 103
byte 1 108
byte 1 110
byte 1 111
byte 1 109
byte 1 97
byte 1 100
byte 1 95
byte 1 103
byte 1 114
byte 1 117
byte 1 110
byte 1 116
byte 1 46
byte 1 112
byte 1 110
byte 1 103
byte 1 0
align 1
LABELV $192
byte 1 116
byte 1 101
byte 1 120
byte 1 116
byte 1 117
byte 1 114
byte 1 101
byte 1 115
byte 1 47
byte 1 115
byte 1 112
byte 1 114
byte 1 105
byte 1 116
byte 1 101
byte 1 115
byte 1 47
byte 1 103
byte 1 108
byte 1 110
byte 1 111
byte 1 109
byte 1 97
byte 1 100
byte 1 95
byte 1 114
byte 1 97
byte 1 105
byte 1 111
byte 1 95
byte 1 98
byte 1 97
byte 1 115
byte 1 101
byte 1 46
byte 1 112
byte 1 110
byte 1 103
byte 1 0
align 1
LABELV $190
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 115
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 32
byte 1 115
byte 1 112
byte 1 114
byte 1 105
byte 1 116
byte 1 101
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $189
byte 1 70
byte 1 105
byte 1 110
byte 1 105
byte 1 115
byte 1 104
byte 1 101
byte 1 100
byte 1 32
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 115
byte 1 102
byte 1 120
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $188
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 119
byte 1 101
byte 1 97
byte 1 112
byte 1 111
byte 1 110
byte 1 115
byte 1 47
byte 1 114
byte 1 101
byte 1 118
byte 1 111
byte 1 108
byte 1 118
byte 1 101
byte 1 114
byte 1 95
byte 1 114
byte 1 108
byte 1 100
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $186
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 119
byte 1 101
byte 1 97
byte 1 112
byte 1 111
byte 1 110
byte 1 115
byte 1 47
byte 1 114
byte 1 101
byte 1 118
byte 1 111
byte 1 108
byte 1 118
byte 1 101
byte 1 114
byte 1 95
byte 1 102
byte 1 105
byte 1 114
byte 1 101
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $184
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 112
byte 1 97
byte 1 105
byte 1 110
byte 1 50
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $182
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 112
byte 1 97
byte 1 105
byte 1 110
byte 1 49
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $180
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 112
byte 1 97
byte 1 105
byte 1 110
byte 1 48
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $179
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 100
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 51
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $177
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 100
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 50
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $175
byte 1 115
byte 1 102
byte 1 120
byte 1 47
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 47
byte 1 100
byte 1 101
byte 1 97
byte 1 116
byte 1 104
byte 1 49
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $173
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 115
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 32
byte 1 115
byte 1 102
byte 1 120
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $144
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $135
byte 1 83
byte 1 71
byte 1 95
byte 1 80
byte 1 114
byte 1 105
byte 1 110
byte 1 116
byte 1 102
byte 1 58
byte 1 32
byte 1 98
byte 1 117
byte 1 102
byte 1 102
byte 1 101
byte 1 114
byte 1 32
byte 1 111
byte 1 118
byte 1 101
byte 1 114
byte 1 102
byte 1 108
byte 1 111
byte 1 119
byte 1 0
align 1
LABELV $120
byte 1 115
byte 1 103
byte 1 95
byte 1 110
byte 1 117
byte 1 109
byte 1 83
byte 1 97
byte 1 118
byte 1 101
byte 1 115
byte 1 0
align 1
LABELV $119
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 100
byte 1 97
byte 1 116
byte 1 97
byte 1 46
byte 1 110
byte 1 103
byte 1 100
byte 1 0
align 1
LABELV $118
byte 1 115
byte 1 103
byte 1 95
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $117
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
LABELV $116
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
LABELV $115
byte 1 115
byte 1 103
byte 1 95
byte 1 103
byte 1 105
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $114
byte 1 51
byte 1 0
align 1
LABELV $113
byte 1 115
byte 1 103
byte 1 95
byte 1 100
byte 1 101
byte 1 99
byte 1 97
byte 1 108
byte 1 68
byte 1 101
byte 1 116
byte 1 97
byte 1 105
byte 1 108
byte 1 0
align 1
LABELV $112
byte 1 115
byte 1 103
byte 1 95
byte 1 112
byte 1 114
byte 1 105
byte 1 110
byte 1 116
byte 1 76
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
LABELV $111
byte 1 103
byte 1 95
byte 1 109
byte 1 111
byte 1 117
byte 1 115
byte 1 101
byte 1 65
byte 1 99
byte 1 99
byte 1 101
byte 1 108
byte 1 101
byte 1 114
byte 1 97
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $110
byte 1 103
byte 1 95
byte 1 109
byte 1 111
byte 1 117
byte 1 115
byte 1 101
byte 1 73
byte 1 110
byte 1 118
byte 1 101
byte 1 114
byte 1 116
byte 1 0
align 1
LABELV $109
byte 1 49
byte 1 46
byte 1 48
byte 1 0
align 1
LABELV $108
byte 1 112
byte 1 109
byte 1 95
byte 1 98
byte 1 97
byte 1 115
byte 1 101
byte 1 83
byte 1 112
byte 1 101
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $107
byte 1 49
byte 1 46
byte 1 50
byte 1 0
align 1
LABELV $106
byte 1 112
byte 1 109
byte 1 95
byte 1 98
byte 1 97
byte 1 115
byte 1 101
byte 1 65
byte 1 99
byte 1 99
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $105
byte 1 48
byte 1 46
byte 1 53
byte 1 0
align 1
LABELV $104
byte 1 112
byte 1 109
byte 1 95
byte 1 119
byte 1 97
byte 1 116
byte 1 101
byte 1 114
byte 1 65
byte 1 99
byte 1 99
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $103
byte 1 49
byte 1 46
byte 1 53
byte 1 0
align 1
LABELV $102
byte 1 112
byte 1 109
byte 1 95
byte 1 97
byte 1 105
byte 1 114
byte 1 65
byte 1 99
byte 1 99
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $101
byte 1 49
byte 1 0
align 1
LABELV $100
byte 1 103
byte 1 95
byte 1 112
byte 1 97
byte 1 117
byte 1 115
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $99
byte 1 48
byte 1 0
align 1
LABELV $98
byte 1 115
byte 1 103
byte 1 95
byte 1 100
byte 1 101
byte 1 98
byte 1 117
byte 1 103
byte 1 80
byte 1 114
byte 1 105
byte 1 110
byte 1 116
byte 1 0
align 1
LABELV $96
byte 1 118
byte 1 109
byte 1 77
byte 1 97
byte 1 105
byte 1 110
byte 1 58
byte 1 32
byte 1 117
byte 1 110
byte 1 114
byte 1 101
byte 1 99
byte 1 111
byte 1 103
byte 1 110
byte 1 105
byte 1 122
byte 1 101
byte 1 100
byte 1 32
byte 1 99
byte 1 111
byte 1 109
byte 1 109
byte 1 97
byte 1 110
byte 1 100
byte 1 32
byte 1 37
byte 1 105
byte 1 0
