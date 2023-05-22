#ifndef _N_CONSOLE_
#define _N_CONSOLE_

#pragma once

void Cmd_AddVar(const char* name, void* value);
void Cmd_RemoveVar(const char* name);
void* Cmd_GetVar(const char* name);

void Con_Printf(const char *fmt, ...);
void Con_Error(const char *fmt, ...);
void Con_Flush(void);

#endif