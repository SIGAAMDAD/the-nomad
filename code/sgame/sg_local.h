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

#define NUMLEVELS 1

//==============================================================
// enums
//

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
	I_WEAPON,

    NUMITEMS,
} itemtype_t;

typedef enum
{
	ET_ITEM,
	ET_WEAPON,
	ET_MOB,
	ET_BOT,
	ET_WALL, // a tile with pre-determined collision physics
	ET_PLAYR,
	
	NUMENTITYTYPES
} entitytype_t;

typedef enum
{
	MT_GRUNT,
	MT_SHOTTY,
	MT_HULK,
	MT_DEATH,
	
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
	// its attacking
	EF_FIGHTING		= 32,
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

//==============================================================
// data types
//

typedef struct sgentity_s sgentity_t;

//
// entity state tracking
//
typedef void (*actionf_p1)( sgentity_t * );

typedef union {
	actionf_p1 acp1;
} actionp_t;

#include "sg_info.h"

//
// sgentity: the base entity data type
//
struct sgentity_s
{
	linkEntity_t link;
	bbox_t bounds;
	vec3_t origin;
	vec3_t vel;

	void *entPtr;
	state_t *state;

	const char *classname;

	sgentity_t *next;
	sgentity_t *prev;

	entitytype_t type;
	
	int health;
	int ticker;
	int frame;

	int facing;

	nhandle_t hShader;

	float width;
	float height;
	float angle;
	dirtype_t dir;
	entityflags_t flags;

	int sprite;
	nhandle_t hSpriteSheet;
};

typedef struct {
	sgentity_t *ent;

	const char *name;
	int cost; // in the market, not implemented yet
	itemtype_t type;
} item_t;

typedef struct {
	item_t *base; // weapons ARE techinally items
	
	ammotype_t ammo;
	int addammo;
	int damage;
	int range;
} weapon_t;

typedef struct
{
	sgentity_t *ent;
	
//	pmove_t pm;
	
	const weapon_t *weaponInv[MAXPLAYERWEAPONS];
	int ammocounts[NUMAMMOTYPES];
	const weapon_t *curwpn;
	
	spritenum_t foot_sprite;
	int foot_frame;
	int flags;
} playr_t;

typedef struct
{
	sgentity_t *ent;
	sgentity_t *target;

	mobtype_t type;
	
	float sight_range;
	float melee_range;
	float missile_range;
} mobj_t;

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
	sfxHandle_t player_parry;

	sfxHandle_t revolver_fire;
	sfxHandle_t revolver_rld;

	nhandle_t raio_shader;
	nhandle_t grunt_shader;
	nhandle_t shotty_shader;

	nhandle_t raio_sprites;
	nhandle_t grunt_sprites;
	nhandle_t shotty_sprites;

	stringHash_t *pickupShotgun;
} sgameMedia_t;

typedef struct
{
	// resources
	sgameMedia_t media;
	
	// general game state
	sgameState_t state;

	int frameTime;

	int numLevels;
	int numEntities;
	int numSaves;

	int previousTime;
	int levelTime;
	int framenum;

	// current map's name
	char mapname[MAX_GDR_PATH];

	float cameraWidth;
	float cameraHeight;

	mobj_t mobs[MAXMOBS];
	int numMobs;

	item_t items[MAXITEMS];
	int numItems;

	weapon_t weapons[MAXWEAPONS];
	int numWeapons;

	playr_t playr;
	qboolean playrReady;

	// for rendering to screen based coordinates
	gpuConfig_t gpuConfig;
	float scale;
	float bias;

	int checkpointIndex;
	linkEntity_t activeEnts;

	int soundBits[MAX_MAP_WIDTH*MAX_MAP_HEIGHT];

	vec2_t cameraPos;
	
    mapinfo_t mapInfo; // this structure's pretty big, so only 1 instance in the vm
} sgGlobals_t;

//==============================================================
// globals
//

extern const vec3_t dirvectors[NUMDIRS];
extern const dirtype_t inversedirs[NUMDIRS];

extern sgentity_t sg_entities[MAXENTITIES];

extern sgGlobals_t sg;

// there may be weapon mods in the future, if that does happen, we'll just may a copy in the
// player data instead of un-consting this stuff
extern const weapon_t weaponinfo[NUMWEAPONTYPES];
extern const item_t iteminfo[NUMITEMS];
extern const mobj_t mobinfo[NUMMOBS];
extern const int ammoCaps[NUMAMMOTYPES];
extern state_t stateinfo[NUMSTATES];

extern vmCvar_t sg_debugPrint;
extern vmCvar_t sg_paused;
extern vmCvar_t sg_mouseInvert;
extern vmCvar_t sg_mouseAcceleration;
extern vmCvar_t sg_printLevelStats;
extern vmCvar_t sg_decalDetail;
extern vmCvar_t sg_gibs;
extern vmCvar_t sg_levelIndex;
extern vmCvar_t sg_savename;
extern vmCvar_t sg_numSaves;
extern vmCvar_t sg_memoryDebug;

extern vmCvar_t pm_groundFriction;
extern vmCvar_t pm_waterFriction;
extern vmCvar_t pm_airFriction;
extern vmCvar_t pm_waterAccel;
extern vmCvar_t pm_baseAccel;
extern vmCvar_t pm_baseSpeed;
extern vmCvar_t pm_airAccel;
extern vmCvar_t pm_wallrunAccelVertical;
extern vmCvar_t pm_wallrunAccelMove;
extern vmCvar_t pm_wallTime;

//==============================================================
// functions
//


//
// sg_draw.c
//
int SG_DrawFrame( void );

//
// sg_cmds.c
//
void SGameCommand( void );
void SG_BuildMoveCommand( void );

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
qboolean SG_StartLevel( void );
int SG_EndLevel( void );
void SG_SaveLevelData( void );
void SG_LoadLevelData( void );

//
// sg_items.c
//
item_t *SG_SpawnItem( itemtype_t type );
weapon_t *SG_SpawnWeapon( weapontype_t type );
void SG_PickupWeapon();

//
// sg_entity.c
//
qboolean Ent_CheckWallCollision( const sgentity_t *ent );
sgentity_t *Ent_CheckEntityCollision( const sgentity_t *ent );
void Ent_RunTic( void );
sgentity_t *SG_AllocEntity( entitytype_t type );
void SG_FreeEntity( sgentity_t *e );
void SG_BuildBounds( bbox_t *bounds, float width, float height, const vec3_t *origin );
void Ent_BuildBounds( sgentity_t *ent );
void SG_InitEntities( void );
qboolean Ent_SetState( sgentity_t *ent, statenum_t state );
void SG_Spawn( uint32_t id, uint32_t type, const uvec3_t *origin );

//
// sg_mob.c
//
mobj_t *SG_SpawnMob( mobtype_t type );

//
// sg_mem.c
//
const char *String_Alloc( const char *str );
void *SG_MemAlloc( int size );
void SG_MemInit( void );
int SG_MakeMemoryMark( void );
void SG_ClearToMemoryMark( int mark );
qboolean SG_OutOfMemory( void );

//
// sg_playr.c
//
void SG_InitPlayer( void );
void SG_KeyEvent( int key, qboolean down );
void SG_MouseEvent( int dx, int dy );
void SG_SendUserCmd( int rightmove, int forwardmove, int upmove );
void P_Ticker( sgentity_t *self );
void P_MeleeTicker( sgentity_t *self );
void P_ParryTicker( sgentity_t *self );
qboolean P_GiveItem( itemtype_t item );
qboolean P_GiveWeapon( weapontype_t weapon );

//===============================================

//
// system traps
// These functions are how the sgame communicates with the main game system
//

// print a message on the local console
void trap_Print( const char *str );

// abort the vm
void trap_Error( const char *str );

// console command access
int trap_Argc( void );
void trap_Argv( int n, char *buf, int bufferLength );
void trap_Args( char *buf, int bufferLength );

//
// archive file handling
//

// save
void trap_BeginSaveSection( const char *name );
void trap_EndSaveSection( void );
void trap_SaveChar( const char *name, char data );
void trap_SaveInt( const char *name, int data );
void trap_SaveUInt( const char *name, unsigned int data );
void trap_SaveString( const char *name, const char *data );
void trap_SaveFloat( const char *name, float data );
void trap_SaveVec2( const char *name, const vec2_t *data );
void trap_SaveVec3( const char *name, const vec3_t *data );
void trap_SaveVec4( const char *name, const vec4_t *data );

// load
nhandle_t trap_GetSaveSection( const char *name );
unsigned int trap_LoadUInt( const char *name, nhandle_t hSection );
int trap_LoadInt( const char *name, nhandle_t hSection );
float trap_LoadFloat( const char *name, nhandle_t hSection );
void trap_LoadString( const char *name, char *pBuffer, int maxLength, nhandle_t hSection );
void trap_LoadVec2( const char *name, vec2_t *data, nhandle_t hSection );
void trap_LoadVec3( const char *name, vec3_t *data, nhandle_t hSection );
void trap_LoadVec4( const char *name, vec4_t *data, nhandle_t hSection );

//===============================================

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void trap_AddCommand( const char *cmdName );

void trap_RemoveCommand( const char *cmdName );

int trap_MemoryRemaining( void );

void Sys_SnapVector( float *v );

// sets the desired camera position, zoom, rotation, etc.
void G_SetCameraData( const vec2_t *origin, float zoom, float rotation );

nhandle_t G_LoadMap( const char *name );
void G_SetActiveMap( nhandle_t mapHandle, mapinfo_t *info, int *soundBits, linkEntity_t *activeEnts );
void G_CastRay( ray_t *ray );
void G_SoundRecursive( int width, int height, float volume, const vec3_t *origin );
qboolean trap_CheckWallHit( const vec3_t *origin, dirtype_t dir );

int trap_Milliseconds( void );

void trap_Key_SetCatcher( int catcher );
int trap_Key_GetCatcher( void );
int trap_Key_GetKey( const char *key );
void trap_Key_ClearStates( void );

sfxHandle_t trap_Snd_RegisterSfx( const char *npath );
sfxHandle_t trap_Snd_RegisterTrack( const char *npath );
void trap_Snd_QueueTrack( sfxHandle_t track );
void trap_Snd_PlaySfx( sfxHandle_t sfx );
void trap_Snd_StopSfx( sfxHandle_t sfx );
void trap_Snd_SetLoopingTrack( sfxHandle_t track );
void trap_Snd_ClearLoopingTrack( void );

nhandle_t RE_RegisterShader( const char *npath );
nhandle_t RE_RegisterSpriteSheet( const char *npath, int sheetWidth, int sheetHeight, int spriteWidth, int spriteHeight );
nhandle_t RE_RegisterSprite( nhandle_t hSpriteSheet, int index );
void RE_LoadWorldMap( const char *npath );
void RE_ClearScene( void );
void RE_RenderScene( const renderSceneRef_t *fd );
void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, int numVerts );
void RE_AddSpriteToScene( const vec3_t *origin, nhandle_t hSpriteSheet, nhandle_t hSprite );

void Sys_GetGPUConfig( gpuConfig_t *config );

// filesystem access
uint32_t trap_FS_FOpenFile( const char *npath, file_t *f, fileMode_t mode );
file_t trap_FS_FOpenWrite( const char *npath );
file_t trap_FS_FOpenRead( const char *npath );
void trap_FS_FClose( file_t f );
int trap_FS_Write( const void *data, int size, file_t f );
int trap_FS_Read( void *data, int size, file_t f );
int trap_FS_GetFileList( const char *path, const char *extension, char *listbuf, int bufsize ); 
int trap_FS_FileSeek( file_t f, fileOffset_t offset, int whence );
int trap_FS_FileLength( file_t f );
int trap_FS_FileTell( file_t f );
void GDR_ATTRIBUTE((format(printf, 2, 3))) trap_FS_Printf( file_t f, const char *fmt, ... );

// console variable interaction
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void Cvar_Update( vmCvar_t *vmCvar );
void Cvar_Set( const char *varName, const char *value );
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

#endif
