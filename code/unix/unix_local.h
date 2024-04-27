#ifndef __UNIX_LOCAL__
#define __UNIX_LOCAL__

#pragma once

#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include <execinfo.h>
#include <sys/vfs.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/resource.h>

// Input subsystem
void IN_Init( void );
void IN_Frame( void );
void IN_Shutdown( void );

void IN_JoyMove( void );
void IN_StartupJoystick( void );

// OpenGL subsystem
qboolean GL_Init( const char *dllname );
void GL_Shutdown( qboolean unloadDLL );

// Vulkan subsystem
qboolean VK_Init( void );
void VK_Shutdown( qboolean unloadDLL );

char *strlwr( char *s );
void InitSig( void );

typedef struct {
    uint32_t id;
    qboolean safe;
    const char *str;
} exittype_t;

extern const exittype_t signals[8];
extern const exittype_t *exit_type;

#endif