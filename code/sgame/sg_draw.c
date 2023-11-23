#include "sg_local.h"
#include "sg_imgui.h"

typedef struct {
    ImGuiWindow fps;
    uint32_t polyCount;
    uint32_t vertexCount;
    float realCamWidth;
    float realCamHeight;
    bbox_t frustum;
} drawdata_t;

static drawdata_t data;

static void SG_DrawFPS( void )
{
    if (!sg_drawFPS.i) {
        return;
    }

    ImGui_BeginWindow( &data.fps );
    ImGui_EndWindow();
}

static qboolean EntityIsInView( const bbox_t *bounds ) {
    return BoundsIntersect( bounds->mins, bounds->maxs, data.frustum.mins, data.frustum.maxs );
}

static void SG_DrawEntity( const sgentity_t *ent )
{
    polyVert_t verts[4];

    // is it visible?
    if (!EntityIsInView( &ent->bounds )) {
        return;
    }

    data.polyCount++;
    data.vertexCount += 4;

    verts[0].xyz[0] = ent->origin[0];

    trap_RE_AddPolyToScene( ent->hShader, verts, 4 );
}

static void SG_AddSpritesToFrame( void )
{
    const sgentity_t *ent;
    uint32_t numActiveEnts;

    numActiveEnts = 0;
    for (ent = &sg_activeEnts; ent; ent = ent->next) {
        SG_DrawEntity( ent );
        numActiveEnts++;
    }
}

int32_t SG_DrawFrame( void )
{
    renderSceneRef_t refdef;
    renderCameraDef_t camera;

    // setup scene
    memset( &refdef, 0, sizeof(refdef) );
    memset( &camera, 0, sizeof(camera) );
    
    refdef.width = data.realCamWidth;
    refdef.height = data.realCamHeight;
    refdef.x = 0;
    refdef.y = 0;
    refdef.flags = RSF_USE_ORTHO_ASPECT;
    refdef.time = sg.leveltime;
    refdef.camera = &camera;

    camera.realWidth = sg.cameraWidth + 10;
    camera.realHeight = sg.cameraHeight + 10;
    camera.width = sg.cameraWidth;
    camera.height = sg.cameraHeight;

    SG_BuildBounds( &data.frustum, sg.cameraPos, camera.realWidth, camera.realHeight );

    SG_DrawFPS();
    SG_AddSpritesToFrame();


    // draw everything
    trap_RE_ClearScene();

    trap_RE_RenderScene( &refdef );
}

