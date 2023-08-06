#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

#include "../common/n_vm.h"
#include "../sgame/sg_public.h"

/*
Common functionality for the engine and vm alike
*/

void I_NomadInit(void);
uint32_t crc32_buffer(const byte *buf, uint32_t len);
void Com_Init(void);
void GDR_DECL Com_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
uint64_t Com_GenerateHashValue(const char *fname, const uint64_t size);
void Con_RenderConsole(void);
void Com_UpdateEvents(void);
void Com_WriteConfig(void);
qboolean Com_FilterExt( const char *filter, const char *name );
int Com_Filter( const char *filter, const char *name );
int Com_FilterPath( const char *filter, const char *name );
qboolean Com_HasPatterns( const char *str );
uint32_t Com_GetWindowEvents(void);
void* Com_GetEvents(void);
qboolean** Com_GetKeyboard(void);
const char* Com_Base64Decode(void);
void Com_TruncateLongString( char *buffer, const char *s );
int Com_HexStrToInt(const char *str);
void COM_StripExtension(const char *in, char *out, uint64_t destsize);
const char *Com_SkipTokens( const char *s, int numTokens, const char *sep );
const char *Com_SkipCharset( const char *s, const char *sep );
void COM_BeginParseSession( const char *name );
int COM_GetCurrentParseLine( void );
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

#define MAXPRINTMSG			8192

/* vm error codes for Com_Error/G_Error */
#define ERR_RESTART			0x0000 // restart the vm, don't stop the game though
#define ERR_FATAL			0x2000 // restart the vm and cancel the game loop, don't crash, only the engine can do that
#define ERR_FRAME			0x4000 // don't run the next vm frame

void Cmd_Init(void);
void Cmd_AddCommand(const char* name, cmdfunc_t function);
void Cmd_RemoveCommand(const char* name);
void Cmd_ExecuteCommand(const char* name);
void Cmd_ExecuteText(const char *str);
void Cmd_ExecuteString(const char *str);
uint32_t Cmd_Argc(void);
void Cmd_TokenizeString(const char *text_p);
void Cmd_TokenizeStringIgnoreQuotes(const char *text_p);
void Cmd_SetCommandCompletionFunc(const char *name, completionFunc_t fn);
char* Cmd_ArgsFrom(int32_t index);
const char* Cmd_Argv(uint32_t index);
void Cmd_Clear(void);
const char* GDR_DECL va(const char *format, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
#ifndef Q3_VM
void GDR_DECL Com_Error(vm_t *vm, int level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 3, 4)));
#endif

void Cbuf_InsertText( const char *text );
void Cbuf_Execute(void);
void Cbuf_AddText( const char *text );
void Cbuf_Clear(void);
void Cbuf_Init(void);

qboolean Key_IsPressed(qboolean **keys, uint32_t key);
const char *Key_GetBinding(uint32_t key);

/*

Edit fields and command line history/completion

*/

#define TRUNCATE_LENGTH 64
#define	MAX_EDIT_LINE 512
#if MAX_EDIT_LINE > MAX_CMD_LINE
#error "MAX_EDIT_LINE > MAX_CMD_LINE"
#endif
typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
} field_t;

void Field_CompleteKeyBind( uint32_t key );
void Field_CompleteFilename(const char *dir, const char *ext, qboolean stripExt, int flags);
void Field_CompleteCommand(const char *cmd, qboolean doCommands, qboolean doCvars);

/*
The filesystem, heavily based on quake's filesystem
*/

typedef enum {
	H_ENGINE,
	H_SCRIPT,
	H_SGAME,
	H_ALLOCATOR,
	H_RENDERER
} handleOwner_t;

#define NUM_GDR_BFFS 1

#define FS_MATCH_BASEDIR	(1<<0)
#define FS_MATCH_EXTERN		(2<<0)
#define FS_MATCH_BFFs		(FS_MATCH_EXTERN | FS_MATCH_BASEDIR)
#define FS_MATCH_ANY		(FS_MATCH_EXTERN | FS_MATCH_BASEDIR)

#define MAX_FOUND_FILES 0x8000

#define FS_INVALID_HANDLE 0

typedef int32_t file_t;
typedef time_t fileTime_t;
#if defined(_MSVC_VER) || defined(__clang__)
typedef _off_t fileOffset_t;
#elif !defined(Q3_VM)
typedef off_t fileOffset_t;
#else
typedef long fileOffset_t;
#endif

#define FS_SEEK_END 0
#define FS_SEEK_CUR 1
#define FS_SEEK_BEGIN 2

#define FS_SEEK_SET FS_SEEK_BEGIN

#ifndef Q3_VM
extern file_t logfile;
#endif

void FS_Init(void);
void FS_InitFilesystem(void);
void FS_Shutdown(void);
void FS_Restart(void);

/* vm specific file handling */
void FS_VM_FOpenWrite(const char *path, file_t *f);
void FS_VM_FOpenRead(const char *path, file_t *f);
void FS_VM_FClose(file_t *f);
void FS_VM_CreateTmp(char *name, const char *ext, file_t *f);

void FS_Remove(const char *path);
uint64_t FS_Read(void *buffer, uint64_t size, file_t f);
uint64_t FS_Write(const void *buffer, uint64_t size, file_t f);
void FS_WriteFile(const char *npath, const void *buffer, uint64_t size);
uint64_t FS_LoadFile(const char *npath, void **buffer);
void FS_FClose(file_t f);
void FS_ForceFlush(file_t f);
void FS_Flush(file_t f);

void GDR_DECL FS_Printf(file_t f, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));

file_t FS_FOpenRead(const char *path);
file_t FS_FOpenWrite(const char *path);
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

qboolean FS_StripExt(char *filename, const char *ext);
qboolean FS_FileIsInBFF(const char *path);
qboolean FS_Initialized(void);

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

#ifndef Q3_VM
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

fileOffset_t Sys_SeekMappedFile(fileOffset_t offset, uint32_t whence, memoryMap_t *file);
fileOffset_t Sys_TellMappedFile(memoryMap_t *file);
memoryMap_t *Sys_MapFile(const char *path, qboolean temp);
uint64_t Sys_ReadMappedFile(void *buffer, uint64_t size, memoryMap_t *file);
void *Sys_GetMappedFileBuffer(memoryMap_t *file);
void Sys_UnmapFile(memoryMap_t *file);

void GDR_DECL Sys_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
const char *Sys_pwd(void);
void *Sys_LoadDLL(const char *path);
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
