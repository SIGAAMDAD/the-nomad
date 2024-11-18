// dear imgui: Renderer Backend for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 2.x 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Backend (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID!
//  [x] Renderer: Large meshes support (64k+ vertices) with 16-bit indices (Desktop OpenGL only).

// About WebGL/ES:
// - You need to '#define IMGUI_IMPL_OPENGL_ES2' or '#define IMGUI_IMPL_OPENGL_ES3' to use WebGL or OpenGL ES.
// - This is done automatically on iOS, Android and Emscripten targets.
// - For other targets, the define needs to be visible from the imgui_impl_opengl3.cpp compilation unit. If unsure, define globally or in imconfig.h.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2023-10-05: OpenGL: Rename symbols in our internal loader so that LTO compilation with another copy of gl3w is possible. (#6875, #6668, #4445)
//  2023-06-20: OpenGL: Fixed erroneous use glGetIntegerv(GL_CONTEXT_PROFILE_MASK) on contexts lower than 3.2. (#6539, #6333)
//  2023-05-09: OpenGL: Support for glBindSampler() backup/restore on ES3. (#6375)
//  2023-04-18: OpenGL: Restore front and back polygon mode separately when supported by context. (#6333)
//  2023-03-23: OpenGL: Properly restoring "no shader program bound" if it was the case prior to running the rendering function. (#6267, #6220, #6224)
//  2023-03-15: OpenGL: Fixed GL loader crash when GL_VERSION returns NULL. (#6154, #4445, #3530)
//  2023-03-06: OpenGL: Fixed restoration of a potentially deleted OpenGL program, by calling glIsProgram(). (#6220, #6224)
//  2022-11-09: OpenGL: Reverted use of glBufferSubData(), too many corruptions issues + old issues seemingly can't be reproed with Intel drivers nowadays (revert 2021-12-15 and 2022-05-23 changes).
//  2022-10-11: Using 'nullptr' instead of 'NULL' as per our switch to C++11.
//  2022-09-27: OpenGL: Added ability to '#define IMGUI_IMPL_OPENGL_DEBUG'.
//  2022-05-23: OpenGL: Reworking 2021-12-15 "Using buffer orphaning" so it only happens on Intel GPU, seems to cause problems otherwise. (#4468, #4825, #4832, #5127).
//  2022-05-13: OpenGL: Fixed state corruption on OpenGL ES 2.0 due to not preserving GL_ELEMENT_ARRAY_BUFFER_BINDING and vertex attribute states.
//  2021-12-15: OpenGL: Using buffer orphaning + glBufferSubData(), seems to fix leaks with multi-viewports with some Intel HD drivers.
//  2021-08-23: OpenGL: Fixed ES 3.0 shader ("#version 300 es") use normal precision floats to avoid wobbly rendering at HD resolutions.
//  2021-08-19: OpenGL: Embed and use our own minimal GL loader (imgui_impl_opengl3_loader.h), removing requirement and support for third-party loader.
//  2021-06-29: Reorganized backend to pull data from a single structure to facilitate usage with multiple-contexts (all g_XXXX access changed to bd->XXXX).
//  2021-06-25: OpenGL: Use OES_vertex_array extension on Emscripten + backup/restore current state.
//  2021-06-21: OpenGL: Destroy individual vertex/fragment shader objects right after they are linked into the main shader.
//  2021-05-24: OpenGL: Access GL_CLIP_ORIGIN when "GL_ARB_clip_control" extension is detected, inside of just OpenGL 4.5 version.
//  2021-05-19: OpenGL: Replaced direct access to ImDrawCmd::TextureId with a call to ImDrawCmd::GetTexID(). (will become a requirement)
//  2021-04-06: OpenGL: Don't try to read GL_CLIP_ORIGIN unless we're OpenGL 4.5 or greater.
//  2021-02-18: OpenGL: Change blending equation to preserve alpha in output buffer.
//  2021-01-03: OpenGL: Backup, setup and restore GL_STENCIL_TEST state.
//  2020-10-23: OpenGL: Backup, setup and restore GL_PRIMITIVE_RESTART state.
//  2020-10-15: OpenGL: Use glGetString(GL_VERSION) instead of glGetIntegerv(GL_MAJOR_VERSION, ...) when the later returns zero (e.g. Desktop GL 2.x)
//  2020-09-17: OpenGL: Fix to avoid compiling/calling glBindSampler() on ES or pre 3.3 context which have the defines set by a loader.
//  2020-07-10: OpenGL: Added support for glad2 OpenGL loader.
//  2020-05-08: OpenGL: Made default GLSL version 150 (instead of 130) on OSX.
//  2020-04-21: OpenGL: Fixed handling of glClipControl(GL_UPPER_LEFT) by inverting projection matrix.
//  2020-04-12: OpenGL: Fixed context version check mistakenly testing for 4.0+ instead of 3.2+ to enable ImGuiBackendFlags_RendererHasVtxOffset.
//  2020-03-24: OpenGL: Added support for glbinding 2.x OpenGL loader.
//  2020-01-07: OpenGL: Added support for glbinding 3.x OpenGL loader.
//  2019-10-25: OpenGL: Using a combination of GL define and runtime GL version to decide whether to use glDrawElementsBaseVertex(). Fix building with pre-3.2 GL loaders.
//  2019-09-22: OpenGL: Detect default GL loader using __has_include compiler facility.
//  2019-09-16: OpenGL: Tweak initialization code to allow application calling ImGui_ImplOpenGL3_CreateFontsTexture() before the first NewFrame() call.
//  2019-05-29: OpenGL: Desktop GL only: Added support for large mesh (64K+ vertices), enable ImGuiBackendFlags_RendererHasVtxOffset flag.
//  2019-04-30: OpenGL: Added support for special ImDrawCallback_ResetRenderState callback to reset render state.
//  2019-03-29: OpenGL: Not calling glBindBuffer more than necessary in the render loop.
//  2019-03-15: OpenGL: Added a GL call + comments in ImGui_ImplOpenGL3_Init() to detect uninitialized GL function loaders early.
//  2019-03-03: OpenGL: Fix support for ES 2.0 (WebGL 1.0).
//  2019-02-20: OpenGL: Fix for OSX not supporting OpenGL 4.5, we don't try to read GL_CLIP_ORIGIN even if defined by the headers/loader.
//  2019-02-11: OpenGL: Projecting clipping rectangles correctly using draw_data->FramebufferScale to allow multi-viewports for retina display.
//  2019-02-01: OpenGL: Using GLSL 410 shaders for any version over 410 (e.g. 430, 450).
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-11-13: OpenGL: Support for GL 4.5's glClipControl(GL_UPPER_LEFT) / GL_CLIP_ORIGIN.
//  2018-08-29: OpenGL: Added support for more OpenGL loaders: glew and glad, with comments indicative that any loader can be used.
//  2018-08-09: OpenGL: Default to OpenGL ES 3 on iOS and Android. GLSL version default to "#version 300 ES".
//  2018-07-30: OpenGL: Support for GLSL 300 ES and 410 core. Fixes for Emscripten compilation.
//  2018-07-10: OpenGL: Support for more GLSL versions (based on the GLSL version string). Added error output when shaders fail to compile/link.
//  2018-06-08: Misc: Extracted imgui_impl_opengl3.cpp/.h away from the old combined GLFW/SDL+OpenGL3 examples.
//  2018-06-08: OpenGL: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-05-25: OpenGL: Removed unnecessary backup/restore of GL_ELEMENT_ARRAY_BUFFER_BINDING since this is part of the VAO state.
//  2018-05-14: OpenGL: Making the call to glBindSampler() optional so 3.2 context won't fail if the function is a nullptr pointer.
//  2018-03-06: OpenGL: Added const char* glsl_version parameter to ImGui_ImplOpenGL3_Init() so user can override the GLSL version e.g. "#version 150".
//  2018-02-23: OpenGL: Create the VAO in the render function so the setup can more easily be used with multiple shared GL context.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed ImGui_ImplSdlGL3_RenderDrawData() in the .h file so you can call it yourself.
//  2018-01-07: OpenGL: Changed GLSL shader version from 330 to 150.
//  2017-09-01: OpenGL: Save and restore current bound sampler. Save and restore current polygon mode.
//  2017-05-01: OpenGL: Fixed save and restore of current blend func state.
//  2017-05-01: OpenGL: Fixed save and restore of current GL_ACTIVE_TEXTURE.
//  2016-09-05: OpenGL: Fixed save and restore of current scissor rectangle.
//  2016-07-29: OpenGL: Explicitly setting GL_UNPACK_ROW_LENGTH to reduce issues because SDL changes it. (#752)

//----------------------------------------
// OpenGL    GLSL      GLSL
// version   version   string
//----------------------------------------
//  2.0       110       "#version 110"
//  2.1       120       "#version 120"
//  3.0       130       "#version 130"
//  3.1       140       "#version 140"
//  3.2       150       "#version 150"
//  3.3       330       "#version 330 core"
//  4.0       400       "#version 400 core"
//  4.1       410       "#version 410 core"
//  4.2       420       "#version 410 core"
//  4.3       430       "#version 430 core"
//  ES 2.0    100       "#version 100"      = WebGL 1.0
//  ES 3.0    300       "#version 300 es"   = WebGL 2.0
//----------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../engine/n_shared.h"
#include "../engine/n_cvar.h"
#include "../rendergl/ngl.h"
#include "../game/g_game.h"
#include "../rendercommon/r_types.h"
#include "../rendergl/rgl_local.h"
#include "imgui.h"
// #ifndef IMGUI_DISABLE
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdint.h> // intptr_t
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DEFAULT_VERTEX_BUFFER_SIZE sizeof( ImDrawVert ) * ( 16 * 1024 )
#define DEFAULT_INDEX_BUFFER_SIZE sizeof( ImDrawIdx ) * ( 16 * 1024 )

#include <GL/gl.h>

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"  // warning: use of old-style cast
#pragma clang diagnostic ignored "-Wsign-conversion" // warning: implicit conversion changes signedness
#pragma clang diagnostic ignored "-Wunused-macros"   // warning: macro is not used
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wcast-function-type" // warning: cast between incompatible function types (for loader)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wunknown-warning-option" // warning: unknown warning group 'xxx'
#pragma GCC diagnostic ignored "-Wcast-function-type"     // warning: cast between incompatible function types (for loader)
#endif

// GL includes
#if defined(IMGUI_IMPL_OPENGL_ES2)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES2/gl.h> // Use GL ES 2
#else
#include <GLES2/gl2.h> // Use GL ES 2
#endif
#if defined(__EMSCRIPTEN__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#endif
#elif defined(IMGUI_IMPL_OPENGL_ES3)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES3/gl.h> // Use GL ES 3
#else
#include <GLES3/gl3.h> // Use GL ES 3
#endif
#elif !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
// Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
// Helper libraries are often used for this purpose! Here we are using our own minimal custom loader based on gl3w.
// In the rest of your app/engine, you can use another loader of your choice (gl3w, glew, glad, glbinding, glext, glLoadGen, etc.).
// If you happen to be developing a new feature for this backend (imgui_impl_opengl3.cpp):
// - You may need to regenerate imgui_impl_opengl3_loader.h to add new symbols. See https://github.com/dearimgui/gl3w_stripped
// - You can temporarily use an unstripped version. See https://github.com/dearimgui/gl3w_stripped/releases
// Changes to this backend using new APIs should be accompanied by a regenerated stripped loader version.

// Vertex arrays are not supported on ES2/WebGL1 unless Emscripten which uses an extension
#ifndef IMGUI_IMPL_OPENGL_ES2
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#elif defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define GL_VERTEX_ARRAY_BINDING GL_VERTEX_ARRAY_BINDING_OES
#endif

// Desktop GL 2.0+ has glPolygonMode() which GL ES and WebGL don't have.
#ifdef GL_POLYGON_MODE
#define IMGUI_IMPL_HAS_POLYGON_MODE
#endif

// Desktop GL 3.2+ has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
#endif

// Desktop GL 3.3+ and GL ES 3.0+ have glBindSampler()
#if !defined(IMGUI_IMPL_OPENGL_ES2) && (defined(IMGUI_IMPL_OPENGL_ES3) || defined(GL_VERSION_3_3))
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
#endif

// Desktop GL 3.1+ has GL_PRIMITIVE_RESTART state
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_1)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
#endif

// Desktop GL use extension detection
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_EXTENSIONS
#endif

// [Debugging]
// #define IMGUI_IMPL_OPENGL_DEBUG
#ifdef IMGUI_IMPL_OPENGL_DEBUG
#include <stdio.h>
#define GL_CALL(_CALL)                                                              \
	do                                                                              \
	{                                                                               \
		_CALL;                                                                      \
		GLenum gl_err = glGetError();                                               \
		if (gl_err != 0)                                                            \
			fprintf(stderr, "GL error 0x%x returned from '%s'.\n", gl_err, #_CALL); \
	} while (0) // Call with error check
#else
#define GL_CALL(_CALL) _CALL // Call without error check
#endif

// OpenGL Data
struct ImGui_ImplOpenGL3_Data
{
	GLuint GlVersion;           // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
	char GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
	int GlProfileIsES2;
	int GlProfileIsES3;
	int GlProfileIsCompat;
	int GlDirectStateAccess;
	GLint GlProfileMask;
	GLuint FontTexture;
	nhandle_t FontShader;

	GLuint ShaderHandle;
	GLint AttribLocationTex; // Uniforms location
	GLint AttribLocationGamma;
	GLint AttribLocationProjMtx;
	GLuint AttribLocationVtxPos; // Vertex attributes location
	GLuint AttribLocationVtxUV;
	GLuint AttribLocationVtxColor;
	unsigned int VboHandle, ElementsHandle;
	GLsizeiptr VertexBufferSize;
	GLsizeiptr VertexBufferOffset;
	GLsizeiptr IndexBufferSize;
	GLsizeiptr IndexBufferOffset;
	GLuint vaoId;
	int HasClipOrigin;
	int UseBufferSubData;
	void *VertexBuffer;
	void *IndexBuffer;

	ImGui_ImplOpenGL3_Data() { memset((void *)this, 0, sizeof(*this)); }
};

static GLuint imguiShader;
static ImGui_ImplOpenGL3_Data *bd;
imguiGL3Import_t renderImport;
static cvar_t *r_gamma;

// Backend data stored in io.BackendRendererUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
static ImGui_ImplOpenGL3_Data *ImGui_ImplOpenGL3_GetBackendData()
{
	return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data *)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// OpenGL vertex attribute state (for ES 1.0 and ES 2.0 only)
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
struct ImGui_ImplOpenGL3_VtxAttribState
{
	GLint Enabled, Size, Type, Normalized, Stride;
	GLvoid *Ptr;

	void GetState(GLint index)
	{
		renderImport.glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &Enabled);
		renderImport.glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &Size);
		renderImport.glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &Type);
		renderImport.glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &Normalized);
		renderImport.glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &Stride);
		renderImport.glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &Ptr);
	}
	void SetState(GLint index)
	{
		renderImport.glVertexAttribPointer(index, Size, Type, (GLintean)Normalized, Stride, Ptr);
		if (Enabled)
		{
			renderImport.glEnableVertexAttribArray(index);
		}
		else
		{
			renderImport.glDisableVertexAttribArray(index);
		}
	}
};
#endif

// if this failed, oh shit...
static_assert(sizeof(GLuint) == sizeof(uint32_t));

// Functions
void ImGui_ImplOpenGL3_Init(void *shaderData, const char *glsl_version, const imguiGL3Import_t *import)
{
	GLint current_texture;
	GLint major, minor;
	const char *gl_version;
	ImGui_ImplOpenGL3_Data *bd;

	renderImport = *import;
	imguiShader = (GLuint)(uintptr_t)shaderData;

	ImGuiIO &io = ImGui::GetIO();
	IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

	// Setup backend capabilities flags
	bd = (ImGui_ImplOpenGL3_Data *)Hunk_Alloc( sizeof(*bd), h_low );
	memset(bd, 0, sizeof(*bd));
	io.BackendRendererUserData = (void *)bd;
	io.BackendRendererName = "imgui_impl_opengl3";

	r_gamma = Cvar_Get( "r_gammaAmount", "1.0", CVAR_SAVE );

	bd->GlDirectStateAccess = strstr( gi.gpuConfig.extensions_string, "ARB_direct_state_access" ) != NULL;

	// Query for GL version (e.g. 320 for GL 3.2)
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GLES 2
	bd->GlVersion = 200;
	bd->GlProfileIsES2 = true;
#else
	// Desktop or GLES 3
	major = 0;
	minor = 0;
	renderImport.glGetIntegerv(GL_MAJOR_VERSION, &major);
	renderImport.glGetIntegerv(GL_MINOR_VERSION, &minor);
	if (major == 0 && minor == 0)
	{
		// Query GL_VERSION in desktop GL 2.x, the string will start with "<major>.<minor>"
		gl_version = (const char *)renderImport.glGetString(GL_VERSION);
		sscanf(gl_version, "%d.%d", &major, &minor);
	}
	bd->GlVersion = (GLuint)(major * 100 + minor * 10);
#if defined(GL_CONTEXT_PROFILE_MASK)
	if (bd->GlVersion >= 320)
	{
		renderImport.glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &bd->GlProfileMask);
	}
	bd->GlProfileIsCompat = (bd->GlProfileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) != 0;
#endif

#if defined(IMGUI_IMPL_OPENGL_ES3)
	bd->GlProfileIsES3 = true;
#endif

	bd->UseBufferSubData = false;
	/*
	// Query vendor to enable glBufferSubData kludge
#ifdef _WIN32
	if (const char* vendor = (const char*)glGetString(GL_VENDOR))
		if (strncmp(vendor, "Intel", 5) == 0)
			bd->UseBufferSubData = true;
#endif
	*/
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
	if (bd->GlVersion >= 320)
	{
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	}
#endif

	// Store GLSL version string so we can refer to it later in case we recreate shaders.
	// Note: GLSL version is NOT the same as GL version. Leave this to nullptr if unsure.
	if (glsl_version == nullptr)
	{
#if defined(IMGUI_IMPL_OPENGL_ES2)
		glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
		glsl_version = "#version 300 es";
#elif defined(__APPLE__)
		glsl_version = "#version 150";
#else
		glsl_version = "#version 130";
#endif
	}
	IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(bd->GlslVersionString));
	strcpy(bd->GlslVersionString, glsl_version);
	strcat(bd->GlslVersionString, "\n");

	// Make an arbitrary GL call (we don't actually need the result)
	// IF YOU GET A CRASH HERE: it probably means the OpenGL function loader didn't do its job. Let us know!
	renderImport.glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

	// Detect extensions we support
	bd->HasClipOrigin = (bd->GlVersion >= 450);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_EXTENSIONS
	GLint num_extensions = 0;
	renderImport.glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	for (GLint i = 0; i < num_extensions; i++)
	{
		const char *extension = (const char *)renderImport.glGetStringi(GL_EXTENSIONS, i);
		if (extension != nullptr && strcmp(extension, "GL_ARB_clip_control") == 0)
			bd->HasClipOrigin = true;
	}
#endif
//    ImGui_ImplOpenGL3_CreateFontsTexture();
	ImGui_ImplOpenGL3_CreateDeviceObjects();
}

void ImGui_ImplOpenGL3_Shutdown(void)
{
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();

	if (!bd) {
		return;
	}

	ImGui_ImplOpenGL3_DestroyDeviceObjects();
	ImGui_ImplOpenGL3_DestroyFontsTexture();
}

void ImGui_ImplOpenGL3_NewFrame(void) {
}

static void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData *draw_data, int fb_width, int fb_height, GLuint vertex_array_object)
{
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	renderImport.glEnable(GL_BLEND);
	renderImport.glBlendEquation(GL_FUNC_ADD);
	renderImport.glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	renderImport.glDisable(GL_DEPTH_TEST);
	renderImport.glDisable(GL_STENCIL_TEST);
	renderImport.glEnable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
	if (bd->GlVersion >= 310)
		renderImport.glDisable(GL_PRIMITIVE_RESTART);
#endif
#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
	renderImport.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

	// Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
#if defined(GL_CLIP_ORIGIN)
	int clip_origin_lower_left = true;
	if (bd->HasClipOrigin)
	{
		GLint current_clip_origin = 0;
		renderImport.glGetIntegerv(GL_CLIP_ORIGIN, &current_clip_origin);
		if (current_clip_origin == GL_UPPER_LEFT)
		{
			clip_origin_lower_left = false;
		}
	}
#endif

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	float L = 0;
	float R = draw_data->DisplaySize.x;
	float T = 0;
	float B = draw_data->DisplaySize.y;
#if defined(GL_CLIP_ORIGIN)
	if (!clip_origin_lower_left)
	{
		// Swap top and bottom if origin is upper left
		float tmp = T;
		T = B;
		B = tmp;
	}
#endif
	glm::mat4 ortho = glm::ortho( L, R, B, T, -1.0f, 1.0f );
	renderImport.glUseProgram( imguiShader );
	if ( r_gamma->modified ) {
		renderImport.glUniform1f( bd->AttribLocationGamma, r_gamma->f );
	}
	renderImport.glUniformMatrix4fv( bd->AttribLocationProjMtx, 1, GL_FALSE, &ortho[0][0] );

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
	if ( bd->GlVersion >= 330 || bd->GlProfileIsES3 ) {
		renderImport.glBindSampler( 0, 0 ); // We use combined texture/sampler state. Applications using GL 3.3 and GL ES 3.0 may set that otherwise.
	}
#endif
	
	renderImport.glBindVertexArray( bd->vaoId );
	if ( !bd->GlDirectStateAccess ) {
		// Bind vertex/index buffers and setup attributes for ImDrawVert
		renderImport.glBindBuffer( GL_ARRAY_BUFFER, bd->VboHandle );
		renderImport.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle );
	}
}

// OpenGL3 Render function.
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly.
// This is in order to be able to run within an OpenGL engine that doesn't do so.
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData *draw_data)
{
	GLint last_element_array_buffer, last_array_buffer;
	GLint last_vertex_array_object;
	GLint last_sampler, last_program;
	GLint last_active_texture, last_texture;
	GLint last_polygon_mode[2];
	GLint last_viewport[4];
	GLint last_scissor_box[4];
	GLint last_blend_src_rgb, last_blend_src_alpha;
	GLint last_blend_dst_rgb, last_blend_dst_alpha;
	GLint last_blend_equation_alpha, last_blend_equation_rgb;
	GLint last_enable_blend, last_enable_cull_face;
	GLint last_enable_depth_test, last_enable_stencil_test;
	GLint last_enable_scissor_test;
	GLint last_enable_primitive_restart;
	GLuint vertex_array_object = 0;
	ImVec2 clip_off, clip_scale;
	int fb_width, fb_height;
	ImGui_ImplOpenGL3_Data *bd;

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
	{
		return;
	}

	bd = ImGui_ImplOpenGL3_GetBackendData();

	// Backup GL state
	renderImport.glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
	renderImport.glActiveTexture(GL_TEXTURE0);
	renderImport.glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	renderImport.glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
	if (bd->GlVersion >= 330 || bd->GlProfileIsES3) {
		renderImport.glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
	}
	else {
		last_sampler = 0;
	}
#endif
	renderImport.glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	if ( !bd->GlDirectStateAccess ) {
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		// This is part of VAO on OpenGL 3.0+ and OpenGL ES 3.0+.
		renderImport.glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_pos;
		last_vtx_attrib_state_pos.GetState(bd->AttribLocationVtxPos);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_uv;
		last_vtx_attrib_state_uv.GetState(bd->AttribLocationVtxUV);
		ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_color;
		last_vtx_attrib_state_color.GetState(bd->AttribLocationVtxColor);
#endif
	}
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
	renderImport.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object);
#endif
#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
	renderImport.glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
	renderImport.glGetIntegerv(GL_VIEWPORT, last_viewport);
	renderImport.glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	renderImport.glGetIntegerv(GL_BLEND_SRC_RGB, &last_blend_src_rgb);
	renderImport.glGetIntegerv(GL_BLEND_DST_RGB, &last_blend_dst_rgb);
	renderImport.glGetIntegerv(GL_BLEND_SRC_ALPHA, &last_blend_src_alpha);
	renderImport.glGetIntegerv(GL_BLEND_DST_ALPHA, &last_blend_dst_alpha);
	renderImport.glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
	renderImport.glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
	last_enable_blend = renderImport.glIsEnabled(GL_BLEND);
	last_enable_cull_face = renderImport.glIsEnabled(GL_CULL_FACE);
	last_enable_depth_test = renderImport.glIsEnabled(GL_DEPTH_TEST);
	last_enable_stencil_test = renderImport.glIsEnabled(GL_STENCIL_TEST);
	last_enable_scissor_test = renderImport.glIsEnabled(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
	last_enable_primitive_restart = (bd->GlVersion >= 310) ? renderImport.glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

	PROFILE_FUNCTION();

	ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
	renderImport.glEnable(GL_ALPHA_TEST);
	renderImport.glAlphaFunc( GL_ALWAYS, 0.5f );

	// Will project scissor/clipping rectangles into framebuffer space
	clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for ( int n = 0; n < draw_data->CmdListsCount; n++ ) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];

		// Upload vertex/index buffers
		// - OpenGL drivers are in a very sorry state nowadays....
		//   During 2021 we attempted to switch from glBufferData() to orphaning+glBufferSubData() following reports
		//   of leaks on Intel GPU when using multi-viewports on Windows.
		// - After this we kept hearing of various display corruptions issues. We started disabling on non-Intel GPU, but issues still got reported on Intel.
		// - We are now back to using exclusively glBufferData(). So bd->UseBufferSubData IS ALWAYS FALSE in this code.
		//   We are keeping the old code path for a while in case people finding new issues may want to test the bd->UseBufferSubData path.
		// - See https://github.com/ocornut/imgui/issues/4468 and please report any corruption issues.
		const GLsizeiptr vtx_buffer_size = (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof( ImDrawVert );
		const GLsizeiptr idx_buffer_size = (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof( ImDrawIdx );

		if ( vtx_buffer_size > bd->VertexBufferSize ) {
			bd->VertexBufferSize = vtx_buffer_size;
			if ( bd->GlDirectStateAccess ) {
				renderImport.glNamedBufferData( bd->VboHandle, bd->VertexBufferSize, NULL, GL_STREAM_DRAW );
			} else {
				renderImport.glBufferData( GL_ARRAY_BUFFER, bd->VertexBufferSize, NULL, GL_STREAM_DRAW );
			}
		}
		if ( idx_buffer_size > bd->IndexBufferSize ) {
			bd->IndexBufferSize = idx_buffer_size;
			if ( bd->GlDirectStateAccess ) {
				renderImport.glNamedBufferData( bd->ElementsHandle, bd->IndexBufferSize, NULL, GL_STREAM_DRAW );
			} else {
				renderImport.glBufferData( GL_ELEMENT_ARRAY_BUFFER, bd->IndexBufferSize, NULL, GL_STREAM_DRAW );
			}
		}
		{
			PROFILE_BLOCK_BEGIN( "SwapBuffer GLBuffer" );
			if ( vtx_buffer_size > 0 ) {
				if ( bd->GlDirectStateAccess ) {
					renderImport.glNamedBufferSubData( bd->VboHandle, 0, vtx_buffer_size, cmd_list->VtxBuffer.Data );
				} else {
					renderImport.glBufferSubData( GL_ARRAY_BUFFER, 0, vtx_buffer_size, cmd_list->VtxBuffer.Data );
				}
			}
			if ( idx_buffer_size > 0 ) {
				if ( bd->GlDirectStateAccess ) {
					renderImport.glNamedBufferSubData( bd->ElementsHandle, 0, idx_buffer_size, cmd_list->IdxBuffer.Data );
				} else {
					renderImport.glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, idx_buffer_size, cmd_list->IdxBuffer.Data );
				}
			}
			PROFILE_BLOCK_END;
		}

		for ( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ ) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != nullptr)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				const ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
				const ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
				if ( clip_max.x <= clip_min.x || clip_max.y <= clip_min.y ) {
					continue;
				}

				// Apply scissor/clipping rectangle (Y is inverted in OpenGL)
//				renderImport.glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y));

				// Bind texture, Draw

//                renderImport.glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)( (shader_t *)pcmd->GetTexID() )->index );
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
				if (bd->GlVersion >= 320) {
					renderImport.DrawShaderStages( (nhandle_t)(intptr_t)pcmd->GetTexID(), pcmd->ElemCount,
						sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
						(GLint)pcmd->VtxOffset  );
					//renderImport.glDrawElementsBaseVertex( GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
					//    sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
					//    (GLint)pcmd->VtxOffset );
				} else
#endif
				{
					renderImport.DrawShaderStages( (nhandle_t)(intptr_t)pcmd->GetTexID(), pcmd->ElemCount,
						sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
						-1 );
					//renderImport.glDrawElements( GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
					//    sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void *)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx) ) );
				}
			}
		}
	}
	renderImport.glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	renderImport.glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);

	// Restore modified GL state
	// This "glIsProgram()" check is required because if the program is "pending deletion" at the time of binding backup, it will have been deleted by now and will cause an OpenGL error. See #6220.
	if (last_program == 0 || renderImport.glIsProgram(last_program)) {
		renderImport.glUseProgram(last_program);
	}

	renderImport.glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
	if (bd->GlVersion >= 330 || bd->GlProfileIsES3) {
		renderImport.glBindSampler(0, last_sampler);
	}
#endif
	renderImport.glActiveTexture(last_active_texture);

#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
	renderImport.glBindVertexArray(last_vertex_array_object);
#endif

	if ( !bd->GlDirectStateAccess ) {
		renderImport.glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		renderImport.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
		last_vtx_attrib_state_pos.SetState(bd->AttribLocationVtxPos);
		last_vtx_attrib_state_uv.SetState(bd->AttribLocationVtxUV);
		last_vtx_attrib_state_color.SetState(bd->AttribLocationVtxColor);
#endif
	}
	if (last_enable_blend) {
		renderImport.glEnable(GL_BLEND);
	} else {
		renderImport.glDisable(GL_BLEND);
	}

	if (last_enable_cull_face) {
		renderImport.glEnable(GL_CULL_FACE);
	} else {
		renderImport.glDisable(GL_CULL_FACE);
	}

	if (last_enable_depth_test) {
		renderImport.glEnable(GL_DEPTH_TEST);
	} else {
		renderImport.glDisable(GL_DEPTH_TEST);
	}

	if (last_enable_stencil_test) {
		renderImport.glEnable(GL_STENCIL_TEST);
	} else {
		renderImport.glDisable(GL_STENCIL_TEST);
	}

	if (last_enable_scissor_test) {
		renderImport.glEnable(GL_SCISSOR_TEST);
	} else {
		renderImport.glDisable(GL_SCISSOR_TEST);
	}

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
	if (bd->GlVersion >= 310) {
		if (last_enable_primitive_restart) {
			renderImport.glEnable(GL_PRIMITIVE_RESTART);
		}
		else {
			renderImport.glDisable(GL_PRIMITIVE_RESTART);
		}
	}
#endif

#ifdef IMGUI_IMPL_HAS_POLYGON_MODE
	// Desktop OpenGL 3.0 and OpenGL 3.1 had separate polygon draw modes for front-facing and back-facing faces of polygons
	if (bd->GlVersion <= 310 || bd->GlProfileIsCompat) {
		renderImport.glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]);
		renderImport.glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
	}
	else {
		renderImport.glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
	}
#endif // IMGUI_IMPL_HAS_POLYGON_MODE

	renderImport.glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	renderImport.glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	(void)bd; // Not all compilation paths use this
}

int ImGui_ImplOpenGL3_CreateFontsTexture( void )
{
	unsigned char *pixels;
	int width, height;
	GLint last_texture;
	textureFilter_t filter;
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();
	char *buf;
	shader_t *shader;
	texture_t *fontsTex;

	// Build texture atlas
	io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height ); // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

	if ( bd->FontTexture ) {
		renderImport.glDeleteTextures( 1, &bd->FontTexture );
	}
	
	shader = (shader_t *)renderImport.GetShaderByHandle( re.RegisterShader( "gfx/fonts" ) );
	fontsTex = shader->stages[0]->bundle[0].image[0];

	if ( Cvar_VariableInteger( "r_loadTexturesOnDemand" ) ) {
		renderImport.glGenTextures( 1, &fontsTex->id );
	}

	bd->FontTexture = fontsTex->id;

	renderImport.glBindTexture( GL_TEXTURE_2D, bd->FontTexture );
	renderImport.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	renderImport.glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
#ifdef GL_UNPACK_ROW_LENGTH // Not on WebGL/ES
	renderImport.glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
#endif
	renderImport.glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );

	fontsTex->data = pixels;
	fontsTex->picFormat = GL_RGBA;
	fontsTex->internalFormat = GL_RGBA8;
	fontsTex->evicted = qtrue;
	fontsTex->width = width;
	fontsTex->height = height;
	fontsTex->uploadWidth = width;
	fontsTex->uploadHeight = height;
	if ( strstr( gi.gpuConfig.extensions_string, "ARB_bindless_texture" ) ) {
		fontsTex->handle = renderImport.glGetTextureHandleARB( fontsTex->id );
		renderImport.glMakeTextureHandleResidentARB( fontsTex->handle );
	}

	// Store our identifier
	io.Fonts->SetTexID( (ImTextureID)(intptr_t)shader->index );

	return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture( void )
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();
	if (bd->FontTexture)
	{
		renderImport.glDeleteTextures( 1, &bd->FontTexture );
		io.Fonts->SetTexID( 0 );
		bd->FontTexture = 0;
	}
}

int ImGui_ImplOpenGL3_CreateDeviceObjects( void )
{
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();
	GLint last_texture, last_array_buffer;
	GLint last_vertex_array;

	// Backup GL state
	renderImport.glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	renderImport.glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
	renderImport.glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

	bd->AttribLocationTex = renderImport.glGetUniformLocation( imguiShader, "u_DiffuseMap" );
	bd->AttribLocationProjMtx = renderImport.glGetUniformLocation( imguiShader, "u_ModelViewProjection" );
	bd->AttribLocationGamma = renderImport.glGetUniformLocation( imguiShader, "u_GammaAmount" );
	bd->AttribLocationVtxPos = (GLuint)renderImport.glGetAttribLocation( imguiShader, "a_Position" );
	bd->AttribLocationVtxUV = (GLuint)renderImport.glGetAttribLocation( imguiShader, "a_TexCoords" );
	bd->AttribLocationVtxColor = (GLuint)renderImport.glGetAttribLocation( imguiShader, "a_Color" );

	if ( bd->GlDirectStateAccess ) {
		renderImport.glCreateVertexArrays( 1, &bd->vaoId );

		renderImport.glCreateBuffers( 1, &bd->VboHandle );
		renderImport.glCreateBuffers( 1, &bd->ElementsHandle );

		renderImport.glNamedBufferStorage( bd->VboHandle, DEFAULT_VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );
		bd->VertexBufferSize = DEFAULT_VERTEX_BUFFER_SIZE;
		bd->VertexBufferOffset = 0;

		renderImport.glNamedBufferStorage( bd->ElementsHandle, DEFAULT_INDEX_BUFFER_SIZE, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );
		bd->IndexBufferSize = DEFAULT_INDEX_BUFFER_SIZE;
		bd->IndexBufferOffset = 0;

		renderImport.glEnableVertexArrayAttrib( bd->vaoId, 0 );
		renderImport.glVertexArrayAttribFormat( bd->vaoId, 0, 3, GL_FLOAT, GL_FALSE, offsetof( ImDrawVert, pos ) );
		renderImport.glVertexArrayAttribBinding( bd->vaoId, 0, 0 );

		renderImport.glEnableVertexArrayAttrib( bd->vaoId, 1 );
		renderImport.glVertexArrayAttribFormat( bd->vaoId, 1, 2, GL_FLOAT, GL_FALSE, offsetof( ImDrawVert, uv ) );
		renderImport.glVertexArrayAttribBinding( bd->vaoId, 1, 0 );

		renderImport.glEnableVertexArrayAttrib( bd->vaoId, 2 );
		renderImport.glVertexArrayAttribFormat( bd->vaoId, 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof( ImDrawVert, col ) );
		renderImport.glVertexArrayAttribBinding( bd->vaoId, 2, 0 );

		renderImport.glVertexArrayVertexBuffer( bd->vaoId, 0, bd->VboHandle, 0, sizeof( drawVert_t ) );
		renderImport.glVertexArrayElementBuffer( bd->vaoId, bd->ElementsHandle );
	}
	else {
		renderImport.glGenVertexArrays( 1, &bd->vaoId );
		renderImport.glBindVertexArray( bd->vaoId );

		// Create buffers
		renderImport.glGenBuffers( 1, &bd->VboHandle );
		renderImport.glGenBuffers( 1, &bd->ElementsHandle );

		renderImport.glBindBuffer( GL_ARRAY_BUFFER, bd->VboHandle );
		renderImport.glBufferStorage( GL_ARRAY_BUFFER, DEFAULT_VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
		//renderImport.glBufferData( GL_ARRAY_BUFFER, DEFAULT_VERTEX_BUFFER_SIZE, NULL, GL_STREAM_DRAW );
		bd->VertexBufferSize = DEFAULT_VERTEX_BUFFER_SIZE;
		bd->VertexBuffer = renderImport.glMapBufferRange( GL_ARRAY_BUFFER, 0, DEFAULT_VERTEX_BUFFER_SIZE, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
			GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

		renderImport.glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle );
		renderImport.glBufferStorage( GL_ELEMENT_ARRAY_BUFFER, DEFAULT_INDEX_BUFFER_SIZE, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
		//renderImport.glBufferData( GL_ELEMENT_ARRAY_BUFFER, DEFAULT_INDEX_BUFFER_SIZE, NULL, GL_STREAM_DRAW );
		bd->IndexBufferSize = DEFAULT_INDEX_BUFFER_SIZE;
		bd->IndexBuffer = renderImport.glMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, DEFAULT_INDEX_BUFFER_SIZE, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT |
			GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT );

		renderImport.glEnableVertexAttribArray( 0 );
		renderImport.glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( ImDrawVert ), (const void *)offsetof( ImDrawVert, pos ) );
		renderImport.glEnableVertexAttribArray( 1 );
		renderImport.glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( ImDrawVert ), (const void *)offsetof( ImDrawVert, uv ) );
		renderImport.glEnableVertexAttribArray( 2 );
		renderImport.glVertexAttribPointer( 2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( ImDrawVert ), (const void *)offsetof( ImDrawVert, col ) );
	
		// Restore modified GL state
		renderImport.glBindTexture(GL_TEXTURE_2D, last_texture);
		renderImport.glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
		renderImport.glBindVertexArray(last_vertex_array);
	#endif
	}

	ImGui_ImplOpenGL3_CreateFontsTexture();

	return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects(void)
{
	ImGui_ImplOpenGL3_Data *bd = ImGui_ImplOpenGL3_GetBackendData();
	if ( bd->VboHandle ) {
		renderImport.glDeleteBuffers( 1, &bd->VboHandle );
		bd->VboHandle = 0;
	}
	if ( bd->ElementsHandle ) {
		renderImport.glDeleteBuffers( 1, &bd->ElementsHandle );
		bd->ElementsHandle = 0;
	}
	if ( bd->vaoId ) {
		renderImport.glDeleteVertexArrays( 1, &bd->vaoId );
		bd->vaoId = 0;
	}
	ImGui_ImplOpenGL3_DestroyFontsTexture();
}

//-----------------------------------------------------------------------------

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE