#include "sg_local.h"

sgentity_t sg_entities[MAXENTITIES];

// Use a heuristic approach to detect infinite state cycles: Count the number
// of times the loop in Ent_SetState() executes and exit with an error once
// an arbitrary very large limit is reached.

#define ENTITY_CYCLE_LIMIT 100000

qboolean Ent_SetState(sgentity_t *self, statenum_t state)
{
	state_t *st;
	uint32_t counter;
	
	do {
		if (state == ST_NULL) {
			self->state = &stateinfo[ ST_NULL ];
			SG_FreeEntity(self);
			return qfalse;
		}
		
		st = &stateinfo[state];
		self->state = st;
		self->ticker = st->ticcount;
		self->sprite = st->sprite;
		
		state = st->next;
		if (counter++ > ENTITY_CYCLE_LIMIT) {
			G_Error("Ent_SetState: infinite state cycle detected!");
		}
	} while (!self->ticker);
	
	return qtrue;
}

void SG_BuildBounds( sgentity_t *ent )
{
	ent->bounds.mins[0] = ent->origin[0] - ent->height;
	ent->bounds.mins[1] = ent->origin[1] - ent->width;
	ent->bounds.mins[2] = ent->origin[2];

	ent->bounds.maxs[0] = ent->origin[0] + ent->height;
	ent->bounds.maxs[1] = ent->origin[1] + ent->width;
	ent->bounds.maxs[2] = ent->origin[2];
}

void SG_FreeEntity( sgentity_t *ent )
{
	if ( !ent->health ) {
		trap_Print( COLOR_YELLOW "WARNING: SG_FreeEntity: freed a freed entity" );
		return;
	}

	sg.numEntities--;
}

sgentity_t *SG_AllocEntity( entitytype_t type )
{
	sgentity_t *ent;

	if ( sg.numEntities == MAXENTITIES ) {
		trap_Error( "SG_AllocEntity: MAXENTITIES hit" );
	}

	ent = &sg_entities[sg.numEntities];
	sg.numEntities++;

	memset( ent, 0, sizeof(*ent) );
	ent->type = type;
	ent->ticker = -1;
	ent->width = ent->height = 0;

	SG_BuildBounds( ent );
}

void SG_InitEntities( void ) {
	memset( sg_entities, 0, sizeof(sg_entities) );
	sg.numEntities = 0;
}

