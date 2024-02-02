#include "sg_local.h"
#include "sg_imgui.h"

typedef struct {
    int polyCount;
    int vertexCount;
    float realCamWidth;
    float realCamHeight;
    bbox_t frustum;
} drawdata_t;

static drawdata_t data;

static qboolean EntityIsInView( const bbox_t *bounds ) {
    return BoundsIntersect( bounds, &data.frustum );
}

static void SG_CalcVerts( const sgentity_t *ent, polyVert_t *verts )
{
    trap_Error( "SG_CalcVerts: called" );
    // FIXME: implement
}

static void SG_DrawEntity( const sgentity_t *ent )
{
    polyVert_t verts[4];

    // is it visible?
//    if ( !EntityIsInView( &ent->bounds ) ) {
//        return;
//    }

    data.polyCount++;
    data.vertexCount += 4;

//    SG_CalcVerts( ent, verts );

    RE_AddPolyToScene( ent->hShader, verts, 4 );
}

static void SG_AddSpritesToFrame( void )
{
    const sgentity_t *ent;
    int i;

    ent = &sg_entities[0];
    for ( i = 0; i < sg.numEntities; i++, ent++ ) {
        SG_DrawEntity( ent );
    }
}

/*
* SG_DrawPlayer: draws the player, the player's sprite is special
* because it has two parts to draw
*/
static void SG_DrawPlayer( void )
{
    const sgentity_t *ent;

    ent = sg.playr.ent;

    if ( ent->state->frames ) {
        RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ent->sprite + ent->frame );
    } else {
        RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ent->sprite );
    }

    RE_AddSpriteToScene( &ent->origin, ent->hSpriteSheet, ( sg.playr.foot_sprite + ent->facing ) + sg.playr.foot_frame );
}

int SG_DrawFrame( void )
{
    renderSceneRef_t refdef;

    // setup scene
    memset( &refdef, 0, sizeof(refdef) );

    refdef.width = data.realCamWidth;
    refdef.height = data.realCamHeight;
    refdef.x = 0;
    refdef.y = 0;
    refdef.time = sg.levelTime;

    G_SetCameraData( &sg.cameraPos, 1.6f, 0.0f );

    // draw the player
    SG_DrawPlayer();

    // draw everything
    RE_ClearScene();

    RE_RenderScene( &refdef );

    return 1;
}
