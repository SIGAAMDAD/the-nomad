export vmMain
code
proc vmMain 20 8
file "../sg_main.c"
line 18
;1:#include "../engine/n_shared.h"
;2:#include "sg_local.h"
;3:
;4:int32_t SG_Init(void);
;5:int32_t SG_Shutdown(void);
;6:int32_t SG_RunLoop(void);
;7:
;8:sgGlobals_t sg;
;9:
;10:/*
;11:vmMain
;12:
;13:this is the only way control passes into the module.
;14:this must be the very first function compiled into the .qvm file
;15:*/
;16:int32_t vmMain(int32_t command, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, int32_t arg6, int32_t arg7,
;17:    int32_t arg8, int32_t arg9, int32_t arg10)
;18:{
line 19
;19:    switch (command) {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 0
LTI4 $81
ADDRLP4 0
INDIRI4
CNSTI4 6
GTI4 $81
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $88
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $88
address $83
address $81
address $84
address $81
address $81
address $85
address $86
code
LABELV $83
line 21
;20:    case SGAME_INIT:
;21:        return SG_Init();
ADDRLP4 4
ADDRGP4 SG_Init
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
RETI4
ADDRGP4 $80
JUMPV
LABELV $84
line 23
;22:    case SGAME_SHUTDOWN:
;23:        return SG_Shutdown();
ADDRLP4 8
ADDRGP4 SG_Shutdown
CALLI4
ASGNI4
ADDRLP4 8
INDIRI4
RETI4
ADDRGP4 $80
JUMPV
LABELV $85
line 25
;24:    case SGAME_STARTLEVEL:
;25:        return SG_InitLevel( arg0 );
ADDRFP4 4
INDIRI4
ARGI4
ADDRLP4 12
ADDRGP4 SG_InitLevel
CALLI4
ASGNI4
ADDRLP4 12
INDIRI4
RETI4
ADDRGP4 $80
JUMPV
LABELV $86
line 27
;26:    case SGAME_ENDLEVEL:
;27:        return SG_EndLevel();
ADDRLP4 16
ADDRGP4 SG_EndLevel
CALLI4
ASGNI4
ADDRLP4 16
INDIRI4
RETI4
ADDRGP4 $80
JUMPV
LABELV $81
line 29
;28:    default:
;29:        SG_Error("vmMain: invalid command id: %i", command);
ADDRGP4 $87
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 30
;30:        break;
LABELV $82
line 31
;31:    };
line 32
;32:    return -1;
CNSTI4 -1
RETI4
LABELV $80
endproc vmMain 20 8
export G_Printf
proc G_Printf 4108 12
line 36
;33:}
;34:
;35:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
;36:{
line 41
;37:    va_list argptr;
;38:    char msg[4096];
;39:    int32_t length;
;40:
;41:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 42
;42:    length = vsprintf(msg, fmt, argptr);
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
line 43
;43:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 45
;44:
;45:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 46
;46:}
LABELV $89
endproc G_Printf 4108 12
export G_Error
proc G_Error 4108 12
line 49
;47:
;48:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
;49:{
line 54
;50:    va_list argptr;
;51:    char msg[4096];
;52:    int32_t length;
;53:
;54:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 55
;55:    length = vsprintf(msg, err, argptr);
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
line 56
;56:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 58
;57:
;58:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 59
;59:}
LABELV $91
endproc G_Error 4108 12
export SG_Printf
proc SG_Printf 4108 12
line 62
;60:
;61:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
;62:{
line 67
;63:    va_list argptr;
;64:    char msg[4096];
;65:    int32_t length;
;66:
;67:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 68
;68:    length = vsprintf(msg, fmt, argptr);
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
line 69
;69:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 71
;70:
;71:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $95
line 72
;72:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $97
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 73
;73:    }
LABELV $95
line 75
;74:
;75:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 76
;76:}
LABELV $93
endproc SG_Printf 4108 12
export SG_Error
proc SG_Error 4108 12
line 79
;77:
;78:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error(const char *err, ...)
;79:{
line 84
;80:    va_list argptr;
;81:    char msg[4096];
;82:    int32_t length;
;83:
;84:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 85
;85:    length = vsprintf(msg, err, argptr);
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
line 86
;86:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 88
;87:
;88:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $100
line 89
;89:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $97
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 90
;90:    }
LABELV $100
line 92
;91:
;92:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 93
;93:}
LABELV $98
endproc SG_Error 4108 12
export N_Error
proc N_Error 4108 12
line 96
;94:
;95:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
;96:{
line 101
;97:    va_list argptr;
;98:    char msg[4096];
;99:    int32_t length;
;100:
;101:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 102
;102:    length = vsprintf(msg, err, argptr);
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
line 103
;103:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 105
;104:
;105:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $104
line 106
;106:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $97
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 107
;107:    }
LABELV $104
line 109
;108:
;109:    SG_Error("%s", msg);
ADDRGP4 $106
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 110
;110:}
LABELV $102
endproc N_Error 4108 12
export Con_Printf
proc Con_Printf 4108 12
line 116
;111:
;112://#ifndef SGAME_HARD_LINKED
;113:// this is only here so the functions in n_shared.c and bg_*.c can link
;114:
;115:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
;116:{
line 121
;117:    va_list argptr;
;118:    char msg[4096];
;119:    int32_t length;
;120:
;121:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 122
;122:    length = vsprintf(msg, fmt, argptr);
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
line 123
;123:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 125
;124:
;125:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $109
line 126
;126:        trap_Error( "SG_Printf: buffer overflow" );
ADDRGP4 $97
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 127
;127:    }
LABELV $109
line 129
;128:
;129:    SG_Printf("%s", msg);
ADDRGP4 $106
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 130
;130:}
LABELV $107
endproc Con_Printf 4108 12
proc SG_LoadMedia 12 4
line 135
;131:
;132://#endif
;133:
;134:static qboolean SG_LoadMedia( void )
;135:{
line 136
;136:    if ((sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" )) == FS_INVALID_HANDLE)
ADDRGP4 $114
ARGP4
ADDRLP4 0
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg
ADDRLP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 -1
NEI4 $112
line 137
;137:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $111
JUMPV
LABELV $112
line 139
;138:    
;139:    if ((sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" )) == FS_INVALID_HANDLE)
ADDRGP4 $118
ARGP4
ADDRLP4 4
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+4
ADDRLP4 4
INDIRI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 -1
NEI4 $115
line 140
;140:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $111
JUMPV
LABELV $115
line 142
;141:    
;142:    if ((sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" )) == FS_INVALID_HANDLE)
ADDRGP4 $122
ARGP4
ADDRLP4 8
ADDRGP4 trap_Snd_RegisterSfx
CALLI4
ASGNI4
ADDRGP4 sg+8
ADDRLP4 8
INDIRI4
ASGNI4
ADDRLP4 8
INDIRI4
CNSTI4 -1
NEI4 $119
line 143
;143:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $111
JUMPV
LABELV $119
line 145
;144:
;145:    return qtrue;
CNSTI4 1
RETI4
LABELV $111
endproc SG_LoadMedia 12 4
export SG_Init
proc SG_Init 12 4
line 149
;146:}
;147:
;148:int32_t SG_Init(void)
;149:{
line 150
;150:    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 4
ADDRGP4 trap_Key_GetCatcher
CALLU4
ASGNU4
ADDRLP4 4
INDIRU4
CNSTU4 0
NEU4 $127
ADDRLP4 0
CNSTI4 1
ASGNI4
ADDRGP4 $128
JUMPV
LABELV $127
ADDRLP4 0
CNSTI4 0
ASGNI4
LABELV $128
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 8192
BANDU4
CNSTU4 0
EQU4 $124
line 151
;151:        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
ADDRLP4 8
ADDRGP4 trap_Key_GetCatcher
CALLU4
ASGNU4
ADDRLP4 8
INDIRU4
CNSTU4 8192
BANDU4
ARGU4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 152
;152:    }
LABELV $124
line 154
;153:
;154:    if (!SG_LoadMedia()) {
ADDRLP4 8
ADDRGP4 SG_LoadMedia
CALLI4
ASGNI4
ADDRLP4 8
INDIRI4
CNSTI4 0
NEI4 $129
line 155
;155:        G_Printf( COLOR_RED "SG_LoadMedia: failed!\n" );
ADDRGP4 $131
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 156
;156:        return -1; // did we fail to load the required resources?
CNSTI4 -1
RETI4
ADDRGP4 $123
JUMPV
LABELV $129
line 159
;157:    }
;158:
;159:    SG_MemInit();
ADDRGP4 SG_MemInit
CALLV
pop
line 161
;160:
;161:    return 1;
CNSTI4 1
RETI4
LABELV $123
endproc SG_Init 12 4
export SG_Shutdown
proc SG_Shutdown 8 4
line 165
;162:}
;163:
;164:int32_t SG_Shutdown(void)
;165:{
line 167
;166:
;167:    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLU4
ASGNU4
ADDRLP4 0
INDIRU4
CNSTU4 8192
BANDU4
CNSTU4 0
EQU4 $133
line 168
;168:        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
ADDRLP4 4
ADDRGP4 trap_Key_GetCatcher
CALLU4
ASGNU4
ADDRLP4 4
INDIRU4
CNSTU4 4294959103
BANDU4
ARGU4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 169
;169:    }
LABELV $133
line 171
;170:
;171:    return 0;
CNSTI4 0
RETI4
LABELV $132
endproc SG_Shutdown 8 4
import SG_RunLoop
import trap_RE_SetColor
import trap_RE_RenderScene
import trap_RE_ClearScene
import trap_RE_AddPolyListToScene
import trap_RE_AddPolyToScene
import trap_Key_IsDown
import trap_Key_GetKey
import trap_Key_SetCatcher
import trap_Key_GetCatcher
import trap_RE_RegisterShader
import trap_Snd_StopSfx
import trap_Snd_PlaySfx
import trap_Snd_RegisterSfx
import trap_AddCommand
import trap_FS_FClose
import trap_FS_FileTell
import trap_FS_FileSeek
import trap_FS_Read
import trap_FS_Write
import trap_FS_FileLength
import trap_FS_FOpenWrite
import trap_FS_FOpenRead
import trap_Args
import trap_Argv
import trap_Argc
import trap_Cvar_VariableStringBuffer
import trap_Cvar_Set
import trap_Cvar_Update
import trap_Cvar_Register
import trap_Milliseconds
import trap_Error
import trap_Print
import G_LoadMap
import P_GiveWeapon
import P_GiveItem
import SG_MouseEvent
import SG_KeyEvent
import SG_MemInit
import SG_MemAlloc
import String_Alloc
import Lvl_AddKillEntity
import SG_EndLevel
import SG_InitLevel
import sg_pmAccel
import stateinfo
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
bss
export sg
align 4
LABELV sg
skip 41966784
import inversedirs
import dirvectors
import I_GetParm
import Com_TouchMemory
import Hunk_TempIsClear
import Hunk_Check
import Hunk_Print
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
import Z_Strdup
import Z_AvailableMemory
import Z_FreeTags
import Z_Free
import Z_SMalloc
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
import FS_VM_FOpenFileWrite
import FS_VM_FileSeek
import FS_VM_FOpenFileRead
import FS_VM_CreateTmp
import FS_VM_WriteFile
import FS_VM_Write
import FS_VM_Read
import FS_VM_FClose
import FS_VM_FOpenRead
import FS_VM_FOpenWrite
import com_errorMessage
import com_errorEntered
import com_cacheLine
import com_frameTime
import com_fps
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
import Cvar_SetBooleanValue
import Cvar_SetStringValue
import Cvar_SetFloatValue
import Cvar_SetIntegerValue
import Cvar_SetModified
import Cvar_SetValueSafe
import Cvar_Set
import Cvar_SetSafe
import Cvar_SetDescription
import Cvar_SetGroup
import Cvar_Reset
import Cvar_Command
import Cvar_Get
import Cvar_Update
import Cvar_Flags
import Cvar_CheckRange
import Cvar_VariableString
import Cvar_VariableBoolean
import Cvar_VariableFloat
import Cvar_VariableInteger
import Cvar_VariableStringBufferSafe
import Cvar_VariableStringBuffer
import Cvar_Set2
import Cvar_CommandCompletion
import Cvar_CompleteCvarName
import Cvar_Register
import Cvar_Restart
import Cvar_Init
import Cvar_ForceReset
import Cvar_CheckGroup
import Cvar_ResetGroup
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
LABELV $131
byte 1 94
byte 1 49
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 77
byte 1 101
byte 1 100
byte 1 105
byte 1 97
byte 1 58
byte 1 32
byte 1 102
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 33
byte 1 10
byte 1 0
align 1
LABELV $122
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
LABELV $118
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
LABELV $114
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
LABELV $106
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $97
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
LABELV $87
byte 1 118
byte 1 109
byte 1 77
byte 1 97
byte 1 105
byte 1 110
byte 1 58
byte 1 32
byte 1 105
byte 1 110
byte 1 118
byte 1 97
byte 1 108
byte 1 105
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
byte 1 105
byte 1 100
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 0
