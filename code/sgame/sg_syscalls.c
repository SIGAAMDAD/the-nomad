
// sg_syscalls.c: this file is only included in development dll builds, the .asm file is used instead when building the VM file
#include "../rendercommon/r_public.h"
#include "sg_local.h"

#ifdef Q3_VM
    #error Never use is VM build
#endif

static int (GDR_DECL *syscall)(int arg, ...) = (int (GDR_DECL *)(int, ...)) - 1;

void dllEntry(int (GDR_DECL *syscallptr)(int arg, ...))
{
    syscall = syscallptr;
}

int PASSFLOAT(float x)
{
    float floatTemp;
    floatTemp = x;
    return *(int *)&floatTemp;
}

void trap_Print(const char *fmt)
{
    syscall(SG_PRINT, fmt);
}

void trap_Error(const char *fmt)
{
    syscall(SG_ERROR, fmt);
}

void trap_Cvar_Update(vmCvar_t *vmCvar)
{
    syscall(SG_CVAR_UPDATE, vmCvar);
}

void trap_Cvar_Set(const char *var_name, const char *value)
{
    syscall(SG_CVAR_SET, var_name, value);
}

void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, unsigned int bufLen)
{
    syscall(SG_CVAR_VARIABLESTRINBUFFER, var_name, buffer, bufLen);
}

void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, unsigned int flags)
{
    syscall(SG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

int trap_FS_FOpenWrite(const char *npath, file_t *f)
{
    return syscall(SG_FS_FOPENWRITE, npath, f);
}

int trap_FS_FOpenRead(const char *npath, file_t *f)
{
    return syscall(SG_FS_FOPENREAD, npath, f);
}

unsigned int trap_FS_Write(const void *buffer, unsigned int len, file_t f)
{
    return syscall(SG_FS_WRITE, buffer, len, f);
}

unsigned int trap_FS_Read(void *buffer, unsigned int len, file_t f)
{
    return syscall(SG_FS_READ, buffer, len, f);
}

void trap_FS_WriteFile(const void *buffer, unsigned int len, file_t f)
{
    syscall(SG_FS_WRITEFILE, buffer, len, f);
}

void trap_FS_FClose(file_t f)
{
    syscall(SG_FS_FCLOSE, f);
}

int trap_Key_GetCatcher(void)
{
    return syscall(SG_KEY_GETCATCHER);
}

void trap_Key_SetCatcher(int catcher)
{
    syscall(SG_KEY_SETCATCHER, catcher);
}

int trap_Key_GetKey(const char *binding)
{
    return syscall(SG_KEY_GETKEY, binding);
}

qboolean trap_Key_IsDown(uint32_t keynum)
{
    return syscall(SG_KEY_ISDOWN, keynum);
}

int trap_MemoryRemaining(void)
{
    return syscall(SG_MEMORY_REMAINING);
}

void trap_RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
{
    syscall(SG_RE_ADDPOLYTOSCENE, hShader, verts, numVerts);
}

void trap_RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys )
{
    syscall(SG_RE_ADDPOLYLISTTOSCENE, polys, numPolys);
}

void trap_RE_SetColor(const float *rgba)
{
    syscall(SG_RE_SETCOLOR, rgba);
}

void trap_Snd_PlaySfx(sfxHandle_t sfx)
{
    syscall(SG_SND_PLAYSFX, sfx);
}

sfxHandle_t trap_Snd_RegisterSfx(const char *npath)
{
    return syscall(SG_SND_REGISTERSFX, npath);
}

void trap_Snd_StopSfx(sfxHandle_t sfx)
{
    syscall(SG_SND_STOPSFX, sfx);
}

nhandle_t trap_RE_RegisterShader(const char *npath)
{
    return syscall(SG_RE_REGISTERSHADER, npath);
}

void trap_GetGameState(gamestate_t *state)
{
    syscall(SG_GETGAMESTATE, state);
}
