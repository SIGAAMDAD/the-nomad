export vmMain
code
proc vmMain 4 8
file "../sg_main.c"
line 15
;1:#include "../engine/n_shared.h"
;2:#include "../engine/n_scf.h"
;3:#include "sg_local.h"
;4:
;5:world_t sg_world;
;6:playr_t playrs[MAX_PLAYR_COUNT];
;7:mobj_t mobs[MAX_MOBS_ACTIVE];
;8:
;9:int SG_Init(void);
;10:int SG_Shutdown(void);
;11:int SG_RunLoop(void);
;12:
;13:int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
;14:    int arg8, int arg9, int arg10)
;15:{
line 16
;16:    switch (command) {
ADDRFP4 0
INDIRI4
CNSTI4 0
EQI4 $46
ADDRGP4 $44
JUMPV
LABELV $46
line 18
;17:    case SGAME_INIT:
;18:        return SG_Init();
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
line 20
;19:    default:
;20:        SG_Error("vmMain: invalid command id: %i", command);
ADDRGP4 $47
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Error
CALLV
pop
line 21
;21:        break;
LABELV $45
line 22
;22:    };
line 23
;23:    return -1;
CNSTI4 -1
RETI4
LABELV $43
endproc vmMain 4 8
export SG_Printf
proc SG_Printf 1028 12
line 27
;24:}
;25:
;26:void GDR_DECL SG_Printf(const char *fmt, ...)
;27:{
line 31
;28:    va_list argptr;
;29:    char msg[1024];
;30:
;31:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 32
;32:    vsprintf(msg, fmt, argptr);
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
line 33
;33:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 35
;34:
;35:    trap_Print(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 36
;36:}
LABELV $48
endproc SG_Printf 1028 12
export SG_Error
proc SG_Error 1028 12
line 39
;37:
;38:void GDR_DECL SG_Error(const char *fmt, ...)
;39:{
line 43
;40:    va_list argptr;
;41:    char msg[1024];
;42:
;43:    va_start(argptr, fmt);
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 44
;44:    vsprintf(msg, fmt, argptr);
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
line 45
;45:    va_end(argptr);
ADDRLP4 0
CNSTP4 0
ASGNP4
line 47
;46:
;47:    trap_Error(msg);
ADDRLP4 4
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 48
;48:}
LABELV $50
endproc SG_Error 1028 12
data
export mobinfo
align 4
LABELV mobinfo
byte 1 90
byte 1 117
byte 1 114
byte 1 103
byte 1 117
byte 1 116
byte 1 32
byte 1 72
byte 1 117
byte 1 108
byte 1 107
byte 1 0
skip 68
skip 44
byte 1 82
byte 1 97
byte 1 118
byte 1 97
byte 1 103
byte 1 101
byte 1 114
byte 1 0
skip 72
skip 44
byte 1 71
byte 1 114
byte 1 117
byte 1 110
byte 1 116
byte 1 0
skip 74
skip 44
byte 1 83
byte 1 104
byte 1 111
byte 1 116
byte 1 116
byte 1 121
byte 1 0
skip 73
skip 44
export SG_Init
code
proc SG_Init 20 12
line 58
;49:
;50:const mobj_t mobinfo[NUMMOBS] = {
;51:    {"Zurgut Hulk"},
;52:    {"Ravager"},
;53:    {"Grunt"},
;54:    {"Shotty"},
;55:};
;56:
;57:int SG_Init(void)
;58:{
line 61
;59:    playr_t* p;
;60:
;61:    SG_Printf("SG_Init: initializing solo-player campaign variables");
ADDRGP4 $53
ARGP4
ADDRGP4 SG_Printf
CALLV
pop
line 63
;62:
;63:    sg_world.state = GS_LEVEL;
ADDRGP4 sg_world+16
CNSTI4 1
ASGNI4
line 65
;64:    
;65:    sg_world.playr = &playrs[0];
ADDRGP4 sg_world+12
ADDRGP4 playrs
ASGNP4
line 66
;66:    sg_world.numPlayrs = 0;
ADDRGP4 sg_world+20
CNSTI4 0
ASGNI4
line 67
;67:    sg_world.playrs = playrs;
ADDRGP4 sg_world+28
ADDRGP4 playrs
ASGNP4
line 68
;68:    p = &playrs[0];
ADDRLP4 0
ADDRGP4 playrs
ASGNP4
line 69
;69:    memset(playrs, 0, MAX_PLAYR_COUNT * sizeof(*playrs));
ADDRGP4 playrs
ARGP4
CNSTI4 0
ARGI4
CNSTU4 18000
ARGU4
ADDRGP4 memset
CALLP4
pop
line 71
;70:
;71:    p->alive = qtrue;
ADDRLP4 0
INDIRP4
CNSTI4 36
ADDP4
CNSTI4 1
ASGNI4
line 72
;72:    p->dir = D_NORTH;
ADDRLP4 0
INDIRP4
CNSTI4 32
ADDP4
CNSTI4 0
ASGNI4
line 73
;73:    p->health = 100;
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
CNSTI4 100
ASGNI4
line 74
;74:    memset(p->inventory, 0, MAX_PLAYR_INVENTORY * sizeof(*p->inventory));
ADDRLP4 0
INDIRP4
CNSTI4 40
ADDP4
ARGP4
CNSTI4 0
ARGI4
CNSTU4 1760
ARGU4
ADDRGP4 memset
CALLP4
pop
line 75
;75:    VectorCopy(p->to, vec2_origin);
ADDRGP4 vec2_origin
ADDRLP4 0
INDIRP4
CNSTI4 24
ADDP4
INDIRB
ASGNB 12
line 76
;76:    VectorCopy(p->thrust, vec2_origin);
ADDRGP4 vec2_origin
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
INDIRB
ASGNB 12
line 77
;77:    VectorCopy(p->pos, vec2_origin);
ADDRGP4 vec2_origin
ADDRLP4 0
INDIRP4
CNSTI4 8
ADDP4
INDIRB
ASGNB 12
line 79
;78:
;79:    sg_world.numMobs = 0;
ADDRGP4 sg_world+24
CNSTI4 0
ASGNI4
line 80
;80:    sg_world.mobs = mobs;
ADDRGP4 sg_world+32
ADDRGP4 mobs
ASGNP4
line 81
;81:    memset(mobs, 0, MAX_MOBS_ACTIVE * sizeof(*mobs));
ADDRGP4 mobs
ARGP4
CNSTI4 0
ARGI4
CNSTU4 18600
ARGU4
ADDRGP4 memset
CALLP4
pop
line 83
;82:
;83:    SG_InitMem();
ADDRGP4 SG_InitMem
CALLV
pop
line 85
;84:
;85:    sg_world.mapwidth = 0;
ADDRGP4 sg_world
CNSTI4 0
ASGNI4
line 86
;86:    sg_world.mapheight = 0;
ADDRGP4 sg_world+4
CNSTI4 0
ASGNI4
line 87
;87:    sg_world.spritemap = (sprite_t **)SG_AllocMem(sizeof(sprite_t *) * sg_world.mapwidth);
ADDRGP4 sg_world
INDIRI4
CVIU4 4
CNSTI4 2
LSHU4
CVUI4 4
ARGI4
ADDRLP4 4
ADDRGP4 SG_AllocMem
CALLP4
ASGNP4
ADDRGP4 sg_world+8
ADDRLP4 4
INDIRP4
ASGNP4
line 89
;88:
;89:    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 12
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $65
ADDRLP4 8
CNSTI4 1
ASGNI4
ADDRGP4 $66
JUMPV
LABELV $65
ADDRLP4 8
CNSTI4 0
ASGNI4
LABELV $66
ADDRLP4 8
INDIRI4
CNSTI4 8192
BANDI4
CNSTI4 0
EQI4 $62
line 90
;90:        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
ADDRLP4 16
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 8192
BANDI4
ARGI4
ADDRGP4 trap_Key_SetCatcher
CALLV
pop
line 91
;91:    }
LABELV $62
line 93
;92:
;93:    return 0;
CNSTI4 0
RETI4
LABELV $52
endproc SG_Init 20 12
export SG_Shutdown
proc SG_Shutdown 8 4
line 97
;94:}
;95:
;96:int SG_Shutdown(void)
;97:{
line 98
;98:    SG_ClearMem();
ADDRGP4 SG_ClearMem
CALLV
pop
line 100
;99:
;100:    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
ADDRLP4 0
ADDRGP4 trap_Key_GetCatcher
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 8192
BANDI4
CNSTI4 0
EQI4 $68
line 101
;101:        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
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
line 102
;102:    }
LABELV $68
line 104
;103:
;104:    return 0;
CNSTI4 0
RETI4
LABELV $67
endproc SG_Shutdown 8 4
import SG_RunLoop
import G_SpawnMob
bss
export sg_world
align 4
LABELV sg_world
skip 36
export mobs
align 4
LABELV mobs
skip 18600
import P_Teleport
export playrs
align 4
LABELV playrs
skip 18000
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
import SG_InitMem
import SG_FreeMem
import SG_ClearMem
import SG_AllocMem
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
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 58
byte 1 32
byte 1 105
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
byte 1 115
byte 1 111
byte 1 108
byte 1 111
byte 1 45
byte 1 112
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 32
byte 1 99
byte 1 97
byte 1 109
byte 1 112
byte 1 97
byte 1 105
byte 1 103
byte 1 110
byte 1 32
byte 1 118
byte 1 97
byte 1 114
byte 1 105
byte 1 97
byte 1 98
byte 1 108
byte 1 101
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
