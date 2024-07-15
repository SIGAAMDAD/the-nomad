#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_types.h"

typedef enum {
    REF_KEEP_CONTEXT, // don't destroy window and context, just deallocate buffers and shaders
    REF_KEEP_WINDOW, // destroy context, keep window
    REF_DESTROY_WINDOW,
    REF_UNLOAD_DLL
} refShutdownCode_t;

typedef struct {
    uint64_t msec;
    uint64_t postprocessMsec;
    uint64_t c_drawCalls;

    uint32_t c_glslShaderBinds;
    uint32_t c_bufferIndices, c_bufferVertices;
    uint32_t c_iboBinds, c_vboBinds, c_vaoBinds;
    uint32_t c_dynamicBufferDraws;
    uint32_t c_staticBufferDraws;
    uint32_t c_bufferBinds;
    uint32_t c_surfaces;
    uint32_t c_overDraw;
    uint32_t c_lightallDraws;
    uint32_t c_genericDraws;
} backendCounters_t;

typedef struct {
    // in bytes
    uint32_t estTotalMemUsed;
    uint32_t estBufferMemUsed;
    uint32_t estTextureMemUsed;
    uint32_t estVertexMemUsed;
    uint32_t estIndexMemUsed;

    uint32_t numVertexArrays;
    uint32_t numVertexBuffers;
    uint32_t numIndexBufers;
    uint32_t numBuffers;
    uint32_t numTextures;

    // polled from glGetIntegerv
    uint32_t dedicatedMem;
    uint32_t totalMem;
} gpuMemory_t;

//
// refimport_t: for use with external engine system libraries
//
typedef struct {
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
    void *(*Hunk_AllocateTempMemory)(uint64_t size);
    void (*Hunk_FreeTempMemory)(void *buf);

    void (*Sys_FreeFileList)(char **list);

    void (GDR_DECL *Printf)(int level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
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

    void (*GLM_MakeVPM)( const vec4_t ortho, float *zoom, float zNear, float zFar, vec3_t origin, mat4_t vpm,
        mat4_t projection, mat4_t view, uint32_t orthoFlags );
    void (*GLM_TransformToGL)( const vec3_t world, vec3_t *xyz, float scale, float rotation, mat4_t vpm );

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
    void (*FS_Remove)( const char *npath );
    void (*FS_HomeRemove)( const char *npath );

    void (*G_SetScaling)(float factor, uint32_t captureWidth, uint32_t captureHeight);
    size_t (*G_SaveJPGToBuffer)( byte *buffer, size_t bufSize, int32_t quality, int32_t imageWidth, int32_t imageHeight, byte *imageBuffer, int32_t padding );
    void (*G_SaveJPG)( const char *filename, int32_t quality, int32_t imageWidth, int32_t imageHeight, byte *imageBuffer, int32_t padding );
    void (*G_LoadJPG)( const char *filename, byte **pic, int32_t *width, int32_t *height );

    void (*GLimp_Init)( gpuConfig_t *config );
    void (*GLimp_InitGamma)( gpuConfig_t *config );
    void (*GLimp_SetGamma)( unsigned char red[256], unsigned char green[256], unsigned char blue[256] );
    void (*GLimp_HideFullscreenWindow)(void);
    void (*GLimp_EndFrame)(void);
    void (*GLimp_Shutdown)(qboolean unloadDLL);
    void (*GLimp_LogComment)(const char *comment);
    void (*GLimp_Minimize)( void );
    void *(*GL_GetProcAddress)(const char *name);

    void *(*Sys_LoadDLL)(const char *name);
    void *(*Sys_GetProcAddress)(void *handle, const char *name);
    void (*Sys_CloseDLL)(void *handle);

    void (*ImGui_Init)(void *shaderData, const void *importData);
    void (*ImGui_Shutdown)(void);
    void (*ImGui_NewFrame)(void);
    void (*ImGui_Draw)(void);
} refimport_t;

typedef struct {
    // called before the library is unloaded
	// if the system is just reconfiguring, pass destroyWindow = qfalse,
	// which will keep the screen from flashing to the desktop.
	void (*Shutdown)( refShutdownCode_t code );

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// BeginRegistration makes any existing media pointers invalid
	// and returns the current gl configuration, including screen width
	// and height, which can be used by the client to intelligently
	// size display elements
	void (*BeginRegistration)( gpuConfig_t *config );
	nhandle_t (*RegisterShader)( const char *name );
    nhandle_t (*RegisterSpriteSheet)( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight );
    nhandle_t (*RegisterSprite)( nhandle_t hSpriteSheet, uint32_t index );
	void (*LoadWorld)( const char *name );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void (*EndRegistration)( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void (*ClearScene)( void );
    void (*BeginScene)( const renderSceneRef_t *fd );
    void (*EndScene)( void );
    void (*AddSpriteToScene)( const vec3_t origin, nhandle_t hShader );
    void (*AddPolyToScene)( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
    void (*AddPolyListToScene)( const poly_t *polys, uint32_t numPolys );
    void (*AddEntityToScene)( const renderEntityRef_t *ent );
    void (*AddDynamicLightToScene)( const vec3_t origin, float brightness, const vec3_t color );
    void (*DrawImage)( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
	void (*RenderScene)( const renderSceneRef_t *fd );

    void *(*ImGui_TextureData)( nhandle_t hShader );

    void (*GetConfig)( gpuConfig_t *config );

    void (*GetGPUFrameStats)( uint32_t *time, uint32_t *samples, uint32_t *primitives );

	void (*SetColor)( const float *rgba );	// NULL = 1,1,1,1

	void	(*BeginFrame)( stereoFrame_t stereoFrame );
	void	(*EndFrame)( uint64_t *frontEndMsec, uint64_t *backEndMsec, backendCounters_t *pc );

//	void	(*RegisterFont)(const char *fontName, int pointSize, fontInfo_t *font);
	void	(*TakeVideoFrame)( int32_t h, int32_t w, byte* captureBuffer, byte *encodeBuffer, qboolean motionJpeg );

	void	(*ThrottleBackend)( void );
	void	(*FinishBloom)( void );

	void	(*SetColorMappings)( void );

	qboolean (*CanMinimize)( void ); // == fbo enabled

//	const glconfig_t *(*GetConfig)( void );

	void	(*VertexLighting)( qboolean allowed );
	void	(*SyncRender)( void );

    void    (*GetGPUMemStats)( gpuMemory_t *memstats );
} renderExport_t;

extern refimport_t ri;
typedef renderExport_t *(GDR_DECL *GetRenderAPI_t)(uint32_t version, refimport_t *import);

#endif
