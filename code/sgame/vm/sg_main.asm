export vmMain
code
proc vmMain 16 8
file "../sg_main.c"
line 20
;1:#include "../engine/n_shared.h"
;2:#include "sg_local.h"
;3:
;4:void SG_Init( void );
;5:void SG_Shutdown( void );
;6:int SG_RunLoop( int levelTime, int frameTime );
;7:int SG_DrawFrame( void );
;8:
;9:void SaveGame( void );
;10:void LoadGame( void );
;11:
;12:/*
;13:vmMain
;14:
;15:this is the only way control passes into the module.
;16:this must be the very first function compiled into the .qvm file
;17:*/
;18:int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
;19:    int arg8, int arg9, int arg10 )
;20:{
line 21
;21:    switch ( command ) {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 0
LTI4 $91
ADDRLP4 0
INDIRI4
CNSTI4 15
GTI4 $91
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $105
ADDP4
INDIRP4
JUMPV
data
align 4
LABELV $105
address $92
address $93
address $102
address $96
address $91
address $91
address $103
address $104
address $100
address $99
address $91
address $101
address $101
address $94
address $97
address $98
code
LABELV $92
line 23
;22:    case SGAME_INIT:
;23:        SG_Init();
ADDRGP4 SG_Init
CALLV
pop
line 24
;24:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $93
line 26
;25:    case SGAME_SHUTDOWN:
;26:        SG_Shutdown();
ADDRGP4 SG_Shutdown
CALLV
pop
line 27
;27:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $94
line 29
;28:    case SGAME_GET_STATE:
;29:        return sg.state;
ADDRGP4 sg+4204
INDIRI4
RETI4
ADDRGP4 $89
JUMPV
LABELV $96
line 31
;30:    case SGAME_ENDLEVEL:
;31:        return SG_EndLevel();
ADDRLP4 4
ADDRGP4 SG_EndLevel
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
RETI4
ADDRGP4 $89
JUMPV
LABELV $97
line 33
;32:    case SGAME_LOAD_GAME:
;33:        LoadGame();
ADDRGP4 LoadGame
CALLV
pop
line 34
;34:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $98
line 36
;35:    case SGAME_SAVE_GAME:
;36:        SaveGame();
ADDRGP4 SaveGame
CALLV
pop
line 37
;37:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $99
line 39
;38:    case SGAME_MOUSE_EVENT:
;39:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $100
line 41
;40:    case SGAME_KEY_EVENT:
;41:        SG_KeyEvent( arg0, arg1 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRFP4 8
INDIRI4
ARGI4
ADDRGP4 SG_KeyEvent
CALLV
pop
line 42
;42:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $101
line 45
;43:    case SGAME_EVENT_HANDLING:
;44:    case SGAME_EVENT_NONE:
;45:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $102
line 47
;46:    case SGAME_LOADLEVEL:
;47:        return SG_StartLevel();
ADDRLP4 8
ADDRGP4 SG_StartLevel
CALLI4
ASGNI4
ADDRLP4 8
INDIRI4
RETI4
ADDRGP4 $89
JUMPV
LABELV $103
line 49
;48:    case SGAME_CONSOLE_COMMAND:
;49:        SGameCommand();
ADDRGP4 SGameCommand
CALLV
pop
line 50
;50:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $104
line 52
;51:    case SGAME_RUNTIC:
;52:        return SG_RunLoop( arg0, arg1 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRFP4 8
INDIRI4
ARGI4
ADDRLP4 12
ADDRGP4 SG_RunLoop
CALLI4
ASGNI4
ADDRLP4 12
INDIRI4
RETI4
ADDRGP4 $89
JUMPV
line 54
;53:    default:
;54:        break;
LABELV $91
line 55
;55:    };
line 57
;56:
;57:    SG_Error( "vmMain: unrecognized command %i", command );
ADDRGP4 $106
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 58
;58:    return -1;
CNSTI4 -1
RETI4
LABELV $89
endproc vmMain 16 8
data
align 4
LABELV cvarTable
byte 4 0
address $108
address $109
byte 4 64
byte 4 0
byte 4 0
byte 4 0
address $110
address $111
byte 4 64
byte 4 0
byte 4 0
address sg_printEntities
address $112
address $113
byte 4 0
byte 4 0
byte 4 0
address sg_debugPrint
address $114
address $115
byte 4 256
byte 4 0
byte 4 0
address sg_paused
address $116
address $115
byte 4 288
byte 4 0
byte 4 0
address pm_groundFriction
address $117
address $118
byte 4 33
byte 4 0
byte 4 1
address pm_waterFriction
address $119
address $120
byte 4 33
byte 4 0
byte 4 1
address pm_airFriction
address $121
address $122
byte 4 33
byte 4 0
byte 4 1
address pm_airAccel
address $123
address $124
byte 4 33
byte 4 0
byte 4 0
address pm_waterAccel
address $125
address $126
byte 4 33
byte 4 0
byte 4 0
address pm_baseAccel
address $127
address $128
byte 4 33
byte 4 0
byte 4 0
address pm_baseSpeed
address $129
address $130
byte 4 33
byte 4 0
byte 4 0
address sg_mouseInvert
address $131
address $113
byte 4 33
byte 4 0
byte 4 1
address sg_mouseAcceleration
address $132
address $113
byte 4 33
byte 4 0
byte 4 1
address sg_printLevelStats
address $133
address $115
byte 4 33
byte 4 0
byte 4 0
address sg_decalDetail
address $134
address $135
byte 4 33
byte 4 0
byte 4 1
address sg_gibs
address $136
address $113
byte 4 33
byte 4 0
byte 4 1
address sg_savename
address $137
address $138
byte 4 33
byte 4 0
byte 4 1
address sg_numSaves
address $139
address $113
byte 4 33
byte 4 0
byte 4 0
address sg_memoryDebug
address $140
address $113
byte 4 288
byte 4 0
byte 4 0
align 4
LABELV cvarTableSize
byte 4 20
code
proc SG_RegisterCvars 16 16
line 127
;59:}
;60:
;61:sgGlobals_t sg;
;62:
;63:vmCvar_t sg_printEntities;
;64:vmCvar_t sg_debugPrint;
;65:vmCvar_t sg_paused;
;66:vmCvar_t sg_mouseInvert;
;67:vmCvar_t sg_mouseAcceleration;
;68:vmCvar_t sg_printLevelStats;
;69:vmCvar_t sg_decalDetail;
;70:vmCvar_t sg_gibs;
;71:vmCvar_t sg_levelIndex;
;72:vmCvar_t sg_savename;
;73:vmCvar_t sg_numSaves;
;74:vmCvar_t sg_memoryDebug;
;75:
;76:vmCvar_t pm_groundFriction;
;77:vmCvar_t pm_waterFriction;
;78:vmCvar_t pm_airFriction;
;79:vmCvar_t pm_waterAccel;
;80:vmCvar_t pm_baseAccel;
;81:vmCvar_t pm_baseSpeed;
;82:vmCvar_t pm_airAccel;
;83:vmCvar_t pm_wallrunAccelVertical;
;84:vmCvar_t pm_wallrunAccelMove;
;85:vmCvar_t pm_wallTime;
;86:
;87:typedef struct {
;88:    vmCvar_t *vmCvar;
;89:    const char *cvarName;
;90:    const char *defaultValue;
;91:    int cvarFlags;
;92:    int modificationCount; // for tracking changes
;93:    qboolean trackChange;       // track this variable, and announce if changed
;94:} cvarTable_t;
;95:
;96:static cvarTable_t cvarTable[] = {
;97:    // noset vars
;98:    { NULL,                     "gamename",             GLN_VERSION,    CVAR_ROM,                   0, qfalse },
;99:    { NULL,                     "gamedate",             __DATE__,       CVAR_ROM,                   0, qfalse },
;100:    { &sg_printEntities,        "sg_printEntities",     "0",            0,                          0, qfalse },
;101:    { &sg_debugPrint,           "sg_debugPrint",        "1",            CVAR_TEMP,                  0, qfalse },
;102:    { &sg_paused,               "g_paused",             "1",            CVAR_TEMP | CVAR_LATCH,     0, qfalse },
;103:    { &pm_groundFriction,       "pm_groundFriction",    "0.6f",         CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;104:    { &pm_waterFriction,        "pm_waterFriction",     "0.06f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;105:    { &pm_airFriction,          "pm_airFriction",       "0.01f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;106:    { &pm_airAccel,             "pm_airAccel",          "1.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;107:    { &pm_waterAccel,           "pm_waterAccel",        "0.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;108:    { &pm_baseAccel,            "pm_baseAccel",         "1.0f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;109:    { &pm_baseSpeed,            "pm_baseSpeed",         "0.02f",        CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;110:    { &sg_mouseInvert,          "g_mouseInvert",        "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;111:    { &sg_mouseAcceleration,    "g_mouseAcceleration",  "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;112:    { &sg_printLevelStats,      "sg_printLevelStats",   "1",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;113:    { &sg_decalDetail,          "sg_decalDetail",       "3",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;114:    { &sg_gibs,                 "sg_gibs",              "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;115:    { &sg_savename,             "sg_savename",          "savedata",     CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;116:    { &sg_numSaves,             "sg_numSaves",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;117:#ifdef _NOMAD_DEBUG
;118:    { &sg_memoryDebug,          "sg_memoryDebug",       "1",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
;119:#else
;120:    { &sg_memoryDebug,          "sg_memoryDebug",       "0",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
;121:#endif
;122:};
;123:
;124:static const int cvarTableSize = arraylen(cvarTable);
;125:
;126:static void SG_RegisterCvars( void )
;127:{
line 131
;128:    int i;
;129:    cvarTable_t *cv;
;130:
;131:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $145
JUMPV
LABELV $142
line 132
;132:        Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultValue, cv->cvarFlags );
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
CNSTI4 8
ADDP4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 12
ADDP4
INDIRI4
ARGI4
ADDRGP4 Cvar_Register
CALLV
pop
line 133
;133:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $146
line 134
;134:            cv->modificationCount = cv->vmCvar->modificationCount;
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
ADDRLP4 0
INDIRP4
INDIRP4
CNSTI4 268
ADDP4
INDIRU4
CVUI4 4
ASGNI4
line 135
;135:        }
LABELV $146
line 136
;136:    }
LABELV $143
line 131
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 24
ADDP4
ASGNP4
LABELV $145
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $142
line 137
;137:}
LABELV $141
endproc SG_RegisterCvars 16 16
export SG_UpdateCvars
proc SG_UpdateCvars 24 12
line 140
;138:
;139:void SG_UpdateCvars( void )
;140:{
line 144
;141:    int i;
;142:    cvarTable_t *cv;
;143:
;144:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $152
JUMPV
LABELV $149
line 145
;145:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $153
line 146
;146:            Cvar_Update( cv->vmCvar );
ADDRLP4 0
INDIRP4
INDIRP4
ARGP4
ADDRGP4 Cvar_Update
CALLV
pop
line 148
;147:
;148:            if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
INDIRI4
CVIU4 4
ADDRLP4 0
INDIRP4
INDIRP4
CNSTI4 268
ADDP4
INDIRU4
EQU4 $155
line 149
;149:                cv->modificationCount = cv->vmCvar->modificationCount;
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
ADDRLP4 0
INDIRP4
INDIRP4
CNSTI4 268
ADDP4
INDIRU4
CVUI4 4
ASGNI4
line 151
;150:
;151:                if ( cv->trackChange ) {
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
INDIRI4
CNSTI4 0
EQI4 $157
line 152
;152:                    trap_SendConsoleCommand( va( "Changed \"%s\" to \"%s\"", cv->cvarName, cv->vmCvar->s ) );
ADDRGP4 $159
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
INDIRP4
ARGP4
ADDRLP4 20
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 20
INDIRP4
ARGP4
ADDRGP4 trap_SendConsoleCommand
CALLV
pop
line 153
;153:                }
LABELV $157
line 154
;154:            }
LABELV $155
line 155
;155:        }
LABELV $153
line 156
;156:    }
LABELV $150
line 144
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 24
ADDP4
ASGNP4
LABELV $152
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $149
line 157
;157:}
LABELV $148
endproc SG_UpdateCvars 24 12
export G_Printf
proc G_Printf 8204 12
line 160
;158:
;159:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf( const char *fmt, ... )
;160:{
line 165
;161:    va_list argptr;
;162:    char msg[MAXPRINTMSG];
;163:    int length;
;164:
;165:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 166
;166:    length = vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 167
;167:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 169
;168:
;169:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $162
line 170
;170:        trap_Error( "G_Printf: buffer overflow" );
ADDRGP4 $164
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 171
;171:    }
LABELV $162
line 173
;172:
;173:    trap_Print( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 174
;174:}
LABELV $160
endproc G_Printf 8204 12
export G_Error
proc G_Error 8204 12
line 177
;175:
;176:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error( const char *fmt, ... )
;177:{
line 182
;178:    va_list argptr;
;179:    char msg[MAXPRINTMSG];
;180:    int length;
;181:
;182:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 183
;183:    length = vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 184
;184:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 186
;185:
;186:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $167
line 187
;187:        trap_Error( "G_Error: buffer overflow" );
ADDRGP4 $169
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 188
;188:    }
LABELV $167
line 190
;189:
;190:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 191
;191:}
LABELV $165
endproc G_Error 8204 12
export SG_Printf
proc SG_Printf 8204 12
line 194
;192:
;193:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf( const char *fmt, ... )
;194:{
line 199
;195:    va_list argptr;
;196:    char msg[MAXPRINTMSG];
;197:    int length;
;198:
;199:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 200
;200:    length = vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 201
;201:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 203
;202:
;203:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $172
line 204
;204:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $174
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 205
;205:    }
LABELV $172
line 207
;206:
;207:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 208
;208:}
LABELV $170
endproc SG_Printf 8204 12
export SG_Error
proc SG_Error 8204 12
line 211
;209:
;210:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error( const char *fmt, ... )
;211:{
line 216
;212:    va_list argptr;
;213:    char msg[MAXPRINTMSG];
;214:    int length;
;215:
;216:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 217
;217:    length = vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 218
;218:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 220
;219:
;220:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $177
line 221
;221:        trap_Error( "SG_Error: buffer overflow" );
ADDRGP4 $179
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 222
;222:    }
LABELV $177
line 224
;223:
;224:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 225
;225:}
LABELV $175
endproc SG_Error 8204 12
export Con_Printf
proc Con_Printf 8204 12
line 231
;226:
;227://#ifndef SGAME_HARD_LINKED
;228:// this is only here so the functions in n_shared.c and bg_*.c can link
;229:
;230:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf( const char *fmt, ... )
;231:{
line 236
;232:    va_list argptr;
;233:    char msg[MAXPRINTMSG];
;234:    int length;
;235:
;236:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 237
;237:    length = vsprintf( msg, fmt, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 238
;238:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 240
;239:
;240:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $182
line 241
;241:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $174
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 242
;242:    }
LABELV $182
line 244
;243:
;244:    trap_Print( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 245
;245:}
LABELV $180
endproc Con_Printf 8204 12
export N_Error
proc N_Error 8204 12
line 248
;246:
;247:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error( errorCode_t code, const char *err, ... )
;248:{
line 253
;249:    va_list argptr;
;250:    char msg[MAXPRINTMSG];
;251:    int length;
;252:
;253:    va_start( argptr, err );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 254
;254:    length = vsprintf( msg, err, argptr );
ADDRLP4 4
ARGP4
ADDRFP4 4
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 8200
ADDRGP4 vsprintf
CALLI4
ASGNI4
ADDRLP4 8196
ADDRLP4 8200
INDIRI4
ASGNI4
line 255
;255:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 257
;256:
;257:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $186
line 258
;258:        trap_Error( "N_Error: buffer overflow" );
ADDRGP4 $188
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 259
;259:    }
LABELV $186
line 261
;260:
;261:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 262
;262:}
LABELV $184
endproc N_Error 8204 12
export SG_RunLoop
proc SG_RunLoop 44 12
line 267
;263:
;264://#endif
;265:
;266:int SG_RunLoop( int levelTime, int frameTime )
;267:{
line 273
;268:    int i;
;269:    int start, end;
;270:    int msec;
;271:    sgentity_t *ent;
;272:
;273:    if ( sg.state == SG_INACTIVE ) {
ADDRGP4 sg+4204
INDIRI4
CNSTI4 2
NEI4 $190
line 274
;274:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $189
JUMPV
LABELV $190
line 278
;275:    }
;276:
;277:    // get any cvar changes
;278:    SG_UpdateCvars();
ADDRGP4 SG_UpdateCvars
CALLV
pop
line 280
;279:
;280:    if ( sg_paused.i ) {
ADDRGP4 sg_paused+260
INDIRI4
CNSTI4 0
EQI4 $193
line 281
;281:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $189
JUMPV
LABELV $193
line 284
;282:    }
;283:
;284:    sg.framenum++;
ADDRLP4 20
ADDRGP4 sg+4232
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 285
;285:    sg.previousTime = sg.framenum;
ADDRGP4 sg+4224
ADDRGP4 sg+4232
INDIRI4
ASGNI4
line 286
;286:    sg.levelTime = levelTime;
ADDRGP4 sg+4228
ADDRFP4 0
INDIRI4
ASGNI4
line 287
;287:    msec = sg.levelTime - sg.previousTime;
ADDRLP4 16
ADDRGP4 sg+4228
INDIRI4
ADDRGP4 sg+4224
INDIRI4
SUBI4
ASGNI4
line 292
;288:
;289:    //
;290:    // go through all allocated entities
;291:    //
;292:    start = Sys_Milliseconds();
ADDRLP4 24
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 24
INDIRI4
ASGNI4
line 293
;293:    ent = &sg_entities[0];
ADDRLP4 0
ADDRGP4 sg_entities
ASGNP4
line 294
;294:    for ( i = 0; i < sg.numEntities; i++) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $205
JUMPV
LABELV $202
line 295
;295:        if ( !ent->health ) {
ADDRLP4 0
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CNSTI4 0
NEI4 $207
line 296
;296:            continue;
ADDRGP4 $203
JUMPV
LABELV $207
line 299
;297:        }
;298:
;299:        ent->ticker--;
ADDRLP4 28
ADDRLP4 0
INDIRP4
CNSTI4 116
ADDP4
ASGNP4
ADDRLP4 28
INDIRP4
ADDRLP4 28
INDIRP4
INDIRI4
CNSTI4 1
SUBI4
ASGNI4
line 301
;300:
;301:        if ( ent->ticker <= -1 ) {
ADDRLP4 0
INDIRP4
CNSTI4 116
ADDP4
INDIRI4
CNSTI4 -1
GTI4 $209
line 302
;302:            Ent_SetState( ent, ent->state->nextstate );
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 16
ADDP4
INDIRI4
ARGI4
ADDRGP4 Ent_SetState
CALLI4
pop
line 303
;303:            continue;
ADDRGP4 $203
JUMPV
LABELV $209
line 307
;304:        }
;305:
;306:        // update the current entity's animation frame
;307:        if ( ent->state->frames > 0 ) {
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
CNSTU4 0
EQU4 $211
line 308
;308:            if ( ent->frame == ent->state->frames ) {
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
INDIRI4
CVIU4 4
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
NEU4 $213
line 309
;309:                ent->frame = 0; // reset animation
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
CNSTI4 0
ASGNI4
line 310
;310:            }
ADDRGP4 $214
JUMPV
LABELV $213
line 311
;311:            else if ( ent->ticker % ent->state->frames ) {
ADDRLP4 0
INDIRP4
CNSTI4 116
ADDP4
INDIRI4
CVIU4 4
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
MODU4
CNSTU4 0
EQU4 $215
line 312
;312:                ent->frame++;
ADDRLP4 40
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
ASGNP4
ADDRLP4 40
INDIRP4
ADDRLP4 40
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 313
;313:            }
LABELV $215
LABELV $214
line 314
;314:        }
LABELV $211
line 316
;315:
;316:        ent->state->action.acp1( ent );
ADDRLP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 12
ADDP4
INDIRP4
CALLV
pop
line 317
;317:    }
LABELV $203
line 294
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $205
ADDRLP4 4
INDIRI4
ADDRGP4 sg+4216
INDIRI4
LTI4 $202
line 318
;318:    end = Sys_Milliseconds();
ADDRLP4 28
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 28
INDIRI4
ASGNI4
line 320
;319:
;320:    SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 322
;321:
;322:    if ( sg_printEntities.i ) {
ADDRGP4 sg_printEntities+260
INDIRI4
CNSTI4 0
EQI4 $217
line 323
;323:        for ( i = 0; i < sg.numEntities; i++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $223
JUMPV
LABELV $220
line 324
;324:            G_Printf( "%4i: %s\n", i, sg_entities[i].classname );
ADDRGP4 $225
ARGP4
ADDRLP4 4
INDIRI4
ARGI4
ADDRLP4 4
INDIRI4
CNSTI4 160
MULI4
ADDRGP4 sg_entities+96
ADDP4
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 325
;325:        }
LABELV $221
line 323
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $223
ADDRLP4 4
INDIRI4
ADDRGP4 sg+4216
INDIRI4
LTI4 $220
line 326
;326:        Cvar_Set( "sg_printEntities", "0" );
ADDRGP4 $112
ARGP4
ADDRGP4 $113
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 327
;327:    }
LABELV $217
line 329
;328:
;329:    return 1;
CNSTI4 1
RETI4
LABELV $189
endproc SG_RunLoop 44 12
proc SG_LoadMedia 20008 0
line 334
;330:}
;331:
;332:
;333:static void SG_LoadMedia( void )
;334:{
line 339
;335:    int len;
;336:    fileHandle_t f;
;337:    char text[20000];
;338:
;339:}
LABELV $227
endproc SG_LoadMedia 20008 0
export SG_Init
proc SG_Init 4 12
line 342
;340:
;341:void SG_Init( void )
;342:{
line 343
;343:    G_Printf( "---------- Game Initialization ----------\n" );
ADDRGP4 $229
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 344
;344:    G_Printf( "gamename: %s\n", GLN_VERSION );
ADDRGP4 $230
ARGP4
ADDRGP4 $109
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 345
;345:    G_Printf( "gamedate: %s\n", __DATE__ );
ADDRGP4 $231
ARGP4
ADDRGP4 $111
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 347
;346:
;347:    trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_SGAME );
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 8192
BORU4
CVUI4 4
ARGI4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 350
;348:
;349:    // clear sgame state
;350:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4296164
ARGU4
ADDRGP4 memset
CALLI4
pop
line 353
;351:    
;352:    // cache redundant calculations
;353:    Sys_GetGPUConfig( &sg.gpuConfig );
ADDRGP4 sg+65820
ARGP4
ADDRGP4 Sys_GetGPUConfig
CALLV
pop
line 356
;354:
;355:    // for 1024x768 virtualized screen
;356:	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
ADDRGP4 sg+78168
ADDRGP4 sg+65820+12324
INDIRI4
CVIF4 4
CNSTF4 984263339
MULF4
ASGNF4
line 357
;357:	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
ADDRGP4 sg+65820+12320
INDIRI4
CNSTI4 768
MULI4
ADDRGP4 sg+65820+12324
INDIRI4
CNSTI4 10
LSHI4
LEI4 $236
line 359
;358:		// wide screen
;359:		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
ADDRGP4 sg+78172
ADDRGP4 sg+65820+12320
INDIRI4
CVIF4 4
ADDRGP4 sg+65820+12324
INDIRI4
CVIF4 4
CNSTF4 1068149419
MULF4
SUBF4
CNSTF4 1056964608
MULF4
ASGNF4
line 360
;360:	}
ADDRGP4 $237
JUMPV
LABELV $236
line 361
;361:	else {
line 363
;362:		// no wide screen
;363:		sg.bias = 0;
ADDRGP4 sg+78172
CNSTF4 0
ASGNF4
line 364
;364:	}
LABELV $237
line 367
;365:
;366:    // register sgame cvars
;367:    SG_RegisterCvars();
ADDRGP4 SG_RegisterCvars
CALLV
pop
line 370
;368:
;369:    // load assets/resources
;370:    SG_LoadMedia();
ADDRGP4 SG_LoadMedia
CALLV
pop
line 372
;371:
;372:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 374
;373:
;374:    sg.state = SG_INACTIVE;
ADDRGP4 sg+4204
CNSTI4 2
ASGNI4
line 376
;375:
;376:    G_Printf( "-----------------------------------\n" );
ADDRGP4 $249
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 377
;377:}
LABELV $228
endproc SG_Init 4 12
export SG_Shutdown
proc SG_Shutdown 0 12
line 380
;378:
;379:void SG_Shutdown( void )
;380:{
line 381
;381:    G_Printf( "Shutting down sgame...\n" );
ADDRGP4 $251
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 383
;382:
;383:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4296164
ARGU4
ADDRGP4 memset
CALLI4
pop
line 385
;384:
;385:    sg.state = SG_INACTIVE;
ADDRGP4 sg+4204
CNSTI4 2
ASGNI4
line 386
;386:}
LABELV $250
endproc SG_Shutdown 0 12
export SaveGame
proc SaveGame 0 4
line 389
;387:
;388:void SaveGame( void )
;389:{
line 390
;390:    G_Printf( "Saving game...\n" );
ADDRGP4 $254
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 392
;391:
;392:    SG_SaveLevelData();
ADDRGP4 SG_SaveLevelData
CALLV
pop
line 394
;393:
;394:    G_Printf( "Done" );
ADDRGP4 $255
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 395
;395:}
LABELV $253
endproc SaveGame 0 4
export LoadGame
proc LoadGame 64 12
line 398
;396:
;397:void LoadGame( void )
;398:{
line 401
;399:    char savename[MAX_NPATH];
;400:
;401:    Cvar_VariableStringBuffer( "sg_savename", savename, sizeof(savename) );
ADDRGP4 $137
ARGP4
ADDRLP4 0
ARGP4
CNSTI4 64
ARGI4
ADDRGP4 Cvar_VariableStringBuffer
CALLV
pop
line 403
;402:
;403:    G_Printf( "Loading save file '%s'...\n", savename );
ADDRGP4 $257
ARGP4
ADDRLP4 0
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 404
;404:}
LABELV $256
endproc LoadGame 64 12
export trap_FS_Printf
proc trap_FS_Printf 8200 12
line 407
;405:
;406:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( fileHandle_t f, const char *fmt, ... )
;407:{
line 411
;408:    va_list argptr;
;409:    char msg[MAXPRINTMSG];
;410:
;411:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 412
;412:    vsprintf( msg, fmt, argptr );
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
line 413
;413:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 415
;414:
;415:    trap_FS_Write( msg, strlen( msg ), f );
ADDRLP4 4
ARGP4
ADDRLP4 8196
ADDRGP4 strlen
CALLI4
ASGNI4
ADDRLP4 4
ARGP4
ADDRLP4 8196
INDIRI4
ARGI4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 trap_FS_Write
CALLI4
pop
line 416
;416:}
LABELV $258
endproc trap_FS_Printf 8200 12
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
import strlen
import memset
import vsprintf
bss
export sg_printEntities
align 4
LABELV sg_printEntities
skip 276
import Cvar_VariableStringBuffer
import Cvar_Set
import Cvar_Update
import Cvar_Register
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
import SG_LoadLevelData
import SG_SaveLevelData
import SG_EndLevel
import SG_StartLevel
import SG_BuildMoveCommand
import SGameCommand
import SG_DrawFrame
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
export pm_airFriction
align 4
LABELV pm_airFriction
skip 276
export pm_waterFriction
align 4
LABELV pm_waterFriction
skip 276
export pm_groundFriction
align 4
LABELV pm_groundFriction
skip 276
export sg_memoryDebug
align 4
LABELV sg_memoryDebug
skip 276
export sg_numSaves
align 4
LABELV sg_numSaves
skip 276
export sg_savename
align 4
LABELV sg_savename
skip 276
export sg_levelIndex
align 4
LABELV sg_levelIndex
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
skip 4296164
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
LABELV $257
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
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
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $255
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 0
align 1
LABELV $254
byte 1 83
byte 1 97
byte 1 118
byte 1 105
byte 1 110
byte 1 103
byte 1 32
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
LABELV $251
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
LABELV $249
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
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 45
byte 1 10
byte 1 0
align 1
LABELV $231
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 100
byte 1 97
byte 1 116
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $230
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 110
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
LABELV $229
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
byte 1 71
byte 1 97
byte 1 109
byte 1 101
byte 1 32
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 105
byte 1 97
byte 1 108
byte 1 105
byte 1 122
byte 1 97
byte 1 116
byte 1 105
byte 1 111
byte 1 110
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
LABELV $225
byte 1 37
byte 1 52
byte 1 105
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $188
byte 1 78
byte 1 95
byte 1 69
byte 1 114
byte 1 114
byte 1 111
byte 1 114
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
LABELV $179
byte 1 83
byte 1 71
byte 1 95
byte 1 69
byte 1 114
byte 1 114
byte 1 111
byte 1 114
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
LABELV $174
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
LABELV $169
byte 1 71
byte 1 95
byte 1 69
byte 1 114
byte 1 114
byte 1 111
byte 1 114
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
LABELV $164
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
LABELV $159
byte 1 67
byte 1 104
byte 1 97
byte 1 110
byte 1 103
byte 1 101
byte 1 100
byte 1 32
byte 1 34
byte 1 37
byte 1 115
byte 1 34
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 34
byte 1 37
byte 1 115
byte 1 34
byte 1 0
align 1
LABELV $140
byte 1 115
byte 1 103
byte 1 95
byte 1 109
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 68
byte 1 101
byte 1 98
byte 1 117
byte 1 103
byte 1 0
align 1
LABELV $139
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
LABELV $138
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 100
byte 1 97
byte 1 116
byte 1 97
byte 1 0
align 1
LABELV $137
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
LABELV $136
byte 1 115
byte 1 103
byte 1 95
byte 1 103
byte 1 105
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $135
byte 1 51
byte 1 0
align 1
LABELV $134
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
LABELV $133
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
LABELV $132
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
LABELV $131
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
LABELV $130
byte 1 48
byte 1 46
byte 1 48
byte 1 50
byte 1 102
byte 1 0
align 1
LABELV $129
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
LABELV $128
byte 1 49
byte 1 46
byte 1 48
byte 1 102
byte 1 0
align 1
LABELV $127
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
LABELV $126
byte 1 48
byte 1 46
byte 1 53
byte 1 102
byte 1 0
align 1
LABELV $125
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
LABELV $124
byte 1 49
byte 1 46
byte 1 53
byte 1 102
byte 1 0
align 1
LABELV $123
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
LABELV $122
byte 1 48
byte 1 46
byte 1 48
byte 1 49
byte 1 102
byte 1 0
align 1
LABELV $121
byte 1 112
byte 1 109
byte 1 95
byte 1 97
byte 1 105
byte 1 114
byte 1 70
byte 1 114
byte 1 105
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $120
byte 1 48
byte 1 46
byte 1 48
byte 1 54
byte 1 102
byte 1 0
align 1
LABELV $119
byte 1 112
byte 1 109
byte 1 95
byte 1 119
byte 1 97
byte 1 116
byte 1 101
byte 1 114
byte 1 70
byte 1 114
byte 1 105
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $118
byte 1 48
byte 1 46
byte 1 54
byte 1 102
byte 1 0
align 1
LABELV $117
byte 1 112
byte 1 109
byte 1 95
byte 1 103
byte 1 114
byte 1 111
byte 1 117
byte 1 110
byte 1 100
byte 1 70
byte 1 114
byte 1 105
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $116
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
LABELV $115
byte 1 49
byte 1 0
align 1
LABELV $114
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
LABELV $113
byte 1 48
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
byte 1 69
byte 1 110
byte 1 116
byte 1 105
byte 1 116
byte 1 105
byte 1 101
byte 1 115
byte 1 0
align 1
LABELV $111
byte 1 70
byte 1 101
byte 1 98
byte 1 32
byte 1 32
byte 1 51
byte 1 32
byte 1 50
byte 1 48
byte 1 50
byte 1 52
byte 1 0
align 1
LABELV $110
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 100
byte 1 97
byte 1 116
byte 1 101
byte 1 0
align 1
LABELV $109
byte 1 71
byte 1 76
byte 1 78
byte 1 111
byte 1 109
byte 1 97
byte 1 100
byte 1 32
byte 1 49
byte 1 46
byte 1 48
byte 1 32
byte 1 65
byte 1 108
byte 1 112
byte 1 104
byte 1 97
byte 1 0
align 1
LABELV $108
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $106
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
