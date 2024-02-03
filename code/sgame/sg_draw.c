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

static void SG_DrawEntities( void )
{
    const sgentity_t *ent;
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

static void SG_DrawHUD( void )
{
    renderSceneRef_t refdef;

    memset( &refdef, 0, sizeof(refdef) );
    refdef.x = 0;
    refdef.y = 0;
    refdef.width = sg.gpuConfig.vidWidth;
    refdef.height = sg.gpuConfig.vidHeight;

    // draw crosshair

    //
    // draw status bars
    //
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

    // draw entities
    SG_DrawPlayer();

    // finish the scene
    RE_ClearScene();
    RE_RenderScene( &refdef );

    return 1;
}
