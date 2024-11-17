#include "ui_lib.h"
#include "../rendercommon/imgui_internal.h"

typedef struct {
	pthread_t uiThread;
	pthread_mutex_t lock;
	std::atomic<qboolean> running;

	SDL_GLContext uiContext;

	// previous state
	qboolean prev_state_stored;
	menuframework_t *prev_menu;
	int prev_depth;
	uiMenu_t prev_state;
} loadscreen_t;

static loadscreen_t s_loadScreen;

extern SDL_Window *SDL_window;
extern SDL_GLContext SDL_glContext;

namespace ImGui {
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
		return true;
    }
};

static void *LoadScreen( void *unused )
{
	while ( s_loadScreen.running.load() ) {
		pthread_mutex_lock( &s_loadScreen.lock );

		re.BeginFrame( STEREO_CENTER );

		ImGui::Begin( "##LoadingScreen", NULL, MENU_DEFAULT_FLAGS );
		ImGui::SetWindowPos( ImVec2( 0, 0 ) );
		ImGui::SetWindowSize( ImVec2( gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight ) );

		ImGui::SetCursorScreenPos( ImVec2( 16 * gi.scale, 660 * gi.scale ) );
		ImGui::Spinner( "##LoadingScreenSpinner", 16.0f, 10, ImGui::GetColorU32( colorRed ) );

		ImGui::End();

		re.EndFrame( NULL, NULL, NULL );

		pthread_mutex_unlock( &s_loadScreen.lock );
	}
	return NULL;
}

void LoadScreen_Begin( void )
{
	int ret;
	return;

	if ( s_loadScreen.running.load() ) {
		return;
	}

	memset( &s_loadScreen, 0, sizeof( s_loadScreen ) );

	// store state
	if ( ui ) {
		s_loadScreen.prev_menu = ui->activemenu;
		s_loadScreen.prev_state = ui->menustate;
		s_loadScreen.prev_depth = ui->menusp;
		s_loadScreen.prev_state_stored = qtrue;
	}
	
	pthread_mutex_init( &s_loadScreen.lock, NULL );

	s_loadScreen.running = qtrue;

	if ( ( ret = pthread_create( &s_loadScreen.uiThread, NULL, LoadScreen, NULL ) ) != 0 ) {
		N_Error( ERR_FATAL, "Error creating uiThread for loading screen!" );
	}
}

void LoadScreen_End( void )
{
	if ( !s_loadScreen.running.load() ) {
		return;
	}
	return;

	s_loadScreen.running.store( qfalse );
	pthread_join( s_loadScreen.uiThread, (void **)NULL );

	SDL_GL_MakeCurrent( SDL_window, NULL );
	SDL_GL_MakeCurrent( SDL_window, SDL_glContext );

	pthread_mutex_destroy( &s_loadScreen.lock );

	if ( s_loadScreen.prev_state_stored ) {
		ui->activemenu = s_loadScreen.prev_menu;
		ui->menusp = s_loadScreen.prev_depth;
		ui->menustate = s_loadScreen.prev_state;
	}
}