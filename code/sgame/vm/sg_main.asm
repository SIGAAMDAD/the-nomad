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
;6:int SG_RunLoop( int msec );
;7:int SG_DrawFrame( void );
;8:
;9:/*
;10:vmMain
;11:
;12:this is the only way control passes into the module.
;13:this must be the very first function compiled into the .qvm file
;14:*/
;15:int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
;16:    int arg8, int arg9, int arg10 )
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
CNSTI4 11
GTI4 $84
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $94
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $94
address $85
address $86
address $91
address $89
address $84
address $92
address $93
address $84
address $84
address $90
address $90
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
ADDRGP4 sg+48
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
;29:    case SGAME_EVENT_HANDLING:
;30:    case SGAME_EVENT_NONE:
;31:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $91
line 33
;32:    case SGAME_LOADLEVEL:
;33:        return SG_InitLevel( arg0 );
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
LABELV $92
line 35
;34:    case SGAME_CONSOLE_COMMAND:
;35:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $93
line 37
;36:    case SGAME_RUNTIC:
;37:        return SG_RunLoop( arg0 );
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
line 39
;38:    default:
;39:        break;
LABELV $84
line 40
;40:    };
line 42
;41:
;42:    SG_Error( "vmMain: unrecognized command %i", command );
ADDRGP4 $95
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 43
;43:    return -1;
CNSTI4 -1
RETI4
LABELV $82
endproc vmMain 16 8
data
align 4
LABELV cvarTable
address $97
address $98
address sg_debugPrint
byte 4 32
address $99
address $100
address sg_paused
byte 4 288
address $101
address $102
address sg_pmAirAcceleration
byte 4 33
address $103
address $104
address sg_pmWaterAcceleration
byte 4 33
address $105
address $106
address sg_pmBaseAcceleration
byte 4 33
address $107
address $108
address sg_pmBaseSpeed
byte 4 33
address $109
address $98
address sg_mouseInvert
byte 4 33
address $110
address $98
address sg_mouseAcceleration
byte 4 33
address $111
address $100
address sg_printLevelStats
byte 4 288
address $112
address $113
address sg_decalDetail
byte 4 33
address $114
address $98
address sg_gibs
byte 4 33
address $115
address $116
address sg_levelInfoFile
byte 4 48
address $117
address $118
address sg_savename
byte 4 288
address $119
address $98
address sg_numSaves
byte 4 288
align 4
LABELV cvarTableSize
byte 4 14
code
proc SG_RegisterCvars 12 16
line 93
;44:}
;45:
;46:sgGlobals_t sg;
;47:
;48:vmCvar_t sg_debugPrint;
;49:vmCvar_t sg_paused;
;50:vmCvar_t sg_pmAirAcceleration;
;51:vmCvar_t sg_pmWaterAcceleration;
;52:vmCvar_t sg_pmBaseAcceleration;
;53:vmCvar_t sg_pmBaseSpeed;
;54:vmCvar_t sg_mouseInvert;
;55:vmCvar_t sg_mouseAcceleration;
;56:vmCvar_t sg_printLevelStats;
;57:vmCvar_t sg_decalDetail;
;58:vmCvar_t sg_gibs;
;59:vmCvar_t sg_levelInfoFile;
;60:vmCvar_t sg_levelIndex;
;61:vmCvar_t sg_levelDataFile;
;62:vmCvar_t sg_savename;
;63:vmCvar_t sg_numSaves;
;64:
;65:
;66:typedef struct {
;67:    const char *name;
;68:    const char *defaultValue;
;69:    vmCvar_t *cvar;
;70:    uint32_t flags;
;71:} cvarTable_t;
;72:
;73:static cvarTable_t cvarTable[] = {
;74:    { "sg_debugPrint",                  "0",            &sg_debugPrint,             CVAR_LATCH },
;75:    { "g_paused",                       "1",            &sg_paused,                 CVAR_LATCH | CVAR_TEMP },
;76:    { "sg_pmAirAcceleration",           "1.5",          &sg_pmAirAcceleration,      CVAR_LATCH | CVAR_SAVE },
;77:    { "sg_pmWaterAcceleratino",         "0.5",          &sg_pmWaterAcceleration,    CVAR_LATCH | CVAR_SAVE },
;78:    { "sg_pmBaseAcceleration",          "1.2",          &sg_pmBaseAcceleration,     CVAR_LATCH | CVAR_SAVE },
;79:    { "sg_pmBaseSpeed",                 "1.0",          &sg_pmBaseSpeed,            CVAR_LATCH | CVAR_SAVE },
;80:    { "g_mouseInvert",                  "0",            &sg_mouseInvert,            CVAR_LATCH | CVAR_SAVE },
;81:    { "g_mouseAcceleration",            "0",            &sg_mouseAcceleration,      CVAR_LATCH | CVAR_SAVE },
;82:    { "sg_printLevelStats",             "1",            &sg_printLevelStats,        CVAR_LATCH | CVAR_TEMP },
;83:    { "sg_decalDetail",                 "3",            &sg_decalDetail,            CVAR_LATCH | CVAR_SAVE },
;84:    { "sg_gibs",                        "0",            &sg_gibs,                   CVAR_LATCH | CVAR_SAVE },
;85:    { "sg_levelInfoFile",               "levels.txt",   &sg_levelInfoFile,          CVAR_INIT | CVAR_LATCH },
;86:    { "sg_savename",                    "savedata.ngd", &sg_savename,               CVAR_LATCH | CVAR_TEMP },
;87:    { "sg_numSaves",                    "0",            &sg_numSaves,               CVAR_LATCH | CVAR_TEMP },
;88:};
;89:
;90:static uint32_t cvarTableSize = arraylen(cvarTable);
;91:
;92:static void SG_RegisterCvars( void )
;93:{
line 97
;94:    uint32_t i;
;95:    cvarTable_t *cv;
;96:
;97:    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
ADDRLP4 4
CNSTU4 0
ASGNU4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $124
JUMPV
LABELV $121
line 98
;98:       Cvar_Register( cv->cvar, cv->name, cv->defaultValue, cv->flags );
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
CALLI4
pop
line 99
;99:    }
LABELV $122
line 97
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
LABELV $124
ADDRLP4 4
INDIRU4
ADDRGP4 cvarTableSize
INDIRU4
LTU4 $121
line 100
;100:}
LABELV $120
endproc SG_RegisterCvars 12 16
export SG_UpdateCvars
proc SG_UpdateCvars 8 4
line 103
;101:
;102:void SG_UpdateCvars( void )
;103:{
line 107
;104:    uint32_t i;
;105:    cvarTable_t *cv;
;106:
;107:    Cvar_Update( &sg_paused );
ADDRGP4 sg_paused
ARGP4
ADDRGP4 Cvar_Update
CALLI4
pop
line 108
;108:}
LABELV $125
endproc SG_UpdateCvars 8 4
export G_Printf
proc G_Printf 4108 12
line 111
;109:
;110:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
;111:{
line 116
;112:    va_list argptr;
;113:    char msg[4096];
;114:    int length;
;115:
;116:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 117
;117:    length = vsprintf(msg, fmt, argptr);
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
line 118
;118:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 120
;119:
;120:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLI4
pop
line 121
;121:}
LABELV $126
endproc G_Printf 4108 12
export G_Error
proc G_Error 4108 12
line 124
;122:
;123:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
;124:{
line 129
;125:    va_list argptr;
;126:    char msg[4096];
;127:    int length;
;128:
;129:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 130
;130:    length = vsprintf(msg, err, argptr);
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
line 131
;131:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 133
;132:
;133:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 134
;134:}
LABELV $128
endproc G_Error 4108 12
export SG_Printf
proc SG_Printf 4108 12
line 137
;135:
;136:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
;137:{
line 142
;138:    va_list argptr;
;139:    char msg[4096];
;140:    int length;
;141:
;142:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 143
;143:    length = vsprintf(msg, fmt, argptr);
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
line 144
;144:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 146
;145:
;146:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $132
line 147
;147:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $134
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 148
;148:    }
LABELV $132
line 150
;149:
;150:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLI4
pop
line 151
;151:}
LABELV $130
endproc SG_Printf 4108 12
export SG_Error
proc SG_Error 4108 12
line 154
;152:
;153:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error(const char *err, ...)
;154:{
line 159
;155:    va_list argptr;
;156:    char msg[4096];
;157:    int length;
;158:
;159:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 160
;160:    length = vsprintf(msg, err, argptr);
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
line 161
;161:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 163
;162:
;163:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $137
line 164
;164:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $134
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 165
;165:    }
LABELV $137
line 167
;166:
;167:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 168
;168:}
LABELV $135
endproc SG_Error 4108 12
export N_Error
proc N_Error 4108 12
line 171
;169:
;170:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
;171:{
line 176
;172:    va_list argptr;
;173:    char msg[4096];
;174:    int length;
;175:
;176:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 177
;177:    length = vsprintf(msg, err, argptr);
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
line 178
;178:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 180
;179:
;180:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $141
line 181
;181:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $134
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 182
;182:    }
LABELV $141
line 184
;183:
;184:    SG_Error("%s", msg);
ADDRGP4 $143
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 185
;185:}
LABELV $139
endproc N_Error 4108 12
export Con_Printf
proc Con_Printf 4108 12
line 191
;186:
;187://#ifndef SGAME_HARD_LINKED
;188:// this is only here so the functions in n_shared.c and bg_*.c can link
;189:
;190:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
;191:{
line 196
;192:    va_list argptr;
;193:    char msg[4096];
;194:    int length;
;195:
;196:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 197
;197:    length = vsprintf(msg, fmt, argptr);
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
line 198
;198:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 200
;199:
;200:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $146
line 201
;201:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $134
ARGP4
ADDRGP4 trap_Error
CALLI4
pop
line 202
;202:    }
LABELV $146
line 204
;203:
;204:    SG_Printf("%s", msg);
ADDRGP4 $143
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 205
;205:}
LABELV $144
endproc Con_Printf 4108 12
export SG_RunLoop
proc SG_RunLoop 32 0
line 210
;206:
;207://#endif
;208:
;209:int SG_RunLoop( int levelTime )
;210:{
line 216
;211:    int i;
;212:    int start, end;
;213:    int msec;
;214:    sgentity_t *ent;
;215:
;216:    if ( sg.state == SG_INACTIVE ) {
ADDRGP4 sg+48
INDIRI4
CNSTI4 0
NEI4 $149
line 217
;217:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $148
JUMPV
LABELV $149
line 221
;218:    }
;219:
;220:    // get any cvar changes
;221:    SG_UpdateCvars();
ADDRGP4 SG_UpdateCvars
CALLV
pop
line 224
;222:
;223:    // even if the game is paused, we still render everything in the background
;224:    if ( sg.state == SG_SHOW_LEVEL_STATS ) {
ADDRGP4 sg+48
INDIRI4
CNSTI4 3
NEI4 $152
line 225
;225:        SG_DrawLevelStats();
ADDRGP4 SG_DrawLevelStats
CALLV
pop
line 226
;226:        return 1; // we don't draw the level if we're ending it
CNSTI4 1
RETI4
ADDRGP4 $148
JUMPV
LABELV $152
line 227
;227:    } else if ( sg.state == SG_ABORT_LEVEL ) {
ADDRGP4 sg+48
INDIRI4
CNSTI4 2
NEI4 $155
line 228
;228:        SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 229
;229:        return SG_DrawAbortMission();
ADDRLP4 20
ADDRGP4 SG_DrawAbortMission
CALLI4
ASGNI4
ADDRLP4 20
INDIRI4
RETI4
ADDRGP4 $148
JUMPV
LABELV $155
line 232
;230:    }
;231:
;232:    SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 234
;233:
;234:    if ( sg_paused.i ) {
ADDRGP4 sg_paused+260
INDIRI4
CNSTI4 0
EQI4 $158
line 235
;235:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $148
JUMPV
LABELV $158
line 238
;236:    }
;237:
;238:    sg.framenum++;
ADDRLP4 20
ADDRGP4 sg+72
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 239
;239:    sg.previousTime = sg.framenum;
ADDRGP4 sg+64
ADDRGP4 sg+72
INDIRI4
ASGNI4
line 240
;240:    sg.levelTime = levelTime;
ADDRGP4 sg+68
ADDRFP4 0
INDIRI4
ASGNI4
line 241
;241:    msec = sg.levelTime - sg.previousTime;
ADDRLP4 16
ADDRGP4 sg+68
INDIRI4
ADDRGP4 sg+64
INDIRI4
SUBI4
ASGNI4
line 246
;242:
;243:    //
;244:    // go through all allocated entities
;245:    //
;246:    start = trap_Milliseconds();
ADDRLP4 24
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 24
INDIRI4
ASGNI4
line 247
;247:    ent = &sg_entities[0];
ADDRLP4 4
ADDRGP4 sg_entities
ASGNP4
line 248
;248:    for ( i = 0; i < sg.numEntities; i++) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $170
JUMPV
LABELV $167
line 249
;249:        if ( !ent->health ) {
ADDRLP4 4
INDIRP4
CNSTI4 52
ADDP4
INDIRI4
CNSTI4 0
NEI4 $172
line 250
;250:            continue;
LABELV $172
line 254
;251:        }
;252:
;253:        
;254:    }
LABELV $168
line 248
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $170
ADDRLP4 0
INDIRI4
CVIU4 4
ADDRGP4 sg+56
INDIRU4
LTU4 $167
line 255
;255:    end = trap_Milliseconds();
ADDRLP4 28
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 28
INDIRI4
ASGNI4
line 257
;256:
;257:    return 1;
CNSTI4 1
RETI4
LABELV $148
endproc SG_RunLoop 32 0
proc SG_LoadMedia 40 4
line 262
;258:}
;259:
;260:
;261:static void SG_LoadMedia( void )
;262:{
line 263
;263:    G_Printf( "Loading sgame sfx...\n" );
ADDRGP4 $175
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 265
;264:
;265:    sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" );
ADDRGP4 $177
ARGP4
ADDRLP4 0
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+12
ADDRLP4 0
INDIRI4
ASGNI4
line 266
;266:    sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" );
ADDRGP4 $179
ARGP4
ADDRLP4 4
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+16
ADDRLP4 4
INDIRI4
ASGNI4
line 267
;267:    sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" ); 
ADDRGP4 $181
ARGP4
ADDRLP4 8
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+20
ADDRLP4 8
INDIRI4
ASGNI4
line 268
;268:    sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" );
ADDRGP4 $182
ARGP4
ADDRLP4 12
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg
ADDRLP4 12
INDIRI4
ASGNI4
line 269
;269:    sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" );
ADDRGP4 $184
ARGP4
ADDRLP4 16
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+4
ADDRLP4 16
INDIRI4
ASGNI4
line 270
;270:    sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" );
ADDRGP4 $186
ARGP4
ADDRLP4 20
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+8
ADDRLP4 20
INDIRI4
ASGNI4
line 271
;271:    sg.media.revolver_fire = trap_Snd_RegisterSfx( "sfx/weapons/revolver_fire.wav" );
ADDRGP4 $188
ARGP4
ADDRLP4 24
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+24
ADDRLP4 24
INDIRI4
ASGNI4
line 272
;272:    sg.media.revolver_rld = trap_Snd_RegisterSfx( "sfx/weapons/revolver_rld.wav" );
ADDRGP4 $190
ARGP4
ADDRLP4 28
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+28
ADDRLP4 28
INDIRI4
ASGNI4
line 274
;273:
;274:    G_Printf( "Finished loading sfx.\n" );
ADDRGP4 $191
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 276
;275:
;276:    G_Printf( "Loading sgame sprites...\n" );
ADDRGP4 $192
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 278
;277:
;278:    sg.media.raio_shader = RE_RegisterShader( "textures/sprites/glnomad_raio_base.png" );
ADDRGP4 $194
ARGP4
ADDRLP4 32
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+32
ADDRLP4 32
INDIRI4
ASGNI4
line 279
;279:    sg.media.grunt_shader = RE_RegisterShader( "textures/sprites/glnomad_grunt.png" );
ADDRGP4 $196
ARGP4
ADDRLP4 36
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+36
ADDRLP4 36
INDIRI4
ASGNI4
line 281
;280:
;281:    G_Printf( "Finished loading sprites.\n" );
ADDRGP4 $197
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 282
;282:}
LABELV $174
endproc SG_LoadMedia 40 4
export SG_Init
proc SG_Init 12 12
line 285
;283:
;284:void SG_Init( void )
;285:{
line 286
;286:    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 4
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $202
ADDRLP4 0
CNSTI4 1
ASGNI4
ADDRGP4 $203
JUMPV
LABELV $202
ADDRLP4 0
CNSTI4 0
ASGNI4
LABELV $203
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 8192
BANDU4
CNSTU4 0
EQU4 $199
line 287
;287:        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
ADDRLP4 8
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 8
INDIRI4
CVIU4 4
CNSTU4 8192
BANDU4
ARGU4
ADDRGP4 trap_Key_SetCatcher
CALLI4
pop
line 288
;288:    }
LABELV $199
line 291
;289:
;290:    // clear sgame state
;291:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 113332
ARGU4
ADDRGP4 memset
CALLP4
pop
line 294
;292:    
;293:    // cache redundant calculations
;294:    Sys_GetGPUConfig( &sg.gpuConfig );
ADDRGP4 sg+74076
ARGP4
ADDRGP4 Sys_GetGPUConfig
CALLV
pop
line 297
;295:
;296:    // for 1024x768 virtualized screen
;297:	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
ADDRGP4 sg+86424
CNSTF4 984263339
ADDRGP4 sg+74076+12312
INDIRI4
CVIF4 4
MULF4
ASGNF4
line 298
;298:	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
CNSTI4 768
ADDRGP4 sg+74076+12308
INDIRI4
MULI4
ADDRGP4 sg+74076+12312
INDIRI4
CNSTI4 10
LSHI4
LEI4 $208
line 300
;299:		// wide screen
;300:		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
ADDRGP4 sg+86428
CNSTF4 1056964608
ADDRGP4 sg+74076+12308
INDIRI4
CVIF4 4
CNSTF4 1068149419
ADDRGP4 sg+74076+12312
INDIRI4
CVIF4 4
MULF4
SUBF4
MULF4
ASGNF4
line 301
;301:	}
ADDRGP4 $209
JUMPV
LABELV $208
line 302
;302:	else {
line 304
;303:		// no wide screen
;304:		sg.bias = 0;
ADDRGP4 sg+86428
CNSTF4 0
ASGNF4
line 305
;305:	}
LABELV $209
line 308
;306:
;307:    // register sgame cvars
;308:    SG_RegisterCvars();
ADDRGP4 SG_RegisterCvars
CALLV
pop
line 311
;309:
;310:    // load assets/resources
;311:    SG_LoadMedia();
ADDRGP4 SG_LoadMedia
CALLV
pop
line 313
;312:
;313:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 315
;314:
;315:    sg.state = SG_INACTIVE;
ADDRGP4 sg+48
CNSTI4 0
ASGNI4
line 316
;316:}
LABELV $198
endproc SG_Init 12 12
export SG_Shutdown
proc SG_Shutdown 4 12
line 319
;317:
;318:void SG_Shutdown( void )
;319:{
line 320
;320:    G_Printf( "Shutting down sgame...\n" );
ADDRGP4 $222
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 322
;321:
;322:    trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_SGAME );
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 4294959103
BANDU4
ARGU4
ADDRGP4 trap_Key_SetCatcher
CALLI4
pop
line 324
;323:
;324:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 113332
ARGU4
ADDRGP4 memset
CALLP4
pop
line 326
;325:
;326:    sg.state = SG_INACTIVE;
ADDRGP4 sg+48
CNSTI4 0
ASGNI4
line 327
;327:}
LABELV $221
endproc SG_Shutdown 4 12
export trap_FS_Printf
proc trap_FS_Printf 8200 12
line 330
;328:
;329:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... )
;330:{
line 334
;331:    va_list argptr;
;332:    char msg[MAXPRINTMSG];
;333:
;334:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 335
;335:    vsprintf( msg, fmt, argptr );
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
line 336
;336:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 338
;337:
;338:    trap_FS_Write( msg, strlen(msg), f );
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
line 339
;339:}
LABELV $224
endproc trap_FS_Printf 8200 12
import trap_Key_SetCatcher
import trap_Key_GetCatcher
import RE_RegisterShader
import trap_Snd_RegisterSfx
import trap_Milliseconds
import trap_Error
import trap_Print
import Cvar_Update
import Cvar_Register
import trap_FS_Read
import trap_FS_Write
import trap_FS_FClose
import trap_FS_FOpenFile
import Sys_GetGPUConfig
import P_GiveWeapon
import P_GiveItem
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
import SG_InitEntities
import SG_BuildBounds
import SG_FreeEntity
import SG_AllocEntity
import Ent_RunTic
import SG_DrawLevelStats
import SG_DrawAbortMission
import Lvl_AddKillEntity
import SG_EndLevel
import SG_InitLevel
import SG_GenerateSpriteSheetTexCoords
import SG_DrawFrame
bss
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
export sg_pmBaseSpeed
align 4
LABELV sg_pmBaseSpeed
skip 276
export sg_pmBaseAcceleration
align 4
LABELV sg_pmBaseAcceleration
skip 276
export sg_pmWaterAcceleration
align 4
LABELV sg_pmWaterAcceleration
skip 276
export sg_pmAirAcceleration
align 4
LABELV sg_pmAirAcceleration
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
skip 113332
import sprites_shotty
import sprites_grunt
import sprites_thenomad
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
LABELV $222
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
LABELV $197
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
LABELV $196
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
LABELV $192
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
LABELV $191
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
LABELV $190
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
LABELV $186
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
byte 1 49
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
byte 1 48
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $181
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
byte 1 50
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
byte 1 49
byte 1 46
byte 1 119
byte 1 97
byte 1 118
byte 1 0
align 1
LABELV $175
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
LABELV $143
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $134
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
LABELV $119
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
LABELV $118
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
LABELV $117
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
LABELV $116
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
LABELV $115
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
LABELV $114
byte 1 115
byte 1 103
byte 1 95
byte 1 103
byte 1 105
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $113
byte 1 51
byte 1 0
align 1
LABELV $112
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
LABELV $111
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
LABELV $110
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
LABELV $109
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
LABELV $108
byte 1 49
byte 1 46
byte 1 48
byte 1 0
align 1
LABELV $107
byte 1 115
byte 1 103
byte 1 95
byte 1 112
byte 1 109
byte 1 66
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
LABELV $106
byte 1 49
byte 1 46
byte 1 50
byte 1 0
align 1
LABELV $105
byte 1 115
byte 1 103
byte 1 95
byte 1 112
byte 1 109
byte 1 66
byte 1 97
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
LABELV $104
byte 1 48
byte 1 46
byte 1 53
byte 1 0
align 1
LABELV $103
byte 1 115
byte 1 103
byte 1 95
byte 1 112
byte 1 109
byte 1 87
byte 1 97
byte 1 116
byte 1 101
byte 1 114
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
byte 1 110
byte 1 111
byte 1 0
align 1
LABELV $102
byte 1 49
byte 1 46
byte 1 53
byte 1 0
align 1
LABELV $101
byte 1 115
byte 1 103
byte 1 95
byte 1 112
byte 1 109
byte 1 65
byte 1 105
byte 1 114
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
LABELV $100
byte 1 49
byte 1 0
align 1
LABELV $99
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
LABELV $98
byte 1 48
byte 1 0
align 1
LABELV $97
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
LABELV $95
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
