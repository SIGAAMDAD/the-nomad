export SG_InitLevel
code
proc SG_InitLevel 8 12
file "../sg_level.c"
line 28
;1:#include "sg_local.h"
;2:#include "sg_imgui.h"
;3:
;4:typedef struct
;5:{
;6:    uint32_t timeStart;
;7:    uint32_t timeEnd;
;8:
;9:    uint32_t stylePoints;
;10:
;11:    uint32_t numDeaths;
;12:    uint32_t numKills;
;13:} levelstats_t;
;14:
;15:typedef struct
;16:{
;17:
;18:    int32_t index;
;19:
;20:    // save data
;21:    levelstats_t stats;
;22:    uint32_t checkpointIndex;
;23:} levelinfo_t;
;24:
;25:static levelinfo_t *linfo;
;26:
;27:qboolean SG_InitLevel( int32_t levelIndex )
;28:{
line 29
;29:    linfo = SG_MemAlloc( sizeof(*linfo) );
CNSTU4 28
ARGU4
ADDRLP4 0
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRGP4 linfo
ADDRLP4 0
INDIRP4
ASGNP4
line 30
;30:    if (!linfo) {
ADDRGP4 linfo
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $83
line 31
;31:        trap_Error( "SG_InitLevel: not enough vm memory" );
ADDRGP4 $85
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 32
;32:    }
LABELV $83
line 33
;33:    assert( linfo );
line 35
;34:
;35:    memset( linfo, 0, sizeof(*linfo) );
ADDRGP4 linfo
INDIRP4
ARGP4
CNSTI4 0
ARGI4
CNSTU4 28
ARGU4
ADDRGP4 memset
CALLP4
pop
line 37
;36:
;37:    if (!G_LoadMap( levelIndex, &sg.mapInfo )) {
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 sg+100
ARGP4
ADDRLP4 4
ADDRGP4 G_LoadMap
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $86
line 38
;38:        SG_Printf( "SG_InitLevel: failed to load map file at index %i\n", levelIndex );
ADDRGP4 $89
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 39
;39:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $82
JUMPV
LABELV $86
line 41
;40:    }
;41:    return qtrue;
CNSTI4 1
RETI4
LABELV $82
endproc SG_InitLevel 8 12
proc SG_DrawLevelStats 20 8
line 51
;42:}
;43:
;44:typedef struct {
;45:    ImGuiWindow window;
;46:} endlevelScreen_t;
;47:
;48:static endlevelScreen_t endLevel;
;49:
;50:static void SG_DrawLevelStats( void )
;51:{
line 55
;52:    float font_scale;
;53:    vec2_t cursorPos;
;54:
;55:    font_scale = ImGui_GetFontScale();
ADDRLP4 12
ADDRGP4 ImGui_GetFontScale
CALLF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ASGNF4
line 57
;56:
;57:    if (ImGui_BeginWindow( &endLevel.window )) {
ADDRGP4 endLevel
ARGP4
ADDRLP4 16
ADDRGP4 ImGui_BeginWindow
CALLI4
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 0
EQI4 $92
line 58
;58:        ImGui_SetWindowFontScale( font_scale * 6 );
CNSTF4 1086324736
ADDRLP4 0
INDIRF4
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 59
;59:        ImGui_TextUnformatted( "Level Statistics" );
ADDRGP4 $94
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 60
;60:        ImGui_SetWindowFontScale( font_scale * 3.5f );
CNSTF4 1080033280
ADDRLP4 0
INDIRF4
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 62
;61:
;62:        ImGui_GetCursorScreenPos( &cursorPos[0], &cursorPos[1] );
ADDRLP4 4
ARGP4
ADDRLP4 4+4
ARGP4
ADDRGP4 ImGui_GetCursorScreenPos
CALLV
pop
line 64
;63:
;64:        ImGui_SetCursorScreenPos( cursorPos[0], cursorPos[1] + 20 );
ADDRLP4 4
INDIRF4
ARGF4
ADDRLP4 4+4
INDIRF4
CNSTF4 1101004800
ADDF4
ARGF4
ADDRGP4 ImGui_SetCursorScreenPos
CALLV
pop
line 66
;65:
;66:    }
LABELV $92
line 67
;67:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 68
;68:}
LABELV $91
endproc SG_DrawLevelStats 20 8
export SG_EndLevel
proc SG_EndLevel 0 12
line 71
;69:
;70:int32_t SG_EndLevel( void )
;71:{
line 72
;72:    memset( &endLevel, 0, sizeof(endLevel) );
ADDRGP4 endLevel
ARGP4
CNSTI4 0
ARGI4
CNSTU4 16
ARGU4
ADDRGP4 memset
CALLP4
pop
line 74
;73:
;74:    endLevel.window.m_Flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
ADDRGP4 endLevel+12
CNSTI4 7
ASGNI4
line 75
;75:    endLevel.window.m_pTitle = "endLevel";
ADDRGP4 endLevel
ADDRGP4 $99
ASGNP4
line 76
;76:    endLevel.window.m_bOpen = qtrue;
ADDRGP4 endLevel+8
CNSTU4 1
ASGNU4
line 77
;77:    endLevel.window.m_bClosable = qfalse;
ADDRGP4 endLevel+4
CNSTU4 0
ASGNU4
line 79
;78:    
;79:    return 1;
CNSTI4 1
RETI4
LABELV $97
endproc SG_EndLevel 0 12
bss
align 4
LABELV endLevel
skip 16
align 4
LABELV linfo
skip 4
import ImGui_ColoredText
import ImGui_Text
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
LABELV $99
byte 1 101
byte 1 110
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $94
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 83
byte 1 116
byte 1 97
byte 1 116
byte 1 105
byte 1 115
byte 1 116
byte 1 105
byte 1 99
byte 1 115
byte 1 0
align 1
LABELV $89
byte 1 83
byte 1 71
byte 1 95
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
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
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 97
byte 1 116
byte 1 32
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $85
byte 1 83
byte 1 71
byte 1 95
byte 1 73
byte 1 110
byte 1 105
byte 1 116
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
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
