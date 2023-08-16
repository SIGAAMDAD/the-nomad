#include "n_shared.h"

static cvar_t *cvar_vars = NULL;

#define MAX_CVARS 2048
static cvar_t cvar_indexes[MAX_CVARS];
static uint64_t cvar_numIndexes;

static uint64_t cvar_group[CVG_MAX];

#define MAX_CVAR_HASH 256
static cvar_t *hashTable[MAX_CVAR_HASH];
static qboolean cvar_sort = qfalse;


static cvar_t *c_cheatsAllowed;
static cvar_t *c_devmode;

#define Cvar_HashName(x) Com_GenerateHashValue((x),MAX_CVAR_HASH)

static cvar_t *Cvar_Find(const char *name)
{
    cvar_t *cvar;
    uint64_t hash;

    if (!name)
        return NULL;
    
    hash = Cvar_HashName(name);

    for (cvar = hashTable[hash]; cvar; cvar = cvar->hashNext) {
        if (!N_stricmp(name, cvar->name)) {
            return cvar;
        }
    }
    return NULL;
}

static qboolean Cvar_IsIntegral(const char *s)
{
    if (*s == '-' && *(s+1) == '\0')
        s++;
    
    while (*s != '\0') {
        if (*s < '0' || *s > '9')
            return qfalse;
        
        s++;
    }
    return qtrue;
}

static const char *Cvar_Validate(cvar_t *cvar, const char *value, qboolean warn)
{
    static char intbuf[32];
    const char *limit;
    float valuef;
    int32_t valuei;

    if (cvar->type = CVT_NONE)
        return value;
    if (!value)
        return value;
    
    limit = NULL;
    
    if (cvar->type == CVT_INT || cvar->type == CVT_FLOAT) {
        if (!N_isanumber(value)) {
            if (warn)
                Con_Printf(WARNING, "cvar '%s' must be numeric", cvar->name);
            
            limit = cvar->s;
        }
        else {
            if (cvar->type == CVT_INT) {
                if (!Cvar_IsIntegral(value)) {
                    if (warn)
                        Con_Printf(WARNING, "cvar '%s' must be integral", cvar->name);
                    
                    sprintf(intbuf, "%i", atoi(value));
                    value = intbuf;
                }
                valuei = atoi(value);
            }
            // CVT_FLOAT
            else {
                valuef = N_atof(value);
            }
        }
    }
    // TODO: add in filesystem paths for cvars
    if (limit || value == intbuf) {
        if (!limit)
            limit = value;
        if (warn)
            Con_Printf(", setting to '%s", limit);
        
        return limit;
    }

    return value;
}

static void Cvar_Print(const cvar_t *cv)
{
    Con_Printf("\"%s\" is:\"%s\"", cv->name, cv->s);

//    if (!(cv->flags & CVAR_ROM)) {
//        Con_Printf(" default:\"%s" C_WHITE "\"", cv->);
//    }
    Con_Printf(" ");

    if (cv->description) {
        Con_Printf("%s", cv->description);
    }
}


int32_t Cvar_VariableInteger(const char *name)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (!cv)
        return 0;
    
    return cv->i;
}

float Cvar_VariableFloat(const char *name)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (!cv)
        return 0.0f;
    
    return cv->f;
}

const char *Cvar_VariableString(const char *name)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (!cv)
        return "";

    return cv->s;
}

uint32_t Cvar_Flags(const char *name)
{
    const cvar_t *cv;

    if ((cv = Cvar_Find(name)) == NULL)
        return CVAR_NONEXISTENT;
    else {
        if (cv->modified)
            return cv->flags | CVAR_MODIFIED;
        else
            return cv->flags;
    }
}

qboolean Cvar_VariableBoolean(const char *name)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (!cv)
        return qfalse;
    
    return cv->b;
}

void Cvar_VariableStringBuffer(const char *name, char *buffer, uint64_t bufferSize)
{
    cvar_t *cvar;

    cvar = Cvar_Find(name);
    if (!cvar)
        *buffer = '\0';
    else
        N_strncpyz(buffer, cvar->s, bufferSize);
}

void Cvar_VariableStringBufferSafe(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag)
{
    cvar_t *cvar;

    cvar = Cvar_Find(name);
    if (!cvar || cvar->flags & flag)
        *buffer = '\0';
    else
        N_strncpyz(buffer, cvar->s, bufferSize);
}

static qboolean Cvar_ValidateName(const char *name)
{
    const char *s;
    int c;

    if (!name)
        return qfalse;
    
    s = name;
    while ((c = *s++) != '\0') {
        if (c == '\\' || c == '\"')
            return qfalse;
    }
    if (strlen(name) >= MAX_STRING_CHARS)
        return qfalse;
    
    return qtrue;
}

cvar_t *Cvar_Set2(const char *name, const char *value)
{
    cvar_t *cvar;
#ifdef _NOMAD_DEBUG
    const qboolean debug = qtrue;
#else
    const qboolean debug = qfalse;
#endif

    if (!Cvar_ValidateName(name)) {
        Con_Printf("invalid cvar name string: %s", name);
        name = "BADNAME";
    }

    cvar = Cvar_Find(name);
    if (!cvar) {
        if (!value)
            return NULL;
        // create it
        return Cvar_Get(name, value, CVAR_USER_CREATED);
    }

    if (cvar->flags & (CVAR_ROM | CVAR_CHEAT | CVAR_DEV) && !Cvar_VariableBoolean("c_devmode") && !debug) {
        if (cvar->flags & CVAR_ROM) {
            Con_Printf("%s is read only", cvar->name);
            return cvar;
        }
        if ((cvar->flags & CVAR_CHEAT) && !Cvar_VariableBoolean("c_cheatsAllowed")) {
            Con_Printf("%s is cheat protected", cvar->name);
            return cvar;
        }
        if ((cvar->flags & CVAR_DEV)) {
            Con_Printf("%s can be set only in developer mode", cvar->name);
            return cvar;
        }
    }
    if (!value)
        value = cvar->s;
    
    value = Cvar_Validate(cvar, value, qtrue);
    if (strcmp(value, cvar->s) == 0)
        return cvar;
    
    cvar->modified = qtrue;
    cvar_group[cvar->group] = 1;

    Z_Free(cvar->s); // free the old value string

    cvar->s = Z_Strdup(value);
    cvar->i = atoi(cvar->s);
    cvar->f = N_atof(cvar->s);
    
    return cvar;
}

void Cvar_Set(const char *name, const char *value)
{
    Cvar_Set2(name, value);
}

void Cvar_SetSafe(const char *name, const char *value)
{
    uint32_t flags = Cvar_Flags(name);
    
    if (flags != CVAR_NONEXISTENT) {
        if (flags & (CVAR_PRIVATE)) {
            if (value)
                Con_Printf(WARNING, "Restricted source tried to set \"%s\" to \"%s\"", name, value);
            else
                Con_Printf(WARNING, "Restricted source tried to modify \"%s\"", name);
            return;
        }
    }

    Cvar_Set2(name, value);
}

cvar_t *Cvar_Get(const char *name, const char *value, uint32_t flags)
{
    cvar_t *cvar;
    uint64_t hash;
    uint64_t index;

    if (!name || !value)
        N_Error("Cvar_Get: NULL parameter");
    if (!Cvar_ValidateName(name)) {
        Con_Printf("invalid cvar name string: %s", name);
        name = "BADNAME";
    }

    cvar = Cvar_Find(name);
    if (cvar) {
        qboolean vm_created = (qboolean)(flags & CVAR_VM_CREATED);
        value = Cvar_Validate(cvar, value, qfalse);

        // make sure the game code cannot mark engine-added variables as gamecode vars
        if (cvar->flags & CVAR_VM_CREATED) {
            if (!vm_created)
                cvar->flags &= ~CVAR_VM_CREATED;
        }
        else if (!(cvar->flags & CVAR_USER_CREATED)) {
            if (vm_created)
                flags &= ~CVAR_VM_CREATED;
        }

        // if the vm code is now specifying a variable that the user already
        // set a value for, take the new value as the reset value
        if (cvar->flags & CVAR_USER_CREATED) {
            cvar->flags &= ~CVAR_USER_CREATED;
        }
    }

    // allocate a new cvar

    // find a free cvar
    for (index = 0; index < MAX_CVARS; index++) {
        if (!cvar_indexes[index].name)
            break;
    }
    if (index >= MAX_CVARS) {
        N_Error("Too many cvars, cannot create a new one!");
    }

    cvar = &cvar_indexes[index];
    if (index >= cvar_numIndexes)
        cvar_numIndexes = index + 1;
    
    cvar->name = Z_Strdup(name);
    cvar->s = Z_Strdup(value);
    cvar->modified = qtrue;
    cvar->f = N_atof(cvar->s);
    cvar->i = atoi(cvar->s);
    cvar->type = CVT_NONE;
    cvar->description = NULL;
    cvar->group = CVG_NONE;
    cvar_group[cvar->group] = 1;

    // link the variable in
    cvar->next = cvar_vars;
    if (cvar_vars)
        cvar_vars->prev = cvar;
    
    cvar->prev = NULL;
    cvar_vars = cvar;

    cvar->flags = flags;

    hash = Cvar_HashName(name);
    cvar->hashIndex = hash;

    cvar->hashNext = hashTable[hash];
    if (hashTable[hash])
        hashTable[hash]->hashPrev = cvar;
    
    cvar->hashPrev = NULL;
    hashTable[hash] = cvar;

    // sort on write
    cvar_sort = qtrue;

    return cvar;
}

void Cvar_CommandCompletion(void (*callback)(const char *s))
{
    const cvar_t *cvar;

    for (cvar = cvar_vars; cvar; cvar = cvar->hashNext) {
        if (cvar->name && !(cvar->flags & CVAR_NOTABCOMPLETE)) {
            callback(cvar->name);
        }
    }
}



const char *Cvar_GetValue(const cvar_t* cvar);


void Cvar_Reset(const char *name)
{
    Cvar_Set2(name, NULL);
}



qboolean Cvar_Command(void)
{
    cvar_t *cvar;

    // check variables
    cvar = Cvar_Find(Cmd_Argv(0));
    if (!cvar) {
        return qfalse;
    }

    // perform a variable print or set
    if (Cmd_Argc() == 1) {
        Cvar_Print(cvar);
        return qtrue;
    }

    // set the value
    Cvar_Set(cvar->name, Cmd_ArgsFrom(1));
    return qtrue;
}

static void Cvar_Print_f(void)
{
	const char *name;
	cvar_t *cv;
	
	if (Cmd_Argc() != 2) {
		Con_Printf("usage: print <variable>");
		return;
	}

	name = Cmd_Argv(1);

	cv = Cvar_Find(name);
	
	if (cv)
		Cvar_Print(cv);
	else
		Con_Printf("Cvar '%s' does not exist", name);
}

void Cvar_SetValueSafe(const char *name, float value)
{
    char val[32];

    if (N_isintegral(value))
        snprintf(val, sizeof(val), "%i", (int)value);
    else
        snprintf(val, sizeof(val), "%f", value);
    
    Cvar_SetSafe(name, val);
}

qboolean Cvar_SetModified(const char *name, qboolean modified)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (cv) {
        cv->modified = modified;
        return qtrue;
    }
    
    return qfalse;
}

void Cvar_SetIntegerValue( const char *name, int32_t value )
{
	char val[32];
	snprintf( val, sizeof(val), "%i", value );
	Cvar_Set( name, val );
}

void Cvar_SetStringValue(const char *name, const char *value)
{
    Cvar_Set(name, value);
}

void Cvar_SetBooleanValue(const char *name, qboolean value)
{
    const char *s = N_booltostr(value);
    Cvar_Set(name, s);
}

void Cvar_SetFloatValue(const char *name, float value)
{
    char val[32];
    snprintf(val, sizeof(val), "%f", value);    
    Cvar_Set(name, val);
}

/*
Cvar_Update: updates an interpreted modules' version of a cvar
*/
void Cvar_Update(vmCvar_t *vmCvar, int privateFlag)
{
    uint64_t len;
    cvar_t *cv = NULL;
    assert(vmCvar);

    if ((unsigned)vmCvar->handle >= cvar_numIndexes) {
        Con_Error(false, "Cvar_Update: handle out of range");
        return;
    }

    cv = cvar_indexes + vmCvar->handle;

    if (!cv->s)
        return; // variable might have been cleared by Cvar_Restart
    
    if (cv->flags & CVAR_PRIVATE) {
        if (privateFlag)
            return;
    }

    len = strlen(cv->s);
    if (len + 1 > MAX_CVAR_VALUE) {
        Con_Printf(WARNING, "Cvar_Update: src %s length %lu exceeds MAX_CVAR_VALUE - truncate", cv->s, len);
    }

    N_strncpyz(vmCvar->s, cv->s, sizeof(vmCvar->s));
    vmCvar->f = cv->f;
    vmCvar->i = cv->i;
}

#define INVALID_FLAGS (CVAR_USER_CREATED | CVAR_DEV | CVAR_MODIFIED | CVAR_NONEXISTENT | CVAR_PRIVATE)
void Cvar_Register(vmCvar_t *vmCvar, const char *name, const char *value, uint32_t flags)
{
    cvar_t *cv;

    if ((flags & (CVAR_SAVE | CVAR_ROM)) == (CVAR_SAVE | CVAR_ROM)) {
        Con_Printf(WARNING, "Unsetting CVAR_ROM from cvar '%s', since it is also CVAR_SAVE", name);
        flags &= ~CVAR_ROM;
    }

    // don't allow VM to specify a different creator or other internal flags
    if (flags & INVALID_FLAGS) {
        Con_Printf(WARNING, "VM tried to set invalid flags 0x%02x on cvar '%s'", (flags & INVALID_FLAGS), name);
        flags &= ~INVALID_FLAGS;
    }

    cv = Cvar_Find(name);

    // don't modify cvar if it's protected
    if (cv && (cv->flags & (CVAR_PRIVATE))) {
        Con_Printf(WARNING, "VM tried to register protected cvar '%s' with value '%s'%s",
            name, value, (flags & ~cv->flags) != 0 ? " and new flags" : "");
        if (cv->flags & CVAR_PRIVATE) {
            return;
        }
    }
    else {
        cv = Cvar_Get(name, value, flags | CVAR_VM_CREATED);
    }

    if (!vmCvar)
        return;
    
    vmCvar->handle = cv - cvar_indexes;
    vmCvar->modificationCount = -1;
    
    Cvar_Update(vmCvar, 0);
}

#define DEFAULT_CFG_NAME "default.cfg"

static void Cvar_Sort_f(void);

static void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    file_t out = FS_FOpenWrite("default.scf");
    if (out == FS_INVALID_HANDLE) {
        N_Error("FS_FOpenWrite: failed to open write stream for default.scf");
    }
    char buffer[1024];
    for (cvar = cvar_vars; cvar; cvar = cvar->next) {
        if (!(cvar->flags & CVAR_SAVE) || !cvar->name)
            continue; // don't write it
    
        FS_Printf(out, "sets %s\n", cvar->name);
    }
    FS_FClose(out);
}

static void Cvar_List_f(void)
{
    cvar_t *var;
	uint64_t i;
	const char *match;

	// sort to get more predictable output
	if ( cvar_sort ) {
		cvar_sort = qfalse;
		Cvar_Sort_f();
	}

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    memset(buffer, ' ', 5);
	for (var = cvar_vars ; var ; var = var->next, i++) {
		if(!var->name || (match && !Com_Filter(match, var->name)))
			continue;
        
        if (var->flags & CVAR_SAVE)
            buffer[0] = 'S';
        if (var->flags & CVAR_CHEAT)
            buffer[1] = 'C';
        if (var->flags & CVAR_DEV) 
            buffer[2] = 'D';
        if (var->flags & CVAR_ROM)
            buffer[3] = 'R';
        if (var->flags & CVAR_USER_CREATED)
            buffer[4] = 'U';

		Con_Printf ("%s %s \"%s\"", buffer,  var->name, var->s);
	}

	Con_Printf ("\n%lu total cvars", i);
	Con_Printf ("%lu cvar indexes", cvar_numIndexes);
}

static void Cvar_Set_f(void)
{
    uint32_t c;
    const char *cmd;
    cvar_t *cvar;

    c = Cmd_Argc();
    cmd = Cmd_Argv(0);

    if (c < 2) {
        Con_Printf("usage: %s <variable>  <value>", cmd);
        return;
    }
    if (c == 2) {
        Cvar_Print_f();
        return;
    }

    cvar = Cvar_Set2(Cmd_Argv(1), Cmd_ArgsFrom(2));
    if (!cvar)
        return;
    switch (cmd[3]) {
    case 's':
        if (!(cvar->flags & CVAR_SAVE))
            cvar->flags |= CVAR_SAVE;
        break;
    case 'u':
        if (!(cvar->flags & CVAR_USER_CREATED))
            cvar->flags |= CVAR_USER_CREATED;
        break;
    case 'd':
        if (!Cvar_VariableBoolean("c_devmode")) {
            Con_Printf("cannot set cvar '%s' to developer cvar when not in developer mode", cvar->name);
            return;
        }
        if (!(cvar->flags & CVAR_DEV))
            cvar->flags |= CVAR_DEV;
        break;
    };
}

static void Cvar_Reset_f(void)
{
    if (Cmd_Argc() != 2) {
        Con_Printf("usage: reset <variable>");
        return;
    }
    Cvar_Reset(Cmd_Argv(1));
}

static void Cvar_QSortByName( cvar_t **a, uint64_t n )
{
	cvar_t *temp;
	cvar_t *m;
	int32_t i, j;

	i = 0;
	j = n;
	m = a[ n>>1 ];

	do {
		// sort in descending order
		while ( strcmp( a[i]->name, m->name ) > 0 ) i++;
		while ( strcmp( a[j]->name, m->name ) < 0 ) j--;

		if ( i <= j ) {
			temp = a[i]; 
			a[i] = a[j]; 
			a[j] = temp;
			i++; 
			j--;
		}
	} while ( i <= j );

	if ( j > 0 ) Cvar_QSortByName( a, j );
	if ( n > i ) Cvar_QSortByName( a+i, n-i );
}


static void Cvar_Sort_f( void ) 
{
	cvar_t *list[ MAX_CVARS ], *var;
	uint64_t count;
	uint64_t i;

	for ( count = 0, var = cvar_vars; var; var = var->next ) {
		if ( var->name ) {
			list[ count++ ] = var;
		} else {
			N_Error( "%s: NULL cvar name", __func__ );
		}
	}

	if ( count < 2 ) {
		return; // nothing to sort
	}

	Cvar_QSortByName( &list[0], count-1 );
	
	cvar_vars = NULL;

	// relink cvars
	for ( i = 0; i < count; i++ ) {
		var = list[ i ];
		// link the variable in
		var->next = cvar_vars;
		if ( cvar_vars )
			cvar_vars->prev = var;
		var->prev = NULL;
		cvar_vars = var;
	}
}

void Cvar_SetDescription(cvar_t *cv, const char *description)
{
    if (description && description[0] != '\0') {
        if (cv->description != NULL)
            Z_Free(cv->description);
        
        cv->description = Z_Strdup(description);
    }
}

void Cvar_SetGroup(cvar_t *cv, cvarGroup_t group)
{
    if (group < CVG_MAX)
        cv->group = group;
    else
        Con_Printf(ERROR, "bad group index %i for %s", group, cv->name);
}

void Cvar_CompleteCvarName(const char *args, uint32_t argnum)
{
    if (argnum == 2) {
        // skip "<cmd> "
        const char *p = Com_SkipTokens(args, 1, " ");

    }
}

void Cvar_Init(void)
{
    memset(cvar_indexes, 0, sizeof(cvar_indexes));
    memset(hashTable, 0, sizeof(hashTable));

    c_devmode = Cvar_Get("c_devmode", "0", CVAR_DEV | CVAR_ROM);
    Cvar_SetDescription(c_devmode, "Toggles developer mode. Pendantic and detailed logging, and provides a handful more commands");
    c_cheatsAllowed = Cvar_Get("c_cheatsAllowed", "0", CVAR_ROM | CVAR_DEV | CVAR_SAVE);
    Cvar_SetDescription(c_cheatsAllowed, "Enable cheats");

    Cmd_AddCommand("print", Cvar_Print_f);
    Cmd_AddCommand("set", Cvar_Set_f);
    Cmd_AddCommand("sets", Cvar_Set_f);
    Cmd_AddCommand("setd", Cvar_Set_f);
    Cmd_AddCommand("setu", Cvar_Set_f);
//    Cmd_AddCommand("unset", Cvar_Unset_f);
    Cmd_AddCommand("reset", Cvar_Reset_f);
    Cmd_AddCommand("cvarlist", Cvar_List_f);
}