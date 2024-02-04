code
proc SG_GetLevelInfoByMapName 8 8
file "../sg_level.c"
line 66
;1:#include "sg_local.h"
;2:#include "sg_imgui.h"
;3:
;4:typedef struct
;5:{
;6:    int timeStart;
;7:    int timeEnd;
;8:
;9:    int stylePoints;
;10:
;11:    int numDeaths;
;12:    int numKills;
;13:} levelstats_t;
;14:
;15:typedef struct
;16:{
;17:    int index;
;18:
;19:    // save data
;20:    levelstats_t stats;
;21:    int checkpointIndex;
;22:} level_t;
;23:
;24:typedef enum {
;25:    LEVEL_RANK_A,
;26:    LEVEL_RANK_B,
;27:    LEVEL_RANK_C,
;28:    LEVEL_RANK_D,
;29:    LEVEL_RANK_WERE_U_BOTTING
;30:} rank_t;
;31:
;32:typedef struct {
;33:    rank_t rank;
;34:    int minStyle;
;35:    int minKills;
;36:    int maxTime;
;37:
;38:    qboolean requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
;39:} levelRank_t;
;40:
;41:typedef struct {
;42:    levelstats_t stats;
;43:
;44:    char name[MAX_NPATH];
;45:    gamedif_t difficulty;
;46:    nhandle_t handle;
;47:} levelMap_t;
;48:
;49:typedef struct levelInfo_s
;50:{
;51:    levelMap_t maphandles[NUMDIFS];
;52:    char name[MAX_NPATH];
;53:
;54:    // ranking info
;55:    levelRank_t a;
;56:    levelRank_t b;
;57:    levelRank_t c;
;58:    levelRank_t d;
;59:    levelRank_t f;
;60:} levelInfo_t;
;61:
;62:static levelInfo_t *sg_levelInfoData;
;63:static level_t level;
;64:
;65:static levelInfo_t *SG_GetLevelInfoByMapName( const char *mapname )
;66:{
line 69
;67:    int i;
;68:
;69:    for ( i = 0; i < sg.numLevels; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $98
JUMPV
LABELV $95
line 70
;70:        if ( !N_stricmp( sg_levelInfoData[i].name, mapname ) )  {
ADDRLP4 0
INDIRI4
CNSTI4 716
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
CNSTI4 552
ADDP4
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 4
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $100
line 71
;71:            return &sg_levelInfoData[i];
ADDRLP4 0
INDIRI4
CNSTI4 716
MULI4
ADDRGP4 sg_levelInfoData
INDIRP4
ADDP4
RETP4
ADDRGP4 $94
JUMPV
LABELV $100
line 73
;72:        }
;73:    }
LABELV $96
line 69
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $98
ADDRLP4 0
INDIRI4
ADDRGP4 sg+2220
INDIRI4
LTI4 $95
line 74
;74:    return NULL;
CNSTP4 0
RETP4
LABELV $94
endproc SG_GetLevelInfoByMapName 8 8
proc SG_SpawnLevelEntities 12 12
line 81
;75:}
;76:
;77://
;78:// SG_SpawnLevelEntities
;79://
;80:static void SG_SpawnLevelEntities( void )
;81:{
line 85
;82:    int i;
;83:    const mapspawn_t *spawn;
;84:
;85:    spawn = sg.mapInfo.spawns;
ADDRLP4 0
ADDRGP4 sg+4270540
ASGNP4
line 86
;86:    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
ADDRLP4 4
CNSTI4 0
ASGNI4
ADDRGP4 $107
JUMPV
LABELV $104
line 87
;87:        SG_Spawn( spawn->entityid, spawn->entitytype, &spawn->xyz );
ADDRLP4 0
INDIRP4
CNSTI4 16
ADDP4
INDIRU4
ARGU4
ADDRLP4 0
INDIRP4
CNSTI4 12
ADDP4
INDIRU4
ARGU4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 SG_Spawn
CALLV
pop
line 88
;88:    }
LABELV $105
line 86
ADDRLP4 4
ADDRLP4 4
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 20
ADDP4
ASGNP4
LABELV $107
ADDRLP4 4
INDIRI4
ADDRGP4 sg+4270540+23624
INDIRI4
LTI4 $104
line 89
;89:}
LABELV $102
endproc SG_SpawnLevelEntities 12 12
export SG_StartLevel
proc SG_StartLevel 0 0
endproc SG_StartLevel 0 0
export SG_SaveLevelData
proc SG_SaveLevelData 0 0
endproc SG_SaveLevelData 0 0
export SG_LoadLevelData
proc SG_LoadLevelData 0 0
endproc SG_LoadLevelData 0 0
export SG_DrawLevelStats
proc SG_DrawLevelStats 0 0
endproc SG_DrawLevelStats 0 0
export SG_EndLevel
proc SG_EndLevel 0 0
endproc SG_EndLevel 0 0
export SG_ParseInfos
proc SG_ParseInfos 0 0
endproc SG_ParseInfos 0 0
proc SG_LoadLevelsFromFile 0 0
endproc SG_LoadLevelsFromFile 0 0
export SG_LoadLevels
proc SG_LoadLevels 0 0
endproc SG_LoadLevels 0 0
import atoi
import strcat
import strcpy
import strlen
import memset
bss
align 4
LABELV sg_levelInfos
skip 4096
align 4
LABELV level
skip 28
align 4
LABELV sg_levelInfoData
skip 4
import Cvar_VariableStringBuffer
import Cvar_Set
import Cvar_Update
import Cvar_Register
import trap_FS_Printf
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
import SG_UpdateCvars
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import SG_BuildMoveCommand
import SGameCommand
import SG_DrawFrame
import pm_wallTime
import pm_wallrunAccelMove
import pm_wallrunAccelVertical
import pm_airAccel
import pm_baseSpeed
import pm_baseAccel
import pm_waterAccel
import pm_airFriction
import pm_waterFriction
import pm_groundFriction
import sg_memoryDebug
import sg_numSaves
import sg_savename
import sg_levelIndex
import sg_gibs
import sg_decalDetail
import sg_printLevelStats
import sg_mouseAcceleration
import sg_mouseInvert
import sg_paused
import sg_debugPrint
import ammoCaps
import mobinfo
import iteminfo
import weaponinfo
import sg
import sg_entities
import sg_activeEnts
import sg_freeEnts
import inversedirs
import dirvectors
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
import Con_Printf
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
LABELV $296
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $295
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $294
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $293
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 70
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $292
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $291
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $290
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $289
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 68
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $288
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $287
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $286
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $285
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 67
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $284
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $283
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $282
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $281
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 66
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $280
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 99
byte 1 108
byte 1 101
byte 1 97
byte 1 110
byte 1 82
byte 1 117
byte 1 110
byte 1 82
byte 1 101
byte 1 113
byte 1 117
byte 1 105
byte 1 114
byte 1 101
byte 1 100
byte 1 0
align 1
LABELV $279
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 97
byte 1 120
byte 1 84
byte 1 105
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $278
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 83
byte 1 116
byte 1 121
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $277
byte 1 114
byte 1 97
byte 1 110
byte 1 107
byte 1 65
byte 1 95
byte 1 109
byte 1 105
byte 1 110
byte 1 75
byte 1 105
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $276
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
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 32
byte 1 102
byte 1 111
byte 1 114
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 10
byte 1 0
align 1
LABELV $273
byte 1 110
byte 1 111
byte 1 110
byte 1 101
byte 1 0
align 1
LABELV $270
byte 1 109
byte 1 97
byte 1 112
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 95
byte 1 100
byte 1 105
byte 1 102
byte 1 102
byte 1 105
byte 1 99
byte 1 117
byte 1 108
byte 1 116
byte 1 121
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $265
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 101
byte 1 100
byte 1 32
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $264
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
align 1
LABELV $257
byte 1 110
byte 1 117
byte 1 109
byte 1 0
align 1
LABELV $251
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 97
byte 1 110
byte 1 111
byte 1 117
byte 1 103
byte 1 104
byte 1 32
byte 1 109
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 112
byte 1 111
byte 1 111
byte 1 108
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 108
byte 1 111
byte 1 97
byte 1 100
byte 1 32
byte 1 97
byte 1 108
byte 1 108
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $247
byte 1 37
byte 1 105
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 32
byte 1 112
byte 1 97
byte 1 114
byte 1 115
byte 1 101
byte 1 100
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $246
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 47
byte 1 0
align 1
LABELV $241
byte 1 46
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $240
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $239
byte 1 115
byte 1 99
byte 1 114
byte 1 105
byte 1 112
byte 1 116
byte 1 115
byte 1 47
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
LABELV $236
byte 1 0
align 1
LABELV $235
byte 1 115
byte 1 103
byte 1 95
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 70
byte 1 105
byte 1 108
byte 1 101
byte 1 0
align 1
LABELV $229
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 116
byte 1 111
byte 1 111
byte 1 32
byte 1 108
byte 1 97
byte 1 114
byte 1 103
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 105
byte 1 44
byte 1 32
byte 1 109
byte 1 97
byte 1 120
byte 1 32
byte 1 97
byte 1 108
byte 1 108
byte 1 111
byte 1 119
byte 1 101
byte 1 100
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $226
byte 1 94
byte 1 49
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 58
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 102
byte 1 111
byte 1 117
byte 1 110
byte 1 100
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 10
byte 1 0
align 1
LABELV $220
byte 1 92
byte 1 110
byte 1 117
byte 1 109
byte 1 92
byte 1 0
align 1
LABELV $219
byte 1 60
byte 1 78
byte 1 85
byte 1 76
byte 1 76
byte 1 62
byte 1 0
align 1
LABELV $214
byte 1 117
byte 1 110
byte 1 101
byte 1 120
byte 1 112
byte 1 101
byte 1 99
byte 1 116
byte 1 101
byte 1 100
byte 1 32
byte 1 101
byte 1 110
byte 1 100
byte 1 32
byte 1 111
byte 1 102
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 10
byte 1 0
align 1
LABELV $208
byte 1 109
byte 1 97
byte 1 120
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 115
byte 1 32
byte 1 101
byte 1 120
byte 1 99
byte 1 101
byte 1 101
byte 1 100
byte 1 101
byte 1 100
byte 1 10
byte 1 0
align 1
LABELV $205
byte 1 109
byte 1 105
byte 1 115
byte 1 115
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 39
byte 1 123
byte 1 39
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 10
byte 1 0
align 1
LABELV $190
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
LABELV $189
byte 1 69
byte 1 110
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $181
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 68
byte 1 97
byte 1 116
byte 1 97
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
byte 1 102
byte 1 101
byte 1 116
byte 1 99
byte 1 104
byte 1 32
byte 1 34
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 34
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
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
byte 1 33
byte 1 0
align 1
LABELV $178
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 0
align 1
LABELV $170
byte 1 115
byte 1 103
byte 1 95
byte 1 99
byte 1 97
byte 1 109
byte 1 101
byte 1 114
byte 1 97
byte 1 95
byte 1 112
byte 1 111
byte 1 115
byte 1 105
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $169
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 95
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $167
byte 1 99
byte 1 104
byte 1 101
byte 1 99
byte 1 107
byte 1 112
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 95
byte 1 105
byte 1 110
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $166
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $156
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $154
byte 1 68
byte 1 111
byte 1 110
byte 1 101
byte 1 46
byte 1 0
align 1
LABELV $148
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $147
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
byte 1 100
byte 1 101
byte 1 120
byte 1 0
align 1
LABELV $144
byte 1 109
byte 1 97
byte 1 112
byte 1 115
byte 1 47
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $141
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 72
byte 1 101
byte 1 105
byte 1 103
byte 1 104
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $138
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 87
byte 1 105
byte 1 100
byte 1 116
byte 1 104
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $135
byte 1 83
byte 1 112
byte 1 97
byte 1 119
byte 1 110
byte 1 32
byte 1 67
byte 1 111
byte 1 117
byte 1 110
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $132
byte 1 67
byte 1 104
byte 1 101
byte 1 99
byte 1 107
byte 1 112
byte 1 111
byte 1 105
byte 1 110
byte 1 116
byte 1 32
byte 1 67
byte 1 111
byte 1 117
byte 1 110
byte 1 116
byte 1 58
byte 1 32
byte 1 37
byte 1 105
byte 1 10
byte 1 0
align 1
LABELV $129
byte 1 77
byte 1 97
byte 1 112
byte 1 32
byte 1 78
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
LABELV $128
byte 1 10
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
byte 1 76
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 73
byte 1 110
byte 1 102
byte 1 111
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
LABELV $118
byte 1 37
byte 1 115
byte 1 95
byte 1 37
byte 1 115
byte 1 95
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $115
byte 1 83
byte 1 116
byte 1 97
byte 1 114
byte 1 116
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 117
byte 1 112
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $114
byte 1 67
byte 1 111
byte 1 117
byte 1 108
byte 1 100
byte 1 110
byte 1 39
byte 1 116
byte 1 32
byte 1 102
byte 1 105
byte 1 110
byte 1 100
byte 1 32
byte 1 108
byte 1 101
byte 1 118
byte 1 101
byte 1 108
byte 1 32
byte 1 105
byte 1 110
byte 1 102
byte 1 111
byte 1 32
byte 1 102
byte 1 111
byte 1 114
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 0
align 1
LABELV $111
byte 1 109
byte 1 97
byte 1 112
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 0
