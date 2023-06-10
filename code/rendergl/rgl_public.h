#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

// ardous if not here
#include "../src/n_shared.h"
#include "../src/g_bff.h"

#pragma once

typedef struct
{
    void (*GDR_DECL Printf)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
    void (*GDR_DECL LPrintf)(loglevel_t level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void (*GDR_DECL Error)(const char *err, ...) __attribute__((noreturn)) __attribute__((format(printf, 1, 2)));

    void *(*Hunk_Alloc)(uint64_t size, const char *name, int where);
    void *(*Hunk_TempAlloc)(uint64_t size);
    void *(*Mem_Alloc)(uint32_t size);
    void (*Mem_Free)(void *ptr);
    void *(*Malloc)(uint32_t size);
    void *(*Realloc)(void *p, uint32_t nsize);
    void (*Free)(void *ptr);
    void *(*Z_Malloc)(uint32_t size, int tag, void *user, const char *name);
    void *(*Z_Calloc)(void *user, uint32_t size, int tag, const char *name);
    void *(*Z_Realloc)(void *ptr, uint32_t nsize, void *user, int tag, const char *name);
    void (*Z_Free)(void *ptr);
    void (*Z_ChangeTag)(void *user, int tag);
    void (*Z_ChangeUser)(void *ptr, void *user);
    void (*Z_FreeTags)(int lowtag, int hightag);

    void *(*Sys_LoadLibrary)(const char *libname);
    void *(*Sys_LoadProc)(void *handle, const char *name);
    void (*Sys_FreeLibrary)(void *handle);

    uint64_t (*Com_GenerateHashValue)(const char *fname, const uint32_t size);

    void (*Cvar_Register)(cvar_t *cvar);
    void (*Cvar_RegisterName)(const char *name, const char *value, cvartype_t type, qboolean save);
    void (*Cvar_ChangeValue)(const char *name, const char *value);
    cvar_t *(*Cvar_Find)(const char *name);

    void (*Cmd_AddCommand)(const char *name, cmdfunc_t func);
    void (*Cmd_RemoveCommand)(const char *name);

    bffinfo_t *(*BFF_FetchInfo)(void);

    uint64_t (*N_LoadFile)(const char *filepath, void **buffer);
} renderImport_t;

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texcoords;
    glm::vec4 color;
};

struct Quad
{
    Vertex vertices[4];
    glm::vec4 color;
    qboolean use_color;
};

extern renderImport_t ri;

/*
rendergl exported functions go here
*/

GDR_EXPORT void RE_Init(renderImport_t *imports);
GDR_EXPORT void RE_Shutdown(void);

#endif