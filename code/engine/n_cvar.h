#ifndef __N_CVAR__
#define __N_CVAR__

#pragma once

#include "../engine/n_common.h"

extern uint32_t cvar_modifiedFlags;

void Cvar_ResetGroup( cvarGroup_t group, qboolean resetModifiedFlags );
int Cvar_CheckGroup(cvarGroup_t group);
void Cvar_ForceReset(const char *name);
void Cvar_Init(void);
void Cvar_Restart(qboolean unsetVM);
void Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags, uint32_t privateFlag);
void Cvar_CompleteCvarName(const char *args, uint32_t argNum);
void Cvar_CommandCompletion( void (*callback)(const char *s) );
cvar_t *Cvar_Set2(const char *var_name, const char *value, qboolean force);
void Cvar_VariableStringBuffer(const char *name, char *buffer, uint64_t bufferSize);
void Cvar_VariableStringBufferSafe(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
int32_t Cvar_VariableInteger(const char *name);
float Cvar_VariableFloat(const char *name);
const char *Cvar_VariableString(const char *name);
void Cvar_CheckRange(cvar_t *var, const char *mins, const char *maxs, cvartype_t type);
uint32_t Cvar_Flags(const char *name);
void Cvar_Update(vmCvar_t *vmCvar, uint32_t privateFlag);
cvar_t *Cvar_Get(const char *name, const char *value, uint32_t flags);
qboolean Cvar_Command(void);
void Cvar_Reset(const char *name);
void Cvar_SetGroup(cvar_t *cv, cvarGroup_t group);
void Cvar_SetDescription(cvar_t *cv, const char *description);
void Cvar_SetSafe(const char *name, const char *value);
void Cvar_Set(const char *name, const char *value);
void Cvar_SetValueSafe(const char *name, float value);
qboolean Cvar_SetModified(const char *name, qboolean modified);
void Cvar_SetIntegerValue(const char *name, int32_t value);
void Cvar_SetFloatValue(const char *name, float value);
void Cvar_SetStringValue(const char *name, const char *value);
void Cvar_WriteVariables(fileHandle_t f);

#endif