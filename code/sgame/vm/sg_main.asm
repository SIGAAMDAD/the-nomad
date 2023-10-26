export vmMain
code
proc vmMain 4 8
file "../sg_main.c"
line 16
;1:#include "../engine/n_shared.h"
;2:#include "sg_local.h"
;3:
;4:int SG_Init(void);
;5:int SG_Shutdown(void);
;6:int SG_RunLoop(void);
;7:
;8:/*
;9:vmMain
;10:
;11:this is the only way control passes into the module.
;12:this must be the very first function compiled into the .qvm file
;13:*/
;14:int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
;15:    int arg8, int arg9, int arg10)
;16:{
line 17
;17:    switch (command) {
ADDRFP4 0
INDIRI4
CNSTI4 0
EQI4 $46
ADDRGP4 $44
JUMPV
LABELV $46
line 19
;18:    case SGAME_INIT:
;19:        return SG_Init();
ADDRLP4 0
ADDRGP4 SG_Init
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
RETI4
ADDRGP4 $43
JUMPV
LABELV $44
line 21
;20:    default:
;21:        SG_Error("vmMain: invalid command id: %i", command);
ADDRGP4 $47
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 22
;22:        break;
LABELV $45
line 23
;23:    };
line 24
;24:    return -1;
CNSTI4 -1
RETI4
LABELV $43
endproc vmMain 4 8
export SG_Printf
proc SG_Printf 4100 12
line 28
;25:}
;26:
;27:void GDR_DECL SG_Printf(const char *fmt, ...)
;28:{
line 32
;29:    va_list argptr;
;30:    char msg[4096];
;31:
;32:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 33
;33:    vsprintf(msg, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 vsprintf
CALLI4
pop
line 34
;34:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 36
;35:
;36:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 37
;37:}
LABELV $48
endproc SG_Printf 4100 12
export SG_Error
proc SG_Error 4100 12
line 40
;38:
;39:void GDR_DECL SG_Error(const char *fmt, ...)
;40:{
line 44
;41:    va_list argptr;
;42:    char msg[4096];
;43:
;44:    memset(msg, 0, sizeof(msg));
ADDRLP4 0
ARGP4
CNSTI4 0
ARGI4
CNSTU4 4096
ARGU4
ADDRGP4 memset
CALLP4
pop
line 45
;45:    va_start(argptr, fmt);
ADDRLP4 4096
ADDRFP4 0+4
ASGNP4
line 46
;46:    vsprintf(msg, fmt, argptr);
ADDRLP4 0
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4096
INDIRP4
ARGP4
ADDRGP4 vsprintf
CALLI4
pop
line 47
;47:    va_end(argptr);
ADDRLP4 4096
CNSTP4 0
ASGNP4
line 49
;48:
;49:    trap_Error(msg);
ADDRLP4 0
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 50
;50:}
LABELV $50
endproc SG_Error 4100 12
export N_Error
proc N_Error 4100 12
line 53
;51:
;52:void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
;53:{
line 57
;54:    va_list argptr;
;55:    char msg[4096];
;56:
;57:    va_start(argptr, err);
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 58
;58:    vsprintf(msg, err, argptr);
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
line 59
;59:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 61
;60:
;61:    SG_Error("%s", msg);
ADDRGP4 $54
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 62
;62:}
LABELV $52
endproc N_Error 4100 12
export Con_Printf
proc Con_Printf 4100 12
line 68
;63:
;64:#ifndef SGAME_HARD_LINKED
;65:// this is only here so the functions in n_shared.c and bg_*.c can link
;66:
;67:void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
;68:{
line 72
;69:    va_list argptr;
;70:    char msg[4096];
;71:
;72:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 73
;73:    vsprintf(msg, fmt, argptr);
ADDRLP4 4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 vsprintf
CALLI4
pop
line 74
;74:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 76
;75:
;76:    SG_Printf("%s", msg);
ADDRGP4 $54
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 77
;77:}
LABELV $55
endproc Con_Printf 4100 12
export SG_Init
proc SG_Init 0 0
line 82
;78:
;79:#endif
;80:
;81:int SG_Init(void)
;82:{
line 87
;83://    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
;84://        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
;85://    }
;86:
;87:    return 0;
CNSTI4 0
RETI4
LABELV $57
endproc SG_Init 0 0
export SG_Shutdown
proc SG_Shutdown 8 4
line 91
;88:}
;89:
;90:int SG_Shutdown(void)
;91:{
line 92
;92:    SG_ClearMem();
ADDRGP4 SG_ClearMem
CALLV
pop
line 94
;93:
;94:    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 8192
BANDI4
CNSTI4 0
EQI4 $59
line 95
;95:        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
ADDRLP4 4
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 -8193
BANDI4
ARGI4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 96
;96:    }
LABELV $59
line 98
;97:
;98:    return 0;
CNSTI4 0
RETI4
LABELV $58
endproc SG_Shutdown 8 4
import SG_RunLoop
import trap_GetGameState
import trap_RE_SetColor
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
import SG_InitMem
import SG_FreeMem
import SG_ClearMem
import SG_AllocMem
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
LABELV $54
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $47
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
