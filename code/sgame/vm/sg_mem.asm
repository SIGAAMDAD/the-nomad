export SG_InitMem
code
proc SG_InitMem 0 12
file "../sg_mem.c"
line 9
;1:#include "sg_local.h"
;2:
;3:// 256 KiB of static memory for the vm to use
;4:#define MEMPOOL_SIZE (256*1024)
;5:static char mempool[MEMPOOL_SIZE];
;6:static int allocPoint;
;7:
;8:void SG_InitMem(void)
;9:{
line 10
;10:    allocPoint = 0;
ADDRGP4 allocPoint
CNSTI4 0
ASGNI4
line 11
;11:    memset(mempool, 0, sizeof(mempool));
ADDRGP4 mempool
ARGP4
CNSTI4 0
ARGI4
CNSTU4 262144
ARGU4
ADDRGP4 memset
CALLP4
pop
line 12
;12:}
LABELV $43
endproc SG_InitMem 0 12
export SG_AllocMem
proc SG_AllocMem 8 8
line 15
;13:
;14:void* SG_AllocMem(int size)
;15:{
line 18
;16:    char *ptr;
;17:
;18:    if (!size)
ADDRFP4 0
INDIRI4
CNSTI4 0
NEI4 $45
line 19
;19:        return NULL;
CNSTP4 0
RETP4
ADDRGP4 $44
JUMPV
LABELV $45
line 21
;20:
;21:    size += sizeof(int);
ADDRFP4 0
ADDRFP4 0
INDIRI4
CVIU4 4
CNSTU4 4
ADDU4
CVUI4 4
ASGNI4
line 22
;22:    size = (size + 31) & ~31;
ADDRFP4 0
ADDRFP4 0
INDIRI4
CNSTI4 31
ADDI4
CNSTI4 -32
BANDI4
ASGNI4
line 23
;23:    if (allocPoint + size >= MEMPOOL_SIZE) {
ADDRGP4 allocPoint
INDIRI4
ADDRFP4 0
INDIRI4
ADDI4
CNSTI4 262144
LTI4 $47
line 24
;24:        SG_Error("SG_AllocMem: failed to allocate %i bytes", size);
ADDRGP4 $49
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 25
;25:        return NULL;
CNSTP4 0
RETP4
ADDRGP4 $44
JUMPV
LABELV $47
line 27
;26:    }
;27:    allocPoint += size;
ADDRLP4 4
ADDRGP4 allocPoint
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ADDRFP4 0
INDIRI4
ADDI4
ASGNI4
line 28
;28:    ptr = &mempool[allocPoint];
ADDRLP4 0
ADDRGP4 allocPoint
INDIRI4
ADDRGP4 mempool
ADDP4
ASGNP4
line 29
;29:    *(int *)ptr = size;
ADDRLP4 0
INDIRP4
ADDRFP4 0
INDIRI4
ASGNI4
line 30
;30:    return (void *)(ptr + sizeof(int));
ADDRLP4 0
INDIRP4
CNSTU4 4
ADDP4
RETP4
LABELV $44
endproc SG_AllocMem 8 8
export SG_FreeMem
proc SG_FreeMem 8 4
line 34
;31:}
;32:
;33:void SG_FreeMem(void *ptr)
;34:{
line 36
;35:    int size;
;36:    if (ptr == NULL) {
ADDRFP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $51
line 37
;37:        SG_Error("SG_FreeMem: null pointer");
ADDRGP4 $53
ARGP4
ADDRGP4 SG_Error
CALLV
pop
line 38
;38:        return;
ADDRGP4 $50
JUMPV
LABELV $51
line 41
;39:    }
;40:
;41:    size = *(int *)((char *)ptr - sizeof(int));
ADDRLP4 0
ADDRFP4 0
INDIRP4
CNSTI4 -4
ADDP4
INDIRI4
ASGNI4
line 42
;42:    allocPoint -= size;
ADDRLP4 4
ADDRGP4 allocPoint
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 4
INDIRP4
INDIRI4
ADDRLP4 0
INDIRI4
SUBI4
ASGNI4
line 43
;43:}
LABELV $50
endproc SG_FreeMem 8 4
export SG_ClearMem
proc SG_ClearMem 0 0
line 46
;44:
;45:void SG_ClearMem(void)
;46:{
line 47
;47:    SG_InitMem();
ADDRGP4 SG_InitMem
CALLV
pop
line 48
;48:}
LABELV $54
endproc SG_ClearMem 0 0
bss
align 4
LABELV allocPoint
skip 4
align 1
LABELV mempool
skip 262144
import G_SpawnMob
import sg_world
import mobs
import mobinfo
import P_Teleport
import playrs
import iteminfo
import trap_GetGameState
import trap_RE_SetColor
import trap_RE_DrawRect
import trap_RE_AddEntity
import trap_Key_IsDown
import trap_Key_GetKey
import trap_Key_SetCatcher
import trap_Key_GetCatcher
import trap_RE_RegisterShader
import trap_RE_RegisterTexture
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
import SG_Printf
import SG_Error
import RE_DrawRect
import RE_SetColor
import RE_AddDrawEntity
import RE_RegisterShader
import RE_RegisterTexture
import G_LoadBFF
import BFF_FetchTexture
import BFF_FetchLevel
import BFF_FetchScript
import BFF_FreeInfo
import BFF_FetchInfo
import BFF_OpenArchive
import BFF_CloseArchive
import B_GetChunk
import I_GetParm
import CPU_flags
import FS_ReadLine
import FS_ListFiles
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
import FS_Init
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
import crc32_buffer
import I_NomadInit
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
import N_booltostr
import N_strtobool
import Com_Clamp
import N_isnan
import PerpendicularVector
import AngleVectors
import MatrixMultiply
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
import Q_exp2f
import Q_log2f
import Q_rsqrt
import Q_fabs
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
import Com_Error
import Com_Printf
import G_Printf
import vsprintf
import fabs
import abs
import _atoi
import atoi
import _atof
import atof
import rand
import srand
import qsort
import toupper
import tolower
import strncmp
import strcmp
import strstr
import strchr
import strrchr
import strcat
import strncpy
import strcpy
import memmove
import memset
import memccpy
import strlen
import memchr
import memcpy
lit
align 1
LABELV $53
byte 1 83
byte 1 71
byte 1 95
byte 1 70
byte 1 114
byte 1 101
byte 1 101
byte 1 77
byte 1 101
byte 1 109
byte 1 58
byte 1 32
byte 1 110
byte 1 117
byte 1 108
byte 1 108
byte 1 32
byte 1 112
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 101
byte 1 114
byte 1 0
align 1
LABELV $49
byte 1 83
byte 1 71
byte 1 95
byte 1 65
byte 1 108
byte 1 108
byte 1 111
byte 1 99
byte 1 77
byte 1 101
byte 1 109
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
byte 1 97
byte 1 108
byte 1 108
byte 1 111
byte 1 99
byte 1 97
byte 1 116
byte 1 101
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 98
byte 1 121
byte 1 116
byte 1 101
byte 1 115
byte 1 0
