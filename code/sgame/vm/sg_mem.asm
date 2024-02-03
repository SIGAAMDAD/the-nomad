data
align 4
LABELV $90
address $91
export String_Alloc
code
proc String_Alloc 40 8
file "../sg_mem.c"
line 26
;1:#include "sg_local.h"
;2:
;3:// 12 KiB of static memory for the vm to use
;4:#define MEMPOOL_SIZE (12*1024*1024)
;5:static char mempool[MEMPOOL_SIZE];
;6:static int allocPoint;
;7:
;8:#define STRINGPOOL_SIZE (8*1024)
;9:
;10:#define HASH_TABLE_SIZE 2048
;11:
;12:typedef struct stringDef_s {
;13:	struct stringDef_s *next;
;14:	const char *str;
;15:} stringDef_t;
;16:
;17:static int strPoolIndex;
;18:static char strPool[STRINGPOOL_SIZE];
;19:
;20:static int strHandleCount;
;21:static stringDef_t *strHandle[HASH_TABLE_SIZE];
;22:
;23:static qboolean outOfMemory;
;24:
;25:const char *String_Alloc( const char *p )
;26:{
line 32
;27:	int len;
;28:	uint64_t hash;
;29:	stringDef_t *str, *last;
;30:	static const char *staticNULL = "";
;31:
;32:	if (p == NULL) {
ADDRFP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $92
line 33
;33:		return NULL;
CNSTP4 0
RETP4
ADDRGP4 $89
JUMPV
LABELV $92
line 36
;34:	}
;35:
;36:	if (*p == 0) {
ADDRFP4 0
INDIRP4
INDIRI1
CVII4 1
CNSTI4 0
NEI4 $94
line 37
;37:		return staticNULL;
ADDRGP4 $90
INDIRP4
RETP4
ADDRGP4 $89
JUMPV
LABELV $94
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
ADDRGP4 $97
JUMPV
LABELV $96
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
NEI4 $99
line 45
;45:			return str->str;
ADDRLP4 0
INDIRP4
CNSTI4 4
ADDP4
INDIRP4
RETP4
ADDRGP4 $89
JUMPV
LABELV $99
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
LABELV $97
line 43
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $96
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
CVUI4 4
ASGNI4
line 51
;51:	if (len + strPoolIndex + 1 < STRINGPOOL_SIZE) {
ADDRLP4 12
INDIRI4
ADDRGP4 strPoolIndex
INDIRI4
ADDI4
CNSTI4 1
ADDI4
CNSTI4 8192
GEI4 $101
line 52
;52:		int ph = strPoolIndex;
ADDRLP4 24
ADDRGP4 strPoolIndex
INDIRI4
ASGNI4
line 53
;53:		strcpy(&strPool[strPoolIndex], p);
ADDRGP4 strPoolIndex
INDIRI4
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
INDIRI4
ADDRLP4 12
INDIRI4
CNSTI4 1
ADDI4
ADDI4
ASGNI4
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
ADDRGP4 $104
JUMPV
LABELV $103
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
LABELV $104
line 58
ADDRLP4 0
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $106
ADDRLP4 0
INDIRP4
INDIRP4
CVPU4 4
CNSTU4 0
NEU4 $103
LABELV $106
line 63
;62:
;63:		str = (stringDef_t *)SG_MemAlloc( sizeof(stringDef_t) );
CNSTI4 8
ARGI4
ADDRLP4 36
ADDRGP4 SG_MemAlloc
CALLP4
ASGNP4
ADDRLP4 0
ADDRLP4 36
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
INDIRI4
ADDRGP4 strPool
ADDP4
ASGNP4
line 66
;66:		if (last) {
ADDRLP4 4
INDIRP4
CVPU4 4
CNSTU4 0
EQU4 $107
line 67
;67:			last->next = str;
ADDRLP4 4
INDIRP4
ADDRLP4 0
INDIRP4
ASGNP4
line 68
;68:		} else {
ADDRGP4 $108
JUMPV
LABELV $107
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
LABELV $108
line 71
;71:		return &strPool[ph];
ADDRLP4 24
INDIRI4
ADDRGP4 strPool
ADDP4
RETP4
ADDRGP4 $89
JUMPV
LABELV $101
line 73
;72:	}
;73:	return NULL;
CNSTP4 0
RETP4
LABELV $89
endproc String_Alloc 40 8
export String_Report
proc String_Report 4 16
line 77
;74:}
;75:
;76:void String_Report( void )
;77:{
line 79
;78:	float f;
;79:	Con_Printf( "Memory/String Pool Info\n");
ADDRGP4 $110
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 80
;80:	Con_Printf( "----------------\n");
ADDRGP4 $111
ARGP4
ADDRGP4 Con_Printf
CALLV
pop
line 82
;81:	
;82:    f = strPoolIndex;
ADDRLP4 0
ADDRGP4 strPoolIndex
INDIRI4
CVIF4 4
ASGNF4
line 83
;83:	f /= STRINGPOOL_SIZE;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 956301312
MULF4
ASGNF4
line 84
;84:	f *= 100;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 1120403456
MULF4
ASGNF4
line 85
;85:	Con_Printf( "String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRINGPOOL_SIZE );
ADDRGP4 $112
ARGP4
ADDRLP4 0
INDIRF4
ARGF4
ADDRGP4 strPoolIndex
INDIRI4
ARGI4
CNSTI4 8192
ARGI4
ADDRGP4 Con_Printf
CALLV
pop
line 87
;86:	
;87:    f = allocPoint;
ADDRLP4 0
ADDRGP4 allocPoint
INDIRI4
CVIF4 4
ASGNF4
line 88
;88:	f /= MEMPOOL_SIZE;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 866822827
MULF4
ASGNF4
line 89
;89:	f *= 100;
ADDRLP4 0
ADDRLP4 0
INDIRF4
CNSTF4 1120403456
MULF4
ASGNF4
line 90
;90:	Con_Printf( "Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEMPOOL_SIZE );
ADDRGP4 $113
ARGP4
ADDRLP4 0
INDIRF4
ARGF4
ADDRGP4 allocPoint
INDIRI4
ARGI4
CNSTI4 12582912
ARGI4
ADDRGP4 Con_Printf
CALLV
pop
line 91
;91:}
LABELV $109
endproc String_Report 4 16
export SG_MemAlloc
proc SG_MemAlloc 8 12
line 94
;92:
;93:void *SG_MemAlloc( int size )
;94:{
line 97
;95:    char *buf;
;96:
;97:    if ( !size ) {
ADDRFP4 0
INDIRI4
CNSTI4 0
NEI4 $115
line 98
;98:        trap_Error( "SG_MemAlloc: bad size" );
ADDRGP4 $117
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 99
;99:    }
LABELV $115
line 101
;100:
;101:    size = PAD( size, (unsigned)16 ); // round to 16-byte alignment
ADDRFP4 0
ADDRFP4 0
INDIRI4
CVIU4 4
CNSTU4 16
ADDU4
CNSTU4 1
SUBU4
CNSTU4 4294967280
BANDU4
CVUI4 4
ASGNI4
line 103
;102:
;103:    if ( allocPoint + size >= sizeof(mempool) ) {
ADDRGP4 allocPoint
INDIRI4
ADDRFP4 0
INDIRI4
ADDI4
CVIU4 4
CNSTU4 12582912
LTU4 $118
line 104
;104:        trap_Print( COLOR_RED "(ERROR) SG_MemAlloc: not enough vm memory\n" );
ADDRGP4 $120
ARGP4
ADDRGP4 trap_Print
CALLV
pop
line 105
;105:		outOfMemory = qtrue;
ADDRGP4 outOfMemory
CNSTI4 1
ASGNI4
line 106
;106:        return NULL;
CNSTP4 0
RETP4
ADDRGP4 $114
JUMPV
LABELV $118
line 109
;107:    }
;108:
;109:    buf = &mempool[ allocPoint ];
ADDRLP4 0
ADDRGP4 allocPoint
INDIRI4
ADDRGP4 mempool
ADDP4
ASGNP4
line 110
;110:    allocPoint += size;
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
line 113
;111:
;112:    // zero init
;113:    memset( buf, 0, size );
ADDRLP4 0
INDIRP4
ARGP4
CNSTI4 0
ARGI4
ADDRFP4 0
INDIRI4
CVIU4 4
ARGU4
ADDRGP4 memset
CALLP4
pop
line 115
;114:
;115:    return buf;
ADDRLP4 0
INDIRP4
RETP4
LABELV $114
endproc SG_MemAlloc 8 12
export SG_MemoryRemaining
proc SG_MemoryRemaining 0 0
line 118
;116:}
;117:
;118:int SG_MemoryRemaining( void ) {
line 119
;119:    return sizeof(mempool) - allocPoint;
CNSTU4 12582912
ADDRGP4 allocPoint
INDIRI4
CVIU4 4
SUBU4
CVUI4 4
RETI4
LABELV $121
endproc SG_MemoryRemaining 0 0
export SG_OutOfMemory
proc SG_OutOfMemory 0 0
line 122
;120:}
;121:
;122:qboolean SG_OutOfMemory( void ) {
line 123
;123:	return outOfMemory;
ADDRGP4 outOfMemory
INDIRI4
RETI4
LABELV $122
endproc SG_OutOfMemory 0 0
export SG_ClearToMemoryMark
proc SG_ClearToMemoryMark 4 4
line 126
;124:}
;125:
;126:void SG_ClearToMemoryMark( int mark ) {
line 127
;127:    if ( mark < 0 || mark > allocPoint ) {
ADDRLP4 0
ADDRFP4 0
INDIRI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 0
LTI4 $126
ADDRLP4 0
INDIRI4
ADDRGP4 allocPoint
INDIRI4
LEI4 $124
LABELV $126
line 128
;128:        trap_Error( "SG_ClearToMemoryMark: invalid memory mark" );
ADDRGP4 $127
ARGP4
ADDRGP4 trap_Error
CALLV
pop
line 129
;129:    }
LABELV $124
line 130
;130:    allocPoint = mark;
ADDRGP4 allocPoint
ADDRFP4 0
INDIRI4
ASGNI4
line 131
;131:}
LABELV $123
endproc SG_ClearToMemoryMark 4 4
export SG_MakeMemoryMark
proc SG_MakeMemoryMark 0 0
line 133
;132:
;133:int SG_MakeMemoryMark( void ) {
line 134
;134:    return allocPoint;
ADDRGP4 allocPoint
INDIRI4
RETI4
LABELV $128
endproc SG_MakeMemoryMark 0 0
export SG_MemInit
proc SG_MemInit 4 12
line 138
;135:}
;136:
;137:void SG_MemInit( void )
;138:{
line 141
;139:    int i;
;140:
;141:	outOfMemory = qfalse;
ADDRGP4 outOfMemory
CNSTI4 0
ASGNI4
line 143
;142:
;143:    memset( mempool, 0, sizeof(mempool) );
ADDRGP4 mempool
ARGP4
CNSTI4 0
ARGI4
CNSTU4 12582912
ARGU4
ADDRGP4 memset
CALLP4
pop
line 144
;144:    memset( strPool, 0, sizeof(strPool) );
ADDRGP4 strPool
ARGP4
CNSTI4 0
ARGI4
CNSTU4 8192
ARGU4
ADDRGP4 memset
CALLP4
pop
line 146
;145:
;146:	for ( i = 0; i < HASH_TABLE_SIZE; i++ ) {
ADDRLP4 0
CNSTI4 0
ASGNI4
LABELV $130
line 147
;147:		strHandle[i] = 0;
ADDRLP4 0
INDIRI4
CNSTI4 2
LSHI4
ADDRGP4 strHandle
ADDP4
CNSTP4 0
ASGNP4
line 148
;148:	}
LABELV $131
line 146
ADDRLP4 0
ADDRLP4 0
INDIRI4
CNSTI4 1
ADDI4
ASGNI4
ADDRLP4 0
INDIRI4
CNSTI4 2048
LTI4 $130
line 149
;149:	strHandleCount = 0;
ADDRGP4 strHandleCount
CNSTI4 0
ASGNI4
line 150
;150:	strPoolIndex = 0;
ADDRGP4 strPoolIndex
CNSTI4 0
ASGNI4
line 151
;151:}
LABELV $129
endproc SG_MemInit 4 12
bss
align 4
LABELV outOfMemory
skip 4
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
skip 12582912
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
import inversedirs
import dirvectors
import stateinfo
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
LABELV $127
byte 1 83
byte 1 71
byte 1 95
byte 1 67
byte 1 108
byte 1 101
byte 1 97
byte 1 114
byte 1 84
byte 1 111
byte 1 77
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 77
byte 1 97
byte 1 114
byte 1 107
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
byte 1 109
byte 1 101
byte 1 109
byte 1 111
byte 1 114
byte 1 121
byte 1 32
byte 1 109
byte 1 97
byte 1 114
byte 1 107
byte 1 0
align 1
LABELV $120
byte 1 94
byte 1 49
byte 1 40
byte 1 69
byte 1 82
byte 1 82
byte 1 79
byte 1 82
byte 1 41
byte 1 32
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
byte 1 10
byte 1 0
align 1
LABELV $117
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
LABELV $113
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
LABELV $112
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
LABELV $111
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
LABELV $110
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
LABELV $91
byte 1 0
