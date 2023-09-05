#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

#include "../common/n_vm.h"
#include "../sgame/sg_public.h"

/*
Common functionality for the engine and vm alike
*/

#ifndef Q3_VM
void Com_PushToPool(void (*work)(int));
#endif

void I_NomadInit(void);
uint32_t crc32_buffer(const byte *buf, uint32_t len);
void Com_Init(void);
void Com_Shutdown(void);
void GDR_DECL Com_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
uint64_t Com_GenerateHashValue(const char *fname, const uint64_t size);
void Con_RenderConsole(void);
void Com_WriteConfig(void);
void COM_DefaultExtension( char *path, uint64_t maxSize, const char *extension );
int32_t Com_HexStrToInt(const char *str);
qboolean Com_FilterExt( const char *filter, const char *name );
int Com_Filter( const char *filter, const char *name );
int Com_FilterPath( const char *filter, const char *name );
qboolean Com_HasPatterns( const char *str );
const char* Com_Base64Decode(void);
void Com_SortFileList( char **list, uint64_t nfiles, uint64_t fastSort );
void Com_TruncateLongString( char *buffer, const char *s );
int Com_HexStrToInt(const char *str);
void COM_StripExtension(const char *in, char *out, uint64_t destsize);
void COM_BeginParseSession( const char *name );
uint64_t COM_GetCurrentParseLine( void );
const char *COM_Parse( const char **data_p );
const char *COM_ParseExt( const char **data_p, qboolean allowLineBreak );
int COM_Compress( char *data_p );
void COM_ParseError( const char *format, ... ) GDR_ATTRIBUTE((format(printf, 1, 2)));
void COM_ParseWarning( const char *format, ... ) GDR_ATTRIBUTE((format(printf, 1, 2)));

// md4 functions
uint32_t Com_BlockChecksum (const void *buffer, uint64_t length);

//int		COM_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] );

char *COM_ParseComplex( const char **data_p, qboolean allowLineBreak );

typedef enum {
	TK_GENEGIC = 0, // for single-char tokens
	TK_STRING,
	TK_QUOTED,
	TK_EQ,
	TK_NEQ,
	TK_GT,
	TK_GTE,
	TK_LT,
	TK_LTE,
	TK_MATCH,
	TK_OR,
	TK_AND,
	TK_SCOPE_OPEN,
	TK_SCOPE_CLOSE,
	TK_NEWLINE,
	TK_EOF,
} tokenType_t;

extern tokenType_t com_tokentype;

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

qboolean SkipBracedSection( const char **program, int depth );
void SkipRestOfLine( const char **data );

int ParseHex(const char* text);
void Parse1DMatrix( const char **buf_p, int x, float *m);
void Parse2DMatrix( const char **buf_p, int y, int x, float *m);
void Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m);

/*
commands, shouldn't be called by the vm
*/

typedef void (*completionFunc_t)(const char* args, uint32_t argnum);
typedef void (*cmdfunc_t)(void);

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define MAX_USERINFO_LENGTH (MAX_INFO_STRING-13) // incl. length of 'connect ""' or 'userinfo ""' and reserving one byte to avoid q3msgboom
													
#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192
#define MAX_CMD_LINE		1024
#define MAX_CMD_BUFFER  65536

typedef enum {
  // bk001129 - make sure SE_NONE is zero
	SE_NONE = 0,		// evTime is still valid
	SE_KEY,				// evValue is a key code, evValue2 is whether its pressed or not
	SE_MOUSE,			// evValue and evValue2 are relative signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE,			// evPtr is a char*
	SE_WINDOW,			// really only used by the rendering engine for window resisizing
	SE_MAX,
} sysEventType_t;

typedef struct
{
	uint32_t		evTime;
	sysEventType_t	evType;
	uint32_t		evValue, evValue2;
	uint32_t		evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

void Com_InitKeyCommands( void );
void Com_QueueEvent(uint32_t evTime, sysEventType_t evType, uint32_t evValue, uint32_t evValue2, uint32_t ptrLength, void *ptr);
void Com_SendKeyEvents(void);
void Com_KeyEvent(uint32_t key, qboolean down, uint32_t time);
uint64_t Com_EventLoop(void);

#include "keycodes.h"

#define KEYCATCH_SGAME	0x2000
#define KEYCATCH_SCRIPT	0x0400
#define KEYCATCH_UI		0x0080

void Key_KeynameCompletion( void(*callback)(const char *s) );
uint32_t Key_StringToKeynum( const char *str );
const char *Key_KeynumToString(uint32_t keynum);
qboolean Key_IsDown(uint32_t keynum);
const char *Key_GetBinding(uint32_t keynum);
void Key_ClearStates(void);
void Key_SetCatcher(uint32_t catcher);
uint32_t Key_GetCatcher(void);
uint32_t Key_GetKey( const char *binding );

typedef struct
{
	qboolean down;
	qboolean bound;
	uint32_t repeats;
	char *binding;
} nkey_t;

extern nkey_t keys[NUMKEYS];

#define MAXPRINTMSG			8192

/* vm error codes for Com_Error/G_Error */
#define ERR_RESTART			0x0000 // restart the vm, don't stop the game though
#define ERR_FATAL			0x2000 // restart the vm and cancel the game loop, don't crash, only the engine can do that
#define ERR_FRAME			0x4000 // don't run the next vm frame

void Cmd_Init(void);
void Cmd_AddCommand(const char* name, cmdfunc_t function);
void Cmd_RemoveCommand(const char* name);
void Cmd_ExecuteCommand(const char* name);
void Cmd_ArgsBuffer( char *buffer, uint32_t bufferLength );
void Cmd_ExecuteText(const char *str);
void Cmd_ExecuteString(const char *str);
uint32_t Cmd_Argc(void);
void Cmd_ArgvBuffer(uint32_t arg, char *buffer, uint32_t bufLen);
void Cmd_TokenizeString(const char *text_p);
void Cmd_TokenizeStringIgnoreQuotes(const char *text_p);
void Cmd_SetCommandCompletionFunc(const char *name, completionFunc_t fn);
char* Cmd_ArgsFrom(int32_t index);
const char* Cmd_Argv(uint32_t index);
void Cmd_Clear(void);
const char* GDR_DECL va(const char *format, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
#ifdef __cplusplus
void GDR_DECL Com_Error(vm_t *vm, int level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 3, 4)));
#endif
void Cmd_CommandCompletion( void(*callback)(const char *s) );
qboolean Cmd_CompleteArgument(const char *command, const char *args, uint32_t argnum);

typedef enum {
	EXEC_NOW = 0,
	EXEC_INSERT,
	EXEC_APPEND
} cbufExec_t;

void Cbuf_ExecuteText( cbufExec_t exec_when, const char *text );
void Cbuf_InsertText( const char *text );
void Cbuf_Execute(void);
void Cbuf_AddText( const char *text );
void Cbuf_Clear(void);
void Cbuf_Init(void);

/*

Edit fields and command line history/completion

*/

#define TRUNCATE_LENGTH 64
#define	MAX_EDIT_LINE 1024
#if MAX_EDIT_LINE > MAX_CMD_LINE
#error "MAX_EDIT_LINE > MAX_CMD_LINE"
#endif
typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

void Field_Clear( field_t *edit );
void Field_AutoComplete( field_t *edit );
void Field_CompleteKeyname( void );
void Field_CompleteKeyBind( uint32_t key );
void Field_CompleteFilename( const char *dir, const char *ext, qboolean stripExt, int flags );
void Field_CompleteCommand( const char *cmd, qboolean doCommands, qboolean doCvars );

void Con_ResetHistory( void );
void Con_SaveField( const field_t *field );
qboolean Con_HistoryGetPrev( field_t *field );
qboolean Con_HistoryGetNext( field_t *field );

/*
The filesystem, heavily based on quake's filesystem
*/

typedef enum {
	H_ENGINE,
	H_SCRIPT,
	H_SGAME,
	H_UI,
	H_ALLOCATOR,
	H_RENDERER
} handleOwner_t;

#define NUM_GDR_BFFS 1

#define FS_MATCH_BASEDIR	(1<<0)
#define FS_MATCH_EXTERN		(2<<0)
#define FS_MATCH_BFFs		(FS_MATCH_EXTERN | FS_MATCH_BASEDIR)
#define FS_MATCH_ANY		(FS_MATCH_EXTERN | FS_MATCH_BASEDIR)

#define MAX_FOUND_FILES 0x8000

#define FS_INVALID_HANDLE -1

typedef int32_t file_t;
#ifdef Q3_VM
typedef unsigned long int fileTime_t;
#else
typedef time_t fileTime_t;
#endif
#if defined(_MSVC_VER) || defined(__clang__)
typedef _off_t fileOffset_t;
#elif !defined(Q3_VM)
typedef off_t fileOffset_t;
#else
typedef long fileOffset_t;
#endif

typedef enum {
	FS_OPEN_READ,
	FS_OPEN_WRITE,
	FS_OPEN_RW = FS_OPEN_READ | FS_OPEN_WRITE
} fileMode_t;

#define FS_SEEK_END 0
#define FS_SEEK_CUR 1
#define FS_SEEK_BEGIN 2

#define FS_SEEK_SET FS_SEEK_BEGIN

#ifndef Q3_VM
extern file_t logfile;
#endif

/* vm specific file handling */
file_t FS_VM_FOpenWrite(const char *path, file_t *f, handleOwner_t owner);
file_t FS_VM_FOpenRead(const char *path, file_t *f, handleOwner_t owner);
void FS_VM_FClose(file_t f);
uint32_t FS_VM_Read(void *buffer, uint32_t len, file_t f, handleOwner_t owner);
uint32_t FS_VM_Write(const void *buffer, uint32_t len, file_t f, handleOwner_t owner);
void FS_VM_WriteFile(const void *buffer, uint32_t len, file_t f, handleOwner_t owner);
void FS_VM_CreateTmp(char *name, const char *ext, file_t *f, handleOwner_t owner);
uint64_t FS_VM_FOpenFileRead(const char *path, file_t *f, handleOwner_t owner);
fileOffset_t FS_VM_FileSeek(file_t f, fileOffset_t offset, uint32_t whence, handleOwner_t owner);
uint64_t FS_VM_FOpenFileWrite(const char *path, file_t *f, handleOwner_t owner);
void FS_VM_CloseFiles(handleOwner_t owner);

void FS_Init(void);
void FS_InitFilesystem(void);
void FS_Shutdown(qboolean closeFiles);
void FS_Restart(void);

void FS_Remove(const char *path);
uint64_t FS_Read(void *buffer, uint64_t size, file_t f);
uint64_t FS_Write(const void *buffer, uint64_t size, file_t f);
void FS_WriteFile(const char *npath, const void *buffer, uint64_t size);
uint64_t FS_LoadFile(const char *npath, void **buffer);
void FS_FClose(file_t f);
void FS_ForceFlush(file_t f);
void FS_Flush(file_t f);

const char *FS_GetCurrentGameDir( void );
const char *FS_GetBaseGameDir( void );
const char *FS_GetBasePath( void );
const char *FS_GetHomePath( void );
const char *FS_GetGamePath( void );

void GDR_DECL FS_Printf(file_t f, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));

int FS_FileToFileno(file_t f);
file_t FS_FOpenWithMode(const char *path, fileMode_t mode);
uint64_t FS_FOpenFileWithMode(const char *path, file_t *f, fileMode_t mode);

file_t FS_FOpenRead(const char *path);
file_t FS_FOpenWrite(const char *path);
file_t FS_FOpenRW(const char *path);
uint64_t FS_FOpenFileRead(const char *path, file_t *fd);
void FS_Rename(const char *from, const char *to);
uint64_t FS_LoadStack(void);
int64_t FS_LastBFFIndex(void);
qboolean FS_FileExists(const char *filename);
fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
uint64_t FS_FileLength(file_t f);
fileOffset_t FS_FileTell(file_t f);
qboolean FS_FilenameCompare(const char *s1, const char *s2);
char *FS_BuildOSPath(const char *base, const char *game, const char *npath);
char *FS_CopyString(const char *s);
void *FS_LoadLibrary(const char *filename);

qboolean FS_AllowedExtension(const char *fileName, qboolean allowBFFs, const char **ext);
qboolean FS_StripExt(char *filename, const char *ext);
qboolean FS_FileIsInBFF(const char *path);
qboolean FS_Initialized(void);
char **FS_GetCurrentChunkList(uint64_t *numchunks);
void FS_SetBFFIndex(uint64_t index);
void FS_FreeFile(void *buffer);
char **FS_ListFiles(const char *path, const char *extension, uint64_t *numfiles);
char *FS_ReadLine(char *buf, uint64_t size, file_t f);

extern int CPU_flags;

// x86 flags
#define CPU_MMX		0x02
#define CPU_SSE		0x04
#define CPU_SSE2	0x08
#define CPU_SSE3	0x10
#define CPU_SSE41	0x20


/*
System calls, engine only stuff
*/

#ifdef __cplusplus
typedef struct {
	time_t mtime;
	time_t ctime;
	uint64_t size;
	qboolean exists;
} fileStats_t;

typedef struct memoryMap_s memoryMap_t;

const char *Sys_GetSteamPath(void);
uint64_t Sys_Milliseconds(void);
void Sys_InitConsole(void);
void Sys_FreeConsole(void);
FILE *Sys_FOpen(const char *filepath, const char *mode);

// Sys_MapMemory should only be called by the filesystem
memoryMap_t *Sys_MapMemory(FILE *fp, qboolean temp, file_t fd);
// like Sys_MapMemory but frees the mapped file, doesn't close it though
void Sys_UnmapMemory(memoryMap_t *file);

#define Sys_GetFreeRAM_Physical() (uint64_t)(Sys_GetTotalRAM_Physical() - Sys_GetUsedRAM_Physical())
#define Sys_GetFreeRAM_Virtual() (uint64_t)(Sys_GetTotalRAM_Virtual() - Sys_GetUsedRAM_Virtual())

uint64_t Sys_GetUsedRAM_Physical(void);
uint64_t Sys_GetUsedRAM_Virtual(void);
uint64_t Sys_GetTotalRAM_Virtual(void);
uint64_t Sys_GetTotalRAM_Physical(void);

uint64_t Sys_EventSubtime(uint64_t time);

fileOffset_t Sys_SeekMappedFile(fileOffset_t offset, uint32_t whence, memoryMap_t *file);
fileOffset_t Sys_TellMappedFile(memoryMap_t *file);
memoryMap_t *Sys_MapFile(const char *path, qboolean temp);
uint64_t Sys_ReadMappedFile(void *buffer, uint64_t size, memoryMap_t *file);
void *Sys_GetMappedFileBuffer(memoryMap_t *file);
void Sys_UnmapFile(memoryMap_t *file);

qboolean Sys_mkdir(const char *name);

void Sys_Print(const char *str);
void GDR_DECL Sys_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
const char *Sys_pwd(void);
void *Sys_LoadDLL(const char *name);
void Sys_CloseDLL(void *handle);
void *Sys_GetProcAddress(void *handle, const char *name);
const char *Sys_GetError(void);
void GDR_NORETURN Sys_Exit(int code);
qboolean Sys_GetFileStats(fileStats_t *stats, const char *path);
void Sys_FreeFileList(char **list);
void Sys_ListFilteredFiles(const char *basedir, const char *subdirs, const char *filter, char **list, uint64_t *numfiles);
char **Sys_ListFiles(const char *directory, const char *extension, const char *filter, uint64_t *numfiles, qboolean wantsubs);
const char *Sys_DefaultHomePath(void);
const char *Sys_DefaultBasePath(void);
qboolean Sys_RandomBytes(byte *s, uint64_t len);
#endif

#endif
