export vmMain
code
proc vmMain 16 12
file "../sg_main.c"
line 17
;1:#include "../engine/n_shared.h"
;2:#include "sg_local.h"
;3:
;4:void SG_Init( void );
;5:void SG_Shutdown( void );
;6:int SG_RunLoop( int levelTime, int frameTime );
;7:int SG_DrawFrame( void );
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
LTI4 $92
ADDRLP4 0
INDIRI4
CNSTI4 13
GTI4 $92
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
address $93
address $94
address $102
address $97
address $92
address $92
address $103
address $104
address $100
address $99
address $98
address $101
address $101
address $95
code
LABELV $93
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
ADDRGP4 $90
JUMPV
LABELV $94
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
ADDRGP4 $90
JUMPV
LABELV $95
line 26
;25:    case SGAME_GET_STATE:
;26:        return sg.state;
ADDRGP4 sg+64
INDIRI4
RETI4
ADDRGP4 $90
JUMPV
LABELV $97
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
ADDRGP4 $90
JUMPV
LABELV $98
line 30
;29:    case SGAME_SEND_USER_CMD:
;30:        SG_SendUserCmd( arg0, arg1, arg2 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRFP4 8
INDIRI4
ARGI4
ADDRFP4 12
INDIRI4
ARGI4
ADDRGP4 SG_SendUserCmd
CALLV
pop
line 31
;31:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $90
JUMPV
LABELV $99
line 33
;32:    case SGAME_MOUSE_EVENT:
;33:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $90
JUMPV
LABELV $100
line 35
;34:    case SGAME_KEY_EVENT:
;35:        SG_KeyEvent( arg0, arg1 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRFP4 8
INDIRI4
ARGI4
ADDRGP4 SG_KeyEvent
CALLV
pop
line 36
;36:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $90
JUMPV
LABELV $101
line 39
;37:    case SGAME_EVENT_HANDLING:
;38:    case SGAME_EVENT_NONE:
;39:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $90
JUMPV
LABELV $102
line 41
;40:    case SGAME_LOADLEVEL:
;41:        return SG_InitLevel( arg0 );
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
ADDRGP4 $90
JUMPV
LABELV $103
line 43
;42:    case SGAME_CONSOLE_COMMAND:
;43:        SGameCommand();
ADDRGP4 SGameCommand
CALLV
pop
line 44
;44:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $90
JUMPV
LABELV $104
line 46
;45:    case SGAME_RUNTIC:
;46:        return SG_RunLoop( arg0, arg1 );
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
ADDRGP4 $90
JUMPV
line 48
;47:    default:
;48:        break;
LABELV $92
line 49
;49:    };
line 51
;50:
;51:    SG_Error( "vmMain: unrecognized command %i", command );
ADDRGP4 $106
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 52
;52:    return -1;
CNSTI4 -1
RETI4
LABELV $90
endproc vmMain 16 12
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
address sg_levelInfoFile
address $137
address $138
byte 4 33
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
align 4
LABELV cvarTableSize
byte 4 20
code
proc SG_RegisterCvars 16 16
line 118
;53:}
;54:
;55:sgGlobals_t sg;
;56:
;57:vmCvar_t sg_printEntities;
;58:vmCvar_t sg_debugPrint;
;59:vmCvar_t sg_paused;
;60:vmCvar_t sg_mouseInvert;
;61:vmCvar_t sg_mouseAcceleration;
;62:vmCvar_t sg_printLevelStats;
;63:vmCvar_t sg_decalDetail;
;64:vmCvar_t sg_gibs;
;65:vmCvar_t sg_levelInfoFile;
;66:vmCvar_t sg_levelIndex;
;67:vmCvar_t sg_levelDataFile;
;68:vmCvar_t sg_savename;
;69:vmCvar_t sg_numSaves;
;70:
;71:vmCvar_t pm_groundFriction;
;72:vmCvar_t pm_waterFriction;
;73:vmCvar_t pm_airFriction;
;74:vmCvar_t pm_waterAccel;
;75:vmCvar_t pm_baseAccel;
;76:vmCvar_t pm_baseSpeed;
;77:vmCvar_t pm_airAccel;
;78:vmCvar_t pm_wallrunAccelVertical;
;79:vmCvar_t pm_wallrunAccelMove;
;80:vmCvar_t pm_wallTime;
;81:
;82:typedef struct {
;83:    vmCvar_t *vmCvar;
;84:    const char *cvarName;
;85:    const char *defaultValue;
;86:    int cvarFlags;
;87:    int modificationCount; // for tracking changes
;88:    qboolean trackChange;       // track this variable, and announce if changed
;89:} cvarTable_t;
;90:
;91:static cvarTable_t cvarTable[] = {
;92:    // noset vars
;93:    { NULL,                     "gamename",             GLN_VERSION,    CVAR_ROM,                   0, qfalse },
;94:    { NULL,                     "gamedate",             __DATE__,       CVAR_ROM,                   0, qfalse },
;95:    { &sg_printEntities,        "sg_printEntities",     "0",            0,                          0, qfalse },
;96:    { &sg_debugPrint,           "sg_debugPrint",        "1",            CVAR_TEMP,                  0, qfalse },
;97:    { &sg_paused,               "g_paused",             "1",            CVAR_TEMP | CVAR_LATCH,     0, qfalse },
;98:    { &pm_groundFriction,       "pm_groundFriction",    "0.6f",         CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;99:    { &pm_waterFriction,        "pm_waterFriction",     "0.06f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;100:    { &pm_airFriction,          "pm_airFriction",       "0.01f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;101:    { &pm_airAccel,             "pm_airAccel",          "1.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;102:    { &pm_waterAccel,           "pm_waterAccel",        "0.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;103:    { &pm_baseAccel,            "pm_baseAccel",         "1.0f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;104:    { &pm_baseSpeed,            "pm_baseSpeed",         "0.02f",        CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;105:    { &sg_mouseInvert,          "g_mouseInvert",        "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;106:    { &sg_mouseAcceleration,    "g_mouseAcceleration",  "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;107:    { &sg_printLevelStats,      "sg_printLevelStats",   "1",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;108:    { &sg_decalDetail,          "sg_decalDetail",       "3",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;109:    { &sg_gibs,                 "sg_gibs",              "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;110:    { &sg_levelInfoFile,        "sg_levelInfoFile",     "levels.txt",   CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;111:    { &sg_savename,             "sg_savename",          "savedata",     CVAR_LATCH | CVAR_SAVE,     0, qtrue },
;112:    { &sg_numSaves,             "sg_numSaves",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
;113:};
;114:
;115:static const int cvarTableSize = arraylen(cvarTable);
;116:
;117:static void SG_RegisterCvars( void )
;118:{
line 122
;119:    int i;
;120:    cvarTable_t *cv;
;121:
;122:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $146
JUMPV
LABELV $143
line 123
;123:        Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultValue, cv->cvarFlags );
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
line 124
;124:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $147
line 125
;125:            cv->modificationCount = cv->vmCvar->modificationCount;
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
line 126
;126:        }
LABELV $147
line 127
;127:    }
LABELV $144
line 122
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
LABELV $146
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $143
line 128
;128:}
LABELV $142
endproc SG_RegisterCvars 16 16
export SG_UpdateCvars
proc SG_UpdateCvars 24 12
line 131
;129:
;130:void SG_UpdateCvars( void )
;131:{
line 135
;132:    int i;
;133:    cvarTable_t *cv;
;134:
;135:    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRLP4 0
ADDRGP4 cvarTable
ASGNP4
ADDRGP4 $153
JUMPV
LABELV $150
line 136
;136:        if ( cv->vmCvar ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $154
line 137
;137:            Cvar_Update( cv->vmCvar );
ADDRLP4 0
INDIRP4
INDIRP4
ARGP4
ADDRGP4 Cvar_Update
CALLV
pop
line 139
;138:
;139:            if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
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
EQU4 $156
line 140
;140:                cv->modificationCount = cv->vmCvar->modificationCount;
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
line 142
;141:
;142:                if ( cv->trackChange ) {
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
INDIRI4
CNSTI4 0
EQI4 $158
line 143
;143:                    trap_SendConsoleCommand( va( "Changed \"%s\" to \"%s\"", cv->cvarName, cv->vmCvar->s ) );
ADDRGP4 $160
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
line 144
;144:                }
LABELV $158
line 145
;145:            }
LABELV $156
line 146
;146:        }
LABELV $154
line 147
;147:    }
LABELV $151
line 135
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
LABELV $153
ADDRLP4 4
INDIRI4
ADDRGP4 cvarTableSize
INDIRI4
LTI4 $150
line 148
;148:}
LABELV $149
endproc SG_UpdateCvars 24 12
export G_Printf
proc G_Printf 4108 12
line 151
;149:
;150:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
;151:{
line 156
;152:    va_list argptr;
;153:    char msg[4096];
;154:    int32_t length;
;155:
;156:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 157
;157:    length = vsprintf(msg, fmt, argptr);
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
line 158
;158:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 160
;159:
;160:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 161
;161:}
LABELV $161
endproc G_Printf 4108 12
export G_Error
proc G_Error 4108 12
line 164
;162:
;163:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
;164:{
line 169
;165:    va_list argptr;
;166:    char msg[4096];
;167:    int32_t length;
;168:
;169:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 170
;170:    length = vsprintf(msg, err, argptr);
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
line 171
;171:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 173
;172:
;173:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 174
;174:}
LABELV $163
endproc G_Error 4108 12
export SG_Printf
proc SG_Printf 4108 12
line 177
;175:
;176:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
;177:{
line 182
;178:    va_list argptr;
;179:    char msg[4096];
;180:    int32_t length;
;181:
;182:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 183
;183:    length = vsprintf(msg, fmt, argptr);
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
line 184
;184:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 186
;185:
;186:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $167
line 187
;187:        trap_Error( "SG_Printf: buffer overflow" );
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
;190:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 191
;191:}
LABELV $165
endproc SG_Printf 4108 12
export SG_Error
proc SG_Error 4108 12
line 194
;192:
;193:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error(const char *err, ...)
;194:{
line 199
;195:    va_list argptr;
;196:    char msg[4096];
;197:    int32_t length;
;198:
;199:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 200
;200:    length = vsprintf(msg, err, argptr);
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
line 201
;201:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 203
;202:
;203:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $172
line 204
;204:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $169
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 205
;205:    }
LABELV $172
line 207
;206:
;207:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 208
;208:}
LABELV $170
endproc SG_Error 4108 12
export N_Error
proc N_Error 4108 12
line 211
;209:
;210:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
;211:{
line 216
;212:    va_list argptr;
;213:    char msg[4096];
;214:    int32_t length;
;215:
;216:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 217
;217:    length = vsprintf(msg, err, argptr);
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
line 218
;218:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 220
;219:
;220:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $176
line 221
;221:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $169
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 222
;222:    }
LABELV $176
line 224
;223:
;224:    SG_Error("%s", msg);
ADDRGP4 $178
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 225
;225:}
LABELV $174
endproc N_Error 4108 12
export Con_Printf
proc Con_Printf 4108 12
line 231
;226:
;227://#ifndef SGAME_HARD_LINKED
;228:// this is only here so the functions in n_shared.c and bg_*.c can link
;229:
;230:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
;231:{
line 236
;232:    va_list argptr;
;233:    char msg[4096];
;234:    int32_t length;
;235:
;236:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 237
;237:    length = vsprintf(msg, fmt, argptr);
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
line 238
;238:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 240
;239:
;240:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $181
line 241
;241:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $169
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 242
;242:    }
LABELV $181
line 244
;243:
;244:    SG_Printf("%s", msg);
ADDRGP4 $178
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 245
;245:}
LABELV $179
endproc Con_Printf 4108 12
export SG_RunLoop
proc SG_RunLoop 44 12
line 250
;246:
;247://#endif
;248:
;249:int SG_RunLoop( int levelTime, int frameTime )
;250:{
line 256
;251:    int i;
;252:    int start, end;
;253:    int msec;
;254:    sgentity_t *ent;
;255:
;256:    if ( sg.state == SG_INACTIVE ) {
ADDRGP4 sg+64
INDIRI4
CNSTI4 0
NEI4 $184
line 257
;257:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $183
JUMPV
LABELV $184
line 261
;258:    }
;259:
;260:    // get any cvar changes
;261:    SG_UpdateCvars();
ADDRGP4 SG_UpdateCvars
CALLV
pop
line 264
;262:
;263:    // even if the game is paused, we still render everything in the background
;264:    if ( sg.state == SG_SHOW_LEVEL_STATS ) {
ADDRGP4 sg+64
INDIRI4
CNSTI4 2
NEI4 $187
line 265
;265:        SG_DrawLevelStats();
ADDRGP4 SG_DrawLevelStats
CALLV
pop
line 266
;266:        return 1; // we don't draw the level if we're ending it
CNSTI4 1
RETI4
ADDRGP4 $183
JUMPV
LABELV $187
line 269
;267:    }
;268:
;269:    if ( sg_paused.i ) {
ADDRGP4 sg_paused+260
INDIRI4
CNSTI4 0
EQI4 $190
line 270
;270:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $183
JUMPV
LABELV $190
line 273
;271:    }
;272:
;273:    sg.framenum++;
ADDRLP4 20
ADDRGP4 sg+92
ASGNP4
ADDRLP4 20
INDIRP4
ADDRLP4 20
INDIRP4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
line 274
;274:    sg.previousTime = sg.framenum;
ADDRGP4 sg+84
ADDRGP4 sg+92
INDIRI4
ASGNI4
line 275
;275:    sg.levelTime = levelTime;
ADDRGP4 sg+88
ADDRFP4 0
INDIRI4
ASGNI4
line 276
;276:    msec = sg.levelTime - sg.previousTime;
ADDRLP4 16
ADDRGP4 sg+88
INDIRI4
ADDRGP4 sg+84
INDIRI4
SUBI4
ASGNI4
line 284
;277:
;278:    // build player's movement command
;279://    SG_BuildMoveCommand();
;280:
;281:    //
;282:    // go through all allocated entities
;283:    //
;284:    start = trap_Milliseconds();
ADDRLP4 24
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 8
ADDRLP4 24
INDIRI4
ASGNI4
line 285
;285:    ent = &sg_entities[0];
ADDRLP4 0
ADDRGP4 sg_entities
ASGNP4
line 286
;286:    for ( i = 0; i < sg.numEntities; i++) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $202
JUMPV
LABELV $199
line 287
;287:        if ( !ent->health ) {
ADDRLP4 0
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CNSTI4 0
NEI4 $204
line 288
;288:            continue;
ADDRGP4 $200
JUMPV
LABELV $204
line 291
;289:        }
;290:
;291:        ent->ticker--;
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
line 293
;292:
;293:        if ( ent->ticker <= -1 ) {
ADDRLP4 0
INDIRP4
CNSTI4 116
ADDP4
INDIRI4
CNSTI4 -1
GTI4 $206
line 294
;294:            Ent_SetState( ent, ent->state->nextstate );
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
line 295
;295:            continue;
ADDRGP4 $200
JUMPV
LABELV $206
line 299
;296:        }
;297:
;298:        // update the current entity's animation frame
;299:        if ( ent->state->frames > 0 ) {
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
CNSTU4 0
EQU4 $208
line 300
;300:            if ( ent->frame == ent->state->frames ) {
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
NEU4 $210
line 301
;301:                ent->frame = 0; // reset animation
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
CNSTI4 0
ASGNI4
line 302
;302:            }
ADDRGP4 $211
JUMPV
LABELV $210
line 303
;303:            else if ( ent->ticker % ent->state->frames ) {
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
EQU4 $212
line 304
;304:                ent->frame++;
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
line 305
;305:            }
LABELV $212
LABELV $211
line 306
;306:        }
LABELV $208
line 308
;307:
;308:        ent->state->action.acp1( ent );
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
line 309
;309:    }
LABELV $200
line 286
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $202
ADDRLP4 4
INDIRI4
ADDRGP4 sg+76
INDIRI4
LTI4 $199
line 310
;310:    end = trap_Milliseconds();
ADDRLP4 28
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRLP4 12
ADDRLP4 28
INDIRI4
ASGNI4
line 312
;311:
;312:    SG_DrawFrame();
ADDRGP4 SG_DrawFrame
CALLI4
pop
line 314
;313:
;314:    if ( sg_printEntities.i ) {
ADDRGP4 sg_printEntities+260
INDIRI4
CNSTI4 0
EQI4 $214
line 315
;315:        for ( i = 0; i < sg.numEntities; i++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $220
JUMPV
LABELV $217
line 316
;316:            G_Printf( "%4i: %s\n", i, sg_entities[i].classname );
ADDRGP4 $222
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
line 317
;317:        }
LABELV $218
line 315
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $220
ADDRLP4 4
INDIRI4
ADDRGP4 sg+76
INDIRI4
LTI4 $217
line 318
;318:        Cvar_Set( "sg_printEntities", "0" );
ADDRGP4 $112
ARGP4
ADDRGP4 $113
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 319
;319:    }
LABELV $214
line 321
;320:
;321:    return 1;
CNSTI4 1
RETI4
LABELV $183
endproc SG_RunLoop 44 12
proc SG_LoadMedia 44 20
line 326
;322:}
;323:
;324:
;325:static void SG_LoadMedia( void )
;326:{
line 327
;327:    sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" );
ADDRGP4 $226
ARGP4
ADDRLP4 0
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+12
ADDRLP4 0
INDIRI4
ASGNI4
line 328
;328:    sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" );
ADDRGP4 $228
ARGP4
ADDRLP4 4
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+16
ADDRLP4 4
INDIRI4
ASGNI4
line 329
;329:    sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" ); 
ADDRGP4 $230
ARGP4
ADDRLP4 8
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+20
ADDRLP4 8
INDIRI4
ASGNI4
line 330
;330:    sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" );
ADDRGP4 $231
ARGP4
ADDRLP4 12
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg
ADDRLP4 12
INDIRI4
ASGNI4
line 331
;331:    sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" );
ADDRGP4 $233
ARGP4
ADDRLP4 16
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+4
ADDRLP4 16
INDIRI4
ASGNI4
line 332
;332:    sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" );
ADDRGP4 $235
ARGP4
ADDRLP4 20
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+8
ADDRLP4 20
INDIRI4
ASGNI4
line 333
;333:    sg.media.revolver_fire = trap_Snd_RegisterSfx( "sfx/weapons/revolver_fire.wav" );
ADDRGP4 $237
ARGP4
ADDRLP4 24
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+28
ADDRLP4 24
INDIRI4
ASGNI4
line 334
;334:    sg.media.revolver_rld = trap_Snd_RegisterSfx( "sfx/weapons/revolver_rld.wav" );
ADDRGP4 $239
ARGP4
ADDRLP4 28
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+32
ADDRLP4 28
INDIRI4
ASGNI4
line 336
;335:
;336:    sg.media.raio_shader = RE_RegisterShader( "textures/sprites/glnomad_raio_base.png" );
ADDRGP4 $241
ARGP4
ADDRLP4 32
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+36
ADDRLP4 32
INDIRI4
ASGNI4
line 337
;337:    sg.media.grunt_shader = RE_RegisterShader( "textures/sprites/glnomad_grunt.png" );
ADDRGP4 $243
ARGP4
ADDRLP4 36
ADDRGP4 RE_RegisterShader
CALLI4
ASGNI4
ADDRGP4 sg+40
ADDRLP4 36
INDIRI4
ASGNI4
line 339
;338:
;339:    sg.media.raio_sprites = RE_RegisterSpriteSheet( "textures/sprites/glnomad_raio_base.png", 512, 512, 32, 32 );
ADDRGP4 $241
ARGP4
CNSTI4 512
ARGI4
CNSTI4 512
ARGI4
CNSTI4 32
ARGI4
CNSTI4 32
ARGI4
ADDRLP4 40
ADDRGP4 RE_RegisterSpriteSheet
CALLI4
ASGNI4
ADDRGP4 sg+48
ADDRLP4 40
INDIRI4
ASGNI4
line 340
;340:}
LABELV $224
endproc SG_LoadMedia 44 20
export SG_Init
proc SG_Init 4 12
line 343
;341:
;342:void SG_Init( void )
;343:{
line 344
;344:    G_Printf( "---------- Game Initialization ----------\n" );
ADDRGP4 $246
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 345
;345:    G_Printf( "gamename: %s\n", GLN_VERSION );
ADDRGP4 $247
ARGP4
ADDRGP4 $109
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 346
;346:    G_Printf( "gamedate: %s\n", __DATE__ );
ADDRGP4 $248
ARGP4
ADDRGP4 $111
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 348
;347:
;348:    trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_SGAME );
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
line 351
;349:
;350:    // clear sgame state
;351:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4307576
ARGU4
ADDRGP4 memset
CALLP4
pop
line 354
;352:    
;353:    // cache redundant calculations
;354:    Sys_GetGPUConfig( &sg.gpuConfig );
ADDRGP4 sg+73968
ARGP4
ADDRGP4 Sys_GetGPUConfig
CALLV
pop
line 357
;355:
;356:    // for 1024x768 virtualized screen
;357:	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
ADDRGP4 sg+86316
ADDRGP4 sg+73968+12312
INDIRI4
CVIF4 4
CNSTF4 984263339
MULF4
ASGNF4
line 358
;358:	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
ADDRGP4 sg+73968+12308
INDIRI4
CNSTI4 768
MULI4
ADDRGP4 sg+73968+12312
INDIRI4
CNSTI4 10
LSHI4
LEI4 $253
line 360
;359:		// wide screen
;360:		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
ADDRGP4 sg+86320
ADDRGP4 sg+73968+12308
INDIRI4
CVIF4 4
ADDRGP4 sg+73968+12312
INDIRI4
CVIF4 4
CNSTF4 1068149419
MULF4
SUBF4
CNSTF4 1056964608
MULF4
ASGNF4
line 361
;361:	}
ADDRGP4 $254
JUMPV
LABELV $253
line 362
;362:	else {
line 364
;363:		// no wide screen
;364:		sg.bias = 0;
ADDRGP4 sg+86320
CNSTF4 0
ASGNF4
line 365
;365:	}
LABELV $254
line 368
;366:
;367:    // register sgame cvars
;368:    SG_RegisterCvars();
ADDRGP4 SG_RegisterCvars
CALLV
pop
line 371
;369:
;370:    // load assets/resources
;371:    SG_LoadMedia();
ADDRGP4 SG_LoadMedia
CALLV
pop
line 373
;372:
;373:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 375
;374:
;375:    sg.state = SG_INACTIVE;
ADDRGP4 sg+64
CNSTI4 0
ASGNI4
line 377
;376:
;377:    G_Printf( "-----------------------------------\n" );
ADDRGP4 $266
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 378
;378:}
LABELV $245
endproc SG_Init 4 12
export SG_Shutdown
proc SG_Shutdown 0 12
line 381
;379:
;380:void SG_Shutdown( void )
;381:{
line 382
;382:    G_Printf( "Shutting down sgame...\n" );
ADDRGP4 $268
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 384
;383:
;384:    memset( &sg, 0, sizeof(sg) );
ADDRGP4 sg
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4307576
ARGU4
ADDRGP4 memset
CALLP4
pop
line 386
;385:
;386:    sg.state = SG_INACTIVE;
ADDRGP4 sg+64
CNSTI4 0
ASGNI4
line 387
;387:}
LABELV $267
endproc SG_Shutdown 0 12
export trap_FS_Printf
proc trap_FS_Printf 8200 12
line 390
;388:
;389:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... )
;390:{
line 394
;391:    va_list argptr;
;392:    char msg[MAXPRINTMSG];
;393:
;394:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 395
;395:    vsprintf( msg, fmt, argptr );
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
line 396
;396:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 398
;397:
;398:    trap_FS_Write( msg, strlen(msg), f );
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
CVUI4 4
ARGI4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 trap_FS_Write
CALLI4
pop
line 399
;399:}
LABELV $270
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
import trap_FS_FileSeek
import trap_FS_GetFileList
import trap_FS_Read
import trap_FS_Write
import trap_FS_FClose
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
import G_LoadMap
import G_SetCameraData
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
skip 4307576
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
import N_crandom
import N_random
import N_rand
import N_fabs
import N_acos
import N_log2
import ColorBytes4
import ColorBytes3
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
LABELV $268
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
LABELV $266
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
LABELV $248
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
LABELV $247
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
LABELV $246
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
LABELV $243
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
LABELV $241
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
LABELV $239
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
LABELV $237
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
LABELV $235
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
LABELV $233
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
LABELV $231
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
LABELV $230
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
LABELV $228
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
LABELV $226
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
LABELV $222
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
LABELV $178
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $169
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
LABELV $160
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
LABELV $137
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
byte 1 68
byte 1 101
byte 1 99
byte 1 32
byte 1 50
byte 1 53
byte 1 32
byte 1 50
byte 1 48
byte 1 50
byte 1 51
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
