#ifndef __SG_LOCAL__
#define __SG_LOCAL__

#include "../engine/n_shared.h"
#include "sg_public.h"
#include "../rendercommon/r_types.h"
#include "sg_imgui.h"
#include "../game/g_game.h"

#pragma once

//==============================================================
// defines & macros
//

#define MAXENTITIES 2048

#define MAXMOBS 1024
#define MAXITEMS 1024
#define MAXWEAPONS 1024

#define ARCHIVE_SAVEGAME 0
#define ARCHIVE_LOADGAME 1

#define DMG_EXPLOSION 16

#define MAXPLAYERWEAPONS 6

//==============================================================
// enums
//

typedef enum {
	SGAME_INACTIVE,
	SGAME_IN_LEVEL,
	SGAME_SHOW_LEVEL_STATS,
} sgameState_t;

//
// weapontype_t: enumerations for weapons, feel free to add your own
// NOTE: you will have to customize the weapon code in sg_combat to
// actually make the weapons useable in-game
//
typedef enum
{
	WT_SHOTTY_DB,		// shotgun -- double barreled
	WT_SHOTTY_AUTO,		// shotgun -- full auto
	WT_SHOTTY_3B_PUMP,	// shotgun -- 3-burst pump
	
	WT_RIFLE_HOSIG,		// rifle -- hosig
	WT_RIFLE_FUSION,	// rifle -- fusion cannon
	WT_RIFLE_4B_BOLT,	// rifle -- 4-burst bolt action
	WT_RIFLE_DESERT,	// rifle -- desert nomad variant
	
	WT_PISTOL_DB,		// pistol -- sawed-off
	WT_PISTOL_DEAGLE,	// pistol -- deagle
	WT_PISTOL_ROCKET,	// pistol -- single shot rocket launcher
	
	WT_MELEE_STAR,		// melee -- the morning star
	WT_MELEE_FISTS,		// melee -- just your bare fists
	WT_MELEE_KANTANA,	// melee -- kantana
	WT_MELEE_BO,		// melee -- bo stick
	WT_MELEE_BS,		// melee -- bitch slap (only useable in DIF_HARDEST)
	
	WT_ARM_BLADE,		// arm attachment -- wrist blade
	WT_ARM_HANDCANNON,	// arm attachment -- hand cannon
	WT_ARM_GRAPPLE,		// arm attachment -- grappling hook
	WT_ARM_SD,			// arm attachment -- sonic disruptor
	WT_ARM_FT,			// arm attachment -- flamethrower
	WT_ARM_TT,			// arm attachment -- time travel
	WT_ARM_MP,			// arm attachment -- mega punch
	
	NUMWEAPONTYPES
} weapontype_t;

typedef enum {
	COD_UNKNOWN,
	COD_SHOTGUN,
	COD_PUNCH,
	COD_BLADE,
	COD_IMPLODED,
	COD_EXPLODED,
	COD_DROWNED,
	COD_TELEFRAG,
	COD_SUICIDE,
	COD_FALLING,
} causeofdeath_t;

typedef enum
{
    I_NULL,

    NUMITEMS,
} itemtype_t;

typedef enum
{
	ET_ITEM,
	ET_WEAPON,
	ET_PLAYR,
	ET_MOB,
	ET_BOT,
	ET_WALL, // a tile with pre-determined collision physics
	
	NUMENTITYTYPES
} entitytype_t;

typedef enum
{
	MT_HULK,
	MT_GRUNT,
	MT_CHAINSAW,
	MT_SHOTTY,
	
	NUMMOBS
} mobtype_t;

typedef enum
{
	// will it bleed?
	EF_KILLABLE		= 0,
	// is it parryable?
	EF_PARRY		= 2,
	// is it stationary?
	EF_SENTRY		= 4,
	// is it invincible?
	EF_INVUL		= 8,
	// can it be interacted with?
	EF_INTER		= 16,
} entityflags_t;

typedef enum {
	AM_SHELL,
	AM_BULLET,
	AM_ROCKET,
	
	NUMAMMOTYPES
} ammotype_t;

//typedef enum
//{} powerup_t;

//typedef enum
//{} playrflags_t;

#include "sg_sprites.h"
#include "sg_state.h"

//==============================================================
// data types
//

// bounding boxes
typedef struct {
	vec3_t mins;
	vec3_t maxs;
} bbox_t;

typedef struct {
	const char *name;
	uint32_t cost; // in the market, not implemented yet
	itemtype_t type;
} item_t;

typedef struct {
	item_t base; // weapons ARE techinally items
	
	ammotype_t ammo;
	uint32_t addammo;
	uint32_t damage;
	uint32_t range;
} weapon_t;

typedef vec2_t spriteCoords_t[4];

typedef struct {
	uint32_t numSprites;
	spriteCoords_t *texCoords;
} spritesheet_t;

typedef struct sgentity_s sgentity_t;

//
// entity state tracking
//
typedef void (*actionf_p1)( sgentity_t * );

typedef union {
	actionf_p1 acp1;
} actionp_t;

typedef struct
{
	statenum_t id;
	statenum_t next;
	actionp_t action;
	spritenum_t sprite;
	int32_t ticcount;
} state_t;

//
// sgentity: the base entity data type
//
struct sgentity_s
{
	entitytype_t type;
	
	vec3_t origin;
	vec3_t vel;
	bbox_t bounds;
	
	int32_t health;
	int32_t ticker;

	nhandle_t hShader;

	float angle;
	dirtype_t dir;

	void *entPtr;
	
	statenum_t stateOffset;
	spritenum_t sprite;
	state_t *state;

	spritesheet_t *sheet;
	
	sgentity_t *next;
	sgentity_t *prev;
};

typedef struct
{
	sgentity_t *ent;
	
//	pmove_t pm;
	
	const weapon_t *weaponInv[MAXPLAYERWEAPONS];
	uint32_t ammocounts[NUMAMMOTYPES];
	const weapon_t *curwpn;
	
	uint32_t flags;
} playr_t;

typedef struct
{
	sgentity_t *ent;
	
	float sight_range;
	float melee_range;
	float missile_range;
} mobj_t;

// used for collision physics
typedef struct
{
	vec3_t start;
	vec3_t end;
	vec3_t origin;
	float speed;
	float length;
	float angle;
	sgentity_t *hitData;
} ray_t;

typedef struct stringHash_s
{
	char name[MAX_STRING_CHARS];
	char string[MAX_STRING_CHARS];
	struct stringHash_s *next;
} stringHash_t;

typedef struct
{
	sfxHandle_t player_pain0;
    sfxHandle_t player_pain1;
    sfxHandle_t player_pain2;

    sfxHandle_t player_death0;
    sfxHandle_t player_death1;
    sfxHandle_t player_death2;

	nhandle_t raio_shader;

	stringHash_t *pickupShotgun;
} sgameMedia_t;

typedef struct
{
	// resources
	sgameMedia_t media;
	
	// general game state
	sgameState_t state;

	sgentity_t wallEntity; // for raycasts

	// current map's name
	char mapname[MAX_GDR_PATH];

	vec3_t cameraPos;
	float cameraWidth;
	float cameraHeight;

	mobj_t mobs[MAXMOBS];
	uint32_t numMobs;

	item_t items[MAXITEMS];
	uint32_t numItems;

	weapon_t weapons[MAXWEAPONS];
	uint32_t numWeapons;

	playr_t playr;
	qboolean playrReady;

	// for rendering to screen based coordinates
	gpuConfig_t gpuConfig;
	float scale;
	float bias;

	int32_t checkpointIndex;
	
    mapinfo_t mapInfo; // this structure's pretty big, so only 1 instance in the vm
	int32_t leveltime;
} sgGlobals_t;

//==============================================================
// globals
//

extern uint32_t sg_numLevels;

extern const vec3_t dirvectors[NUMDIRS];
extern const dirtype_t inversedirs[NUMDIRS];

extern sgentity_t sg_entList[MAXENTITIES];
// doubly-linked list of allocated entities
extern sgentity_t sg_activeEnts;
// single-linked list of unused entities
extern sgentity_t *sg_freeEnts;

extern spritesheet_t sprites_thenomad;

extern sgGlobals_t sg;

// there may be weapon mods in the future, if that does happen, we'll just may a copy in the
// player data instead of un-consting this stuff
extern const weapon_t weaponinfo[NUMWEAPONTYPES];
extern const item_t iteminfo[NUMITEMS];
extern const mobj_t mobinfo[NUMMOBS];
extern const uint32_t ammoCaps[NUMAMMOTYPES];
extern state_t stateinfo[NUMSTATES];

extern vmCvar_t sg_debugPrint;
extern vmCvar_t sg_paused;
extern vmCvar_t sg_pmAirAcceleration;
extern vmCvar_t sg_pmWaterAcceleration;
extern vmCvar_t sg_pmBaseAcceleration;
extern vmCvar_t sg_pmBaseSpeed;
extern vmCvar_t sg_mouseInvert;
extern vmCvar_t sg_mouseAcceleration;
extern vmCvar_t sg_printLevelStats;
extern vmCvar_t sg_decalDetail;
extern vmCvar_t sg_gibs;
extern vmCvar_t sg_levelInfoFile;
extern vmCvar_t sg_levelIndex;
extern vmCvar_t sg_levelDataFile;
extern vmCvar_t sg_savename;

//==============================================================
// functions
//

//
// sg_main.c
//
void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error( const char *err, ... );
void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf( const char *fmt, ... );
void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error( const char *err, ... );
void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf( const char *fmt, ... );
void SG_UpdateCvars( void );

//
// sg_level.c
//
qboolean SG_InitLevel( int32_t index );
int32_t SG_EndLevel( void );
void Lvl_AddKillEntity( entitytype_t type, causeofdeath_t cod );

//
// sg_entity.c
//
void Ent_RunTic( void );
sgentity_t *SG_AllocEntity( entitytype_t type );
void SG_FreeEntity( sgentity_t *e );
void SG_BuildBounds( bbox_t *bounds, const vec3_t origin, float w, float h );
void SG_InitEntities(void);

//
// sg_archive.c
//
typedef void (*archiveFunc_t)( file_t, int );
void SG_SaveGame( void );
void SG_LoadGame( const char *filename );
void SG_AddArchiveHandle( archiveFunc_t pFunc );

//
// sg_mthink.c
//
mobj_t *SG_SpawnMob( mobtype_t type );
void SG_SpawnMobOnMap( mobtype_t id, float x, float y, float elevation );

//
// sg_mem.c
//
const char *String_Alloc( const char *str );
void *SG_MemAlloc( uint32_t size );
void SG_MemInit( void );
qboolean SG_OutOfMemory( void );

//
// sg_playr.c
//
void SG_InitPlayer( void );
void P_Thinker( sgentity_t *self );
void SG_KeyEvent( int32_t key, qboolean down );
void SG_MouseEvent(  );
qboolean P_GiveItem( itemtype_t item );
qboolean P_GiveWeapon( weapontype_t weapon );

//===============================================

//
// system traps
// These functions are how the sgame communicates with the main game system
//

int32_t G_LoadMap( int32_t index, mapinfo_t *info );

// print a message to the console
void trap_Print(const char *fmt);

// abort the vm
void trap_Error(const char *fmt);


// milliseconds should only be used for performance tuning, never
// for anything game related
uint32_t trap_Milliseconds( void );

// console variable interaction
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
void Cvar_Update( vmCvar_t *vmCvar );
void Cvar_Set( const char *var_name, const char *value );
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize );

// ConsoleCommand parameter access
uint32_t trap_Argc( void );
void trap_Argv( uint32_t n, char *buffer, uint32_t bufferLength );
void trap_Args( char *buffer, uint32_t bufferLength );

// filesystem access
file_t trap_FS_FOpenRead( const char *npath );
file_t trap_FS_FOpenWrite( const char *npath );
file_t trap_FS_FOpenAppend( const char *npath );
file_t trap_FS_FOpenRW( const char *npath );
fileOffset_t trap_FS_FileSeek( file_t file, fileOffset_t offset, uint32_t whence );
fileOffset_t trap_FS_FileTell( file_t file );
uint64_t trap_FS_FOpenFile( const char *npath, file_t *file, fileMode_t mode );
file_t trap_FS_FOpenFileWrite( const char *npath, file_t *file );
uint64_t trap_FS_FOpenFileRead( const char *npath, file_t *file );
void trap_FS_FClose( file_t file );
uint64_t trap_FS_WriteFile( const void *buffer, uint64_t len, file_t file );
uint64_t trap_FS_Write( const void *buffer, uint64_t len, file_t file );
uint64_t trap_FS_Read( void *buffer, uint64_t len, file_t file );
uint64_t trap_FS_FileLength( file_t file );
uint64_t trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, uint64_t bufsize );
void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... );

void trap_GetGPUConfig( gpuConfig_t *config );

void G_SetBindNames( const char **bindnames, uint32_t numbindings );

// register a command name so the console can perform command completion.
void trap_AddCommand( const char *cmdName );

// register a sound effect
sfxHandle_t trap_Snd_RegisterSfx(const char *npath);

// queue a sound effect
void trap_Snd_PlaySfx(sfxHandle_t sfx);

// force stop an sfx
void trap_Snd_StopSfx(sfxHandle_t sfx);

// register a shader (technically a texture)
nhandle_t RE_RegisterShader(const char *npath);

uint32_t trap_Key_GetCatcher(void);
void trap_Key_SetCatcher(uint32_t catcher);
uint32_t trap_Key_GetKey(const char *binding);
qboolean trap_Key_IsDown(uint32_t keynum);

// drawing functions
void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
void RE_ClearScene( void );
void RE_RenderScene( const renderSceneRef_t *fd );

// load a level map
void RE_LoadWorldMap( const char *filename );

// set the rendering color
void RE_SetColor(const float *rgba);

#endif
