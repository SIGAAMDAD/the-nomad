#include "n_shared.h"
#include "g_bff.h"


typedef struct
{
    bffsound_t *chunk;
    nomadsnd_t *source;

    float maxvolume;
    float minvolume;

    uint32_t audiotype; // TAG_SFX or TAG_MUSIC
    qboolean hassource;
    qboolean global;
} soundshader_t;

typedef union
{
    soundshader_t snd;
    textureshader_t tex;
} shaderparms_t;

typedef enum : uint32_t
{
    SHADER_SFX,
    SHADER_MUSIC,
    SHADER_TEXTURE,
    SHADER_BOT
} shadertype_t;

typedef struct
{
    char name[256];
    shaderparms_t parms;
    shadertype_t type;
} shader_t;

typedef struct
{
    char name[MAX_BFF_CHUNKNAME];
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