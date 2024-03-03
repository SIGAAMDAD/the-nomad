#include "n_shared.h"
#include "n_threads.h"
#include "../game/g_game.h"
#include "../rendercommon/imgui.h"

namespace ImGui {
    
    bool BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        const float circleStart = size.x * 0.7f;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;
        
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
        
        const float t = g.Time;
        const float r = size.y / 2;
        const float speed = 1.5f;
        
        const float a = speed*0;
        const float b = speed*0.333f;
        const float c = speed*0.666f;
        
        const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
        const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
        const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;
        
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
        window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    }

    bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);
        
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;
        
        // Render
        window->DrawList->PathClear();
        
        int num_segments = 30;
        int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));
        
        const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);
        
        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                                centre.y + ImSin(a+g.Time*8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
    }
    
}

typedef struct loadQueue_s {
    char name[MAX_NPATH];
    void (*loadFn)( void );
    struct loadQueue_s *next, *prev;
} loadQueue_t;

#define MAX_LOADING_ITEMS 8192

typedef struct {
    loadQueue_t loadBuffer[MAX_LOADING_ITEMS];
    loadQueue_t loadQueue;
    loadQueue_t *currentItem;
    uint32_t queueLength;
} loader_t;

static loader_t *s_LoadQueue;

void Com_LoadResources( void )
{
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
    const ImU32 col = ImGui::GetColorU32( ImGuiCol_ButtonHovered );
    const ImU32 bg = ImGui::GetColorU32( ImGuiCol_Button );
    float progress;
    uint32_t completedItems;

    s_LoadQueue->currentItem = s_LoadQueue->loadQueue.next;

    progress = 0;
    completedItems = 0;
    while ( s_LoadQueue->currentItem != &s_LoadQueue->loadQueue ) {
        ImGui::Begin( "##LoadingScreen", NULL, windowFlags );
        ImGui::SetWindowSize( { (float)r_customWidth->i, (float)r_customHeight->i } );
        ImGui::SetWindowPos( { 0.0f, 0.0f } );

        ImGui::Text( "Loading %s...", s_LoadQueue->currentItem->name );

        progress = (float)( (double)s_LoadQueue->queueLength / (double)completedItems );
        ImGui::Text( "Total Progress %0.3f/100.0%%", progress );
        ImGui::Spinner( "##LoadingSpinner", 16.0f, 6.0f, col );
        ImGui::BufferingBar( "##TotalProgressBufferingBar", progress, ImVec2( 400, 6 ), bg, col );

        s_LoadQueue->queueLength--;
        s_LoadQueue->currentItem->loadFn();
        s_LoadQueue->currentItem = s_LoadQueue->currentItem->next;

        ImGui::End();
    }

    Z_Free( s_LoadQueue );
}

void Com_BeginLoadingScreen( void )
{
    s_LoadQueue = (loader_t *)Z_Malloc( sizeof( *s_LoadQueue ), TAG_STATIC );
    memset( s_LoadQueue, 0, sizeof( *s_LoadQueue ) );

    s_LoadQueue->loadQueue.next =
    s_LoadQueue->loadQueue.prev =
        &s_LoadQueue->loadQueue;
}

void Com_AddLoadResource( const char *name, nhandle_t (*fn)( const char * ) )
{
    loadQueue_t *item;

    item = (loadQueue_t *)Z_Malloc( sizeof(*item), TAG_STATIC );
    memset( item, 0, sizeof( *item ) );

    if ( s_LoadQueue->queueLength >= MAX_LOADING_ITEMS ) {
        N_Error( ERR_DROP, "Com_AddLoadResource: MAX_LOADING_ITEMS reached" );
    }

    item->next = &s_LoadQueue->loadQueue;
    item->prev = s_LoadQueue->loadQueue.prev;
    s_LoadQueue->loadQueue.prev->next = item;
    s_LoadQueue->loadQueue.prev = item;
    N_strncpyz( item->name, name, sizeof( item->name ) );
    item->loadFn = fn;

    s_LoadQueue->queueLength++;
}
