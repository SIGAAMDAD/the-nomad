#include "rgl_local.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "code/rendercommon/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "code/rendercommon/imstb_truetype.h"
#define STB_SPRINTF_IMPLEMENTATION
#include "code/game/stb_sprintf.h"

#ifdef _NOMAD_DEBUG
static void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);
#endif

void GDR_DECL N_Error(errorCode_t errCode, const char *fmt, ...)
{
    va_list argptr;
    char buf[4096];

    va_start(argptr, fmt);
    N_vsnprintf(buf, sizeof(buf), fmt, argptr);
    va_end(argptr);

    ri.Error(errCode, "%s", buf);
}
void GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, fmt);
    N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    ri.Printf(PRINT_INFO, "%s", msg);
}

GDR_EXPORT void RE_BeginRegisteration(gpuConfig_t *configOut)
{
    R_Init();

    *configOut = glConfig;

    RE_ClearScene();

    rg.registered = qtrue;
}

void GL_CameraZoomIn(void)
{
    if (rg.viewData.camera.zoom < 0.1f)
        return;
    
    rg.viewData.camera.zoom -= 0.05f;
    RB_MakeModelViewProjection();
}

void GL_CameraZoomOut(void)
{
    if (rg.viewData.camera.zoom > 15.0f)
        return;
    
    rg.viewData.camera.zoom += 0.05f;
    RB_MakeModelViewProjection();
}

void RB_RotateRight(void)
{
    rg.viewData.camera.angle += 0.75f;
    if (rg.viewData.camera.angle > 180.0f) {
        rg.viewData.camera.angle -= 360.0f;
    }
    else if (rg.viewData.camera.angle <= -180.0f) {
        rg.viewData.camera.angle += 360.0f;
    }
}

void RB_RotateLeft(void)
{
    rg.viewData.camera.angle -= 0.75f;
    if (rg.viewData.camera.angle > 180.0f) {
        rg.viewData.camera.angle -= 360.0f;
    }
    else if (rg.viewData.camera.angle <= -180.0f) {
        rg.viewData.camera.angle += 360.0f;
    }
}

void RB_MakeModelViewProjection(void)
{
    mat4_t transform, temp;
	vec3_t rotate;

    // adjust for camera resizing
    GL_CameraResize();

    // calculate the view matrix
    rotate[0] = 0;
	rotate[1] = 0;
	rotate[2] = 1;

    rg.viewData.camera.angle = 0.0f;

    VectorClear(rg.viewData.camera.origin);
    Mat4Zero(temp);
	Mat4Translation(rg.viewData.camera.origin, transform);
	Mat4Rotate(rotate, (float)DEG2RAD(rg.viewData.camera.angle), transform, transform);
	Mat4Scale(rg.viewData.camera.zoom, transform, transform);
    Mat4SimpleInverse(transform, temp);

    // finish it up
    GL_SetModelViewMatrix(temp);
}

void GL_CameraResize(void)
{
    float aspect, zoom;
    mat4_t matrix;

    rg.viewData.camera.zoom = 1.0f;

    aspect = rg.viewData.camera.aspect = glConfig.vidWidth / glConfig.vidHeight;
    zoom = rg.viewData.camera.zoom;

    Mat4Ortho(-aspect * zoom, aspect * zoom, -aspect, aspect, -1.0f, 1.0f, matrix);
    GL_SetProjectionMatrix(glState.projection);
}

void GL_CameraMove(dirtype_t dir)
{
    switch (dir) {
    case D_NORTH:
        rg.viewData.camera.origin[0] += -sin(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        rg.viewData.camera.origin[1] += cos(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        break;
    case D_EAST:
        rg.viewData.camera.origin[0] += cos(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        rg.viewData.camera.origin[1] += sin(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        break;
    case D_SOUTH:
        rg.viewData.camera.origin[0] -= -sin(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        rg.viewData.camera.origin[1] -= cos(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        break;
    case D_WEST:
        rg.viewData.camera.origin[0] -= cos(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        rg.viewData.camera.origin[1] -= sin(DEG2RAD(rg.viewData.camera.angle)) * 0.25f;
        break;
    };
}

void R_AddDrawSurf(surfaceType_t *surface, shader_t *shader)
{
    uint32_t index;

    // instead of check for overflow, we just mask the index
    // so it wraps around
    index = backendData->numDrawSurfs & DRAWSURF_MASK;
    // the sort data is packed into a single 32 bit value so it can be
    // compared quickly during the qsorting process
    backendData->drawSurfs[index].sort = shader->index + *surface;
    backendData->drawSurfs[index].surface = surface;
    backendData->numDrawSurfs++;
}

/*
===============
R_Radix
===============
*/
static GDR_INLINE void R_Radix( int byte, uint32_t size, const drawSurf_t *source, drawSurf_t *dest )
{
    uint32_t       count[ 256 ] = { 0 };
    uint32_t       index[ 256 ];
    uint32_t       i;
    unsigned char *sortKey;
    unsigned char *end;

    sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
    end = sortKey + ( size * sizeof( drawSurf_t ) );
    for ( ; sortKey < end; sortKey += sizeof( drawSurf_t ) )
        ++count[ *sortKey ];

    index[ 0 ] = 0;

    for ( i = 1; i < 256; ++i )
      index[ i ] = index[ i - 1 ] + count[ i - 1 ];

    sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
    for ( i = 0; i < size; ++i, sortKey += sizeof( drawSurf_t ) )
        dest[ index[ *sortKey ]++ ] = source[ i ];
}


/*
===============
R_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void R_RadixSort( drawSurf_t *source, uint32_t size )
{
  static drawSurf_t scratch[ MAX_DRAWSURFS ];
#ifdef GDR_LITTLE_ENDIAN
  R_Radix( 0, size, source, scratch );
  R_Radix( 1, size, scratch, source );
  R_Radix( 2, size, source, scratch );
  R_Radix( 3, size, scratch, source );
#else
  R_Radix( 3, size, source, scratch );
  R_Radix( 2, size, scratch, source );
  R_Radix( 1, size, source, scratch );
  R_Radix( 0, size, scratch, source );
#endif //Q3_LITTLE_ENDIAN
}

void R_SortDrawSurfs(drawSurf_t *drawSurfs, uint32_t numDrawSurfs)
{
    // sort the drawsurfs by texture/shader index
    R_RadixSort(drawSurfs, numDrawSurfs);
}

uint32_t R_GenDrawSurfSort(const shader_t *sh)
{
    uint32_t sort = sh->sort;
    sort += sh->sortedIndex << 2;
    return sort;
}

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source)
{
    switch (source) {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION: return "Application User";
    case GL_DEBUG_SOURCE_OTHER: return "Other";
    };
    return "Unknown Source";
}

const char *DBG_GL_TypeToStr(GLenum type)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
    case GL_DEBUG_TYPE_MARKER: return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "Debug Push group";
    case GL_DEBUG_TYPE_POP_GROUP: return "Debug Pop Group";
    case GL_DEBUG_TYPE_OTHER: return "Other";
    };
    return "Unknown Type";
}

const char *DBG_GL_SeverityToStr(GLenum severity)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: return "High";
    case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
    case GL_DEBUG_SEVERITY_LOW: return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
    };
    return "Unknown Severity";
}

static void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
    // nothing massive or useless
    if (length >= 300 || severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        ri.Error(ERR_DROP, "[OpenGL Error: %i] %s", id, message);
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: [OpenGL Debug Log] %s Deprecated Behaviour, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: [OpenGL Debug Log] %s Portability, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: [OpenGL Debug Log] %s Undefined Behaviour, Id: %i", message, id);
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        ri.Printf(PRINT_INFO, "[OpenGL Debug Log (Performance)] %s Id: %i", message, id);
        break;
    default:
        ri.Printf(PRINT_INFO, "[OpenGL Debug Log] %s Id: %i", message, id);
        break;
    };
}

#endif