#include "r_local.h"
#include <stdarg.h>

renownGlobals_t rg;
renownImport_t ri;

cvar_t *r_renownDebug;
cvar_t *r_mobsFile;
cvar_t *r_botsFile;
cvar_t *r_itemsFile;
cvar_t *r_weaponsFile;
cvar_t *r_locationsFile;
cvar_t *r_factionsFile;
cvar_t *r_maxEventBuffer;
cvar_t *r_eventShuffle;

uint32_t r_numMobConfigs;
uint32_t r_numBotConfigs;
uint32_t r_numLocationConfigs;
uint32_t r_numFactionConfigs;

// this is only here so functions in n_math.c and n_shared.c can link
void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error( errorCode_t code, const char *fmt, ... )
{
	char msg[MAXPRINTMSG];
	
	va_start( argptr, fmt );
	N_vsnprintf( msg, sizeof(msg) - 1, fmt, argptr );
	va_end( argptr );
	
	ri.Error( code, "%s", msg );
}

void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf( const char *fmt, ... )
{
	char msg[MAXPRINTMSG];
	
	va_start( argptr, fmt );
	N_vsnprintf( msg, sizeof(msg) - 1, fmt, argptr );
	va_end( argptr );
	
	ri.Printf( PRINT_INFO, "%s", msg );
}

void Renown_Init( void )
{
	if ( !rg.recievedData ) {
		ri.Error( ERR_DROP, "Renown_Init: sgame must give renown system access to data pools before initialization" );
	}
	
	ri.Printf( PRINT_INFO, "========== Renown_Init ==========\n" );
	ri.Printf( PRINT_INFO, "Renown System version: %lu\n", RENOWN_API_VERSION );
	
	// clear all globals
	memset( &rg, 0, sizeof(rg) );
	
	// register cvars
#ifdef RENOWN_DEBUG
	r_renownDebug = ri.Cvar_Get( "r_renownDebug", "1", CVAR_LATCH | CVAR_PRIVATE );
#else
	r_renownDebug = ri.Cvar_Get( "r_renownDebug", "0", CVAR_LATCH | CVAR_PRIVATE );
#endif
	
	r_mobsFile = ri.Cvar_Get( "renown_mobsFile", "renown/mobs.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_mobsFile, "If contains a string, loads all mobs from a dedicated file" );
	r_botsFile = ri.Cvar_Get( "renown_botsFile", "renown/bots.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_botsFile, "If contains a string, loads all bots from a dedicated file" );
	r_itemsFile = ri.Cvar_Get( "renown_itemsFile", "renown/items.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_itemsFile, "If contains a string, loads all items from a dedicated file" );
	r_weaponsFile = ri.Cvar_Get( "renown_weaponsFile", "renown/weapons.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_weaponsFile, "If contains a string, loads all weapons from a dedicated file" );
	r_locationsFile = ri.Cvar_Get( "renown_locationsFile", "renown/locations.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_locationsFile, "If contains a string, loads all locations from a dedicated file" );
	r_factionsFile = ri.Cvar_Get( "renown_factionsFile", "renown/factions.txt", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_factionsFile, "If contains a string, loads all factions from a dedicated file" );
	
	r_maxEventBuffer = ri.Cvar_Get( "renown_maxEventBuffer", "2048", CVAR_LATCH | CVAR_PRIVATE );
	ri.Cvar_SetDescription( r_maxEventBuffer, "Sets the maximum number of renown events that can be buffered before a flush occurs." );
	r_eventShuffle = ri.Cvar_Get( "renown_eventShuffle", "1024", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_eventShuffle, "Sets the amount of events that are flushed, all events not in the buffer are"
											"ignored by the system until a restart occurs.\nSetting this to lower values might"
											"decrease performance." );
	
	ri.Cvar_SetGroup( r_renownDebug, CVG_RENOWN );
	ri.Cvar_SetGroup( r_mobsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_botsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_itemsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_weaponsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_locationsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_factionsFile, CVG_RENOWN );
	ri.Cvar_SetGroup( r_maxEventBuffer, CVG_RENOWN );
	ri.Cvar_SetGroup( r_eventShuffle, CVG_RENOWN );
	
	rg.numEntities = rg.maxMobs + rg.maxBots;
	rg.pEntities = (renown_entity_t *)ri.Malloc( sizeof(*rg.pEntities) * rg.numEntities );
	
	Renown_InitEvents();
	Renown_LoadConfigs();
	Renown_GenerateMissions();
}

/*
* Renown_RecieveVMData: gives the renown system access to the vm's entity data
*/
void Renown_RecieveVMData( mobj_t *pMobs, uint32_t maxMobs, uint32_t maxItems, uint32_t maxWeapons, uint32_t maxBots )
{
	sg.maxMobs = maxMobs;
}

void Renown_Shutdown( void )
{
	ri.Printf( PRINT_INFO, "========== Renown_Shutdown ==========\n" );
	
	Renown_FlushEvents();
	
	// dump all memory
	ri.FreeAll();
}

renownExport_t *GetRenownAPI( uint64_t nVersion, const renownImport_t *imp )
{
	static renownExport_t out;
	
	memset( &out, 0, sizeof(out) );
	memcpy( &ri, imp, sizeof(ri) );
	
	if ( nVersion != RENOWN_API_VERSION ) {
		ri.Error( ERR_FATAL, "GetRenownAPI: incompatible RENOWN_API_VERSION, %lu (this) vs %lu (engine)",
			(uint64_t)RENOWN_API_VERSION, nVersion );
	}
	
	out.Renown_Init = Renown_Init;
	out.Renown_Shutdown = Renown_Shutdown;
	out.Renown_Update = Renown_Update;
	
	return &out;
}
