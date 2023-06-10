#include "rgl_public.h"
#include "rgl_local.h"

shader_t* R_CreateShader(const char *filepath)
{
    shader_t *shader;
    char *filebuf;

    if (!filepath)
        ri.Error("R_CreateShader: NULL filepath");
    if (!*filepath)
        ri.Error("R_CreateShader: empty filepath");

    shader = (shader_t *)ri.Hunk_Alloc(sizeof(shader_t), "scache", h_low);
    memset(shader->uniformCache, 0, sizeof(shader->uniformCache));

    ri.N_LoadFile(filepath, (void **)&filebuf);

    shader->id = nglGenProgramObjectARB();
    nglUseProgramObjectARB(shader->id);

    nglUseProgramObjectARB(0);
}