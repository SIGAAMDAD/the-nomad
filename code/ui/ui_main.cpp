#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

CUILib *ui;
cvar_t *ui_language;
cvar_t *ui_cpuString;
cvar_t *ui_printStrings;
cvar_t *ui_active;
cvar_t *ui_diagnostics;

const char *UI_LangToString( int32_t lang )
{
    switch ((language_t)lang) {
    case LANGUAGE_ENGLISH:
        return "english";
    default:
        break;
    };
    return "Invalid";
}

static void UI_RegisterCvars( void )
{
    ui_language = Cvar_Get( "ui_language", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_CheckRange( ui_language, va("%lu", LANGUAGE_ENGLISH), va("%lu", NUMLANGS), CVT_INT );
    Cvar_SetDescription( ui_language,
                        "Sets the game's language:\n"
                        "  0 - English\n"
                        "  1 - Spanish (Not Supported Yet)\n"
                        "  2 - German (Not Supported Yet)\n"
                    );

    ui_cpuString = Cvar_Get("sys_cpuString", "detect", CVAR_PROTECTED | CVAR_ROM | CVAR_NORESTART);

    ui_printStrings = Cvar_Get( "ui_printStrings", "1", CVAR_LATCH | CVAR_SAVE | CVAR_PRIVATE);
    Cvar_CheckRange( ui_printStrings, "0", "1", CVT_INT );
    Cvar_SetDescription( ui_printStrings, "Print value strings set by the language ui file" );

    ui_active = Cvar_Get( "ui_active", "1", CVAR_LATCH | CVAR_TEMP );
    Cvar_CheckRange( ui_active, "0", "1", CVT_INT );
    Cvar_SetDescription( ui_active, "Set to 0 if the gamestate is not in a menu, otherwise 1" );

#ifdef _NOMAD_DEBUG
	ui_diagnostics = Cvar_Get( "ui_diagnostics", "3", CVAR_LATCH | CVAR_PROTECTED | CVAR_SAVE );
#else
	ui_diagnostics = Cvar_Get( "ui_diagnostics", "0", CVAR_LATCH | CVAR_PROTECTED | CVAR_SAVE );
#endif
	Cvar_SetDescription( ui_diagnostics, "Displays various engine performance diagnostics:\n"
											" 0 - disabled\n"
											" 1 - display fps\n"
											" 2 - display gpu memory usage\n"
											" 3 - display cpu memory usage\n"
											" 4 - SHOW ME EVERYTHING!!!!\n" );
}

void UI_UpdateCvars( void )
{
    if (ui_language->modified) {
        if (!strManager->LanguageLoaded((language_t)ui_language->i)) {
            strManager->LoadFile(va("scripts/ui_strings_%s.txt", UI_LangToString(ui_language->i)));
        }
        ui_language->modified = qfalse;
    }
}

/*
=================
UI_Cache
=================
*/
static void UI_Cache_f( void )
{
    Con_Printf( "Caching ui resources...\n" );

    TitleMenu_Cache();
    IntroMenu_Cache();
    MainMenu_Cache();
    SettingsMenu_Cache();
    SinglePlayerMenu_Cache();
}

extern "C" void UI_Shutdown( void )
{
    if (ui) {
        ui->Shutdown();
    }
    if (strManager) {
        strManager->Shutdown();
    }

    Cmd_RemoveCommand( "ui_cache" );
}

// FIXME: call UI_Shutdown instead
void G_ShutdownUI( void ) {
    UI_Shutdown();
}

extern "C" void UI_Init( void )
{
    Con_Printf( "UI_Init: initializing UI...\n" );

    // register cvars
    UI_RegisterCvars();

    // init the library
    ui = (CUILib *)Hunk_Alloc(sizeof(*ui), h_low);
    memset(ui, 0, sizeof(*ui));
    ui->Init(); // we could call ::new

    // init the string manager
    strManager = (CUIStringManager *)Hunk_Alloc(sizeof(*strManager), h_low);
    memset(strManager, 0, sizeof(*strManager));
    strManager->Init();

    // init the font manager
    fontManager = (CUIFontManager *)Hunk_Alloc(sizeof(*fontManager), h_low);
    memset(fontManager, 0, sizeof(*fontManager));
    ::new ((void *)fontManager) CUIFontManager();

    // load the language string file
    strManager->LoadFile(va("scripts/ui_strings_%s.txt", UI_LangToString(ui_language->i)));
    if (!strManager->NumLangsLoaded()) {
        N_Error(ERR_DROP, "UI_Init: no language loaded");
    }

    ui->SetActiveMenu( UI_MENU_TITLE );

    UI_Cache_f();

    // add commands
    Cmd_AddCommand( "ui_cache", UI_Cache_f );

    // build the font texture
//    fontManager->BuildAltas();
}

void Menu_Cache( void )
{
    ui->charset = re.RegisterShader( "gfx/bigchars" );
    ui->rb_on = re.RegisterShader( "gfx/rb_on" );
    ui->rb_off = re.RegisterShader( "gfx/rb_off" );

    ui->whiteShader = re.RegisterShader( "white" );
//    ui->menubackShader = re.RegisterShader( "menuback" );
}

/*
=================
UI_Refresh
=================
*/

#define FPS_FRAMES 6
extern "C" void UI_DrawFPS( bool useWindow = false )
{
    if (ui_diagnostics->i < 1) {
        return;
    }

    static int32_t previousTimes[FPS_FRAMES];
    static int32_t index;
    static int32_t previous;
    int32_t t, frameTime;
    int32_t total, i;
    int32_t fps;

    fps = 0;

    t = Sys_Milliseconds();
    frameTime = t - previous;
    previous = t;

    previousTimes[index % FPS_FRAMES] = frameTime;
    index++;
    if (index > FPS_FRAMES) {
        // average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
    }

    fps = 1000 * FPS_FRAMES / total;

	if (useWindow) {
		ImGui::Text( "%ifps", fps );
		return;
	}

    ImGui::Begin( "DrawFPS##UI", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar
                                        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoBackground );
    ImGui::SetWindowPos( ImVec2( 900 * ui->scale, 8 * ui->scale ) );
    ImGui::SetWindowFontScale( 1.5f * ui->scale );
    ImGui::Text( "%ifps", fps );
    ImGui::End();
}


void Sys_DisplayEngineStats( void );

extern "C" void UI_DrawDiagnositics( void )
{
    if (!ui_diagnostics->i) {
        return;
    }
    else if ( !com_fullyInitialized || !ImGui::GetFont() ) {
        return;
    }

    Sys_DisplayEngineStats();
}

extern "C" void UI_Refresh( int32_t realtime )
{
	ui->SetFrameTime( realtime - ui->GetRealTime() );
	ui->SetRealTime( realtime );

    // check for pause menu
    if (ui->GetState() == STATE_NONE) {
        if (!ui_active->i) {
            ui->EscapeMenuToggle( STATE_PAUSE );
        }

        if (!ui_active->i && ui->GetState() != STATE_PAUSE) {
            return;
        }

        if (ui->GetState() == STATE_PAUSE) {
            Cvar_Set( "ui_active", "1" );
            Cvar_Set( "sg_paused", "1" );
            Cvar_Set( "g_paused", "1" );
            ui->SetActiveMenu( UI_MENU_PAUSE );
            Key_SetCatcher( KEYCATCH_UI );
        }
    }
    
	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	if ( ui->GetCurrentMenu() ) {
		if (ui->GetCurrentMenu()->fullscreen) {
            ui->DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ui->menubackShader );
		}

		if (ui->GetCurrentMenu()->Draw)
			ui->GetCurrentMenu()->Draw();
		else
			Menu_Draw( ui->GetCurrentMenu() );

		if( ui->GetFirstDraw() ) {
			ui->MouseEvent( 0, 0 );
			ui->SetFirstDraw( qfalse );
		}
	}
    else {
        ui->SetActiveMenu( UI_MENU_TITLE );
    }

	// draw cursor
//	ui->SetColor( NULL );
//	ui->DrawHandlePic( ui->GetCursorX() - 16, ui->GetCursorY() - 16, 32, 32, cursor);

#ifdef _NOMAD_DEBUG
	if (ui->IsDebug()) {
		// cursor coordinates
		ui->DrawString( 0, 0, va("(%d,%d)", ui->GetCursorX(), ui->GetCursorY()), UI_LEFT|UI_SMALLFONT, color_red );
	}
#endif

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound) {
		Snd_PlaySfx( menu_in_sound );
		m_entersound = qfalse;
	}

    UI_DrawDiagnositics();
}


#define TRACE_FRAMES 60

typedef struct
{
	uint64_t virtualHeapUsed;
	uint64_t physicalHeapUsed;
	uint64_t stackMemoryUsed;
	
	double cpuMin;
	double cpuMax;
	double cpuFrames[TRACE_FRAMES];
	double cpuAvg;
	int32_t cpuIndex;
	double cpuPrevious;
} sys_stats_t;


// NOTE: got all the win32 code from stack overflow, don't mess with it!!!!!
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <pdh.h>

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static DWORD dwNumProcessors;
static HANDLE self;
#endif

#ifdef __unix__
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>

static FILE *cpuInfo;
static FILE *selfStatus;

static int32_t numProcessors;
static clock_t lastCPU, lastSysCPU, lastUserCPU;
#endif

CallBeforeMain(Sys_InitCPUMonitor)
{
#ifdef _WIN32
	SYSTEM_INFO sysInfo{};
	FILETIME ftime, fsys, fuser;
	
	GetSystemInfo( &sysInfo );
	dwNumProcessors = sysInfo.dwNumberOfProcessors;
	
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &lastCPU, &ftime, sizeof(FILETIME) );
	
	self = GetCurrentProcess();
	GetProcessTimes( self, &ftime, &ftime, &fsys, &fuser );
	memcpy( &lastSysCPU, &fsys, sizeof(FILETIME) );
	memcpy( &lastUserCPU, &fuser, sizeof(FILETIME) );
#elif defined(__unix__)
	struct tms timeSample;
	char line[128];

	cpuInfo = fopen( "/proc/cpuinfo", "r" );
	if (!cpuInfo) {
		N_Error( ERR_FATAL, "Sys_InitCPUMonitor: failed to open /proc/cpuinfo in readonly mode!" );
	}
	
	numProcessors = 0;
	lastCPU = times( &timeSample );
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;
	
	while (fgets( line, sizeof(line), cpuInfo )) {
		if (strncmp( line, "processor", 9 ) == 0) {
			numProcessors++;
		}
	}
	
	selfStatus = fopen( "/proc/self/status", "r" );
#endif
}

static double Sys_GetCPUUsage( void )
{
#ifdef _WIN32
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &now, &ftime, sizeof(FILETIME) );
	
	GetProcessTimes( self, &ftime, &ftime, &fsys, &fuser );
	memcpy( &sys, &fsys, sizeof(FILETIME) );
	memcpy( &user, &fuser, sizeof(FILETIME) );
	
	percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= dwNumProcessors;
	
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;
	
	return percent * 100;
#elif defined(__APPLE__)
#elif defined(__unix__)
	struct tms timeSample;
	clock_t now;
	double percent;
	
	now = times( &timeSample );
	if (now <= lastCPU || timeSample.tms_stime < lastSysCPU || timeSample.tms_utime < lastUserCPU) {
		// overflow detection, just skip this value
		percent = -1.0f;
	}
	else {
		percent = (timeSample.tms_stime - lastSysCPU) + (timeSample.tms_utime - lastUserCPU);
		percent /= (now - lastCPU);
		percent /= numProcessors;
		percent *= 100.0f;
	}
	
	lastCPU = now;
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;
	
	return percent;
#endif
}

#ifdef __unix__
//
// ParseLine: assumes that a digit will be found and the string ends in " Kb"
//
int64_t ParseLine( char *line ) {
	int64_t i;
	const char *p;
	
	p = line;
	i = strlen( line );
	
	while (*p < '0' || *p > '9')
		p++;
	
	line[i - 3] = 0;
	i = atoi( p );
	
	return i;
}

static int64_t GetValue( const char *name ) {
	char line[128];
	int64_t result;
	size_t len = strlen( name );
	
	while ( fgets( line, sizeof(line), selfStatus ) != NULL) {
		if (strncmp( line, name, len ) == 0) {
			result = ParseLine( line );
			break;
		}
	}
	
	return result;
}

static bool Posix_GetProcessMemoryUsage( uint64_t *virtualMem, uint64_t *physicalMem )
{
	char line[128];
	int64_t result;
	
	if (selfStatus == NULL) {
		selfStatus = fopen( "/proc/self/status", "r" );
		if (!selfStatus) {
			N_Error( ERR_FATAL, "Posix_GetProcessMemoryUsage: failed to open /proc/self/status in readonly mode" );
		}
	}
    else {
        rewind( selfStatus );
    }
	
	//
	// get virtual memory
	//
	*virtualMem = GetValue( "VmSize:" );
	
	//
	// get physical (resident) memory
	//
	*physicalMem = GetValue( "VmRSS:" );
	
	return true;
}
#endif

static void Sys_GetMemoryUsage( sys_stats_t *usage )
{
#ifdef _WIN32
	PROCESS_MEMORY_COUNTERS_EX pmc{};
	MEMORYSTATUSEX memInfo{};
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc) );
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx( &memInfo );
	
	usage->virtualHeapUsed = pmc.PrivateUsage;
	usage->physicalHeapUsed = pmc.WorkingSetSize;
#elif defined(__APPLE__)
#elif defined(__unix__)
	Posix_GetProcessMemoryUsage( &usage->virtualHeapUsed, &usage->physicalHeapUsed );
#endif
}

static sys_stats_t stats;

static void Sys_GetCPUStats( void )
{
	double cpuTime = Sys_GetCPUUsage();
	double total, cpu;
	
	if (cpuTime < stats.cpuMin) {
		stats.cpuMin = cpuTime;
	}
	else if (cpuTime > stats.cpuMax) {
		stats.cpuMax = cpuTime;
	}
	
	stats.cpuPrevious = cpuTime;
	
	stats.cpuFrames[stats.cpuIndex % TRACE_FRAMES] = cpuTime;
	stats.cpuIndex++;
	
	if (stats.cpuIndex > TRACE_FRAMES) {
		// average multiple frames of cpu usage to smooth changes out
		for (uint32_t i = 0; i < TRACE_FRAMES; i++) {
			stats.cpuAvg += stats.cpuFrames[i];
		}
		
		stats.cpuAvg /= TRACE_FRAMES;
	}
}

static void Sys_DrawMemoryUsage( void )
{
	ImGui::SeparatorText( "Memory Usage/Stats" );
	ImGui::Text( "Blocks Currently Allocated: %i", SDL_GetNumAllocations() );
	ImGui::Text( "Total Virtual Memory Used: %lu", stats.virtualHeapUsed );	
    ImGui::Text( "Total Physical Memory Used: %lu", stats.physicalHeapUsed );
    ImGui::Text( "Total Stack Memory Remaining: %lu", Sys_StackMemoryRemaining() );
}

static void Sys_DrawCPUUsage( void )
{
	ImGui::SeparatorText( "CPU Usage" );
	ImGui::Text( "Number of CPU Cores: %i", SDL_GetCPUCount() );

	ImGui::BeginTable( " ", 4 );
    {
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "average" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "min" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "max" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "last" );

    	ImGui::TableNextRow();

    	ImGui::TableNextColumn();
    	ImGui::Text( "%03.3f", stats.cpuAvg );
    	ImGui::TableNextColumn();
    	ImGui::Text( "%03.3f", stats.cpuMin );
    	ImGui::TableNextColumn();
    	ImGui::Text( "%03.3f", stats.cpuMax );
    	ImGui::TableNextColumn();
    	ImGui::Text( "%03.3f", stats.cpuPrevious );
    }
    ImGui::EndTable();
}

void Sys_DisplayEngineStats( void )
{
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;
	
	if (!ui_diagnostics->i) {
		return;
	}
    // draw the fps
	else if (ui_diagnostics->i == 1) {
		UI_DrawFPS();
        return;
	}
	
	ImGui::Begin( "Engine Diagnostics", NULL, windowFlags );
    ImGui::SetWindowPos( ImVec2( 700 * ui->scale, 16 * ui->scale ) );
	
	// draw the cpu usage chart
	if (ui_diagnostics->i == 2) {
		Sys_GetCPUStats();
		Sys_DrawCPUUsage();
        return;
    }
	// draw memory statistics
	else if (ui_diagnostics->i == 3) {
        Sys_GetMemoryUsage( &stats );

        Sys_DrawMemoryUsage();
        return;
	}

	//
	// fetch the data
	//

	Sys_GetCPUStats();
	Sys_GetMemoryUsage( &stats );
	
	//
	// draw EVERYTHING
	//


	UI_DrawFPS( true );
	ImGui::Text( "Frame Number: %lu", com_frameNumber );

	Sys_DrawCPUUsage();
    Sys_DrawMemoryUsage();

	ImGui::SeparatorText( "Computer Information" );
	
	ImGui::Text( "%ix%i", gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight );
	ImGui::Text( "%s", ui->GetConfig().version_str );
	ImGui::Text( "%s", ui->GetConfig().renderer );
    ImGui::Text( "%s", ui_cpuString->s );
	
	ImGui::End();
}
