export ImGui_SetItemTooltip
code
proc ImGui_SetItemTooltip 4108 12
file "../sg_imgui.c"
line 5
;1:#include "sg_local.h"
;2:#include "sg_imgui.h"
;3:
;4:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_SetItemTooltip( const char *fmt, ... )
;5:{
line 10
;6:    va_list argptr;
;7:    char msg[4096];
;8:    int32_t length;
;9:
;10:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 11
;11:    length = vsprintf( msg, fmt, argptr );
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
line 12
;12:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 14
;13:
;14:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $82
line 15
;15:        trap_Error( "ImGui_SetItemTooltip: buffer overflow" );
ADDRGP4 $84
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 16
;16:    }
LABELV $82
line 18
;17:
;18:    ImGui_SetItemTooltipUnformatted( msg );
ADDRLP4 4
ARGP4
ADDRGP4 ImGui_SetItemTooltipUnformatted
CALLV
pop
line 19
;19:}
LABELV $80
endproc ImGui_SetItemTooltip 4108 12
export ImGui_Text
proc ImGui_Text 4108 12
line 22
;20:
;21:void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_Text( const char *fmt, ... )
;22:{
line 27
;23:    va_list argptr;
;24:    char msg[4096];
;25:    int32_t length;
;26:
;27:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 0+4
ASGNP4
line 28
;28:    length = vsprintf( msg, fmt, argptr );
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
line 29
;29:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 31
;30:
;31:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $87
line 32
;32:        trap_Error( "ImGui_Text: buffer overflow" );
ADDRGP4 $89
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 33
;33:    }
LABELV $87
line 35
;34:
;35:    ImGui_TextUnformatted( msg );
ADDRLP4 4
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 36
;36:}
LABELV $85
endproc ImGui_Text 4108 12
export ImGui_ColoredText
proc ImGui_ColoredText 4108 12
line 39
;37:
;38:void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL ImGui_ColoredText( const vec4_t pColor, const char *fmt, ... )
;39:{
line 44
;40:    va_list argptr;
;41:    char msg[4096];
;42:    int32_t length;
;43:
;44:    va_start( argptr, fmt );
ADDRLP4 0
ADDRFP4 4+4
ASGNP4
line 45
;45:    length = vsprintf( msg, fmt, argptr );
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
line 46
;46:    va_end( argptr );
ADDRLP4 0
CNSTP4 0
ASGNP4
line 48
;47:
;48:    if (length >= sizeof(msg)) {
ADDRLP4 4100
INDIRI4
CVIU4 4
CNSTU4 4096
LTU4 $92
line 49
;49:        trap_Error( "ImGui_Text: buffer overflow" );
ADDRGP4 $89
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 50
;50:    }
LABELV $92
line 52
;51:
;52:    ImGui_ColoredTextUnformatted( pColor, msg );
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ARGP4
ADDRGP4 ImGui_ColoredTextUnformatted
CALLV
pop
line 53
;53:}
LABELV $90
endproc ImGui_ColoredText 4108 12
import ImGui_ColoredTextUnformatted
import ImGui_TextUnformatted
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
LABELV $89
byte 1 73
byte 1 109
byte 1 71
byte 1 117
byte 1 105
byte 1 95
byte 1 84
byte 1 101
byte 1 120
byte 1 116
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
LABELV $84
byte 1 73
byte 1 109
byte 1 71
byte 1 117
byte 1 105
byte 1 95
byte 1 83
byte 1 101
byte 1 116
byte 1 73
byte 1 116
byte 1 101
byte 1 109
byte 1 84
byte 1 111
byte 1 111
byte 1 108
byte 1 116
byte 1 105
byte 1 112
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
