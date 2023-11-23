data
align 4
LABELV $81
address $82
export String_Alloc
code
proc String_Alloc 44 8
file "../sg_mem.c"
line 26
;1:#include "sg_local.h"
;2:
;3:// 20 MiB of static memory for the vm to use
;4:#define MEMPOOL_SIZE (20*1024*1024)
;5:static char mempool[MEMPOOL_SIZE];
;6:static uint32_t allocPoint;
;7:
;8:#define STRINGPOOL_SIZE (8*1024)
;9:
;10:
;11:#define HASH_TABLE_SIZE 2048
;12:
;13:typedef struct stringDef_s {
;14:	struct stringDef_s *next;
;15:	const char *str;
;16:} stringDef_t;
;17:
;18:static uint32_t strPoolIndex;
;19:static char strPool[STRINGPOOL_SIZE];
;20:
;21:static uint32_t strHandleCount;
;22:static stringDef_t *strHandle[HASH_TABLE_SIZE];
;23:
;24:
;25:const char *String_Alloc( const char *p )
;26:{
line 32
;27:	uint32_t len;
;28:	uint64_t hash;
;29:	stringDef_t *str, *last;
;30:	static const char *staticNULL = "";
;31:
;32:	if (p == NULL) {
ADDRFP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $83
line 33
;33:		return NULL;
CNSTP4 0
RETP4
ADDRGP4 $80
JUMPV
LABELV $83
line 36
;34:	}
;35:
;36:	if (*p == 0) {
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $85
line 37
;37:		return staticNULL;
ADDRGP4 $81
INDIRP4
RETP4
ADDRGP4 $80
JUMPV
LABELV $85
line 40
;38:	}
;39:
;40:	hash = Com_GenerateHashValue( p, HASH_TABLE_SIZE );
ADDRFP4 0
INDIRP4
ARGP4
CNSTU4 2048
ARGU4
ADDRLP4 16
ADDRGP4 Com_GenerateHashValue
CALLU4
ASGNU4
ADDRLP4 8
ADDRLP4 16
INDIRU4
ASGNU4
line 42
;41:
;42:	str = strHandle[hash];
ADDRLP4 0
ADDRLP4 8
INDIRU4
CNSTI4 2
LSHU4
ADDRGP4 strHandle
ADDP4
INDIRP4
ASGNP4
ADDRGP4 $88
JUMPV
LABELV $87
line 43
;43:	while (str) {
line 44
;44:		if (strcmp(p, str->str) == 0) {
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ARGP4
ADDRLP4 20
ADDRGP4 strcmp
CALLI4
ASGNI4
ADDRLP4 20
INDIRI4
CNSTI4 0
NEI4 $90
line 45
;45:			return str->str;
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
RETP4
ADDRGP4 $80
JUMPV
LABELV $90
line 47
;46:		}
;47:		str = str->next;
ADDRLP4 0
ADDRLP4 0
INDIRP4
INDIRP4
ASGNP4
line 48
;48:	}
LABELV $88
line 43
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $87
line 50
;49:
;50:	len = strlen(p);
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 20
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 12
ADDRLP4 20
INDIRU4
ASGNU4
line 51
;51:	if (len + strPoolIndex + 1 < STRINGPOOL_SIZE) {
ADDRLP4 12
INDIRU4
ADDRGP4 strPoolIndex
INDIRU4
ADDU4
CNSTU4 1
ADDU4
CNSTU4 8192
GEU4 $92
line 52
;52:		uint32_t ph = strPoolIndex;
ADDRLP4 24
ADDRGP4 strPoolIndex
INDIRU4
ASGNU4
line 53
;53:		strcpy(&strPool[strPoolIndex], p);
ADDRGP4 strPoolIndex
INDIRU4
ADDRGP4 strPool
ADDP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 strcpy
CALLP4
pop
line 54
;54:		strPoolIndex += len + 1;
ADDRLP4 28
ADDRGP4 strPoolIndex
ASGNP4
ADDRLP4 28
INDIRP4
ADDRLP4 28
INDIRP4
INDIRU4
ADDRLP4 12
INDIRU4
CNSTU4 1
ADDU4
ADDU4
ASGNU4
line 56
;55:
;56:		str = strHandle[hash];
ADDRLP4 0
ADDRLP4 8
INDIRU4
CNSTI4 2
LSHU4
ADDRGP4 strHandle
ADDP4
INDIRP4
ASGNP4
line 57
;57:		last = str;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
ADDRGP4 $95
JUMPV
LABELV $94
line 58
;58:		while (str && str->next) {
line 59
;59:			last = str;
ADDRLP4 4
ADDRLP4 0
INDIRP4
ASGNP4
line 60
;60:			str = str->next;
ADDRLP4 0
ADDRLP4 0
INDIRP4
INDIRP4
ASGNP4
line 61
;61:		}
LABELV $95
line 58
ADDRLP4 36
CNSTU4 0
ASGNU4
ADDRLP4 0
INDIRP4
CVPU4 4
ADDRLP4 36
INDIRU4
EQU4 $97
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
ADDRLP4 36
INDIRU4
NEU4 $94
LABELV $97
line 63
;62:
;63:		str = (stringDef_t *)SG_MemAlloc( sizeof(stringDef_t) );
CNSTU4 8
ARGU4
ADDRLP4 40
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 40
INDIRP4
ASGNP4
line 64
;64:		str->next = NULL;
ADDRLP4 0
INDIRP4
CNSTP4 0
ASGNP4
line 65
;65:		str->str = &strPool[ph];
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
ADDRLP4 24
INDIRU4
ADDRGP4 strPool
ADDP4
ASGNP4
line 66
;66:		if (last) {
ADDRLP4 4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $98
line 67
;67:			last->next = str;
ADDRLP4 4
INDIRP4
ADDRLP4 0
INDIRP4
ASGNP4
line 68
;68:		} else {
ADDRGP4 $99
JUMPV
LABELV $98
line 69
;69:			strHandle[hash] = str;
ADDRLP4 8
INDIRU4
CNSTI4 2
LSHU4
ADDRGP4 strHandle
ADDP4
ADDRLP4 0
INDIRP4
ASGNP4
line 70
;70:		}
LABELV $99
line 71
;71:		return &strPool[ph];
ADDRLP4 24
INDIRU4
ADDRGP4 strPool
ADDP4
RETP4
ADDRGP4 $80
JUMPV
LABELV $92
line 73
;72:	}
;73:	return NULL;
CNSTP4 0
RETP4
LABELV $80
endproc String_Alloc 44 8
export String_Report
proc String_Report 12 16
line 76
;74:}
;75:
;76:void String_Report() {
line 78
;77:	float f;
;78:	Con_Printf("Memory/String Pool Info\n");
ADDRGP4 $101
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 79
;79:	Con_Printf("----------------\n");
ADDRGP4 $102
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 81
;80:	
;81:    f = strPoolIndex;
ADDRLP4 4
ADDRGP4 strPoolIndex
INDIRU4
ASGNU4
ADDRLP4 0
CNSTF4 1073741824
ADDRLP4 4
INDIRU4
CNSTI4 1
RSHU4
CVUI4 4
CVIF4 4
MULF4
ADDRLP4 4
INDIRU4
CNSTU4 1
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
line 82
;82:	f /= STRINGPOOL_SIZE;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 1174405120
DIVF4
ASGNF4
line 83
;83:	f *= 100;
ADDRLP4 0
CNSTF4 1120403456
ADDRLP4 0
INDIRF4
MULF4
ASGNF4
line 84
;84:	Con_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRINGPOOL_SIZE);
ADDRGP4 $103
ARGP4
ADDRLP4 0
INDIRF4
ARGF4
ADDRGP4 strPoolIndex
INDIRU4
ARGU4
CNSTI4 8192
ARGI4
ADDRGP4 Con_Printf
CALLV
pop
line 86
;85:	
;86:    f = allocPoint;
ADDRLP4 8
ADDRGP4 allocPoint
INDIRU4
ASGNU4
ADDRLP4 0
CNSTF4 1073741824
ADDRLP4 8
INDIRU4
CNSTI4 1
RSHU4
CVUI4 4
CVIF4 4
MULF4
ADDRLP4 8
INDIRU4
CNSTU4 1
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
line 87
;87:	f /= MEMPOOL_SIZE;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 1268776960
DIVF4
ASGNF4
line 88
;88:	f *= 100;
ADDRLP4 0
CNSTF4 1120403456
ADDRLP4 0
INDIRF4
MULF4
ASGNF4
line 89
;89:	Con_Printf("Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEMPOOL_SIZE);
ADDRGP4 $104
ARGP4
ADDRLP4 0
INDIRF4
ARGF4
ADDRGP4 allocPoint
INDIRU4
ARGU4
CNSTI4 20971520
ARGI4
ADDRGP4 Con_Printf
CALLV
pop
line 90
;90:}
LABELV $100
endproc String_Report 12 16
export SG_MemAlloc
proc SG_MemAlloc 8 12
line 93
;91:
;92:void *SG_MemAlloc( uint32_t size )
;93:{
line 96
;94:    char *buf;
;95:
;96:    if (!size) {
ADDRFP4 0
INDIRU4
CNSTU4 0
NEU4 $106
line 97
;97:        G_Error( "SG_MemAlloc: bad size" );
ADDRGP4 $108
ARGP4
ADDRGP4 G_Error
CALLV
pop
line 98
;98:    }
LABELV $106
line 100
;99:
;100:    size = PAD(size, (unsigned)16); // round to 16-byte alignment
ADDRFP4 0
ADDRFP4 0
INDIRU4
CNSTU4 16
ADDU4
CNSTU4 1
SUBU4
CNSTU4 4294967280
BANDU4
ASGNU4
line 102
;101:
;102:    if (allocPoint + size >= sizeof(mempool)) {
ADDRGP4 allocPoint
INDIRU4
ADDRFP4 0
INDIRU4
ADDU4
CNSTU4 20971520
LTU4 $109
line 103
;103:        G_Error( "SG_MemAlloc: not enough vm memory" );
ADDRGP4 $111
ARGP4
ADDRGP4 G_Error
CALLV
pop
line 104
;104:        return NULL;
CNSTP4 0
RETP4
ADDRGP4 $105
JUMPV
LABELV $109
line 107
;105:    }
;106:
;107:    buf = &mempool[ allocPoint ];
ADDRLP4 0
ADDRGP4 allocPoint
INDIRU4
ADDRGP4 mempool
ADDP4
ASGNP4
line 108
;108:    allocPoint += size;
ADDRLP4 4
ADDRGP4 allocPoint
ASGNP4
ADDRLP4 4
INDIRP4
ADDRLP4 4
INDIRP4
INDIRU4
ADDRFP4 0
INDIRU4
ADDU4
ASGNU4
line 111
;109:
;110:    // zero init
;111:    memset( buf, 0, size );
ADDRLP4 0
INDIRP4
ARGP4
CNSTI4 0
ARGI4
ADDRFP4 0
INDIRU4
ARGU4
ADDRGP4 memset
CALLP4
pop
line 113
;112:
;113:    return buf;
ADDRLP4 0
INDIRP4
RETP4
LABELV $105
endproc SG_MemAlloc 8 12
export SG_MemoryRemaining
proc SG_MemoryRemaining 0 0
line 116
;114:}
;115:
;116:uint32_t SG_MemoryRemaining( void ) {
line 117
;117:    return sizeof(mempool) - allocPoint;
CNSTU4 20971520
ADDRGP4 allocPoint
INDIRU4
SUBU4
RETU4
LABELV $112
endproc SG_MemoryRemaining 0 0
export SG_MemInit
proc SG_MemInit 4 12
line 121
;118:}
;119:
;120:void SG_MemInit( void )
;121:{
line 124
;122:    uint32_t i;
;123:
;124:    memset( mempool, 0, sizeof(mempool) );
ADDRGP4 mempool
ARGP4
CNSTI4 0
ARGI4
CNSTU4 20971520
ARGU4
ADDRGP4 memset
CALLP4
pop
line 125
;125:    memset( strPool, 0, sizeof(strPool) );
ADDRGP4 strPool
ARGP4
CNSTI4 0
ARGI4
CNSTU4 8192
ARGU4
ADDRGP4 memset
CALLP4
pop
line 127
;126:
;127:	for (i = 0; i < HASH_TABLE_SIZE; i++) {
ADDRLP4 0
CNSTU4 0
ASGNU4
ADDRGP4 $117
JUMPV
LABELV $114
line 128
;128:		strHandle[i] = 0;
ADDRLP4 0
INDIRU4
CNSTI4 2
LSHU4
ADDRGP4 strHandle
ADDP4
CNSTP4 0
ASGNP4
line 129
;129:	}
LABELV $115
line 127
ADDRLP4 0
ADDRLP4 0
INDIRU4
CNSTU4 1
ADDU4
ASGNU4
LABELV $117
ADDRLP4 0
INDIRU4
CNSTU4 2048
LTU4 $114
line 130
;130:	strHandleCount = 0;
ADDRGP4 strHandleCount
CNSTU4 0
ASGNU4
line 131
;131:	strPoolIndex = 0;
ADDRGP4 strPoolIndex
CNSTU4 0
ASGNU4
line 132
;132:}
LABELV $113
endproc SG_MemInit 4 12
bss
align 4
LABELV strHandle
skip 8192
align 4
LABELV strHandleCount
skip 4
align 1
LABELV strPool
skip 8192
align 4
LABELV strPoolIndex
skip 4
align 4
LABELV allocPoint
skip 4
align 1
LABELV mempool
skip 20971520
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
import Lvl_AddKillEntity
import SG_EndLevel
import SG_InitLevel
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import sg_pmAccel
import stateinfo
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
import sg
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
import Con_Printf
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
LABELV $111
byte 1 83
byte 1 71
byte 1 95
byte 1 77
byte 1 101
byte 1 109
byte 1 65
byte 1 108
byte 1 108
byte 1 111
byte 1 99
byte 1 58
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 101
byte 1 110
byte 1 111
byte 1 117
byte 1 103
byte 1 104
byte 1 32
byte 1 118
byte 1 109
byte 1 32
byte 1 109
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 0
align 1
LABELV $108
byte 1 83
byte 1 71
byte 1 95
byte 1 77
byte 1 101
byte 1 109
byte 1 65
byte 1 108
byte 1 108
byte 1 111
byte 1 99
byte 1 58
byte 1 32
byte 1 98
byte 1 97
byte 1 100
byte 1 32
byte 1 115
byte 1 105
byte 1 122
byte 1 101
byte 1 0
align 1
LABELV $104
byte 1 77
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 32
byte 1 80
byte 1 111
byte 1 111
byte 1 108
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 46
byte 1 49
byte 1 102
byte 1 37
byte 1 37
byte 1 32
byte 1 102
byte 1 117
byte 1 108
byte 1 108
byte 1 44
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 98
byte 1 121
byte 1 116
byte 1 101
byte 1 115
byte 1 32
byte 1 111
byte 1 117
byte 1 116
byte 1 32
byte 1 111
byte 1 102
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 117
byte 1 115
byte 1 101
byte 1 100
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $103
byte 1 83
byte 1 116
byte 1 114
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 80
byte 1 111
byte 1 111
byte 1 108
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 46
byte 1 49
byte 1 102
byte 1 37
byte 1 37
byte 1 32
byte 1 102
byte 1 117
byte 1 108
byte 1 108
byte 1 44
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 98
byte 1 121
byte 1 116
byte 1 101
byte 1 115
byte 1 32
byte 1 111
byte 1 117
byte 1 116
byte 1 32
byte 1 111
byte 1 102
byte 1 32
byte 1 37
byte 1 105
byte 1 32
byte 1 117
byte 1 115
byte 1 101
byte 1 100
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $102
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
LABELV $101
byte 1 77
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 47
byte 1 83
byte 1 116
byte 1 114
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 80
byte 1 111
byte 1 111
byte 1 108
byte 1 32
byte 1 73
byte 1 110
byte 1 102
byte 1 111
byte 1 10
byte 1 0
align 1
LABELV $82
byte 1 0
