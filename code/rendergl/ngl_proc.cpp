#include "rgl_local.h"

#define NGL( ret, name, ... ) PFN##name n##name = NULL;
NGL_Core_Procs
NGL_Debug_Procs
NGL_Shader_Procs
NGL_FBO_Procs
NGL_VAO_Procs
NGL_Buffer_Procs
NGL_BufferARB_Procs
NGL_Texture_Procs
#undef NGL

// to prevent hardcoding the gl proc loading
#define NGL( ret, name, ... )\
{ \
    n##name = (PFN##name)load(#name); \
    if (!n##name) ri.N_Error("RE_Init: failed to load gl proc %s", #name); \
}

void load_gl_procs(NGLloadproc load)
{
    NGL_Core_Procs
    NGL_Debug_Procs
    NGL_Shader_Procs
    NGL_FBO_Procs
    NGL_VAO_Procs
    NGL_Buffer_Procs
    NGL_Texture_Procs
    // don't load any extension procs here
}
#undef NGL