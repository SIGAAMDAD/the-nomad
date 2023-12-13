#include "sg_local.h"
#include "sg_imgui.h"

typedef struct {
    uint32_t polyCount;
    uint32_t vertexCount;
    float realCamWidth;
    float realCamHeight;
    bbox_t frustum;
} drawdata_t;

static drawdata_t data;

static qboolean EntityIsInView( const bbox_t *bounds ) {
    return BoundsIntersect( bounds->mins, bounds->maxs, data.frustum.mins, data.frustum.maxs );
}

static void SG_DetermineEntitySprite( const state_t *state, uint32_t facing, int32_t frame, polyVert_t *verts, const spritesheet_t *sheet ) {
    uint32_t i;
    spritenum_t sprite;

    sprite = state->sprite + facing;

    // more than a single frame for this sprite
    if ( state->frames ) {
        sprite = state->sprite + frame;
    }

    for (i = 0; i < 4; i++) {
        verts[i].uv[0] = sheet->texCoords[state->sprite][i][0];
        verts[i].uv[1] = sheet->texCoords[state->sprite][i][1];
    }
}

static void SG_CalcVerts( const sgentity_t *ent, polyVert_t *verts )
{
    // top right
    verts[0].xyz[0] = ent->bounds.mins[0] + ent->origin[0];
    verts[0].xyz[1] = ent->bounds.mins[1];
    verts[0].xyz[2] = 0.0f;

    // bottom right
    verts[1].xyz[0] = ent->bounds.mins[0];
    verts[1].xyz[1] = ent->bounds.mins[1];
    verts[1].xyz[2] = 0.0f;

    // top left
    verts[2].xyz[0] = ent->bounds.mins[0];
    verts[2].xyz[1] = ent->bounds.mins[1];
    verts[2].xyz[2] = 0.0f;

    // bottom left
    verts[3].xyz[0] = ent->bounds.mins[0] - ent->origin[0];
    verts[3].xyz[1] = ent->bounds.mins[1];
    verts[3].xyz[2] = 0.0f;
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

    SG_CalcVerts( ent, verts );

    SG_DetermineEntitySprite( ent->state, ent->facing, ent->frame, verts, ent->sheet );

    RE_AddPolyToScene( ent->hShader, verts, 4 );
}

static void SG_AddSpritesToFrame( void )
{
    const sgentity_t *ent;
    uint32_t i;

    ent = &sg_entities[0];
    for ( i = 0; i < sg.numEntities; i++, ent++ ) {
        SG_DrawEntity( ent );
    }
}

static void SG_DrawPlayer( void )
{
    spritenum_t base_sprite;
    const sgentity_t *ent;
    polyVert_t verts[4];
    uint32_t i;

    ent = sg.playr.ent;

    SG_CalcVerts( ent, verts );

    for ( i = 0; i < 4; i++ ) {
        verts[i].uv[0] = ent->sheet->texCoords[sg.playr.foot_sprite + sg.playr.foot_frame][i][0];
        verts[i].uv[1] = ent->sheet->texCoords[sg.playr.foot_sprite + sg.playr.foot_frame][i][1];
    }
}

int32_t SG_DrawFrame( void )
{
    renderSceneRef_t refdef;

    // setup scene
    memset( &refdef, 0, sizeof(refdef) );

    refdef.width = data.realCamWidth;
    refdef.height = data.realCamHeight;
    refdef.x = 0;
    refdef.y = 0;
    refdef.time = sg.levelTime;

    // draw everything
    RE_ClearScene();

    RE_RenderScene( &refdef );

    return 1;
}

//
// SG_GenerateSpriteSheetTexCoords: generates a sprite sheet's texture coordinates to reduce redundant calculations when drawing
// NOTE: I'm pretty sure this will only work with OpenGL texture coords
//
void SG_GenerateSpriteSheetTexCoords( spritesheet_t *sheet, uint32_t spriteWidth, uint32_t spriteHeight,
    uint32_t sheetWidth, uint32_t sheetHeight )
{
    uint32_t y, x;
    uint32_t spriteCountX, spriteCountY;
    uint32_t index;
    vec2_t min, max;
    spriteCoords_t *texCoords;

    sheet->texCoords = SG_MemAlloc( sizeof(*sheet->texCoords) * sheet->numSprites );
    texCoords = sheet->texCoords;

    if (spriteWidth % sheetWidth || spriteHeight % sheetHeight) {
        G_Error( "SG_GenerateSpriteSheetTexCoords: please ensure your sprite dimensions and sheet dimensions are powers of two" );
    }

    for (y = 0; y < spriteCountY; y++) {
        for (x = 0; x < spriteCountX; x++) {
            min[0] = (((float)x + 1) * spriteWidth) / sheetWidth;
            min[1] = (((float)y + 1) * spriteHeight) / sheetHeight;
            max[0] = ((float)x * spriteWidth) / sheetWidth;
            max[1] = ((float)y * spriteHeight) / sheetHeight;

            index = y * spriteCountX + x;

            texCoords[index][0][0] = min[0];
            texCoords[index][0][1] = max[1];

            texCoords[index][1][0] = min[0];
            texCoords[index][1][1] = min[1];

            texCoords[index][2][0] = max[0];
            texCoords[index][2][1] = min[1];

            texCoords[index][3][0] = max[0];
            texCoords[index][3][1] = max[1];
        }
    }
}

