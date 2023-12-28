export SG_AddArchiveHandle
code
proc SG_AddArchiveHandle 8 4
file "../sg_archive.c"
line 32
;1:#include "sg_local.h"
;2:
;3:#define IDENT (('d'<<24)+('g'<<16)+('n'<<8)+'!')
;4:
;5:typedef struct {
;6:    int ident;
;7:    int version;
;8:    int versionUpdate;
;9:    int versionPatch;
;10:    int numSections;
;11:} ngdheader_t;
;12:
;13:typedef struct {
;14:    char name[4];
;15:    int size;
;16:    int offset;
;17:} ngdsection_t;
;18:
;19:static ngdsection_t *sections;
;20:static int numSections;
;21:static file_t gamefile;
;22:
;23:typedef struct archiveHandle_s {
;24:    archiveFunc_t fn;
;25:    struct archiveHandle_s *next;
;26:    struct archiveHandle_s *prev;
;27:} archiveHandle_t;
;28:
;29:static archiveHandle_t s_archiveHandleList;
;30:
;31:void SG_AddArchiveHandle( archiveFunc_t pFunc )
;32:{
line 36
;33:    archiveHandle_t *handle;
;34:
;35:    // check if we already have it
;36:    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
ADDRLP4 0
ADDRGP4 s_archiveHandleList
ASGNP4
ADDRGP4 $95
JUMPV
LABELV $92
line 37
;37:        if ( handle->fn == pFunc ) {
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
ADDRFP4 0
INDIRP4
CVPU4 4
NEU4 $96
line 38
;38:            return;
ADDRGP4 $91
JUMPV
LABELV $96
line 40
;39:        }
;40:    }
LABELV $93
line 36
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ASGNP4
LABELV $95
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $92
line 43
;41:
;42:    // allocate it on permanent memory
;43:    handle = SG_MemAlloc( sizeof(*handle) );
CNSTI4 12
ARGI4
ADDRLP4 4
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 4
INDIRP4
ASGNP4
line 44
;44:    handle->fn = pFunc;
ADDRLP4 0
INDIRP4
ADDRFP4 0
INDIRP4
ASGNP4
line 47
;45:
;46:    // link into the list
;47:	handle->next = s_archiveHandleList.next;
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
ADDRGP4 s_archiveHandleList+4
INDIRP4
ASGNP4
line 48
;48:	handle->prev = &s_archiveHandleList;
ADDRLP4 0
INDIRP4
CNSTI4 8
ADDP4
ADDRGP4 s_archiveHandleList
ASGNP4
line 49
;49:	s_archiveHandleList.next->prev = handle;
ADDRGP4 s_archiveHandleList+4
INDIRP4
CNSTI4 8
ADDP4
ADDRLP4 0
INDIRP4
ASGNP4
line 50
;50:	s_archiveHandleList.next = handle;
ADDRGP4 s_archiveHandleList+4
ADDRLP4 0
INDIRP4
ASGNP4
line 51
;51:}
LABELV $91
endproc SG_AddArchiveHandle 8 4
export SG_SaveGame
proc SG_SaveGame 8 8
line 54
;52:
;53:int SG_SaveGame( void )
;54:{
line 57
;55:    archiveHandle_t *handle;
;56:
;57:    gamefile = trap_FS_FOpenWrite( sg_savename.s );
ADDRGP4 sg_savename
ARGP4
ADDRLP4 4
ADDRGP4 trap_FS_FOpenWrite
CALLI4
ASGNI4
ADDRGP4 gamefile
ADDRLP4 4
INDIRI4
ASGNI4
line 58
;58:    if ( gamefile == FS_INVALID_HANDLE ) {
ADDRGP4 gamefile
INDIRI4
CNSTI4 0
NEI4 $102
line 59
;59:        G_Printf( COLOR_RED "Failed to create savefile titled '%s'!\n", sg_savename.s );
ADDRGP4 $104
ARGP4
ADDRGP4 sg_savename
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 60
;60:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $101
JUMPV
LABELV $102
line 63
;61:    }
;62:
;63:    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
ADDRLP4 0
ADDRGP4 s_archiveHandleList
ASGNP4
ADDRGP4 $108
JUMPV
LABELV $105
line 64
;64:        handle->fn( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 0
INDIRP4
INDIRP4
CALLV
pop
line 65
;65:    }
LABELV $106
line 63
ADDRLP4 0
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ASGNP4
LABELV $108
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $105
line 67
;66:
;67:    trap_FS_FClose( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 69
;68:    
;69:    return 1;
CNSTI4 1
RETI4
LABELV $101
endproc SG_SaveGame 8 8
export SG_WriteSection
proc SG_WriteSection 16 12
line 73
;70:}
;71:
;72:void SG_WriteSection( const char *name, int size, const void *data, file_t f )
;73:{
line 76
;74:    ngdsection_t section;
;75:
;76:    if ( strlen( name ) >= sizeof(section.name) ) {
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 12
ADDRGP4 strlen
CALLU4
ASGNU4
ADDRLP4 12
INDIRU4
CNSTU4 4
LTU4 $110
line 77
;77:        trap_Error( "SG_WriteSection: strlen( name ) >= sizeof(section.name)" );
ADDRGP4 $112
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 78
;78:    }
LABELV $110
line 80
;79:
;80:    strcpy( section.name, name );
ADDRLP4 0
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 strcpy
CALLP4
pop
line 81
;81:    section.size = size;
ADDRLP4 0+4
ADDRFP4 4
INDIRI4
ASGNI4
line 83
;82:
;83:    trap_FS_Write( &section, sizeof(section), f );
ADDRLP4 0
ARGP4
CNSTI4 12
ARGI4
ADDRFP4 12
INDIRI4
ARGI4
ADDRGP4 trap_FS_Write
CALLI4
pop
line 84
;84:    trap_FS_Write( data, size, f );
ADDRFP4 8
INDIRP4
ARGP4
ADDRFP4 4
INDIRI4
ARGI4
ADDRFP4 12
INDIRI4
ARGI4
ADDRGP4 trap_FS_Write
CALLI4
pop
line 85
;85:}
LABELV $109
endproc SG_WriteSection 16 12
export SG_LoadSection
proc SG_LoadSection 8 12
line 88
;86:
;87:void SG_LoadSection( const char *name, void *dest, int size )
;88:{
line 91
;89:    int i;
;90:
;91:    for ( i = 0; i < numSections; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $118
JUMPV
LABELV $115
line 92
;92:        if ( !N_stricmp( name, sections[i].name ) ) {
ADDRFP4 0
INDIRP4
ARGP4
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
ARGP4
ADDRLP4 4
ADDRGP4 N_stricmp
CALLI4
ASGNI4
ADDRLP4 4
INDIRI4
CNSTI4 0
NEI4 $119
line 93
;93:            if ( sections[i].size != size ) {
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
CNSTI4 4
ADDP4
INDIRI4
ADDRFP4 8
INDIRI4
EQI4 $121
line 94
;94:                trap_Error( "SG_LoadSection: section size in gamefile != size in vm." );
ADDRGP4 $123
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 95
;95:            }
LABELV $121
line 97
;96:
;97:            trap_FS_FileSeek( gamefile, sections[i].offset, FS_SEEK_SET );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
CNSTI4 8
ADDP4
INDIRI4
ARGI4
CNSTI4 2
ARGI4
ADDRGP4 trap_FS_FileSeek
CALLI4
pop
line 98
;98:            trap_FS_Read( dest, size, gamefile );
ADDRFP4 4
INDIRP4
ARGP4
ADDRFP4 8
INDIRI4
ARGI4
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_Read
CALLI4
pop
line 99
;99:        }
LABELV $119
line 100
;100:    }
LABELV $116
line 91
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $118
ADDRLP4 0
INDIRI4
ADDRGP4 numSections
INDIRI4
LTI4 $115
line 101
;101:    G_Error( "SG_LoadGame: failed to load section '%s'", name );
ADDRGP4 $124
ARGP4
ADDRFP4 0
INDIRP4
ARGP4
ADDRGP4 G_Error
CALLV
pop
line 102
;102:}
LABELV $114
endproc SG_LoadSection 8 12
export SG_LoadGame
proc SG_LoadGame 60 12
line 105
;103:
;104:int SG_LoadGame( void )
;105:{
line 112
;106:    ngdheader_t header;
;107:    archiveHandle_t *handle;
;108:    sgameState_t statePrev;
;109:    int i;
;110:    int mark;
;111:
;112:    mark = SG_MakeMemoryMark();
ADDRLP4 36
ADDRGP4 SG_MakeMemoryMark
CALLI4
ASGNI4
ADDRLP4 32
ADDRLP4 36
INDIRI4
ASGNI4
line 114
;113:
;114:    gamefile = trap_FS_FOpenRead( sg_savename.s );
ADDRGP4 sg_savename
ARGP4
ADDRLP4 40
ADDRGP4 trap_FS_FOpenRead
CALLI4
ASGNI4
ADDRGP4 gamefile
ADDRLP4 40
INDIRI4
ASGNI4
line 115
;115:    if ( gamefile == FS_INVALID_HANDLE ) {
ADDRGP4 gamefile
INDIRI4
CNSTI4 0
NEI4 $126
line 116
;116:        G_Printf( COLOR_RED "Failed to load savefile titled '%s'!\n", sg_savename.s );
ADDRGP4 $128
ARGP4
ADDRGP4 sg_savename
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 117
;117:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $125
JUMPV
LABELV $126
line 120
;118:    }
;119:
;120:    if ( !trap_FS_Read( &header, sizeof(header), gamefile ) ) {
ADDRLP4 8
ARGP4
CNSTI4 20
ARGI4
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 44
ADDRGP4 trap_FS_Read
CALLI4
ASGNI4
ADDRLP4 44
INDIRI4
CNSTI4 0
NEI4 $129
line 121
;121:        G_Printf( COLOR_RED "SG_LoadGame: failed to read savefile header.\n" );
ADDRGP4 $131
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 122
;122:        return 0;
CNSTI4 0
RETI4
ADDRGP4 $125
JUMPV
LABELV $129
line 125
;123:    }
;124:
;125:    if ( header.ident != IDENT ) {
ADDRLP4 8
INDIRI4
CNSTI4 1684500001
EQI4 $132
line 126
;126:        trap_FS_FClose( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 127
;127:        trap_Error( "SG_LoadGame: savefile header identifier is incorrect" );
ADDRGP4 $134
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 128
;128:    }
LABELV $132
line 130
;129:
;130:    if ( header.version != NOMAD_VERSION || header.versionUpdate != NOMAD_VERSION_UPDATE || header.versionPatch != NOMAD_VERSION_PATCH ) {
ADDRLP4 8+4
INDIRI4
CNSTI4 1
NEI4 $141
ADDRLP4 8+8
INDIRI4
CNSTI4 1
NEI4 $141
ADDRLP4 8+12
INDIRI4
CNSTI4 0
EQI4 $135
LABELV $141
line 131
;131:        G_Printf( "SG_LoadGame: header version is not equal to this game's version.\n" );
ADDRGP4 $142
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 132
;132:        goto error;
ADDRGP4 $143
JUMPV
LABELV $135
line 135
;133:    }
;134:
;135:    if ( !header.numSections ) {
ADDRLP4 8+16
INDIRI4
CNSTI4 0
NEI4 $144
line 136
;136:        G_Printf( "SG_LoadGame: header has invalid section count" );
ADDRGP4 $147
ARGP4
ADDRGP4 G_Printf
CALLV
pop
line 137
;137:        goto error;
ADDRGP4 $143
JUMPV
LABELV $144
line 140
;138:    }
;139:
;140:    statePrev = sg.state;
ADDRLP4 28
ADDRGP4 sg+64
INDIRI4
ASGNI4
line 141
;141:    sg.state = SG_LOADGAME;
ADDRGP4 sg+64
CNSTI4 0
ASGNI4
line 143
;142:
;143:    sections = (ngdsection_t *)SG_MemAlloc( sizeof(ngdsection_t) * header.numSections );
ADDRLP4 8+16
INDIRI4
CVIU4 4
CNSTU4 12
MULU4
CVUI4 4
ARGI4
ADDRLP4 48
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRGP4 sections
ADDRLP4 48
INDIRP4
ASGNP4
line 144
;144:    numSections = header.numSections;
ADDRGP4 numSections
ADDRLP4 8+16
INDIRI4
ASGNI4
line 146
;145:
;146:    for ( i = 0; i < header.numSections; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
ADDRGP4 $155
JUMPV
LABELV $152
line 148
;147:        // read info and go to the next section
;148:        trap_FS_Read( sections[i].name, sizeof(sections->name), gamefile );
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
ARGP4
CNSTI4 4
ARGI4
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_Read
CALLI4
pop
line 149
;149:        trap_FS_Read( &sections[i].size, sizeof(sections->size), gamefile );
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
CNSTI4 4
ADDP4
ARGP4
CNSTI4 4
ARGI4
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_Read
CALLI4
pop
line 151
;150:
;151:        sections[i].offset = trap_FS_FileTell( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 52
ADDRGP4 trap_FS_FileTell
CALLI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
CNSTI4 8
ADDP4
ADDRLP4 52
INDIRI4
ASGNI4
line 152
;152:        trap_FS_FileSeek( gamefile, trap_FS_FileTell( gamefile ) + sections[i].size, FS_SEEK_SET );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 56
ADDRGP4 trap_FS_FileTell
CALLI4
ASGNI4
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 56
INDIRI4
ADDRLP4 0
INDIRI4
CNSTI4 12
MULI4
ADDRGP4 sections
INDIRP4
ADDP4
CNSTI4 4
ADDP4
INDIRI4
ADDI4
ARGI4
CNSTI4 2
ARGI4
ADDRGP4 trap_FS_FileSeek
CALLI4
pop
line 153
;153:    }
LABELV $153
line 146
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
LABELV $155
ADDRLP4 0
INDIRI4
ADDRLP4 8+16
INDIRI4
LTI4 $152
line 155
;154:
;155:    for ( handle = &s_archiveHandleList; handle; handle = handle->next ) {
ADDRLP4 4
ADDRGP4 s_archiveHandleList
ASGNP4
ADDRGP4 $160
JUMPV
LABELV $157
line 156
;156:        handle->fn( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRLP4 4
INDIRP4
INDIRP4
CALLV
pop
line 157
;157:    }
LABELV $158
line 155
ADDRLP4 4
ADDRLP4 4
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
ASGNP4
LABELV $160
ADDRLP4 4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $157
line 159
;158:
;159:    trap_FS_FClose( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 160
;160:    gamefile = FS_INVALID_HANDLE;
ADDRGP4 gamefile
CNSTI4 0
ASGNI4
line 162
;161:
;162:    return 1;
CNSTI4 1
RETI4
ADDRGP4 $125
JUMPV
LABELV $143
line 165
;163:
;164:error:
;165:    sg.state = statePrev;
ADDRGP4 sg+64
ADDRLP4 28
INDIRI4
ASGNI4
line 166
;166:    if ( gamefile != FS_INVALID_HANDLE ) {
ADDRGP4 gamefile
INDIRI4
CNSTI4 0
EQI4 $162
line 167
;167:        trap_FS_FClose( gamefile );
ADDRGP4 gamefile
INDIRI4
ARGI4
ADDRGP4 trap_FS_FClose
CALLV
pop
line 168
;168:        gamefile = FS_INVALID_HANDLE;
ADDRGP4 gamefile
CNSTI4 0
ASGNI4
line 169
;169:    }
LABELV $162
line 171
;170:
;171:    return 0;
CNSTI4 0
RETI4
LABELV $125
endproc SG_LoadGame 60 12
bss
align 4
LABELV s_archiveHandleList
skip 12
align 4
LABELV gamefile
skip 4
align 4
LABELV numSections
skip 4
align 4
LABELV sections
skip 4
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
import trap_FS_FOpenRead
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
import SG_ClearToMemoryMark
import SG_MakeMemoryMark
import SG_MemInit
import SG_MemAlloc
import String_Alloc
import SG_SpawnMobOnMap
import SG_SpawnMob
import Ent_SetState
import SG_InitEntities
import Ent_BuildBounds
import SG_BuildBounds
import SG_FreeEntity
import SG_AllocEntity
import Ent_RunTic
import Ent_CheckEntityCollision
import Ent_CheckWallCollision
import SG_DrawLevelStats
import SG_DrawAbortMission
import Lvl_AddKillEntity
import SG_EndLevel
import SG_InitLevel
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
import ClearBounds
import RadiusFromBounds
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
LABELV $147
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 71
byte 1 97
byte 1 109
byte 1 101
byte 1 58
byte 1 32
byte 1 104
byte 1 101
byte 1 97
byte 1 100
byte 1 101
byte 1 114
byte 1 32
byte 1 104
byte 1 97
byte 1 115
byte 1 32
byte 1 105
byte 1 110
byte 1 118
byte 1 97
byte 1 108
byte 1 105
byte 1 100
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 99
byte 1 111
byte 1 117
byte 1 110
byte 1 116
byte 1 0
align 1
LABELV $142
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 71
byte 1 97
byte 1 109
byte 1 101
byte 1 58
byte 1 32
byte 1 104
byte 1 101
byte 1 97
byte 1 100
byte 1 101
byte 1 114
byte 1 32
byte 1 118
byte 1 101
byte 1 114
byte 1 115
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 110
byte 1 111
byte 1 116
byte 1 32
byte 1 101
byte 1 113
byte 1 117
byte 1 97
byte 1 108
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 116
byte 1 104
byte 1 105
byte 1 115
byte 1 32
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 39
byte 1 115
byte 1 32
byte 1 118
byte 1 101
byte 1 114
byte 1 115
byte 1 105
byte 1 111
byte 1 110
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $134
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 71
byte 1 97
byte 1 109
byte 1 101
byte 1 58
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 104
byte 1 101
byte 1 97
byte 1 100
byte 1 101
byte 1 114
byte 1 32
byte 1 105
byte 1 100
byte 1 101
byte 1 110
byte 1 116
byte 1 105
byte 1 102
byte 1 105
byte 1 101
byte 1 114
byte 1 32
byte 1 105
byte 1 115
byte 1 32
byte 1 105
byte 1 110
byte 1 99
byte 1 111
byte 1 114
byte 1 114
byte 1 101
byte 1 99
byte 1 116
byte 1 0
align 1
LABELV $131
byte 1 94
byte 1 49
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 71
byte 1 97
byte 1 109
byte 1 101
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
byte 1 114
byte 1 101
byte 1 97
byte 1 100
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 104
byte 1 101
byte 1 97
byte 1 100
byte 1 101
byte 1 114
byte 1 46
byte 1 10
byte 1 0
align 1
LABELV $128
byte 1 94
byte 1 49
byte 1 70
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
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 116
byte 1 105
byte 1 116
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 33
byte 1 10
byte 1 0
align 1
LABELV $124
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 71
byte 1 97
byte 1 109
byte 1 101
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
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 0
align 1
LABELV $123
byte 1 83
byte 1 71
byte 1 95
byte 1 76
byte 1 111
byte 1 97
byte 1 100
byte 1 83
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 58
byte 1 32
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 32
byte 1 115
byte 1 105
byte 1 122
byte 1 101
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 103
byte 1 97
byte 1 109
byte 1 101
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 33
byte 1 61
byte 1 32
byte 1 115
byte 1 105
byte 1 122
byte 1 101
byte 1 32
byte 1 105
byte 1 110
byte 1 32
byte 1 118
byte 1 109
byte 1 46
byte 1 0
align 1
LABELV $112
byte 1 83
byte 1 71
byte 1 95
byte 1 87
byte 1 114
byte 1 105
byte 1 116
byte 1 101
byte 1 83
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 58
byte 1 32
byte 1 115
byte 1 116
byte 1 114
byte 1 108
byte 1 101
byte 1 110
byte 1 40
byte 1 32
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 32
byte 1 41
byte 1 32
byte 1 62
byte 1 61
byte 1 32
byte 1 115
byte 1 105
byte 1 122
byte 1 101
byte 1 111
byte 1 102
byte 1 40
byte 1 115
byte 1 101
byte 1 99
byte 1 116
byte 1 105
byte 1 111
byte 1 110
byte 1 46
byte 1 110
byte 1 97
byte 1 109
byte 1 101
byte 1 41
byte 1 0
align 1
LABELV $104
byte 1 94
byte 1 49
byte 1 70
byte 1 97
byte 1 105
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 116
byte 1 111
byte 1 32
byte 1 99
byte 1 114
byte 1 101
byte 1 97
byte 1 116
byte 1 101
byte 1 32
byte 1 115
byte 1 97
byte 1 118
byte 1 101
byte 1 102
byte 1 105
byte 1 108
byte 1 101
byte 1 32
byte 1 116
byte 1 105
byte 1 116
byte 1 108
byte 1 101
byte 1 100
byte 1 32
byte 1 39
byte 1 37
byte 1 115
byte 1 39
byte 1 33
byte 1 10
byte 1 0
