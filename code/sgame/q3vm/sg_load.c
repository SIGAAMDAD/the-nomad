#include "sg_local.h"
#include "sg_info.h"
#include "sg_util.h"
#define JSON_IMPLEMENTATION
#include "json.h"
#undef JSON_IMPLEMENTATION

moduleInfo_t sg_moduleInfos;

static qboolean JSON_GetString( const char *text_p, const char *json_end, const char *name, char *str, int maxLength ) {
    text_p = JSON_ObjectGetNamedValue( text_p, json_end, name );
    if ( !text_p ) {
        SG_Printf( COLOR_RED "ERROR: missing value for '%s' in json file!\n", name );
        return qfalse;
    }
    JSON_ValueGetString( text_p, json_end, str, maxLength );
    return qtrue;
}

static int JSON_GetInt( const char *text_p, const char *json_end, const char *name ) {
    text_p = JSON_ObjectGetNamedValue( text_p, json_end, name );
    if ( !text_p ) {
        SG_Printf( COLOR_RED "ERROR: missing value for '%s' in json file!\n", name );
        return -1;
    }
    return JSON_ValueGetInt( text_p, json_end );
}

static float JSON_GetFloat( const char *text_p, const char *json_end, const char *name ) {
    text_p = JSON_ObjectGetNamedValue( text_p, json_end, name );
    if ( !text_p ) {
        SG_Printf( COLOR_RED "ERROR: missing value for '%s' in json file!\n", name );
        return -1;
    }
    return JSON_ValueGetFloat( text_p, json_end );
}

static const char *GetString( const char *name, const char **text ) {
    static char str[MAX_TOKEN_CHARS];
    const char *tok;

    tok = COM_ParseExt( text, qfalse );
    if ( !tok[0] ) {
        COM_ParseError( "missing parameter of type string for '%s' in module info file", name );
    }

    N_strncpyz( str, tok, sizeof(str) );
    return str;
}

static const char *GetStringToken( const char *name, const char **text ) {
    const char *tok;

    tok = COM_ParseExt( text, qfalse );
    if ( !tok[0] ) {
        COM_ParseError( "missing parameter of type string for '%s' in module info file", name );
    }

    return String_Alloc( tok );
}

static void GetStringParm( const char *name, char *str, int maxLength, const char **text ) {
    const char *tok;

    tok = COM_ParseExt( text, qfalse );
    if ( !tok[0] ) {
        COM_ParseError( "missing parameter of type string for '%s' in module info file", name );
    }

    N_strncpyz( str, tok, maxLength );
}

static int GetIntParm( const char *name, const char **text ) {
    const char *tok;

    tok = COM_ParseExt( text, qfalse );
    if ( !tok[0] ) {
        COM_ParseError( "missing parameter of type integer for '%s' in module info file", name );
    }

    return atoi( tok );
}

static float GetFloatParm( const char *name, const char **text ) {
    const char *tok;

    tok = COM_ParseExt( text, qfalse );
    if ( !tok[0] ) {
        COM_ParseError( "missing parameter of type float for '%s' in module info file", name );
    }

    return atof( tok );
}

static qboolean LoadMobInfos( int currentModule )
{
    const char *text_p, **text;
    const char *tok;
    mobinfo_t info;
    moduleInfo_t *m;

    m = &sg_modules[currentModule];

    text_p = SG_LoadFile( va( "modules/%s/mobs.txt", sg_moduleData[currentModule].name ) );
    text = &text_p;

    G_Printf( "- Loading mob infos for \"%s\"...\n", sg_moduleData[currentModule].name );

    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );

        if ( !tok[0] ) {

        }

        if ( tok[0] == '{' ) {
            while ( 1 ) {
                tok = COM_ParseExt( text, qtrue );
                if ( !tok[0] ) {
                }
            }
            ML_BufferAppend( sg_moduleInfos.modules[currentModule].mobs, sizeof(mobinfo_t), &info );
        }
        
    }
}

static qboolean LoadItemInfos( int currentModule )
{
    const char *text_p, *text_end;
    iteminfo_t info;
    const char *json;

    text_p = SG_LoadFile( va( "modules/%s/items.json", sg_moduleData[currentModule].name ) );
    text_end = text_p + strlen( text_p ) + 1;

    G_Printf( "- Loading item infos for \"%s\"...\n", sg_moduleData[currentModule].name );

    if ( JSON_ValueGetType( text_p, text_end ) != JSONTYPE_OBJECT ) {
        SG_Printf( "" );
        return qfalse;
    }

    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );

        if ( !tok[0] ) {

        }

        if ( tok[0] == '{' ) {
            while ( 1 ) {
                tok = COM_ParseExt( text, qtrue );
                if ( !tok[0] ) {
                    
                }

                if ( !N_stricmp( tok, "name" ) ) {
                    info.name = GetStringToken( "item_name", text );
                } else if ( !N_stricmp( tok, "cost" ) ) {
                    info.cost = GetIntParm( "item_cost", text );
                } else if ( !N_stricmp( tok, "icon" ) ) {
                    info.hIconShader = SG_LoadResource( GetString( "item_iconShader", text ), RE_RegisterShader );
                } else {
                    COM_ParseWarning( "unrecognized token '%s' in item info", tok );
                }
            }
            sg_moduleInfos.modules[currentModule].loadedItems++;
            ML_BufferAppend( sg_moduleInfos.modules[currentModule].items, sizeof(iteminfo_t), &info );
        }
        
    }
}

static qboolean LoadWeaponInfos( int currentModule )
{
    const char *text_p, **text;
    const char *tok;
    weaponinfo_t info;
    module_t m;

    ML_GetBufferData( sg_moduleInfos.modules, sizeof(m) * currentModule, sizeof(m) );
    m = &sg_moduleInfos.modules[currentModule];

    text_p = SG_LoadFile( va( "modules/%s/weapons.txt", sg_moduleData[currentModule].name ) );
    text = &text_p;

    SG_Printf( "- Loading weapon infos...\n" );

    info = sg_moduleInfos.modules[currentModule].weapons;
    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );

        if ( !tok[0] ) {

        }

        if ( tok[0] == '{' ) {
            while ( 1 ) {
                tok = COM_ParseExt( text, qtrue );
                if ( !tok[0] ) {
                    COM_ParseError( "unexpected end of weapon info definition in module \"%s\"!", m->name );
                }

                if ( !N_stricmp( tok, "damage" ) ) {
                    info->damage = ;
                } else if ( !N_stricmp( tok, "range" ) ) {
                    info->range = GetIntParm( "weapon_range", text );
                } else if ( !N_stricmp( tok, "accuracy" ) ) {
                    info->accuracy = GetFloatParm( "weapon_accuracy", text );
                } else {
                    COM_ParseWarning( "unrecognized token '%s' in weapon info definition" );
                }

                sg_moduleInfos.modules[currentModule].loadedWeapons++;
                info++;
            }
        }
    }
}

static qboolean LoadPowerupInfos( int currentModule )
{
    const char *text_p, **text;
    const char *tok;
    powerupinfo_t *info;
    moduleInfo_t *m;

    m = &sg_modules[currentModule];

    text_p = SG_LoadFile( va( "modules/%s/powerups.txt", sg_moduleData[currentModule].name ) );
    text = &text_p;

    info = sg_moduleInfos.modules[currentModule].powerups;
    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );

        if ( !tok[0] ) {

        }
        
    }
}

static qboolean LoadDamageTypes( const char *moduleName )
{
    const char *text_p, **text;
    const char *tok;
    damageType_t *info;

    text_p = SG_LoadFile( va( "modules/%s/damage_types.txt", moduleName ) );
    text = &text_p;

    info = info = sg_moduleInfos.modules[currentModule].damage_types;
    while ( 1 ) {
        tok = COM_ParseExt( text, qfalse );
        
        if ( !tok ) {
            break;
        }

        // TODO: allow CSV parsing for multiple random strings, only for the obituary
    }
}

static qboolean LoadModuleConfig( const char *moduleName )
{
    const char *text_p, *text_end;
    const char *tok;
    module_t *m;
    const char *json;
    const char *jsonArray;

    text_p = SG_LoadFile( va( "modules/%s/module.json", moduleName ) );
    text_end = text_p + strlen( text_p ) + 1;
    m = &sg_moduleInfos.modules[sg_moduleInfos.count];

    if ( JSON_ValueGetType( text_p, text_end ) != JSONTYPE_OBJECT ) {
        SG_Printf( COLOR_RED "ERROR: invalid module configuration, not a proper json file!\n" );
        return qfalse;
    }

    JSON_GetString( text_p, text_end, "name", m->name, sizeof(m->name) );

    json = JSON_ObjectGetNamedValue( text_p, text_end, "version" );
    if ( !json ) {
        
    }
    m->modVersionMajor = JSON_GetInt( json, text_end, "version_major" );
    m->modVersionUpdate = JSON_GetInt( json, text_end, "version_update" );
    m->modVersionPatch = JSON_GetInt( json, text_end, "version_patch" );

    m->gameVersionMajor = JSON_GetInt( json, text_end, "game_version_major" );
    m->gameVersionUpdate = JSON_GetInt( json, text_end, "game_version_update" );
    m->gameVersionPatch = JSON_GetInt( json, text_end, "gameVersionPatch" );

    json = JSON_ObjectGetNamedValue( text_p, text_end, "dependencies" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {
        if ( m->numDependencies >= MAX_MODULE_DEPENDENCIES ) {
            SG_Printf( COLOR_YELLOW "WARNING: too many dependencies in module \"%s\"!\n", moduleName );
            break;
        }
        N_strncpyz( m->dependencies[m->numDependencies], jsonArray, MAX_MODULE_NAME );
        m->numDependencies++;
    }

    json = JSON_ObjectGetNamedValue( text_p, text_end, "mobs" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {

    }

    json = JSON_ObjectGetNamedValue( text_p, text_end, "items" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {
        
    }

    json = JSON_ObjectGetNamedValue( text_p, text_end, "weapons" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {
        
    }

    json = JSON_ObjectGetNamedValue( text_p, text_end, "powerups" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {
        
    }

    json = JSON_ObjectGetNamedValue( text_p, text_end, "bots" );
    if ( !json ) {

    }
    for ( jsonArray = JSON_ArrayGetFirstValue( json, text_end ); jsonArray; jsonArray = JSON_ArrayGetNextValue( jsonArray, text_end ) ) {
        
    }

    return qtrue;
}

//
// GetModuleDependencies: loads a module's dependencies before the module
// itself
//
static void GetModuleDependencies( module_t *m )
{
    int i;

    SG_Printf( "Checking dependencies for \"%s\"...\n", m->name );

    ML_GetModulesInfo( MODULE_INFO_DEPENDENCIES, m->dependencies, MAX_MODULE_DEPENDENCIES );
    for ( i = 0; i < MAX_MODULE_DEPENDENCIES; i++ ) {
        if ( !*m->dependencies[i] ) {
            break;
        }
        m->numDependencies++;
    }

    SG_Printf( "... %i total dependencies found.\n", m->numDependencies );
}

static void AddModule( module_t *m )
{
    int i;

    // in the case that this is called for a dependency, check
    // if we've already loaded it
    for ( i = 0; i < sg_moduleInfos.count; i++ ) {
        if ( !N_stricmp( m->name, sg_moduleInfos.modules[i].name ) ) {
            return;
        }
    }

    sg_moduleInfos.count++;

    SG_Printf( "Loading module \"%s\"...\n", m->name );

    GetModuleDependencies( m );
    ML_LoadModuleCode( m->name );
    ML_CreateModuleDataLink( m->name );
    
    for ( i = 0; i < m->numDependencies; i++ ) {
        AddModule( &sg_moduleInfos.modules[sg_moduleInfos.count] );
        SG_Printf( "Loaded dependency \"%s\".", m->dependencies[i] );
    }
}

static void LoadInfos( const char *moduleName )
{
    G_Printf( "Loading info files for \"%s\"...\n", moduleName );
}

void SG_LoadModules( void )
{
    int i;
    char moduleName[MAX_MODULE_COUNT][MAX_MODULE_NAME];


    SG_Printf( "Getting module data...\n" );

    memset( &sg_moduleInfos, 0, sizeof(sg_moduleInfos) );

    //
    // fetch & allocate data
    //
    ML_GetModulesInfo( MODULE_INFO_COUNT, &sg_moduleInfos.total, sizeof(int) );
    ML_GetModulesInfo( MODULE_INFO_NAMES, moduleNames, MAX_MODULE_NAME );
    sg_moduleInfos.modules = SG_MemAlloc( sizeof(module_t) * sg_moduleInfos.total );

    SG_Printf( "Got %i total modules, loading..\n", sg_moduleInfos.total );

    for ( i = 0; i < sg_moduleInfos.total; i++ ) {
        N_strncpyz( sg_moduleInfos.modules[i].name, moduleNames, MAX_MODULE_NAME );

        AddModule( &sg_moduleInfos.modules[i] );
    }
}
