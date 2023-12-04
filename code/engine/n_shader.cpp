#include "n_shared.h"
#include "g_bff.h"

typedef struct
{
    uint32_t minFilter;
    uint32_t magFilter;
    uint32_t wrapS;
    uint32_t wrapT;
} textureShader_t;

typedef struct
{

} soundShader_t;

typedef union
{
    textureShader_t *tex;
    soundShader_t *snd;
} shaderData_t;

typedef struct
{
    char name[MAX_GDR_PATH];
    shaderData_t data;
    shaderType_t type;
} shader_t;

typedef struct
{
    char name[MAX_GDR_PATH];
    uint32_t numShaders;
    shader_t *cache;
} shaderfile_t;

static shaderfile_t *shadercache;

static void S_ParseSfx(const char **text, shaderfile_t *shader)
{
    soundshader_t *parms = &shader->parms.snd;
    const char *tok;

    while (1) {
        tok = COM_ParseExt(text, qfalse);
        
        // end of shader definition
        if (tok[0] == '}') {
            break;
        }

        if (N_strcasecmp(tok, "chunk")) {

        }
    }
}

void I_CacheShaders(bffinfo_t *info)
{
    const char **text;
    char *tok;
    
    shadercache = (shaderfile_t *)Hunk_Alloc(sizeof(shaderfile_t) * info->numShaders, "shaderFiles", h_low);
    for (uint32_t i = 0; i < info->numShaders; i++) {
        text = &info->shaders[i].fileBuffer;
        
        N_strncpy(shadercache[i].name, info->shaders[i].name, MAX_BFF_CHUNKNAME);
        
        while (1) {
            tok = COM_ParseComplex(text, qfalse);
            
            // end of shader defintions
            if (com_tokentype == TK_EOF) {
                break;
            }
            if (N_strcasecmp(tok, "sfx")) {
                S_ParseSfx(text, &shadercache[i]);
            }
            else if (N_strcasecmp(tok, "music")) {
                S_ParseMusic(text, &shadercache[i]);
            }
        }
    }
}