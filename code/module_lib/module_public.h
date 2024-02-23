#ifndef __MODULE_PUBLIC__
#define __MODULE_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "../game/g_archive.h"

typedef struct
{
    void *(*Malloc)(uint64_t size);
    void *(*Realloc)(void *ptr, uint64_t nsize); // really just for stb_image.h
    void (*FreeAll)(void);
	void (*Free)(void *ptr);
    char *(*CopyString)(const char *str);
#ifdef _NOMAD_DEBUG
    void *(*Hunk_AllocDebug)(uint64_t size, ha_pref where, const char *label, const char *file, uint64_t line);
#else
	void *(*Hunk_Alloc)(uint64_t size, ha_pref where);
#endif
    void *(*Hunk_AllocateTempMemory)( uint64_t size );
    void (*Hunk_FreeTempMemory)( void *buf );

    void (*Sys_FreeFileList)( char **list );

    void (GDR_DECL *Printf)( int level, const char *fmt, ... ) GDR_ATTRIBUTE((format(printf, 2, 3)));
    void GDR_NORETURN (GDR_DECL *Error)(errorCode_t code, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));

    void (*Cvar_VariableStringBuffer)(const char *name, char *buffer, uint64_t bufferSize);
    void (*Cvar_VariableStringBufferSafe)(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
    int64_t (*Cvar_VariableInteger)(const char *name);
    cvar_t *(*Cvar_Get)(const char *name, const char *value, uint32_t flags);
    void (*Cvar_SetGroup)(cvar_t *cv, cvarGroup_t group);
    void (*Cvar_SetDescription)(cvar_t *cv, const char *description);
    void (*Cvar_Set)(const char *name, const char *value);
    void (*Cvar_CheckRange)(cvar_t *var, const char *mins, const char *maxs, cvartype_t type);
    int (*Cvar_CheckGroup)(cvarGroup_t group);
    void (*Cvar_ResetGroup)( cvarGroup_t group, qboolean resetModifiedFlags );
    void (*Cvar_Reset)(const char *name);
    const char *(*Cvar_VariableString)(const char *name);

    void (*Cmd_AddCommand)(const char* name, cmdfunc_t function);
    void (*Cmd_RemoveCommand)(const char* name);
    void (*Cmd_ExecuteCommand)(const char* name);
    void (*Cmd_ExecuteString)(const char *str);
    uint32_t (*Cmd_Argc)(void);
    char *(*Cmd_ArgsFrom)(uint32_t index);
    const char *(*Cmd_Argv)(uint32_t index);

    uint64_t (*Milliseconds)(void);

    qboolean (*Key_IsDown)(uint32_t keynum);

    void (*FS_FreeFileList)(char **list);
    uint64_t (*FS_Write)(const void *buffer, uint64_t size, fileHandle_t f);
    uint64_t (*FS_Read)(void *buffer, uint64_t size, fileHandle_t);
    fileOffset_t (*FS_FileSeek)(fileHandle_t f, fileOffset_t offset, uint32_t whence);
    fileOffset_t (*FS_FileTell)(fileHandle_t f);
    uint64_t (*FS_FileLength)(fileHandle_t f);
    qboolean (*FS_FileExists)(const char *filename);
    fileHandle_t (*FS_FOpenRead)(const char *path);
    fileHandle_t (*FS_FOpenWrite)(const char *path);
    void (*FS_FClose)(fileHandle_t f);
    void (*FS_FreeFile)(void *buffer);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    char **(*FS_ListFiles)(const char *path, const char *extension, uint64_t *numfiles);
    void (*FS_WriteFile)(const char *npath, const void *buffer, uint64_t size);

    void *(*Sys_LoadDLL)(const char *name);
    void *(*Sys_GetProcAddress)(void *handle, const char *name);
    void (*Sys_CloseDLL)(void *handle);
} moduleImport_t;

/*
* CModuleMemoryAllocator: a dedicated heap manager for modules to avoid fragmentation or the vm touching
* engine memory, only for the modulelib's hands, no one else's
*/
class CModuleMemoryAllocator
{
public:
private:
    enum {
		ALIGN = 8									// memory alignment in bytes
	};

	enum {
		INVALID_ALLOC	= 0xdd,
		SMALL_ALLOC		= 0xaa,						// small allocation
		MEDIUM_ALLOC	= 0xbb,						// medium allocaction
		LARGE_ALLOC		= 0xcc						// large allocaction
	};

	typedef struct page_s {							// allocation page
		void						*data;			// data pointer to allocated memory
		uint64_t					dataSize;		// number of bytes of memory 'data' points to
		struct page_s				*next;			// next free page in same page manager
		struct page_s				*prev;			// used only when allocated
		uint64_t					largestFree;	// this data used by the medium-size heap manager
		void						*firstFree;		// pointer to first free entry
	} page_t;

	typedef struct mediumHeapEntry_s {
		struct page_s				*page;			// pointer to page
		uint64_t					size;			// size of block
		struct mediumHeapEntry_s	*prev;			// previous block
		struct mediumHeapEntry_s	*next;			// next block
		struct mediumHeapEntry_s	*prevFree;		// previous free block
		struct mediumHeapEntry_s	*nextFree;		// next free block
		uint64_t					freeBlock;		// non-zero if free block
	} mediumHeapEntry_t;

	// variables
	void			*smallFirstFree[256/ALIGN+1];	// small heap allocator lists (for allocs of 1-255 bytes)
	page_t			*smallCurPage;					// current page for small allocations
	uint64_t		smallCurPageOffset;				// byte offset in current page
	page_t			*smallFirstUsedPage;			// first used page of the small heap manager

	page_t			*mediumFirstFreePage;			// first partially free page
	page_t			*mediumLastFreePage;			// last partially free page
	page_t			*mediumFirstUsedPage;			// completely used page

	page_t			*largeFirstUsedPage;			// first page used by the large heap manager

	page_t			*swapPage;

	uint64_t		pagesAllocated;					// number of pages currently allocated
	uint64_t		pageSize;						// size of one alloc page in bytes

	uint64_t		pageRequests;					// page requests
	uint64_t		OSAllocs;						// number of allocs made to the OS

	uint32_t		c_heapAllocRunningCount;

    //
    // methods
    //

    // allocate page from the OS
	page_t          *AllocatePage( uint64_t nBytes );

    // free an OS allocated page
	void			FreePage( CModuleMemoryAllocator::page_t *p );

    // allocate memory (1-255 bytes) from small heap manager
	void            *SmallAllocate( uint64_t nBytes );

    // free memory allocated by small heap manager
	void			SmallFree( void *pBuffer );

	void            *MediumAllocateFromPage( CModuleMemoryAllocator::page_t *p, uint64_t sizeNeeded );

    // allocate memory (256-32768 bytes) from medium heap manager
	void            *MediumAllocate( uint64_t nBytes );

    // free memory allocated by medium heap manager
	void			MediumFree( void *pBuffer );

    // allocate large block from OS directly
	void            *LargeAllocate( uint64_t nBytes );

    // free memory allocated by large heap manager
	void			LargeFree( void *pBuffer );

	void			ReleaseSwappedPages( void );
	void			FreePageReal( CModuleMemoryAllocator::page_t *p );
};

struct CModuleInfo
{
	char m_szName[MAX_NPATH];
	eastl::vector<CModuleInfo *> m_Dependencies;
	
	int32_t m_nModVersionMajor;
	int32_t m_nModVersionUpdate;
	int32_t m_nModVersionPatch;

	version_t m_GameVersion;
};

GDR_EXPORT class CModuleLib
{
public:
    CModuleLib( void ) = default;
    ~CModuleLib() = default;

    extern "C" void Init( const moduleImport_t *pImport );
    extern "C" void Shutdown( void );

	extern "C" void ModuleCallFunc( nhandle_t hModule, uint32_t nFuncId );

	//
	// vm dynamic memory interface (not shared between vms)
	//
	extern "C" nhandle_t ModuleCreateBuffer( nhandle_t hModule, uint32_t nBytes );
	extern "C" void ModuleReleaseBuffer( nhandle_t hBuffer );

	//
	// vm memory link interface
	//
	extern "C" void ModuleCreateLink( void );
private:
	extern "C" void LoadModule( const char *pModuleName );

	eastl::vector<CModuleInfo> m_LoadList;
    CModuleMemoryAllocator m_ModuleAllocator;
};

void *Mem_Alloc( uint32_t nBytes );
void Mem_Free( void *pBuffer );

#ifdef GDR_DLLCOMPILE
#undef new
#undef delete
GDR_INLINE void *operator new( size_t sz ) {
	return Mem_Alloc( sz );
}
GDR_INLINE void *operator new[]( size_t sz ) {
	return Mem_Alloc( sz );
}
GDR_INLINE void operator delete( void *ptr ) {
	Mem_Free( ptr );
}
GDR_INLINE void operator delete[]( void *ptr ) {
	Mem_Free( ptr );
}
#endif

extern moduleImport_t moduleImport;

GDR_EXPORT extern CModuleLib *g_pModuleLib;

#endif