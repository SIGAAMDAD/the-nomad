#ifndef _SG_LOCAL_
#define _SG_LOCAL_

#pragma once

#include "sg_public.h"

// everything is globally or statically allocated within the vm, unless its using the G_AllocMem stuff, but the vm doesn't like it
// (reasonably, raw pointers + vm bytecode = exploits) when you pass pointers back and forth from the vm and native bytecode, so non of that'll happen
#define MAX_PLAYR_COUNT 10
#define MAX_MOBS_ACTIVE 150
#define MAX_PLAYR_INVENTORY 20

extern int g_vert_fov;
extern int g_horz_fov;
extern float g_pspeed;
extern float g_gravity;

extern float snd_musicvol;
extern float snd_sfxvol;
extern qboolean snd_musicon;
extern qboolean snd_sfxon;

extern int r_ticrate;
extern int r_texture_magfilter;
extern int r_texture_minfilter;
extern int r_screenheight;
extern int r_screenwidth;
extern int r_renderapi;
extern qboolean r_vsync;
extern qboolean r_fullscreen;
extern qboolean r_hidden;
extern qboolean r_drawFPS;
extern int r_renderapi;
extern int r_msaa_amount;

extern qboolean c_fastmobs1;
extern qboolean c_fastmobs2;
extern qboolean c_fastmobs3;
extern qboolean c_deafmobs;
extern qboolean c_blindmobs;
extern qboolean c_nosmell;
extern qboolean c_nomobs;
extern qboolean c_godmode;
extern qboolean c_infinite_ammo;
extern qboolean c_bottomless_clip;
extern qboolean c_devmode;

int G_Init();
int G_Shutdown();

int G_StartLevel();
int G_EndLevel();

void* G_AllocMem(int size);
void G_FreeMem(void *ptr);
void G_InitMem(void);

#endif
