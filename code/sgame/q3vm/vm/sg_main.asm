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
ADDRGP4 sg+2212
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
address sg_gameDifficulty
address $137
address $138
byte 4 288
byte 4 0
byte 4 1
address sg_savename
address $139
address $140
byte 4 33
byte 4 0
byte 4 1
address sg_numSaves
address $141
address $113
byte 4 33
byte 4 0
byte 4 0
address sg_memoryDebug
address $142
address $113
byte 4 288
byte 4 0
byte 4 0
address sgc_infiniteHealth
address $143
address $113
byte 4 33
byte 4 0
byte 4 1
address sgc_infiniteAmmo
address $144
address $113
byte 4 33
byte 4 0
byte 4 1
address sgc_infiniteRage
address $145
address $113
byte 4 33
byte 4 0
byte 4 1
address sgc_godmode
address $146
address $113
byte 4 33
byte 4 0
byte 4 1
address sgc_blindMobs
address $147
address $113
byte 4 33
byte 4 0
byte 4 1
address sgc_deafMobs
address $148
address $113
byte 4 33
byte 4 0
byte 4 1
address sg_cheatsOn
address $149
address $113
byte 4 33
byte 4 0
byte 4 1
align 4
LABELV cvarTableSize
byte 4 28
code
proc SG_RegisterCvars 16 16
line 144
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
;73:vmCvar_t sg_gameDifficulty;
;74:vmCvar_t sg_numSaves;
;75:vmCvar_t sg_memoryDebug;
;76:
;77:vmCvar_t pm_groundFriction;
;78:vmCvar_t pm_waterFriction;
;79:vmCvar_t pm_airFriction;
;80:vmCvar_t pm_waterAccel;
;81:vmCvar_t pm_baseAccel;
;82:vmCvar_t pm_baseSpeed;
;83:vmCvar_t pm_airAccel;
;84:vmCvar_t pm_wallrunAccelVertical;
;85:vmCvar_t pm_wallrunAccelMove;
;86:vmCvar_t pm_wallTime;
;87:
;88:vmCvar_t sgc_infiniteHealth;
;89:vmCvar_t sgc_infiniteRage;
;90:vmCvar_t sgc_infiniteAmmo;
;91:vmCvar_t sgc_blindMobs;
;92:vmCvar_t sgc_deafMobs;
;93:vmCvar_t sg_cheatsOn;
;94:vmCvar_t sgc_godmode;
;95:
;96:typedef struct {
;97:    vmCvar_t *vmCvar;
;98:    const char *cvarName;
;99:    const char *defaultValue;
;100:    int cvarFlags;
;101:    int modificationCount; // for tracking changes
;102:    qboolean trackChange;       // track this variable, and announce if changed
;103:} cvarTable_t;
;104:
;105:static cvarTable_t cvarTable[] = {
;106:    // noset vars
;107:    { NULL,                     "gamename",             GLN_VERSION,    CVAR_ROM,                   0, qfalse },
;108:    { NULL,                     "gamedate",             __DATE__,       CVAR_ROM,                   0, qfalse },
;109:    { &sg_printEntities,        "sg_printEntities",     "0",            0,                          0, qfalse },
;110:    { &sg_debugPrint,           "sg_debugPrint",        "1",            CVAR_TEMP,                  0, qfalse },
;111:    { &sg_paused,               "g_paused",             "1",            CVAR_TEMP | CVAR_LATCH,     0, qfalse },
;112:    { &pm_groundFriction,       "pm_groundFriction",    "0.6f",         CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;113:    { &pm_waterFriction,        "pm_waterFriction",     "0.06f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;114:    { &pm_airFriction,          "pm_airFriction",       "0.01f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;115:    { &pm_airAccel,             "pm_airAccel",          "1.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;116:    { &pm_waterAccel,           "pm_waterAccel",        "0.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;117:    { &pm_baseAccel,            "pm_baseAccel",         "1.0f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;118:    { &pm_baseSpeed,            "pm_baseSpeed",         "0.02f",        CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;119:    { &sg_mouseInvert,          "g_mouseInvert",        "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;120:    { &sg_mouseAcceleration,    "g_mouseAcceleration",  "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;121:    { &sg_printLevelStats,      "sg_printLevelStats",   "1",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;122:    { &sg_decalDetail,          "sg_decalDetail",       "3",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;123:    { &sg_gibs,                 "sg_gibs",              "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;124:    { &sg_gameDifficulty,       "sg_gameDifficulty",    "2",            CVAR_LATCH | CVAR_TEMP,     0, qtrue },
;125:    { &sg_savename,             "sg_savename",          "savedata",     CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;126:    { &sg_numSaves,             "sg_numSaves",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;127:#ifdef _NOMAD_DEBUG
;128:    { &sg_memoryDebug,          "sg_memoryDebug",       "1",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
;129:#else
;130:    { &sg_memoryDebug,          "sg_memoryDebug",       "0",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
;131:#endif
;132:    { &sgc_infiniteHealth,      "sgc_infiniteHealth",   "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;133:    { &sgc_infiniteAmmo,        "sgc_infiniteAmmo",     "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;134:    { &sgc_infiniteRage,        "sgc_infiniteRage",     "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;135:    { &sgc_godmode,             "sgc_godmode",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;136:    { &sgc_blindMobs,           "sgc_blindMobs",        "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;137:    { &sgc_deafMobs,            "sgc_deafMobs",         "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;138:    { &sg_cheatsOn,             "sg_cheatsOn",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;139:};
;140:
;141:static const int cvarTableSize = arraylen(cvarTable);
;142:
;143:static void SG_RegisterCvars( void )
;144:{
line 148
;145:    int i;
;146:    cvarTable_t *cv;
;147:
;148:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $154
JUMPV
LABELV $151
line 149
;149:        Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultValue, cv->cvarFlags );
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
line 150
;150:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $155
line 151
;151:            cv->modificationCount = cv->vmCvar->modificationCount;
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
line 152
;152:        }
LABELV $155
line 153
;153:    }
LABELV $152
line 148
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
LABELV $154
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $151
line 154
;154:}
LABELV $150
endproc SG_RegisterCvars 16 16
export SG_UpdateCvars
proc SG_UpdateCvars 24 12
line 157
;155:
;156:void SG_UpdateCvars( void )
;157:{
line 161
;158:    int i;
;159:    cvarTable_t *cv;
;160:
;161:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $161
JUMPV
LABELV $158
line 162
;162:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $162
line 163
;163:            Cvar_Update( cv->vmCvar );
ADDRLP4 0
INDIRP4
INDIRP4
ARGP4
ADDRGP4 Cvar_Update
CALLV
pop
line 165
;164:
;165:            if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
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
EQU4 $164
line 166
;166:                cv->modificationCount = cv->vmCvar->modificationCount;
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
line 168
;167:
;168:                if ( cv->trackChange ) {
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
INDIRI4
CNSTI4 0
EQI4 $166
line 169
;169:                    trap_SendConsoleCommand( va( "Changed \"%s\" to \"%s\"", cv->cvarName, cv->vmCvar->s ) );
ADDRGP4 $168
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
line 170
;170:                }
LABELV $166
line 171
;171:            }
LABELV $164
line 172
;172:        }
LABELV $162
line 173
;173:    }
LABELV $159
line 161
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
LABELV $161
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $158
line 174
;174:}
LABELV $157
endproc SG_UpdateCvars 24 12
export G_Printf
proc G_Printf 8204 12
line 177
;175:
;176:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf( const char *fmt, ... )
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
LTU4 $171
line 187
;187:        trap_Error( "G_Printf: buffer overflow" );
ADDRGP4 $173
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 188
;188:    }
LABELV $171
line 190
;189:
;190:    trap_Print( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 191
;191:}
LABELV $169
endproc G_Printf 8204 12
export G_Error
proc G_Error 8204 12
line 194
;192:
;193:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error( const char *fmt, ... )
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
LTU4 $176
line 204
;204:        trap_Error( "G_Error: buffer overflow" );
ADDRGP4 $178
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 205
;205:    }
LABELV $176
line 207
;206:
;207:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 208
;208:}
LABELV $174
endproc G_Error 8204 12
export SG_Printf
proc SG_Printf 8204 12
line 211
;209:
;210:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf( const char *fmt, ... )
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
LTU4 $181
line 221
;221:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $183
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 222
;222:    }
LABELV $181
line 224
;223:
;224:    trap_Print( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 225
;225:}
LABELV $179
endproc SG_Printf 8204 12
export SG_Error
proc SG_Error 8204 12
line 228
;226:
;227:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error( const char *fmt, ... )
;228:{
line 233
;229:    va_list argptr;
;230:    char msg[MAXPRINTMSG];
;231:    int length;
;232:
;233:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 234
;234:    length = vsprintf( msg, fmt, argptr );
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
line 235
;235:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 237
;236:
;237:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $186
line 238
;238:        trap_Error( "SG_Error: buffer overflow" );
ADDRGP4 $188
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 239
;239:    }
LABELV $186
line 241
;240:
;241:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 242
;242:}
LABELV $184
endproc SG_Error 8204 12
export Con_Printf
proc Con_Printf 8204 12
line 248
;243:
;244://#ifndef SGAME_HARD_LINKED
;245:// this is only here so the functions in n_shared.c and bg_*.c can link
;246:
;247:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf( const char *fmt, ... )
;248:{
line 253
;249:    va_list argptr;
;250:    char msg[MAXPRINTMSG];
;251:    int length;
;252:
;253:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 254
;254:    length = vsprintf( msg, fmt, argptr );
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
LTU4 $191
line 258
;258:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $183
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 259
;259:    }
LABELV $191
line 261
;260:
;261:    trap_Print( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 262
;262:}
LABELV $189
endproc Con_Printf 8204 12
export N_Error
proc N_Error 8204 12
line 265
;263:
;264:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error( errorCode_t code, const char *err, ... )
;265:{
line 270
;266:    va_list argptr;
;267:    char msg[MAXPRINTMSG];
;268:    int length;
;269:
;270:    va_start( argptr, err );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 271
;271:    length = vsprintf( msg, err, argptr );
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
line 272
;272:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 274
;273:
;274:    if ( length >= sizeof(msg) ) {
ADDRLP4 8196
INDIRI4
CVIU4 4
CNSTU4 8192
LTU4 $195
line 275
;275:        trap_Error( "N_Error: buffer overflow" );
ADDRGP4 $197
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 276
;276:    }
LABELV $195
line 278
;277:
;278:    trap_Error( msg );
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 279
;279:}
LABELV $193
endproc N_Error 8204 12
export SG_RunLoop
proc SG_RunLoop 44 12
line 284
;280:
;281://#endif
;282:
;283:int SG_RunLoop( int levelTime, int frameTime )
;284:{
line 290
;285:    int i;
;286:    int start, end;
;287:    int msec;
;288:    sgentity_t *ent;
;289:
;290:    if ( sg.state == SG_INACTIVE ) {
ADDRGP4 sg+2212
INDIRI4
CNSTI4 2
NEI4 $199
line 291
;291:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $198
JUMPV
LABELV $199
line 295
;292:    }
;293:
;294:    // get any cvar changes
;295:    SG_UpdateCvars();
ADDRGP4 SG_UpdateCvars
CALLV
pop
line 297
;296:
;297:    if ( sg_paused.i ) {
ADDRGP4 sg_paused+260
INDIRI4
CNSTI4 0
EQI4 $202
line 298
;298:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $198
JUMPV
LABELV $202
line 301
;299:    }
;300:
;301:    sg.framenum++;
ADDRLP4 20
ADDRGP4 sg+2240
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 302
;302:    sg.previousTime = sg.framenum;
ADDRGP4 sg+2232
ADDRGP4 sg+2240
INDIRI4
ASGNI4
line 303
;303:    sg.levelTime = levelTime;
ADDRGP4 sg+2236
ADDRFP4 0
INDIRI4
ASGNI4
line 304
;304:    msec = sg.levelTime - sg.previousTime;
ADDRLP4 16
ADDRGP4 sg+2236
INDIRI4
ADDRGP4 sg+2232
INDIRI4
SUBI4
ASGNI4
line 309
;305:
;306:    //
;307:    // go through all allocated entities
;308:    //
;309:    start = Sys_Milliseconds();
ADDRLP4 24
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 24
INDIRI4
ASGNI4
line 310
;310:    ent = &sg_entities[0];
ADDRLP4 0
ADDRGP4 sg_entities
ASGNP4
line 311
;311:    for ( i = 0; i < sg.numEntities; i++) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $214
JUMPV
LABELV $211
line 312
;312:        if ( !ent->health ) {
ADDRLP4 0
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CNSTI4 0
NEI4 $216
line 313
;313:            continue;
ADDRGP4 $212
JUMPV
LABELV $216
line 316
;314:        }
;315:
;316:        ent->ticker--;
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
line 318
;317:
;318:        if ( ent->ticker <= -1 ) {
ADDRLP4 0
INDIRP4
CNSTI4 116
ADDP4
INDIRI4
CNSTI4 -1
GTI4 $218
line 319
;319:            Ent_SetState( ent, ent->state->nextstate );
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
line 320
;320:            continue;
ADDRGP4 $212
JUMPV
LABELV $218
line 324
;321:        }
;322:
;323:        // update the current entity's animation frame
;324:        if ( ent->state->frames > 0 ) {
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
CNSTU4 0
EQU4 $220
line 325
;325:            if ( ent->frame == ent->state->frames ) {
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
NEU4 $222
line 326
;326:                ent->frame = 0; // reset animation
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
CNSTI4 0
ASGNI4
line 327
;327:            }
ADDRGP4 $223
JUMPV
LABELV $222
line 328
;328:            else if ( ent->ticker % ent->state->frames ) {
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
EQU4 $224
line 329
;329:                ent->frame++;
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
line 330
;330:            }
LABELV $224
LABELV $223
line 331
;331:        }
LABELV $220
line 333
;332:
;333:        ent->state->action.acp1( ent );
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
line 334
;334:    }
LABELV $212
line 311
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $214
ADDRLP4 4
INDIRI4
ADDRGP4 sg+2224
INDIRI4
LTI4 $211
line 335
;335:    end = Sys_Milliseconds();
ADDRLP4 28
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 28
INDIRI4
ASGNI4
line 337
;336:
;337:    SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 339
;338:
;339:    if ( sg_printEntities.i ) {
ADDRGP4 sg_printEntities+260
INDIRI4
CNSTI4 0
EQI4 $226
line 340
;340:        for ( i = 0; i < sg.numEntities; i++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $232
JUMPV
LABELV $229
line 341
;341:            G_Printf( "%4i: %s\n", i, sg_entities[i].classname );
ADDRGP4 $234
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
line 342
;342:        }
LABELV $230
line 340
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $232
ADDRLP4 4
INDIRI4
ADDRGP4 sg+2224
INDIRI4
LTI4 $229
line 343
;343:        Cvar_Set( "sg_printEntities", "0" );
ADDRGP4 $112
ARGP4
ADDRGP4 $113
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 344
;344:    }
LABELV $226
line 346
;345:
;346:    return 1;
CNSTI4 1
RETI4
LABELV $198
endproc SG_RunLoop 44 12
bss
align 1
LABELV $237
skip 20000
export SG_LoadFile
code
proc SG_LoadFile 12 12
line 350
;347:}
;348:
;349:char *SG_LoadFile( const char *filename )
;350:{
line 355
;351:    int len;
;352:    fileHandle_t f;
;353:    static char text[20000];
;354:
;355:    len = trap_FS_FOpenFile( filename, &f, FS_OPEN_READ );
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ARGP4
CNSTI4 0
ARGI4
ADDRLP4 8
ADDRGP4 trap_FS_FOpenFile
CALLU4
ASGNU4
ADDRLP4 0
ADDRLP4 8
INDIRU4
CVUI4 4
ASGNI4
line 356
;356:    if ( !len ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $238
line 357
;357:        SG_Error( "SG_LoadFile: failed to open file %s", filename );
ADDRGP4 $240
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 358
;358:    }
LABELV $238
line 359
;359:    if ( len >= sizeof(text) ) {
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 20000
LTU4 $241
line 360
;360:        SG_Error( "SG_LoadFile: file %s too long", filename );
ADDRGP4 $243
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 361
;361:    }
LABELV $241
line 362
;362:    trap_FS_Read( text, len, f );
ADDRGP4 $237
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
line 363
;363:    trap_FS_FClose( f );
ADDRLP4 4
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 365
;364:
;365:    return text;
ADDRGP4 $237
RETP4
LABELV $236
endproc SG_LoadFile 12 12
proc SG_LoadResource 8 8
line 369
;366:}
;367:
;368:static nhandle_t SG_LoadResource( const char *name, nhandle_t (*fn)( const char * ) )
;369:{
line 372
;370:    nhandle_t handle;
;371:
;372:    G_Printf( "Loading Resource %s...\n", name );
ADDRGP4 $245
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 373
;373:    handle = fn( name );
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ADDRFP4 4
INDIRP4
CALLI4
ASGNI4
ADDRLP4 0
ADDRLP4 4
INDIRI4
ASGNI4
line 374
;374:    if ( handle == FS_INVALID_HANDLE ) {
ADDRLP4 0
INDIRI4
CNSTI4 0
NEI4 $246
line 375
;375:        G_Printf( COLOR_YELLOW "WARNING: Failed to load %s.\n", name );
ADDRGP4 $248
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 376
;376:    }
LABELV $246
line 378
;377:
;378:    return handle;
ADDRLP4 0
INDIRI4
RETI4
LABELV $244
endproc SG_LoadResource 8 8
proc SG_LoadMedia 56 8
line 382
;379:}
;380:
;381:static void SG_LoadMedia( void )
;382:{
line 386
;383:    int i;
;384:    int start, end;
;385:
;386:    SG_Printf( "Loading media...\n" );
ADDRGP4 $250
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 388
;387:
;388:    start = Sys_Milliseconds();
ADDRLP4 12
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 4
ADDRLP4 12
INDIRI4
ASGNI4
line 393
;389:
;390:    //
;391:    // shaders
;392:    //
;393:    for ( i = 0; i < arraylen( sg.media.bloodSplatterShader ); i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $254
JUMPV
LABELV $251
line 394
;394:        sg.media.bloodSplatterShader[i] = SG_LoadResource( va( "gfx/bloodSplatter%i", i ), RE_RegisterShader );
ADDRGP4 $255
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 20
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 20
INDIRP4
ARGP4
ADDRGP4 RE_RegisterShader
ARGP4
ADDRLP4 24
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 sg
ADDP4
ADDRLP4 24
INDIRI4
ASGNI4
line 395
;395:    }
LABELV $252
line 393
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $254
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 4
LTU4 $251
line 396
;396:    sg.media.bulletMarkShader = SG_LoadResource( "gfx/bulletMarks", RE_RegisterShader );
ADDRGP4 $257
ARGP4
ADDRGP4 RE_RegisterShader
ARGP4
ADDRLP4 16
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+16
ADDRLP4 16
INDIRI4
ASGNI4
line 401
;397:
;398:    //
;399:    // sfx
;400:    //
;401:    sg.media.footstepsGround = SG_LoadResource( "sfx/player/footstepsGround.ogg", Snd_RegisterSfx );
ADDRGP4 $259
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 20
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+48
ADDRLP4 20
INDIRI4
ASGNI4
line 402
;402:    sg.media.footstepsMetal = SG_LoadResource( "sfx/player/footstepsMetal.ogg", Snd_RegisterSfx );
ADDRGP4 $261
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 24
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+40
ADDRLP4 24
INDIRI4
ASGNI4
line 403
;403:    sg.media.footstepsWater = SG_LoadResource( "sfx/player/footstepsWater.ogg", Snd_RegisterSfx );
ADDRGP4 $263
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 28
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+52
ADDRLP4 28
INDIRI4
ASGNI4
line 404
;404:    sg.media.footstepsWood = SG_LoadResource( "sfx/player/footstepsWood.ogg", Snd_RegisterSfx );
ADDRGP4 $265
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 32
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+44
ADDRLP4 32
INDIRI4
ASGNI4
line 406
;405:
;406:    sg.media.grappleHit = SG_LoadResource( "sfx/player/grappleHit.ogg", Snd_RegisterSfx );
ADDRGP4 $267
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 36
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+76
ADDRLP4 36
INDIRI4
ASGNI4
line 408
;407:
;408:    sg.media.bladeModeEnter = SG_LoadResource( "sfx/player/bladeModeEnter.ogg", Snd_RegisterSfx );
ADDRGP4 $269
ARGP4
ADDRGP4 Snd_RegisterSfx
ARGP4
ADDRLP4 40
ADDRGP4 SG_LoadResource
CALLI4
ASGNI4
ADDRGP4 sg+64
ADDRLP4 40
INDIRI4
ASGNI4
line 410
;409:
;410:    end = Sys_Milliseconds();
ADDRLP4 44
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 44
INDIRI4
ASGNI4
line 412
;411:
;412:    if ( sg_debugPrint.i ) {
ADDRGP4 sg_debugPrint+260
INDIRI4
CNSTI4 0
EQI4 $270
line 413
;413:        SG_Printf( "Loaded resource files in %i msec.\n", end - start );
ADDRGP4 $273
ARGP4
ADDRLP4 8
INDIRI4
ADDRLP4 4
INDIRI4
SUBI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 414
;414:    }
LABELV $270
line 420
;415:
;416:    //
;417:    // levels
;418:    //
;419:
;420:    SG_Printf( "Initializing level configs...\n" );
ADDRGP4 $274
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 422
;421:
;422:    start = Sys_Milliseconds();
ADDRLP4 48
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 4
ADDRLP4 48
INDIRI4
ASGNI4
line 423
;423:    SG_LoadLevels();
ADDRGP4 SG_LoadLevels
CALLV
pop
line 424
;424:    end = Sys_Milliseconds();
ADDRLP4 52
ADDRGP4 Sys_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 52
INDIRI4
ASGNI4
line 426
;425:
;426:    if ( sg_debugPrint.i ) {
ADDRGP4 sg_debugPrint+260
INDIRI4
CNSTI4 0
EQI4 $275
line 427
;427:        SG_Printf( "Loaded level configurations in %i msec.\n", end - start );
ADDRGP4 $278
ARGP4
ADDRLP4 8
INDIRI4
ADDRLP4 4
INDIRI4
SUBI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 428
;428:    }
LABELV $275
line 429
;429:}
LABELV $249
endproc SG_LoadMedia 56 8
export SG_Init
proc SG_Init 4 12
line 432
;430:
;431:void SG_Init( void )
;432:{
line 433
;433:    G_Printf( "---------- Game Initialization ----------\n" );
ADDRGP4 $280
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 434
;434:    G_Printf( "gamename: %s\n", GLN_VERSION );
ADDRGP4 $281
ARGP4
ADDRGP4 $109
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 435
;435:    G_Printf( "gamedate: %s\n", __DATE__ );
ADDRGP4 $282
ARGP4
ADDRGP4 $111
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 437
;436:
;437:    trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_SGAME );
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
line 440
;438:
;439:    // clear sgame state
;440:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4294172
ARGU4
ADDRGP4 memset
CALLI4
pop
line 443
;441:    
;442:    // cache redundant calculations
;443:    Sys_GetGPUConfig( &sg.gpuConfig );
ADDRGP4 sg+63828
ARGP4
ADDRGP4 Sys_GetGPUConfig
CALLV
pop
line 446
;444:
;445:    // for 1024x768 virtualized screen
;446:	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
ADDRGP4 sg+76176
ADDRGP4 sg+63828+12324
INDIRI4
CVIF4 4
CNSTF4 984263339
MULF4
ASGNF4
line 447
;447:	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
ADDRGP4 sg+63828+12320
INDIRI4
CNSTI4 768
MULI4
ADDRGP4 sg+63828+12324
INDIRI4
CNSTI4 10
LSHI4
LEI4 $287
line 449
;448:		// wide screen
;449:		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
ADDRGP4 sg+76180
ADDRGP4 sg+63828+12320
INDIRI4
CVIF4 4
ADDRGP4 sg+63828+12324
INDIRI4
CVIF4 4
CNSTF4 1068149419
MULF4
SUBF4
CNSTF4 1056964608
MULF4
ASGNF4
line 450
;450:	}
ADDRGP4 $288
JUMPV
LABELV $287
line 451
;451:	else {
line 453
;452:		// no wide screen
;453:		sg.bias = 0;
ADDRGP4 sg+76180
CNSTF4 0
ASGNF4
line 454
;454:	}
LABELV $288
line 457
;455:
;456:    // register sgame cvars
;457:    SG_RegisterCvars();
ADDRGP4 SG_RegisterCvars
CALLV
pop
line 459
;458:
;459:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 462
;460:
;461:    // load assets/resources
;462:    SG_LoadMedia();
ADDRGP4 SG_LoadMedia
CALLV
pop
line 465
;463:
;464:    // register commands
;465:    SG_InitCommands();
ADDRGP4 SG_InitCommands
CALLV
pop
line 467
;466:
;467:    sg.state = SG_INACTIVE;
ADDRGP4 sg+2212
CNSTI4 2
ASGNI4
line 469
;468:
;469:    G_Printf( "-----------------------------------\n" );
ADDRGP4 $300
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 470
;470:}
LABELV $279
endproc SG_Init 4 12
export SG_Shutdown
proc SG_Shutdown 0 12
line 473
;471:
;472:void SG_Shutdown( void )
;473:{
line 474
;474:    G_Printf( "Shutting down sgame...\n" );
ADDRGP4 $302
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 476
;475:
;476:    SG_ShutdownCommands();
ADDRGP4 SG_ShutdownCommands
CALLV
pop
line 478
;477:
;478:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4294172
ARGU4
ADDRGP4 memset
CALLI4
pop
line 480
;479:
;480:    sg.state = SG_INACTIVE;
ADDRGP4 sg+2212
CNSTI4 2
ASGNI4
line 481
;481:}
LABELV $301
endproc SG_Shutdown 0 12
export SaveGame
proc SaveGame 0 4
line 484
;482:
;483:void SaveGame( void )
;484:{
line 485
;485:    G_Printf( "Saving game...\n" );
ADDRGP4 $305
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 487
;486:
;487:    SG_SaveLevelData();
ADDRGP4 SG_SaveLevelData
CALLV
pop
line 489
;488:
;489:    G_Printf( "Done" );
ADDRGP4 $306
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 490
;490:}
LABELV $304
endproc SaveGame 0 4
export LoadGame
proc LoadGame 64 12
line 493
;491:
;492:void LoadGame( void )
;493:{
line 496
;494:    char savename[MAX_NPATH];
;495:
;496:    Cvar_VariableStringBuffer( "sg_savename", savename, sizeof(savename) );
ADDRGP4 $139
ARGP4
ADDRLP4 0
ARGP4
CNSTI4 64
ARGI4
ADDRGP4 Cvar_VariableStringBuffer
CALLV
pop
line 498
;497:
;498:    G_Printf( "Loading save file '%s'...\n", savename );
ADDRGP4 $308
ARGP4
ADDRLP4 0
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 500
;499:
;500:    SG_LoadLevelData();
ADDRGP4 SG_LoadLevelData
CALLV
pop
line 501
;501:}
LABELV $307
endproc LoadGame 64 12
export trap_FS_Printf
proc trap_FS_Printf 8200 12
line 504
;502:
;503:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( fileHandle_t f, const char *fmt, ... )
;504:{
line 508
;505:    va_list argptr;
;506:    char msg[MAXPRINTMSG];
;507:
;508:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 509
;509:    vsprintf( msg, fmt, argptr );
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
line 510
;510:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 512
;511:
;512:    trap_FS_Write( msg, strlen( msg ), f );
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
line 513
;513:}
LABELV $309
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
import SG_LoadLevels
import SG_LoadLevelData
import SG_SaveLevelData
import SG_EndLevel
import SG_StartLevel
import SG_ShutdownCommands
import SG_InitCommands
import SGameCommand
import SG_DrawFrame
export sgc_godmode
align 4
LABELV sgc_godmode
skip 276
export sg_cheatsOn
align 4
LABELV sg_cheatsOn
skip 276
export sgc_deafMobs
align 4
LABELV sgc_deafMobs
skip 276
export sgc_blindMobs
align 4
LABELV sgc_blindMobs
skip 276
export sgc_infiniteAmmo
align 4
LABELV sgc_infiniteAmmo
skip 276
export sgc_infiniteRage
align 4
LABELV sgc_infiniteRage
skip 276
export sgc_infiniteHealth
align 4
LABELV sgc_infiniteHealth
skip 276
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
export sg_gameDifficulty
align 4
LABELV sg_gameDifficulty
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
skip 4294172
import sg_entities
import sg_activeEnts
import sg_freeEnts
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
LABELV $308
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
LABELV $306
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 0
align 1
LABELV $305
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
LABELV $302
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
LABELV $300
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
LABELV $282
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
LABELV $281
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
LABELV $280
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
LABELV $278
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 101
byte 1 100
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
byte 1 117
byte 1 114
byte 1 97
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 115
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 109
byte 1 115
byte 1 101
byte 1 99
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $274
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 105
byte 1 97
byte 1 108
byte 1 105
byte 1 122
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
LABELV $273
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 101
byte 1 100
byte 1 32
byte 1 114
byte 1 101
byte 1 115
byte 1 111
byte 1 117
byte 1 114
byte 1 99
byte 1 101
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 115
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 109
byte 1 115
byte 1 101
byte 1 99
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $269
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
byte 1 98
byte 1 108
byte 1 97
byte 1 100
byte 1 101
byte 1 77
byte 1 111
byte 1 100
byte 1 101
byte 1 69
byte 1 110
byte 1 116
byte 1 101
byte 1 114
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $267
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
byte 1 103
byte 1 114
byte 1 97
byte 1 112
byte 1 112
byte 1 108
byte 1 101
byte 1 72
byte 1 105
byte 1 116
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $265
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
byte 1 102
byte 1 111
byte 1 111
byte 1 116
byte 1 115
byte 1 116
byte 1 101
byte 1 112
byte 1 115
byte 1 87
byte 1 111
byte 1 111
byte 1 100
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $263
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
byte 1 102
byte 1 111
byte 1 111
byte 1 116
byte 1 115
byte 1 116
byte 1 101
byte 1 112
byte 1 115
byte 1 87
byte 1 97
byte 1 116
byte 1 101
byte 1 114
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $261
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
byte 1 102
byte 1 111
byte 1 111
byte 1 116
byte 1 115
byte 1 116
byte 1 101
byte 1 112
byte 1 115
byte 1 77
byte 1 101
byte 1 116
byte 1 97
byte 1 108
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $259
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
byte 1 102
byte 1 111
byte 1 111
byte 1 116
byte 1 115
byte 1 116
byte 1 101
byte 1 112
byte 1 115
byte 1 71
byte 1 114
byte 1 111
byte 1 117
byte 1 110
byte 1 100
byte 1 46
byte 1 111
byte 1 103
byte 1 103
byte 1 0
align 1
LABELV $257
byte 1 103
byte 1 102
byte 1 120
byte 1 47
byte 1 98
byte 1 117
byte 1 108
byte 1 108
byte 1 101
byte 1 116
byte 1 77
byte 1 97
byte 1 114
byte 1 107
byte 1 115
byte 1 0
align 1
LABELV $255
byte 1 103
byte 1 102
byte 1 120
byte 1 47
byte 1 98
byte 1 108
byte 1 111
byte 1 111
byte 1 100
byte 1 83
byte 1 112
byte 1 108
byte 1 97
byte 1 116
byte 1 116
byte 1 101
byte 1 114
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $250
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 109
byte 1 101
byte 1 100
byte 1 105
byte 1 97
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $248
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
byte 1 70
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
byte 1 37
byte 1 115
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $245
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 82
byte 1 101
byte 1 115
byte 1 111
byte 1 117
byte 1 114
byte 1 99
byte 1 101
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $243
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 70
byte 1 105
byte 1 108
byte 1 101
byte 1 58
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 37
byte 1 115
byte 1 32
byte 1 116
byte 1 111
byte 1 111
byte 1 32
byte 1 108
byte 1 111
byte 1 110
byte 1 103
byte 1 0
align 1
LABELV $240
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 70
byte 1 105
byte 1 108
byte 1 101
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
byte 1 111
byte 1 112
byte 1 101
byte 1 110
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $234
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
LABELV $197
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
LABELV $188
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
LABELV $183
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
LABELV $178
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
LABELV $173
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
LABELV $168
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
LABELV $149
byte 1 115
byte 1 103
byte 1 95
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 115
byte 1 79
byte 1 110
byte 1 0
align 1
LABELV $148
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 100
byte 1 101
byte 1 97
byte 1 102
byte 1 77
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $147
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 98
byte 1 108
byte 1 105
byte 1 110
byte 1 100
byte 1 77
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $146
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 103
byte 1 111
byte 1 100
byte 1 109
byte 1 111
byte 1 100
byte 1 101
byte 1 0
align 1
LABELV $145
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 82
byte 1 97
byte 1 103
byte 1 101
byte 1 0
align 1
LABELV $144
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 65
byte 1 109
byte 1 109
byte 1 111
byte 1 0
align 1
LABELV $143
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 72
byte 1 101
byte 1 97
byte 1 108
byte 1 116
byte 1 104
byte 1 0
align 1
LABELV $142
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
LABELV $141
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
LABELV $140
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
LABELV $139
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
LABELV $138
byte 1 50
byte 1 0
align 1
LABELV $137
byte 1 115
byte 1 103
byte 1 95
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 68
byte 1 105
byte 1 102
byte 1 102
byte 1 105
byte 1 99
byte 1 117
byte 1 108
byte 1 116
byte 1 121
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
byte 1 52
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
