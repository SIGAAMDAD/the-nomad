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
void GDR_DECL Com_Printf(const char *fmt, ...);
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
qboolean* Com_GetKeyboard(void);
const char* Com_Base64Decode();
int Com_HexStrToInt(const char *str);
void GDR_DECL Com_Error(const char *fmt, ...);
void COM_StripExtension(const char *in, char *out, uint64_t destsize);
const char *Com_SkipTokens( const char *s, int numTokens, const char *sep );
const char *Com_SkipCharset( const char *s, const char *sep );
void COM_BeginParseSession( const char *name );
int	 COM_GetCurrentParseLine( void );
const char *COM_Parse( const char **data_p );
const char *COM_ParseExt( const char **data_p, qboolean allowLineBreak );
int COM_Compress( char *data_p );
void COM_ParseError( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void COM_ParseWarning( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));
//int		COM_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] );

char	*COM_ParseComplex( const char **data_p, qboolean allowLineBreak );

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
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192

void Cmd_AddCommand(const char* name, cmdfunc_t function);
void Cmd_RemoveCommand(const char* name);
void Cmd_ExecuteCommand(const char* name);
void Cmd_ExecuteText(const char *str);
void Cmd_ExecuteString(const char *str);
uint32_t Cmd_Argc(void);
char* Cmd_ArgsFrom(int32_t index);
const char* Cmd_Argv(uint32_t index);
void Cmd_Clear(void);
const char* GDR_DECL va(const char *format, ...);

void GDR_DECL Com_Error(const char *fmt, ...);

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

extern cvar_t fs_gamedir;
extern cvar_t fs_numArchives;

extern file_t logfile; // VM USERS DO NOT USE!!!!!!!!!!!!

void FS_Init(void);

uint32_t FS_LoadStack(void);
uint64_t FS_Write(const void *data, uint64_t size, file_t f);
uint64_t FS_Read(void *data, uint64_t size, file_t f);
qboolean FS_Initialized(void);
file_t FS_OpenBFF(int32_t index);
file_t FS_FOpenRead(const char *filepath);
file_t FS_FOpenWrite(const char *filepath);
file_t FS_CreateTmp(char **name, const char *ext);
char* FS_GetOSPath(file_t f);
void FS_ThePurge(void);
void* FS_GetBFFData(file_t handle);
void FS_FClose(file_t handle);
uint64_t FS_FileLength(file_t f);
void FS_Remove(const char *ospath);
uint64_t FS_FileTell(file_t f);
fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
qboolean FS_FileExists(const char *file);
int32_t FS_LastBFFIndex(void);
void FS_FreeFile(void *buffer);
uint64_t FS_LoadFile(const char *path, void **buffer);
void *FS_LoadLibrary(const char *name);

/*
System calls, engine only stuff
*/

#ifndef Q3_VM
typedef struct {
	
	uint64_t length;
	qboolean exists;
} fileStats_t;

const char *Sys_GetSteamPath(void);
uint64_t Sys_Milliseconds(void);
void Sys_InitConsole(void);
void Sys_FreeConsole(void);
FILE *Sys_FOpen(const char *filepath, const char *mode);
void Sys_Print(const char *str, uint64_t nBytes, void *handle);
const char *Sys_pwd(void);
void *Sys_LoadDLL(const char *path);
void Sys_CloseDLL(void *handle);
void *Sys_GetProcAddress(void *handle, const char *name);
const char *Sys_GetError(void);
void GDR_NORETURN Sys_Exit(int code);
void Sys_GetFileState(fileStats_t *stats, const char *path);
#endif

#endif