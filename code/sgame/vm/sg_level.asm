code
proc SG_SpawnLevelEntities 16 0
file "../sg_level.c"
line 30
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
;17:    int32_t index;
;18:
;19:    // save data
;20:    levelstats_t stats;
;21:    uint32_t checkpointIndex;
;22:} levelinfo_t;
;23:
;24:static levelinfo_t level;
;25:
;26://
;27:// SG_SpawnLevelEntities
;28://
;29:static void SG_SpawnLevelEntities(void)
;30:{
line 34
;31:    uint32_t i;
;32:    const mapspawn_t *spawn;
;33:
;34:    spawn = sg.mapInfo.spawns;
ADDRLP4 4
ADDRGP4 sg+4280680
ASGNP4
line 35
;35:    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
ADDRLP4 0
CNSTU4 0
ASGNU4
ADDRGP4 $88
JUMPV
LABELV $85
line 36
;36:        switch ( spawn->entitytype ) {
ADDRLP4 8
ADDRLP4 4
INDIRP4
CNSTI4 12
ADDP4
INDIRU4
ASGNU4
ADDRLP4 12
ADDRLP4 8
INDIRU4
CVUI4 4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 3
GTI4 $91
ADDRLP4 12
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 $97
ADDP4
INDIRP4
JUMPV
lit
align 4
LABELV $97
address $92
address $92
address $92
address $92
code
line 38
;37:        case ET_MOB:
;38:            break;
line 40
;39:        case ET_PLAYR:
;40:            break;
line 43
;41:        case ET_ITEM:
;42:        case ET_WEAPON:
;43:            break;
LABELV $91
LABELV $92
line 44
;44:        };
line 45
;45:    }
LABELV $86
line 35
ADDRLP4 0
ADDRLP4 0
INDIRU4
CNSTU4 1
ADDU4
ASGNU4
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 20
ADDP4
ASGNP4
LABELV $88
ADDRLP4 0
INDIRU4
ADDRGP4 sg+4280680+26888
INDIRU4
LTU4 $85
line 46
;46:}
LABELV $83
endproc SG_SpawnLevelEntities 16 0
export SG_InitLevel
proc SG_InitLevel 48 16
line 49
;47:
;48:qboolean SG_InitLevel( int levelIndex )
;49:{
line 53
;50:    vec2_t cameraPos;
;51:    float zoom;
;52:
;53:    G_Printf( "Starting up level at index %i\n", levelIndex );
ADDRGP4 $99
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 G_Printf
CALLV
pop
line 54
;54:    G_Printf( "Loadng resources...\n" );
ADDRGP4 $100
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 57
;55:
;56:    // clear the old level
;57:    memset( &level, 0, sizeof(level) );
ADDRGP4 level
ARGP4
CNSTI4 0
ARGI4
CNSTU4 28
ARGU4
ADDRGP4 memset
CALLP4
pop
line 59
;58:
;59:    G_Printf( "Loading map from internal cache...\n" );
ADDRGP4 $101
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 61
;60:
;61:    if ( !G_LoadMap( levelIndex, &sg.mapInfo, sg.soundBits, &sg.activeEnts ) ) {
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 sg+4280680
ARGP4
ADDRGP4 sg+86368
ARGP4
ADDRGP4 sg+86328
ARGP4
ADDRLP4 12
ADDRGP4 G_LoadMap
CALLI4
ASGNI4
ADDRLP4 12
INDIRI4
CNSTI4 0
NEI4 $102
line 62
;62:        SG_Printf( COLOR_RED "SG_InitLevel: failed to load map file at index %i\n", levelIndex );
ADDRGP4 $107
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRGP4 SG_Printf
CALLV
pop
line 63
;63:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $98
JUMPV
LABELV $102
line 66
;64:    }
;65:
;66:    G_Printf( "Loading map %s...\n", sg.mapInfo.name );
ADDRGP4 $108
ARGP4
ADDRGP4 sg+4280680+26816
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 68
;67:
;68:    G_Printf( "All done.\n" );
ADDRGP4 $111
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 70
;69:
;70:    SG_InitPlayer();
ADDRGP4 SG_InitPlayer
CALLV
pop
line 73
;71:
;72:    // spawn everything
;73:    SG_SpawnLevelEntities();
ADDRGP4 SG_SpawnLevelEntities
CALLV
pop
line 75
;74:
;75:    sg.state = SG_IN_LEVEL;
ADDRGP4 sg+64
CNSTI4 1
ASGNI4
line 77
;76:
;77:    if ( sg_printLevelStats.i ) {
ADDRGP4 sg_printLevelStats+260
INDIRI4
CNSTI4 0
EQI4 $113
line 78
;78:        G_Printf( "\n---------- Level Info ----------\n" );
ADDRGP4 $116
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 79
;79:        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
ADDRGP4 $117
ARGP4
ADDRGP4 sg+4280680+26816
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 80
;80:        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
ADDRGP4 $120
ARGP4
ADDRGP4 sg+4280680+26892
INDIRU4
ARGU4
ADDRGP4 G_Printf
CALLV
pop
line 81
;81:        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
ADDRGP4 $123
ARGP4
ADDRGP4 sg+4280680+26888
INDIRU4
ARGU4
ADDRGP4 G_Printf
CALLV
pop
line 82
;82:        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
ADDRGP4 $126
ARGP4
ADDRGP4 sg+4280680+26880
INDIRU4
ARGU4
ADDRGP4 G_Printf
CALLV
pop
line 83
;83:        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
ADDRGP4 $129
ARGP4
ADDRGP4 sg+4280680+26884
INDIRU4
ARGU4
ADDRGP4 G_Printf
CALLV
pop
line 84
;84:    }
LABELV $113
line 86
;85:
;86:    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );
ADDRGP4 $132
ARGP4
ADDRGP4 sg+4280680+26816
ARGP4
ADDRLP4 16
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 16
INDIRP4
ARGP4
ADDRGP4 RE_LoadWorldMap
CALLV
pop
line 88
;87:
;88:    Cvar_Set( "sg_levelIndex", va( "%i", levelIndex ) );
ADDRGP4 $136
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRLP4 20
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $135
ARGP4
ADDRLP4 20
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 90
;89:
;90:    level.stats.timeStart = trap_Milliseconds();
ADDRLP4 24
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4
ADDRLP4 24
INDIRI4
CVIU4 4
ASGNU4
line 92
;91:
;92:    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
ADDRLP4 28
CNSTF4 1073741824
ASGNF4
ADDRLP4 32
ADDRGP4 sg+4280680
INDIRU4
ASGNU4
ADDRLP4 36
CNSTI4 1
ASGNI4
ADDRLP4 40
CNSTU4 1
ASGNU4
ADDRLP4 0
ADDRLP4 28
INDIRF4
ADDRLP4 32
INDIRU4
ADDRLP4 36
INDIRI4
RSHU4
CVUI4 4
CVIF4 4
MULF4
ADDRLP4 32
INDIRU4
ADDRLP4 40
INDIRU4
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
ADDRLP4 44
ADDRGP4 sg+4280680+4
INDIRU4
ASGNU4
ADDRLP4 0+4
ADDRLP4 28
INDIRF4
ADDRLP4 44
INDIRU4
ADDRLP4 36
INDIRI4
RSHU4
CVUI4 4
CVIF4 4
MULF4
ADDRLP4 44
INDIRU4
ADDRLP4 40
INDIRU4
BANDU4
CVUI4 4
CVIF4 4
ADDF4
ASGNF4
line 93
;93:    zoom = 1.6f;
ADDRLP4 8
CNSTF4 1070386381
ASGNF4
line 97
;94:
;95://    G_SetCameraData( cameraPos, zoom, 0.0f );
;96:
;97:    return qtrue;
CNSTI4 1
RETI4
LABELV $98
endproc SG_InitLevel 48 16
export SG_SaveLevelData
proc SG_SaveLevelData 0 0
line 101
;98:}
;99:
;100:void SG_SaveLevelData( void )
;101:{
line 102
;102:}
LABELV $142
endproc SG_SaveLevelData 0 0
export SG_DrawLevelStats
proc SG_DrawLevelStats 20 8
line 112
;103:
;104:typedef struct
;105:{
;106:    ImGuiWindow window;
;107:} endlevelScreen_t;
;108:
;109:static endlevelScreen_t endLevel;
;110:
;111:void SG_DrawLevelStats( void )
;112:{
line 116
;113:    float font_scale;
;114:    vec2_t cursorPos;
;115:
;116:    font_scale = ImGui_GetFontScale();
ADDRLP4 12
ADDRGP4 ImGui_GetFontScale
CALLF4
ASGNF4
ADDRLP4 0
ADDRLP4 12
INDIRF4
ASGNF4
line 118
;117:
;118:    if ( ImGui_BeginWindow( &endLevel.window ) ) {
ADDRGP4 endLevel
ARGP4
ADDRLP4 16
ADDRGP4 ImGui_BeginWindow
CALLI4
ASGNI4
ADDRLP4 16
INDIRI4
CNSTI4 0
EQI4 $145
line 119
;119:        ImGui_SetWindowFontScale(font_scale * 6);
CNSTF4 1086324736
ADDRLP4 0
INDIRF4
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 120
;120:        ImGui_TextUnformatted("Level Statistics");
ADDRGP4 $147
ARGP4
ADDRGP4 ImGui_TextUnformatted
CALLV
pop
line 121
;121:        ImGui_SetWindowFontScale(font_scale * 3.5f);
CNSTF4 1080033280
ADDRLP4 0
INDIRF4
MULF4
ARGF4
ADDRGP4 ImGui_SetWindowFontScale
CALLV
pop
line 122
;122:        ImGui_NewLine();
ADDRGP4 ImGui_NewLine
CALLV
pop
line 124
;123:
;124:        ImGui_GetCursorScreenPos(&cursorPos[0], &cursorPos[1]);
ADDRLP4 4
ARGP4
ADDRLP4 4+4
ARGP4
ADDRGP4 ImGui_GetCursorScreenPos
CALLV
pop
line 126
;125:
;126:        ImGui_SetCursorScreenPos(cursorPos[0], cursorPos[1] + 20);
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
line 127
;127:    }
LABELV $145
line 128
;128:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 129
;129:}
LABELV $144
endproc SG_DrawLevelStats 20 8
export SG_EndLevel
proc SG_EndLevel 4 12
line 132
;130:
;131:int SG_EndLevel( void )
;132:{
line 134
;133:    // setup the window
;134:    memset( &endLevel, 0, sizeof(endLevel) );
ADDRGP4 endLevel
ARGP4
CNSTI4 0
ARGI4
CNSTU4 16
ARGU4
ADDRGP4 memset
CALLP4
pop
line 136
;135:
;136:    level.stats.timeEnd = trap_Milliseconds();
ADDRLP4 0
ADDRGP4 trap_Milliseconds
CALLI4
ASGNI4
ADDRGP4 level+4+4
ADDRLP4 0
INDIRI4
CVIU4 4
ASGNU4
line 138
;137:
;138:    endLevel.window.m_Flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
ADDRGP4 endLevel+12
CNSTI4 7
ASGNI4
line 139
;139:    endLevel.window.m_pTitle = "endLevel";
ADDRGP4 endLevel
ADDRGP4 $154
ASGNP4
line 140
;140:    endLevel.window.m_bOpen = qtrue;
ADDRGP4 endLevel+8
CNSTU4 1
ASGNU4
line 141
;141:    endLevel.window.m_bClosable = qfalse;
ADDRGP4 endLevel+4
CNSTU4 0
ASGNU4
line 143
;142:
;143:    sg.state = SG_SHOW_LEVEL_STATS;
ADDRGP4 sg+64
CNSTI4 2
ASGNI4
line 145
;144:
;145:    return 1;
CNSTI4 1
RETI4
LABELV $150
endproc SG_EndLevel 4 12
bss
align 4
LABELV endLevel
skip 16
align 4
LABELV level
skip 28
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
import SG_DrawAbortMission
import Lvl_AddKillEntity
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
import sg_numSaves
import sg_savename
import sg_levelDataFile
import sg_levelIndex
import sg_levelInfoFile
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
import disBetweenOBJ
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
LABELV $154
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
LABELV $147
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
LABELV $136
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $135
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
LABELV $132
byte 1 109
byte 1 97
byte 1 112
byte 1 115
byte 1 47
byte 1 37
byte 1 115
byte 1 0
align 1
LABELV $129
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
LABELV $126
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
LABELV $123
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
LABELV $120
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
LABELV $117
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
LABELV $116
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
LABELV $111
byte 1 65
byte 1 108
byte 1 108
byte 1 32
byte 1 100
byte 1 111
byte 1 110
byte 1 101
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $108
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 37
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $107
byte 1 94
byte 1 49
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
LABELV $101
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 109
byte 1 97
byte 1 112
byte 1 32
byte 1 102
byte 1 114
byte 1 111
byte 1 109
byte 1 32
byte 1 105
byte 1 110
byte 1 116
byte 1 101
byte 1 114
byte 1 110
byte 1 97
byte 1 108
byte 1 32
byte 1 99
byte 1 97
byte 1 99
byte 1 104
byte 1 101
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $100
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 110
byte 1 103
byte 1 32
byte 1 114
byte 1 101
byte 1 115
byte 1 111
byte 1 117
byte 1 114
byte 1 99
byte 1 101
byte 1 115
byte 1 46
byte 1 46
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $99
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
