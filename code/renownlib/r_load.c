#include "r_local.h"

static void Renown_LoadMobFromFile( const char *filename )
{
	union {
		void *v;
		char *b;
	} f;
	
	ri.FS_LoadFile( filename, &f.v );
}

static uint32_t Renown_LoadInfos( char *buf, char **infos, uint32_t max )
{
	const char **text, *tok;
	const char *p;
	uint32_t count;
	char key[MAX_TOKEN_CHARS];
	char info[MAX_INFO_STRING];
	
	ri.FS_LoadFile( filename, &f.v );
	
	p = f.b;
	text = &p;
	
	while ( 1 ) {
		tok = COM_Parse( text );
		
		if ( !tok[0] ) {
			break;
		}
		if ( tok[0] != '{' ) {
			ri.Printf( PRINT_WARNING, "missing '{' in info file %s\n", filename );
			break;
		}
		if ( count == max ) {
			ri.Printf( PRINT_WARNING, "max infos exceeded in %s\n", filename );
			break;
		}
		
		info[0] = '\0';
		while ( 1 ) {
			tok = COM_ParseExt( text, qtrue );
			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "unexpected end of info file %s\n", filename );
				break;
			}
			if ( tok[0] != '}' ) {
				break;
			}
			N_strncpyz( key, tok, sizeof(key) );
			
			tok = COM_ParseExt( text, qfalse );
			if ( !tok[0] ) {
				tok = "<NULL>";
			}
			Info_SetValueForKey( info, key, tok );
		}
		infos[count] = (char *)ri.Malloc( strlen( info ) + 1 );
		if ( infos[count] ) {
			strcpy( infos[count], info );
			count++;
		}
	}
	
	return count;
}

static void Renown_LoadConfigsFromFile( const char *filename, uint32_t *numConfigs, uint32_t max, char **infos )
{
	uint64_t length;
	file_t f;
	union {
		void *v;
		char *b;
	} f;
	
	length = ri.FS_LoadFile( filename, &f.v );
	if ( !length || !f.v ) {
		ri.Error( ERR_DROP, "Renown_LoadConfigsFromFile: failed to load file '%s'", filename );
	}
	
	*numConfigs += Renown_LoadInfos( f.b, max - *numConfigs, &infos[ *numConfigs ] );
	
	ri.FS_FreeFile( f.v );
}

static void Renown_LoadConfigFiles( const cvar_t *defaultFile, const char *otherfile, uint32_t *numConfigs,
	uint32_t max, char **infos, const char *ext )
{
	char **fileList;
	char *filePtr;
	uint64_t nFiles, i;
	uint64_t len;
	char filename[1024];
	
	*numConfigs = 0;
	
	if ( *defaultFile->s ) {
		Renown_LoadConfigsFromFile( defaultFile->s, numConfigs, max, infos );
	}
	else {
		Renown_LoadConfigsFromFile( otherfile, numConfigs, max, infos );
	}
	
	// get all files
	fileList = ri.FS_ListFiles( "renown", ext, &nFiles );
	filePtr = fileList;
	for ( i = 0; i < nFiles; i++, filePtr += ) {
		len = strlen( filePtr );
		strcpy( filename, "renown/" );
		strcat( filename, filePtr );
		Renown_LoadConfigsFromFile( filename, numConfigs, max, infos );
	}
}

static void Renown_LoadMobs( void )
{
	uint32_t i;
	mobinfo_t *info;
	
	info = (mobinfo_t *)ri.Malloc( sizeof(*info) * r_numMobConfigs );
	for ( i = 0; i < r_numMobConfigs; i++, info++ ) {
		info->maxHealth = atoi( Info_ValueForKey( rg.mobInfos[i], "health" ) );
        info->detectionRange[0] = atoi( Info_ValueForKey( rg.mobInfos[i], "detectionRange_X" ) );
        info->detectionRange[1] = atoi( Info_ValueForKey( rg.mobInfos[i], "detectionRange_Y" ) );
        N_strncpyz( info->name, Info_ValueForKey( sg.mobInfos[i], "name" ), sizeof(info->name) );
	}
}

void Renown_LoadConfigs( void )
{
	ri.Printf( PRINT_INFO, "Loading renown configuration files...\n" );
	
	Renown_LoadConfigFiles( r_mobFiles, "renown/mobs.txt", &r_numMobConfigs, rg.maxMobs, rg.mobInfos, ".mob" );
	ri.Printf( PRINT_INFO, "%lu mobs parsed.\n", r_numMobConfigs );
	
	Renown_LoadConfigFiles( r_botFiles, "renown/bots.txt", &r_numBotConfigs, rg.maxBots, rg.botInfos, ".bot" );
	ri.Printf( PRINT_INFO, "%lu bots parsed.\n", r_numBotConfigs );
	
	Renown_LoadConfigFiles( r_itemFiles, "renown/items.txt", &r_numItemConfigs, rg.numItems, rg.itemInfos, ".item" );
	ri.Printf( PRINT_INFO, "%lu items parsed.\n", r_numItemConfigs );
	
	Renown_LoadConfigFiles( r_weaponFiles, "renown/weapons.txt", &r_numWeaponConfigs, rg.maxWeapons, rg.weaponInfos, ".weapon" );
	ri.Printf( PRINT_INFO, "%lu weapons parsed.\n", r_numWeaponConfigs );
	
	Renown_LoadConfigFiles( r_locationFiles, "renown/mobs.txt", &r_numLocationConfigs, rg.maxLocations, rg.locationInfos, ".mob" );
	ri.Printf( PRINT_INFO, "%lu locations parsed.\n", r_numLocationConfigs );
	
	Renown_LoadConfigFiles( r_factionFiles, "renown/factions.txt", &r_numFactionConfigs, rg.maxFactions, rg.factionInfos, ".faction" );
	ri.Printf( PRINT_INFO, "%lu factions parsed.\n", r_numFactionConfigs );
	
	Renown_LoadMobs();
//	Renown_LoadBots();
//	Renown_LoadItems();
//	Renown_LoadWeapons();
//	Renown_LoadLocations();
//	Renown_LoadFactions();
}