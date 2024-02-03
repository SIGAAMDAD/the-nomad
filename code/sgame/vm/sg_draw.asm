code
proc EntityIsInView 4 8
file "../sg_draw.c"
line 14
;1:#include "sg_local.h"
;2:#include "sg_imgui.h"
;3:
;4:typedef struct {
;5:    int polyCount;
;6:    int vertexCount;
;7:    float realCamWidth;
;8:    float realCamHeight;
;9:    bbox_t frustum;
;10:} drawdata_t;
;11:
;12:static drawdata_t data;
;13:
;14:static qboolean EntityIsInView( const bbox_t *bounds ) {
line 15
;15:    return BoundsIntersect( bounds, &data.frustum );
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 data+16
ARGP4
ADDRLP4 0
ADDRGP4 BoundsIntersect
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
RETI4
LABELV $90
endproc EntityIsInView 4 8
proc SG_DrawEntities 4 0
line 19
;16:}
;17:
;18:static void SG_DrawEntities( void )
;19:{
line 21
;20:    const sgentity_t *ent;
;21:}
LABELV $92
endproc SG_DrawEntities 4 0
proc SG_DrawPlayer 8 12
line 28
;22:
;23:/*
;24:* SG_DrawPlayer: draws the player, the player's sprite is special
;25:* because it has two parts to draw
;26:*/
;27:static void SG_DrawPlayer( void )
;28:{
line 31
;29:    const sgentity_t *ent;
;30:
;31:    ent = sg.playr.ent;
ADDRLP4 0
ADDRGP4 sg+65760
INDIRP4
ASGNP4
line 33
;32:
;33:    if ( ent->state->frames ) {
ADDRLP4 0
INDIRP4
CNSTI4 92
ADDP4
INDIRP4
CNSTI4 4
ADDP4
INDIRU4
CNSTU4 0
EQU4 $95
line 34
;34:        RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ent->sprite + ent->frame );
ADDRLP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 156
ADDP4
INDIRI4
ARGI4
ADDRLP4 0
INDIRP4
CNSTI4 152
ADDP4
INDIRI4
ADDRLP4 0
INDIRP4
CNSTI4 120
ADDP4
INDIRI4
ADDI4
ARGI4
ADDRGP4 RE_AddSpriteToScene
CALLV
pop
line 35
;35:    } else {
ADDRGP4 $96
JUMPV
LABELV $95
line 36
;36:        RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ent->sprite );
ADDRLP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 156
ADDP4
INDIRI4
ARGI4
ADDRLP4 0
INDIRP4
CNSTI4 152
ADDP4
INDIRI4
ARGI4
ADDRGP4 RE_AddSpriteToScene
CALLV
pop
line 37
;37:    }
LABELV $96
line 39
;38:
;39:    RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ( sg.playr.foot_sprite + ent->facing ) + sg.playr.foot_frame );
ADDRLP4 0
INDIRP4
CNSTI4 64
ADDP4
ARGP4
ADDRLP4 0
INDIRP4
CNSTI4 156
ADDP4
INDIRI4
ARGI4
ADDRGP4 sg+65760+44
INDIRI4
ADDRLP4 0
INDIRP4
CNSTI4 124
ADDP4
INDIRI4
ADDI4
ADDRGP4 sg+65760+48
INDIRI4
ADDI4
ARGI4
ADDRGP4 RE_AddSpriteToScene
CALLV
pop
line 40
;40:}
LABELV $93
endproc SG_DrawPlayer 8 12
proc SG_DrawHUD 52 12
line 43
;41:
;42:static void SG_DrawHUD( void )
;43:{
line 48
;44:    renderSceneRef_t refdef;
;45:    int windowFlags;
;46:    vec4_t color;
;47:
;48:    memset( &refdef, 0, sizeof(refdef) );
ADDRLP4 0
ARGP4
CNSTI4 0
ARGI4
CNSTU4 32
ARGU4
ADDRGP4 memset
CALLI4
pop
line 49
;49:    refdef.x = 0;
ADDRLP4 0
CNSTU4 0
ASGNU4
line 50
;50:    refdef.y = 0;
ADDRLP4 0+4
CNSTU4 0
ASGNU4
line 51
;51:    refdef.width = sg.gpuConfig.vidWidth;
ADDRLP4 0+8
ADDRGP4 sg+65820+12320
INDIRI4
CVIU4 4
ASGNU4
line 52
;52:    refdef.height = sg.gpuConfig.vidHeight;
ADDRLP4 0+12
ADDRGP4 sg+65820+12324
INDIRI4
CVIU4 4
ASGNU4
line 59
;53:
;54:    // draw crosshair
;55:
;56:    //
;57:    // draw status bars
;58:    //
;59:    windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMouseInputs
ADDRLP4 48
CNSTI4 743
ASGNI4
line 62
;60:                | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
;61:
;62:    ImGui_BeginWindow( "PlayerHealth", NULL, windowFlags );
ADDRGP4 $109
ARGP4
CNSTP4 0
ARGP4
ADDRLP4 48
INDIRI4
ARGI4
ADDRGP4 ImGui_BeginWindow
CALLI4
pop
line 63
;63:    if ( sg.playr.ent->health <= 65 ) {
ADDRGP4 sg+65760
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CNSTI4 65
GTI4 $110
line 64
;64:        color.r = ( 1.0 / (float)sg.playr.ent->health ) * 2.0f;
ADDRLP4 32
CNSTF4 1065353216
ADDRGP4 sg+65760
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CVIF4 4
DIVF4
CNSTF4 1073741824
MULF4
ASGNF4
line 65
;65:        color.g = ( 1.0 / (float)sg.playr.ent->health );
ADDRLP4 32+4
CNSTF4 1065353216
ADDRGP4 sg+65760
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CVIF4 4
DIVF4
ASGNF4
line 66
;66:        color.b = 0;
ADDRLP4 32+8
CNSTF4 0
ASGNF4
line 67
;67:        color.a = 1;
ADDRLP4 32+12
CNSTF4 1065353216
ASGNF4
line 68
;68:    } else {
ADDRGP4 $111
JUMPV
LABELV $110
line 69
;69:        color.r = 0;
ADDRLP4 32
CNSTF4 0
ASGNF4
line 70
;70:        color.g = 1;
ADDRLP4 32+4
CNSTF4 1065353216
ASGNF4
line 71
;71:        color.b = 0;
ADDRLP4 32+8
CNSTF4 0
ASGNF4
line 72
;72:        color.a = 1;
ADDRLP4 32+12
CNSTF4 1065353216
ASGNF4
line 73
;73:    }
LABELV $111
line 74
;74:    ImGui_PushColor( ImGuiCol_FrameBg, &color );
CNSTI4 7
ARGI4
ADDRLP4 32
ARGP4
ADDRGP4 ImGui_PushColor
CALLV
pop
line 75
;75:    ImGui_ProgressBar( (float)sg.playr.ent->health );
ADDRGP4 sg+65760
INDIRP4
CNSTI4 112
ADDP4
INDIRI4
CVIF4 4
ARGF4
ADDRGP4 ImGui_ProgressBar
CALLV
pop
line 76
;76:    ImGui_PopColor();
ADDRGP4 ImGui_PopColor
CALLV
pop
line 77
;77:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 79
;78:
;79:    ImGui_BeginWindow( "PlayerRageMeter", NULL, windowFlags );
ADDRGP4 $122
ARGP4
CNSTP4 0
ARGP4
ADDRLP4 48
INDIRI4
ARGI4
ADDRGP4 ImGui_BeginWindow
CALLI4
pop
line 81
;80:    
;81:    ImGui_EndWindow();
ADDRGP4 ImGui_EndWindow
CALLV
pop
line 82
;82:}
LABELV $101
endproc SG_DrawHUD 52 12
export SG_DrawFrame
proc SG_DrawFrame 56 12
line 85
;83:
;84:int SG_DrawFrame( void )
;85:{
line 89
;86:    renderSceneRef_t refdef;
;87:
;88:    // setup scene
;89:    memset( &refdef, 0, sizeof(refdef) );
ADDRLP4 0
ARGP4
CNSTI4 0
ARGI4
CNSTU4 32
ARGU4
ADDRGP4 memset
CALLI4
pop
line 91
;90:
;91:    refdef.width = data.realCamWidth;
ADDRLP4 36
ADDRGP4 data+8
INDIRF4
ASGNF4
ADDRLP4 40
CNSTF4 1325400064
ASGNF4
ADDRLP4 36
INDIRF4
ADDRLP4 40
INDIRF4
LTF4 $127
ADDRLP4 32
ADDRLP4 36
INDIRF4
ADDRLP4 40
INDIRF4
SUBF4
CVFI4 4
CVIU4 4
CNSTU4 2147483648
ADDU4
ASGNU4
ADDRGP4 $128
JUMPV
LABELV $127
ADDRLP4 32
ADDRLP4 36
INDIRF4
CVFI4 4
CVIU4 4
ASGNU4
LABELV $128
ADDRLP4 0+8
ADDRLP4 32
INDIRU4
ASGNU4
line 92
;92:    refdef.height = data.realCamHeight;
ADDRLP4 48
ADDRGP4 data+12
INDIRF4
ASGNF4
ADDRLP4 52
CNSTF4 1325400064
ASGNF4
ADDRLP4 48
INDIRF4
ADDRLP4 52
INDIRF4
LTF4 $132
ADDRLP4 44
ADDRLP4 48
INDIRF4
ADDRLP4 52
INDIRF4
SUBF4
CVFI4 4
CVIU4 4
CNSTU4 2147483648
ADDU4
ASGNU4
ADDRGP4 $133
JUMPV
LABELV $132
ADDRLP4 44
ADDRLP4 48
INDIRF4
CVFI4 4
CVIU4 4
ASGNU4
LABELV $133
ADDRLP4 0+12
ADDRLP4 44
INDIRU4
ASGNU4
line 93
;93:    refdef.x = 0;
ADDRLP4 0
CNSTU4 0
ASGNU4
line 94
;94:    refdef.y = 0;
ADDRLP4 0+4
CNSTU4 0
ASGNU4
line 95
;95:    refdef.time = sg.levelTime;
ADDRLP4 0+28
ADDRGP4 sg+4228
INDIRI4
CVIU4 4
ASGNU4
line 97
;96:
;97:    G_SetCameraData( &sg.cameraPos, 1.6f, 0.0f );
ADDRGP4 sg+4272524
ARGP4
CNSTF4 1070386381
ARGF4
CNSTF4 0
ARGF4
ADDRGP4 G_SetCameraData
CALLV
pop
line 100
;98:
;99:    // draw entities
;100:    SG_DrawPlayer();
ADDRGP4 SG_DrawPlayer
CALLV
pop
line 103
;101:
;102:    // finish the scene
;103:    RE_ClearScene();
ADDRGP4 RE_ClearScene
CALLV
pop
line 104
;104:    RE_RenderScene( &refdef );
ADDRLP4 0
ARGP4
ADDRGP4 RE_RenderScene
CALLV
pop
line 106
;105:
;106:    return 1;
CNSTI4 1
RETI4
LABELV $123
endproc SG_DrawFrame 56 12
import memset
bss
align 4
LABELV data
skip 40
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
import SG_LoadLevelData
import SG_SaveLevelData
import SG_EndLevel
import SG_StartLevel
import SG_UpdateCvars
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import SG_BuildMoveCommand
import SGameCommand
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
LABELV $122
byte 1 80
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 82
byte 1 97
byte 1 103
byte 1 101
byte 1 77
byte 1 101
byte 1 116
byte 1 101
byte 1 114
byte 1 0
align 1
LABELV $109
byte 1 80
byte 1 108
byte 1 97
byte 1 121
byte 1 101
byte 1 114
byte 1 72
byte 1 101
byte 1 97
byte 1 108
byte 1 116
byte 1 104
byte 1 0
