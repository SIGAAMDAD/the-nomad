#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_types.h"

#ifndef Q3_VM
typedef enum {
    REF_KEEP_CONTEXT, // don't destroy window and context, just deallocate buffers and shaders
    REF_KEEP_WINDOW, // destroy context, keep window
    REF_DESTROY_WINDOW,
    REF_UNLOAD_DLL
} refShutdownCode_t;

typedef struct {
    vec3_t          pos;
    vec2_t          uv;
    unsigned int    col;
} imguiDrawVert_t;

#ifndef __cplusplus
typedef unsigned int ImU32;
typedef uint32_t ImDrawIdx;
typedef imguiDrawVert_t ImDrawVert;
#endif

// Special Draw callback value to request renderer backend to reset the graphics/render state.
// The renderer backend needs to handle this special value, otherwise it will crash trying to call a function at this address.
// This is useful for example if you submitted callbacks which you know have altered the render state and you want it to be restored.
// It is not done by default because they are many perfectly useful way of altering render state for imgui contents (e.g. changing shader/blending settings before an Image call).
#ifndef ImDrawCallback_ResetRenderState
#define ImDrawCallback_ResetRenderState     (ImDrawCallback)(-1)
#endif

typedef struct imguiDrawCmd_s imguiDrawCmd_t;
typedef struct imguiDrawList_s imguiDrawList_t;

struct imguiDrawCmd_s {
    vec4_t          ClipRect;           // 4*4  // Clipping rectangle (x1, y1, x2, y2). Subtract ImDrawData->DisplayPos to get clipping rectangle in "viewport" coordinates
    void            *TextureId;          // 4-8  // User-provided texture ID. Set by user in ImfontAtlas::SetTexID() for fonts or passed to Image*() functions. Ignore if never using images or multiple fonts atlas.
    unsigned int    VtxOffset;          // 4    // Start offset in vertex buffer. ImGuiBackendFlags_RendererHasVtxOffset: always 0, otherwise may be >0 to support meshes larger than 64K vertices with 16-bit indices.
    unsigned int    IdxOffset;          // 4    // Start offset in index buffer.
    unsigned int    ElemCount;          // 4    // Number of indices (multiple of 3) to be rendered as triangles. Vertices are stored in the callee ImDrawList's vtx_buffer[] array, indices in idx_buffer[].
};

typedef struct {
    int Size;
    void *Data;
} imVector_t;

struct imguiDrawList_s {
    // This is what you have to render
    imVector_t CmdBuffer;          // Draw commands. Typically 1 command = 1 GPU draw call, unless the command is a callback
    imVector_t IdxBuffer;          // Index buffer. Each command consume ImDrawCmd::ElemCount of those
    imVector_t VtxBuffer;          // Vertex buffer.
    int Flags;                     // Flags, you may poke into these to adjust anti-aliasing settings per-primitive.
};

typedef struct {
    unsigned        Valid;                  // Only valid after Render() is called and before the next NewFrame() is called.
    int             CmdListsCount;          // Number of ImDrawList* to render
    int             TotalIdxCount;          // For convenience, sum of all ImDrawList's IdxBuffer.Size
    int             TotalVtxCount;          // For convenience, sum of all ImDrawList's VtxBuffer.Size
    vec2_t          DisplayPos;             // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
    vec2_t          DisplaySize;            // Size of the viewport to render (== GetMainViewport()->Size for the main viewport, == io.DisplaySize in most single-viewport applications)
    vec2_t          FramebufferScale;       // Amount of pixels for each unit of DisplaySize. Based on io.DisplayFramebufferScale. Generally (1,1) on normal display, (2,2) on OSX with Retina display.
    imguiDrawList_t**    CmdLists;          // Array of ImDrawList* to render. The ImDrawList are owned by ImGuiContext and only pointed to from here.
} imguiDrawData_t;

//
// refimport_t: for use with external engine system libraries
//
typedef struct {
	void *(*Malloc)(uint32_t size);
    void *(*Realloc)(void *ptr, uint32_t nsize); // really just for stb_image.h
    void (*FreeAll)(void);
	void (*Free)(void *ptr);
    char *(*Strdup)(const char *str);
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
    int32_t (*Cvar_VariableInteger)(const char *name);
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
    char *(*Cmd_ArgsFrom)(int32_t index);
    const char *(*Cmd_Argv)(uint32_t index);

    uint64_t (*Milliseconds)(void);

    qboolean (*Key_IsDown)(uint32_t keynum);

    void (*FS_FreeFileList)(char **list);
    uint64_t (*FS_Write)(const void *buffer, uint64_t size, file_t f);
    uint64_t (*FS_Read)(void *buffer, uint64_t size, file_t);
    fileOffset_t (*FS_FileSeek)(file_t f, fileOffset_t offset, uint32_t whence);
    fileOffset_t (*FS_FileTell)(file_t f);
    uint64_t (*FS_FileLength)(file_t f);
    qboolean (*FS_FileExists)(const char *filename);
    file_t (*FS_FOpenRead)(const char *path);
    file_t (*FS_FOpenWrite)(const char *path);
    void (*FS_FClose)(file_t f);
    void (*FS_FreeFile)(void *buffer);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    char **(*FS_ListFiles)(const char *path, const char *extension, uint64_t *numfiles);
    void (*FS_WriteFile)(const char *npath, const void *buffer, uint64_t size);

    void (*G_SetScaling)(float factor, uint32_t captureWidth, uint32_t captureHeight);

    void (*GLimp_Init)(gpuConfig_t *config);
    void (*GLimp_SetGamma)(const unsigned short r[256], const unsigned short g[256], const unsigned short b[256]);
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
    nhandle_t (*RegisterSpriteSheet)( const char *shaderName, uint32_t numSprites, uint32_t spriteWidth, uint32_t spriteHeight,
        uint32_t sheetWidth, uint32_t sheetHeight );
	nhandle_t (*RegisterShader)( const char *name );
    nhandle_t (*RegisterAnimation)( const char *name );
	void (*LoadWorld)( const char *name );

    void *(*GetTexDateFromShader)( nhandle_t hShader );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void (*EndRegistration)( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void (*ClearScene)( void );
    void (*BeginScene)( const renderSceneRef_t *fd );
    void (*EndScene)( void );
    void (*AddPolyToScene)( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
    void (*AddPolyListToScene)( const poly_t *polys, uint32_t numPolys );
    void (*AddEntityToScene)( const renderEntityRef_t *ent );
    void (*DrawImage)( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
	void (*RenderScene)( const renderSceneRef_t *fd );

    void (*GetConfig)( gpuConfig_t *config );

	void (*SetColor)( const float *rgba );	// NULL = 1,1,1,1

	void	(*BeginFrame)( stereoFrame_t stereoFrame );
	void	(*EndFrame)( uint64_t *frontEndMsec, uint64_t *backEndMsec );

//	void	(*RegisterFont)(const char *fontName, int pointSize, fontInfo_t *font);
	void	(*TakeVideoFrame)( int h, int w, byte* captureBuffer, byte *encodeBuffer, qboolean motionJpeg );

	void	(*ThrottleBackend)( void );
	void	(*FinishBloom)( void );

	void	(*SetColorMappings)( void );

	qboolean (*CanMinimize)( void ); // == fbo enabled

//	const glconfig_t *(*GetConfig)( void );

	void	(*VertexLighting)( qboolean allowed );
	void	(*SyncRender)( void );
} renderExport_t;

extern refimport_t ri;
typedef renderExport_t *(GDR_DECL *GetRenderAPI_t)(uint32_t version, refimport_t *import);

#endif

#endif
