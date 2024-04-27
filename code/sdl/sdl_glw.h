#ifndef __SDL_GLW__
#define __SDL_GLW__

#pragma once

#ifdef USE_LOCAL_HEADERS
#   include "SDL2/SDL.h"
#else
#   include <SDL2/SDL.h>
#endif

#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../game/g_game.h"

typedef struct
{
	FILE *log_fp;

	qboolean isFullscreen;

	gpuConfig_t *config; // feedback to renderer module

	int desktop_width;
	int desktop_height;

	int window_width;
	int window_height;

	int monitorCount;
} glwstate_t;

extern SDL_Window *SDL_window;
extern glwstate_t glw_state;

extern cvar_t *in_nograb;

void IN_Init( void );
void IN_Shutdown( void );

void InitSig( void );

#endif