#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

#include "../common/n_vm.h"

/*
Common functionality for the engine and vm alike
*/
#ifndef Q3_VM
extern qboolean console_open;
typedef struct
{
    SDL_Event event;
    const uint8_t *kbstate;
} eventState_t;
extern eventState_t evState;
#endif


void Com_Init(void);
void GDR_DECL Com_Printf(const char *fmt, ...);
uint64_t Com_GenerateHashValue(const char *fname, const uint32_t size);
void Con_RenderConsole(void);
void Com_UpdateEvents(void);
void GDR_DECL Com_Error(const char *fmt, ...);

const char *Com_SkipTokens( const char *s, int numTokens, const char *sep );
const char *Com_SkipCharset( const char *s, const char *sep );
void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
const char	*COM_Parse( const char **data_p );
const char	*COM_ParseExt( const char **data_p, qboolean allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void	COM_ParseWarning( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));
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

#ifndef Q3_VM

typedef void (*completionFunc_t)(const char* args, uint32_t argnum);
typedef void (*cmdfunc_t)(void);

void Cmd_AddCommand(const char* name, cmdfunc_t function);
void Cmd_RemoveCommand(const char* name);
void Cmd_ExecuteCommand(const char* name);
void Cmd_ExecuteText(const char *str);
void Cmd_ExecuteString(const char *str);
uint32_t Cmd_Argc(void);
const char* Cmd_Argv(uint32_t index);
void Cmd_Clear(void);
const char* GDR_DECL va(const char *format, ...);

void GDR_DECL Com_Error(const char *fmt, ...);

/*
The filesystem, heavily based on quake's filesystem

THIS SHOULD NEVER BE USED BY THE VM
*/

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

void FS_Init(void);

uint64_t FS_Write(const void *data, uint64_t size, file_t f);
uint64_t FS_Read(void *data, uint64_t size, file_t f);
file_t FS_OpenBFF(int32_t index);
file_t FS_FOpenRead(const char *filepath);
file_t FS_FOpenWrite(const char *filepath);
file_t FS_CreateTmp(char **name, const char *ext);
char* FS_GetOSPath(file_t f);
//uint32_t FS_NumBFFs(void);
void* FS_GetBFFData(file_t handle);
void FS_FClose(file_t handle);
uint64_t FS_FileLength(file_t f);
void FS_Remove(const char *ospath);
uint64_t FS_FileTell(file_t f);
fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
file_t FS_BFFOpen(const char *chunkpath);
qboolean FS_FileExists(const char *file);


/*
System calls, engine only stuff
*/

#ifndef Q3_VM
#ifdef _WIN32
#define nstat_t struct __stat64
#else
#include <sys/stat.h>
#define nstat_t struct stat
#endif

FILE* Sys_FOpen(const char *filepath, const char *mode);
int Sys_stat(nstat_t* buffer, const char *filepath);
void GDR_DECL Sys_Print(const char* str);
const char* Sys_pwd(void);

#endif

#endif

#endif