code
proc Cheat_GetToggle 84 12
file "../sg_cmds.c"
line 4
;1:#include "sg_local.h"
;2:
;3:static int Cheat_GetToggle( const char *cheatname )
;4:{
line 7
;5:    char num[64];
;6:
;7:    if ( trap_Argc() != 2 ) {
ADDRLP4 64
ADDRGP4 trap_Argc
CALLI4
ASGNI4
ADDRLP4 64
INDIRI4
CNSTI4 2
EQI4 $90
line 8
;8:        G_Printf( "usage: %s <1|on|toggleon> or <0|off|toggleoff>\n", cheatname );
ADDRGP4 $92
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 9
;9:    }
LABELV $90
line 11
;10:    
;11:    trap_Argv( 1, num, sizeof(num) );
CNSTI4 1
ARGI4
ADDRLP4 0
ARGP4
CNSTI4 64
ARGI4
ADDRGP4 trap_Argv
CALLV
pop
line 13
;12:
;13:    if ( num[0] == '1' || !N_stricmp( num, "on" ) || !N_stricmp( num, "toggleon" ) ) {
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 49
EQI4 $98
ADDRLP4 0
ARGP4
ADDRGP4 $95
ARGP4
ADDRLP4 68
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 68
INDIRI4
CNSTI4 0
EQI4 $98
ADDRLP4 0
ARGP4
ADDRGP4 $96
ARGP4
ADDRLP4 72
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 72
INDIRI4
CNSTI4 0
NEI4 $93
LABELV $98
line 14
;14:        G_Printf( "Cheat '%s' toggled on.\n", cheatname );
ADDRGP4 $99
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 15
;15:        return qtrue;
CNSTI4 1
RETI4
ADDRGP4 $89
JUMPV
LABELV $93
line 16
;16:    } else if ( num[0] == '0' || !N_stricmp( num, "off" ) || !N_stricmp( num, "toggleoff" ) ) {
ADDRLP4 0
INDIRI1
CVII4 1
CNSTI4 48
EQI4 $105
ADDRLP4 0
ARGP4
ADDRGP4 $102
ARGP4
ADDRLP4 76
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 76
INDIRI4
CNSTI4 0
EQI4 $105
ADDRLP4 0
ARGP4
ADDRGP4 $103
ARGP4
ADDRLP4 80
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 80
INDIRI4
CNSTI4 0
NEI4 $100
LABELV $105
line 17
;17:        G_Printf( "Cheat '%s' toggled off.\n", cheatname );
ADDRGP4 $106
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 18
;18:        return qfalse;
CNSTI4 0
RETI4
ADDRGP4 $89
JUMPV
LABELV $100
line 21
;19:    }
;20:    
;21:    G_Printf( "WARNING: unknown option '%s' for cheat command\n", num );
ADDRGP4 $107
ARGP4
ADDRLP4 0
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 22
;22:    return qfalse;
CNSTI4 0
RETI4
LABELV $89
endproc Cheat_GetToggle 84 12
proc Cheat_InfiniteHealth_f 8 8
line 25
;23:}
;24:
;25:static void Cheat_InfiniteHealth_f( void ) {
line 26
;26:    Cvar_Set( "sgc_infiniteHealth", va( "%i", Cheat_GetToggle( "i_am_built_different" ) ) );
ADDRGP4 $111
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $109
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 27
;27:}
LABELV $108
endproc Cheat_InfiniteHealth_f 8 8
proc Cheat_InfiniteAmmo_f 8 8
line 29
;28:
;29:static void Cheat_InfiniteAmmo_f( void ) {
line 30
;30:    Cvar_Set( "sgc_infiniteAmmo", va( "%i", Cheat_GetToggle( "i_need_more_BOOLETS" ) ) );
ADDRGP4 $114
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $113
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 31
;31:}
LABELV $112
endproc Cheat_InfiniteAmmo_f 8 8
proc Cheat_InfiniteRage_f 8 8
line 33
;32:
;33:static void Cheat_InfiniteRage_f( void ) {
line 34
;34:    Cvar_Set( "sgc_infiniteRage", va( "%i", Cheat_GetToggle( "too_angry_to_die" ) ) );
ADDRGP4 $117
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $116
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 35
;35:}
LABELV $115
endproc Cheat_InfiniteRage_f 8 8
proc Cheat_GodMode_f 8 8
line 37
;36:
;37:static void Cheat_GodMode_f( void ) {
line 38
;38:    Cvar_Set( "sgc_godmode", va( "%i", Cheat_GetToggle( "godmode" ) ) );
ADDRGP4 $120
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $119
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 39
;39:}
LABELV $118
endproc Cheat_GodMode_f 8 8
proc Cheats_Set 8 8
line 41
;40:
;41:static void Cheats_Set( int toggle ) {
line 43
;42:    const char *str;
;43:    str = va( "%i", toggle );
ADDRGP4 $110
ARGP4
ADDRFP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 4
INDIRP4
ASGNP4
line 45
;44:
;45:    Cvar_Set( "sgc_infiniteHealth", str );
ADDRGP4 $109
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 46
;46:    Cvar_Set( "sgc_infiniteAmmo", str );
ADDRGP4 $113
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 47
;47:    Cvar_Set( "sgc_infiniteRage", str );
ADDRGP4 $116
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 48
;48:    Cvar_Set( "sgc_godmode", str );
ADDRGP4 $119
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 49
;49:    Cvar_Set( "sgc_blindMobs", str );
ADDRGP4 $122
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 50
;50:    Cvar_Set( "sgc_deafMobs", str );
ADDRGP4 $123
ARGP4
ADDRLP4 0
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 51
;51:}
LABELV $121
endproc Cheats_Set 8 8
proc Cheat_EnableAll_f 0 8
line 53
;52:
;53:static void Cheat_EnableAll_f( void ) {
line 54
;54:    Con_Printf( "Enabling all cheats" );
ADDRGP4 $125
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 55
;55:    Cvar_Set( "sg_cheatsOn", "1" );
ADDRGP4 $126
ARGP4
ADDRGP4 $127
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 56
;56:}
LABELV $124
endproc Cheat_EnableAll_f 0 8
proc Cheat_DisableAll_f 0 8
line 58
;57:
;58:static void Cheat_DisableAll_f( void ) {
line 59
;59:    Con_Printf( "Disabling cheats.\n" );
ADDRGP4 $129
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 60
;60:    Cvar_Set( "sg_cheatsOn", "0" );
ADDRGP4 $126
ARGP4
ADDRGP4 $130
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 61
;61:}
LABELV $128
endproc Cheat_DisableAll_f 0 8
proc Cheat_BlindMobs_f 8 8
line 63
;62:
;63:static void Cheat_BlindMobs_f( void ) {
line 64
;64:    Cvar_Set( "sgc_blindMobs", va( "%i", Cheat_GetToggle( "blindmobs" ) ) );
ADDRGP4 $132
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $122
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 65
;65:}
LABELV $131
endproc Cheat_BlindMobs_f 8 8
proc Cheat_DeafMobs_f 8 8
line 67
;66:
;67:static void Cheat_DeafMobs_f( void ) {
line 68
;68:    Cvar_Set( "sgc_deafMobs", va( "%i", Cheat_GetToggle( "deafmobs" ) ) );
ADDRGP4 $134
ARGP4
ADDRLP4 0
ADDRGP4 Cheat_GetToggle
CALLI4
ASGNI4
ADDRGP4 $110
ARGP4
ADDRLP4 0
INDIRI4
ARGI4
ADDRLP4 4
ADDRGP4 va
CALLP4
ASGNP4
ADDRGP4 $123
ARGP4
ADDRLP4 4
INDIRP4
ARGP4
ADDRGP4 Cvar_Set
CALLV
pop
line 69
;69:}
LABELV $133
endproc Cheat_DeafMobs_f 8 8
data
align 4
LABELV commands
address $111
address Cheat_InfiniteHealth_f
address $136
address Cheat_InfiniteHealth_f
address $114
address Cheat_InfiniteAmmo_f
address $137
address Cheat_InfiniteAmmo_f
address $120
address Cheat_GodMode_f
address $138
address Cheat_GodMode_f
address $132
address Cheat_BlindMobs_f
address $134
address Cheat_DeafMobs_f
address $139
address Cheat_EnableAll_f
address $140
address Cheat_EnableAll_f
address $141
address Cheat_DisableAll_f
address $142
address Cheat_DisableAll_f
address $117
address Cheat_InfiniteRage_f
address $143
address Cheat_InfiniteRage_f
export SGameCommand
code
proc SGameCommand 1032 12
line 94
;70:
;71:typedef struct {
;72:    const char *name;
;73:    void (*fn)( void );
;74:} cmd_t;
;75:
;76:static const cmd_t commands[] = {
;77:    { "i_am_built_different", Cheat_InfiniteHealth_f },
;78:    { "gmathitw", Cheat_InfiniteHealth_f },
;79:    { "i_need_more_BOOLETS", Cheat_InfiniteAmmo_f },
;80:    { "gmataitw", Cheat_InfiniteAmmo_f },
;81:    { "godmode", Cheat_GodMode_f },
;82:    { "iwtbag", Cheat_GodMode_f },
;83:    { "blindmobs", Cheat_BlindMobs_f },
;84:    { "deafmobs", Cheat_DeafMobs_f },
;85:    { "iamacheater", Cheat_EnableAll_f },
;86:    { "iamapussy", Cheat_EnableAll_f },
;87:    { "iamnotacheater", Cheat_DisableAll_f },
;88:    { "ihavetheballs", Cheat_DisableAll_f },
;89:    { "too_angry_to_die", Cheat_InfiniteRage_f },
;90:    { "iambatman", Cheat_InfiniteRage_f }
;91:};
;92:
;93:void SGameCommand( void )
;94:{
line 98
;95:    char cmd[MAX_TOKEN_CHARS];
;96:    int i;
;97:
;98:    trap_Argv( 0, cmd, sizeof(cmd) );
CNSTI4 0
ARGI4
ADDRLP4 4
ARGP4
CNSTI4 1024
ARGI4
ADDRGP4 trap_Argv
CALLV
pop
line 100
;99:
;100:    for ( i = 0; i < arraylen( commands ); i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $148
JUMPV
LABELV $145
line 101
;101:        if ( !N_stricmp( commands[i].name, cmd ) ) {
ADDRLP4 0
INDIRI4
CNSTI4 3
LSHI4
ADDRGP4 commands
ADDP4
INDIRP4
ARGP4
ADDRLP4 4
ARGP4
ADDRLP4 1028
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 1028
INDIRI4
CNSTI4 0
NEI4 $149
line 102
;102:            commands[i].fn();
ADDRLP4 0
INDIRI4
CNSTI4 3
LSHI4
ADDRGP4 commands+4
ADDP4
INDIRP4
CALLV
pop
line 103
;103:        }
LABELV $149
line 104
;104:    }
LABELV $146
line 100
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $148
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 14
LTU4 $145
line 105
;105:}
LABELV $144
endproc SGameCommand 1032 12
export SG_InitCommands
proc SG_InitCommands 4 4
line 108
;106:
;107:void SG_InitCommands( void )
;108:{
line 111
;109:    int i;
;110:
;111:    for ( i = 0; i < arraylen( commands ); i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $156
JUMPV
LABELV $153
line 112
;112:        trap_AddCommand( commands[i].name );
ADDRLP4 0
INDIRI4
CNSTI4 3
LSHI4
ADDRGP4 commands
ADDP4
INDIRP4
ARGP4
ADDRGP4 trap_AddCommand
CALLV
pop
line 113
;113:    }
LABELV $154
line 111
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $156
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 14
LTU4 $153
line 114
;114:}
LABELV $152
endproc SG_InitCommands 4 4
export SG_ShutdownCommands
proc SG_ShutdownCommands 4 4
line 117
;115:
;116:void SG_ShutdownCommands( void )
;117:{
line 120
;118:    int i;
;119:
;120:    for ( i = 0; i < arraylen( commands ); i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $161
JUMPV
LABELV $158
line 121
;121:        trap_RemoveCommand( commands[i].name );
ADDRLP4 0
INDIRI4
CNSTI4 3
LSHI4
ADDRGP4 commands
ADDP4
INDIRP4
ARGP4
ADDRGP4 trap_RemoveCommand
CALLV
pop
line 122
;122:    }
LABELV $159
line 120
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $161
ADDRLP4 0
INDIRI4
CVIU4 4
CNSTU4 14
LTU4 $158
line 123
;123:}
LABELV $157
endproc SG_ShutdownCommands 4 4
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
import SG_LoadLevels
import SG_LoadLevelData
import SG_SaveLevelData
import SG_EndLevel
import SG_StartLevel
import SG_UpdateCvars
import G_Printf
import G_Error
import SG_Printf
import SG_Error
import SG_DrawFrame
import sgc_godmode
import sg_cheatsOn
import sgc_deafMobs
import sgc_blindMobs
import sgc_infiniteAmmo
import sgc_infiniteRage
import sgc_infiniteHealth
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
import sg_gameDifficulty
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
LABELV $143
byte 1 105
byte 1 97
byte 1 109
byte 1 98
byte 1 97
byte 1 116
byte 1 109
byte 1 97
byte 1 110
byte 1 0
align 1
LABELV $142
byte 1 105
byte 1 104
byte 1 97
byte 1 118
byte 1 101
byte 1 116
byte 1 104
byte 1 101
byte 1 98
byte 1 97
byte 1 108
byte 1 108
byte 1 115
byte 1 0
align 1
LABELV $141
byte 1 105
byte 1 97
byte 1 109
byte 1 110
byte 1 111
byte 1 116
byte 1 97
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 101
byte 1 114
byte 1 0
align 1
LABELV $140
byte 1 105
byte 1 97
byte 1 109
byte 1 97
byte 1 112
byte 1 117
byte 1 115
byte 1 115
byte 1 121
byte 1 0
align 1
LABELV $139
byte 1 105
byte 1 97
byte 1 109
byte 1 97
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 101
byte 1 114
byte 1 0
align 1
LABELV $138
byte 1 105
byte 1 119
byte 1 116
byte 1 98
byte 1 97
byte 1 103
byte 1 0
align 1
LABELV $137
byte 1 103
byte 1 109
byte 1 97
byte 1 116
byte 1 97
byte 1 105
byte 1 116
byte 1 119
byte 1 0
align 1
LABELV $136
byte 1 103
byte 1 109
byte 1 97
byte 1 116
byte 1 104
byte 1 105
byte 1 116
byte 1 119
byte 1 0
align 1
LABELV $134
byte 1 100
byte 1 101
byte 1 97
byte 1 102
byte 1 109
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $132
byte 1 98
byte 1 108
byte 1 105
byte 1 110
byte 1 100
byte 1 109
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $130
byte 1 48
byte 1 0
align 1
LABELV $129
byte 1 68
byte 1 105
byte 1 115
byte 1 97
byte 1 98
byte 1 108
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 115
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $127
byte 1 49
byte 1 0
align 1
LABELV $126
byte 1 115
byte 1 103
byte 1 95
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 115
byte 1 79
byte 1 110
byte 1 0
align 1
LABELV $125
byte 1 69
byte 1 110
byte 1 97
byte 1 98
byte 1 108
byte 1 105
byte 1 110
byte 1 103
byte 1 32
byte 1 97
byte 1 108
byte 1 108
byte 1 32
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 115
byte 1 0
align 1
LABELV $123
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 100
byte 1 101
byte 1 97
byte 1 102
byte 1 77
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $122
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 98
byte 1 108
byte 1 105
byte 1 110
byte 1 100
byte 1 77
byte 1 111
byte 1 98
byte 1 115
byte 1 0
align 1
LABELV $120
byte 1 103
byte 1 111
byte 1 100
byte 1 109
byte 1 111
byte 1 100
byte 1 101
byte 1 0
align 1
LABELV $119
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 103
byte 1 111
byte 1 100
byte 1 109
byte 1 111
byte 1 100
byte 1 101
byte 1 0
align 1
LABELV $117
byte 1 116
byte 1 111
byte 1 111
byte 1 95
byte 1 97
byte 1 110
byte 1 103
byte 1 114
byte 1 121
byte 1 95
byte 1 116
byte 1 111
byte 1 95
byte 1 100
byte 1 105
byte 1 101
byte 1 0
align 1
LABELV $116
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 82
byte 1 97
byte 1 103
byte 1 101
byte 1 0
align 1
LABELV $114
byte 1 105
byte 1 95
byte 1 110
byte 1 101
byte 1 101
byte 1 100
byte 1 95
byte 1 109
byte 1 111
byte 1 114
byte 1 101
byte 1 95
byte 1 66
byte 1 79
byte 1 79
byte 1 76
byte 1 69
byte 1 84
byte 1 83
byte 1 0
align 1
LABELV $113
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 65
byte 1 109
byte 1 109
byte 1 111
byte 1 0
align 1
LABELV $111
byte 1 105
byte 1 95
byte 1 97
byte 1 109
byte 1 95
byte 1 98
byte 1 117
byte 1 105
byte 1 108
byte 1 116
byte 1 95
byte 1 100
byte 1 105
byte 1 102
byte 1 102
byte 1 101
byte 1 114
byte 1 101
byte 1 110
byte 1 116
byte 1 0
align 1
LABELV $110
byte 1 37
byte 1 105
byte 1 0
align 1
LABELV $109
byte 1 115
byte 1 103
byte 1 99
byte 1 95
byte 1 105
byte 1 110
byte 1 102
byte 1 105
byte 1 110
byte 1 105
byte 1 116
byte 1 101
byte 1 72
byte 1 101
byte 1 97
byte 1 108
byte 1 116
byte 1 104
byte 1 0
align 1
LABELV $107
byte 1 87
byte 1 65
byte 1 82
byte 1 78
byte 1 73
byte 1 78
byte 1 71
byte 1 58
byte 1 32
byte 1 117
byte 1 110
byte 1 107
byte 1 110
byte 1 111
byte 1 119
byte 1 110
byte 1 32
byte 1 111
byte 1 112
byte 1 116
byte 1 105
byte 1 111
byte 1 110
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
byte 1 99
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 32
byte 1 99
byte 1 111
byte 1 109
byte 1 109
byte 1 97
byte 1 110
byte 1 100
byte 1 10
byte 1 0
align 1
LABELV $106
byte 1 67
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 32
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 111
byte 1 102
byte 1 102
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $103
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 111
byte 1 102
byte 1 102
byte 1 0
align 1
LABELV $102
byte 1 111
byte 1 102
byte 1 102
byte 1 0
align 1
LABELV $99
byte 1 67
byte 1 104
byte 1 101
byte 1 97
byte 1 116
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 32
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 111
byte 1 110
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $96
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $95
byte 1 111
byte 1 110
byte 1 0
align 1
LABELV $92
byte 1 117
byte 1 115
byte 1 97
byte 1 103
byte 1 101
byte 1 58
byte 1 32
byte 1 37
byte 1 115
byte 1 32
byte 1 60
byte 1 49
byte 1 124
byte 1 111
byte 1 110
byte 1 124
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 111
byte 1 110
byte 1 62
byte 1 32
byte 1 111
byte 1 114
byte 1 32
byte 1 60
byte 1 48
byte 1 124
byte 1 111
byte 1 102
byte 1 102
byte 1 124
byte 1 116
byte 1 111
byte 1 103
byte 1 103
byte 1 108
byte 1 101
byte 1 111
byte 1 102
byte 1 102
byte 1 62
byte 1 10
byte 1 0
