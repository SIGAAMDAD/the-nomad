#include "sg_local.h"

typedef enum {
    GFX_EXPLOSION,
    GFX_MARK,
    GFX_BLOODSTAIN,
    GFX_BLOODSPLATTER,
    GFX_BULLET,
    GFX_DUST,
    
    NUMGFXTYPES
} gfxType_t;

typedef struct gfx_s {
    refEntity_t refEntity;
    vec3_t color;
    vec3_t origin;
    vec3_t vel;
    struct gfx_s *next, *prev;
    int type;
    int endTime;
} gfx_t;

static gfx_t *sg_gfx;
static gfx_t sg_activeGfx;		// double linked list
static gfx_t *sg_freeGfx;		// single linked list

static int sg_numGfx;

//
// SG_InitGfx: this is called at startup
//
void SG_InitGfx( void ) {
	int	i;

    SG_Printf( "Allocating %i gfx...\n", sg_maxGfx.i );

    sg_gfx = SG_MemAlloc( sizeof(*sg_gfx) * sg_maxGfx.i );
    sg_numGfx = 0;

    memset( sg_gfx, 0, sizeof(sg_gfx) );
    sg_activeGfx.next = &sg_activeGfx;
    sg_activeGfx.prev = &sg_activeGfx;
    sg_freeEnts = sg_gfx;
	for ( i = 0; i < MAX_GFX - 1; i++ ) {
		sg_gfx[i].next = &sg_gfx[i+1];
	}
}

//
// SG_FreeGfx
//
void SG_FreeGfx( gfx_t *gfx ) {
	if ( !gfx->prev ) {
		SG_Error( "SG_FreeGfx: not active" );
	}

	// remove from the doubly linked active list
	gfx->prev->next = gfx->next;
	gfx->next->prev = gfx->prev;

	// the free list is only singly linked
	gfx->next = sg_freeGfx;
	sg_freeGfx = le;

    sg_numGfx--;
}

//
// SG_AllocGfx: Will allways succeed, even if it requires freeing an old active entity
//
gfx_t *SG_AllocGfx( void ) {
	gfx_t *gfx;

	if ( !sg_freeGfx ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		SG_FreeGfx( sg_activeGfx.prev );
	}

	gfx = sg_freeGfx;
	sg_freeGfx = sg_freeGfx->next;

	memset( gfx, 0, sizeof(*gfx) );

	// link into the active list
	gfx->next = sg_activeGfx.next;
	gfx->prev = &sg_activeGfx;
	sg_activeGfx.next->prev = le;
	sg_activeGfx.next = le;

    sg_numGfx++;

	return gfx;
}

//
// SG_Blood
//
void SG_Blood( const vec3_t *origin )
{
    gfx_t *gfx;

    gfx = SG_AllocGfx();
    gfx->type = GFX_BLOODSPLATTER;
    gfx->endTime = sg.levelTime + sg_gfxTime.i;
    VectorCopy( gfx->origin, *origin );

    gfx->refEntity.hShader = random() & 4;
}

//
// SG_DustTrail
//
void SG_DustTrail( const vec3_t *origin )
{
    gfx_t *gfx;

    gfx = SG_AllocGfx();
    gfx->type = GFX_DUST;
    gfx->endTime = sg.levelTime + sg_gfxTime.i;
    VectorCopy( gfx->origin, *origin );

    gfx->color.x = 0.75f;
    gfx->color.y = 0.5f;
    gfx->color.z = 0.0f;
}

void SG_AddExplosionGfx( const vec3_t *origin )
{
    gfx_t *gfx;

    gfx = SG_AllocGfx();
    gfx->type = GFX_EXPLOSION;
    gfx->endTime = sg.levelTime + sg_gfxTime.i;
    VectorCopy( gfx->origin, *origin );
}

static void SG_AddExplosion( gfx_t *gfx )
{
    polyVert_t verts[4];
}

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;

	trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = le->radius * ( 1.0 - c ) + 8;
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];
	re->radius = le->radius * ( 1.0 - c ) + 8;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}

//
// SG_AddFallScaleFade: This is just an optimized SG_AddMoveScleFade
// For blood mists that drift down, fade out, and are
// removed if the view passes through them.
// There are often 100+ of these, so it needs to be simple.
//
static void SG_AddFallScaleFade( gfx_t *gfx ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

    re = &gfx->refEntity;

	// fade time
	c = ( gfx->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	re->origin[2] = le->pos.trBase[2] - ( 1.0 - c ) * le->pos.trDelta[2];

	re->radius = le->radius * ( 1.0 - c ) + 16;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if ( len < le->radius ) {
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}

void SG_AddGfxEntities( void )
{
    gfx_t *gfx, *next;

	// walk the list backwards, so any new gfx entities generated
	// (trails, marks, etc) will be present this frame
	gfx = sg_activeGfx.prev;
	for ( ; gfx != &sg_activeGfx; gfx = next ) {
		// grab next now, so if the gfx entity is freed we
		// still have it
		next = gfx->prev;

		if ( sg.levelTime >= gfx->endTime ) {
			SG_FreeGfx( gfx );
			continue;
		}

        switch ( gfx->type ) {
        default:
            SG_Error( "SG_AddGfxEntities: bad type %i", gfx->type );
            break;
        case GFX_BULLET:
            break;
        case GFX_BLOODSPLATTER:
            break;
        case GFX_BLOODSTAIN:
            break;
        case GFX_DUST:
            break;
        case GFX_EXPLOSION:
            break;
        case GFX_MARK:
            break;
        };
    }
}
