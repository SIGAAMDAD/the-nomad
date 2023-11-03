
// sg_syscalls.c: this file is only included in development dll builds, the .asm file is used instead when building the VM file
#include "../rendercommon/r_public.h"
#include "../game/g_game.h"
#include "sg_local.h"

#ifdef Q3_VM
    #error Never use is VM build
#endif

vmRefImport_t vmi;

//static intptr_t (GDR_DECL *syscall)(intptr_t arg, uint32_t, ...) = (intptr_t (GDR_DECL *)(intptr_t, uint32_t, ...)) - 1;

void dllEntry(const vmRefImport_t *import)
{
    memcpy(&vmi, import, sizeof(vmi));
}

#if 0
void dllEntry(intptr_t (GDR_DECL *syscallptr)(intptr_t arg, uint32_t numArgs, ...))
{
    syscall = syscallptr;
}
#endif

int PASSFLOAT(float x)
{
    float floatTemp;
    floatTemp = x;
    return *(int *)&floatTemp;
}

void trap_GetGPUConfig(gpuConfig_t *config)
{
    vmi.trap_GetGPUConfig(config);
}

void trap_Cmd_ExecuteText(cbufExec_t exec, const char *text)
{
    vmi.trap_Cmd_ExecuteText(exec, text);
}

void trap_RE_ClearScene(void)
{
    vmi.trap_RE_ClearScene();
}

void trap_GetClipboardData( char *buf, uint32_t bufsize )
{
    vmi.trap_GetClipboardData(buf, bufsize);
}

uint32_t trap_Key_GetCatcher(void)
{
    return vmi.trap_Key_GetCatcher();
}

void trap_Key_SetCatcher(uint32_t catcher)
{
    vmi.trap_Key_SetCatcher(catcher);
}

uint32_t trap_Key_GetKey(const char *binding)
{
    return vmi.trap_Key_GetKey(binding);
}

qboolean trap_Key_IsDown(uint32_t keynum)
{
    return vmi.trap_Key_IsDown(keynum);
}

void trap_Key_ClearStates(void)
{
    vmi.trap_Key_ClearStates();
}

qboolean trap_Key_AnyDown(void)
{
    return vmi.trap_Key_AnyDown();
}

void trap_Print(const char *str)
{
    vmi.trap_Print(str);
}

void trap_Error(const char *str)
{
    vmi.trap_Error(str);
}

void trap_RE_SetColor(const float *rgba)
{
    vmi.trap_RE_SetColor(rgba);
}

void trap_RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
{
    vmi.trap_RE_AddPolyToScene(hShader, verts, numVerts);
}

void trap_RE_AddEntityToScene( const renderEntityRef_t *ent )
{
    vmi.trap_RE_AddEntityToScene(ent);
}

void trap_RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys )
{
    vmi.trap_RE_AddPolyListToScene(polys, numPolys);
}

void trap_RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader )
{
    vmi.trap_RE_DrawImage(x, y, w, h, u1, v1, u2, v2, hShader);
}

void trap_RE_RenderScene( const renderSceneRef_t *fd )
{
    vmi.trap_RE_RenderScene(fd);
}

nhandle_t trap_RE_RegisterShader(const char *name)
{
    return vmi.trap_RE_RegisterShader(name);
}

sfxHandle_t trap_Snd_RegisterSfx(const char *npath)
{
    return vmi.trap_Snd_RegisterSfx(npath);
}

void trap_Snd_PlaySfx(sfxHandle_t sfx)
{
    vmi.trap_Snd_PlaySfx(sfx);
}

void trap_Snd_StopSfx(sfxHandle_t sfx)
{
    vmi.trap_Snd_StopSfx(sfx);
}

void trap_UpdateScreen(void)
{
    vmi.trap_UpdateScreen();
}

void trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags )
{
    vmi.trap_Cvar_Register(vmCvar, varName, defaultValue, flags);
}

void trap_Cvar_Update( vmCvar_t *vmCvar )
{
    vmi.trap_Cvar_Update(vmCvar);
}

void trap_Cvar_Set( const char *var_name, const char *value )
{
    vmi.trap_Cvar_Set(var_name, value);
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize )
{
    vmi.trap_Cvar_VariableStringBuffer(var_name, buffer, bufsize);
}

uint32_t trap_Argc( void )
{
    return vmi.trap_Argc();
}

void trap_Argv( uint32_t n, char *buffer, uint32_t bufferLength )
{
    vmi.trap_Argv(n, buffer, bufferLength);
}

void trap_Args( char *buffer, uint32_t bufferLength )
{
    return; // not meant to called
}

file_t trap_FS_FOpenWrite( const char *path, file_t *f )
{
    return vmi.trap_FS_FOpenWrite(path, f, H_SGAME);
}

file_t trap_FS_FOpenRead( const char *path, file_t *f )
{
    return vmi.trap_FS_FOpenRead(path, f, H_SGAME);
}

void trap_FS_FClose(file_t f)
{
    vmi.trap_FS_FClose(f);
}

uint32_t trap_FS_Read( void *buffer, uint32_t len, file_t f )
{
    return vmi.trap_FS_Read(buffer, len, f, H_SGAME);
}

uint32_t trap_FS_Write( const void *buffer, uint32_t len, file_t f)
{
    return vmi.trap_FS_Write(buffer, len, f, H_SGAME);
}

void trap_FS_WriteFile( const void *buffer, uint32_t len, file_t f )
{
    vmi.trap_FS_WriteFile(buffer, len, f, H_SGAME);
}

uint64_t trap_FS_FOpenFileRead( const char *path, file_t *f )
{
    return vmi.trap_FS_FOpenFileRead(path, f, H_SGAME);
}

fileOffset_t trap_FS_FileSeek( file_t f, fileOffset_t offset, uint32_t whence )
{
    return vmi.trap_FS_FileSeek(f, offset, whence, H_SGAME);
}

uint64_t trap_FS_FOpenFileWrite( const char *path, file_t *f )
{
    return vmi.trap_FS_FOpenFileWrite(path, f, H_SGAME);
}

fileOffset_t trap_FS_FileTell( file_t f )
{
    return vmi.trap_FS_FileTell(f);
}
