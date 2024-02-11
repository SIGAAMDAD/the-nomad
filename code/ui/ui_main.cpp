#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"
#include "../rendercommon/imgui_impl_opengl3.h"

CUILib *ui;
CUIFontCache *g_pFontCache;

cvar_t *ui_language;
cvar_t *ui_cpuString;
cvar_t *ui_printStrings;
cvar_t *ui_active;
cvar_t *ui_diagnostics;
cvar_t *r_gpuDiagnostics;

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
	LegalMenu_Cache();
}

CUIFontCache::CUIFontCache( void ) {
	Con_Printf( "Initializing font cache...\n" );

	memset( m_FontList, 0, sizeof(m_FontList) );
	m_pCurrentFont = NULL;
}

void CUIFontCache::SetActiveFont( ImFont *font )
{
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( m_pCurrentFont ) {
		ImGui::PopFont();
	}

	m_pCurrentFont = font;

	ImGui::PushFont( font );
}

void CUIFontCache::ClearCache( void ) {
	ImGui::GetIO().Fonts->Clear();
	memset( m_FontList, 0, sizeof(m_FontList) );
	m_pCurrentFont = NULL;
}

void CUIFontCache::Finalize( void ) {
	ImGui::GetIO().Fonts->Build();
	ImGui_ImplOpenGL3_CreateFontsTexture();
}

ImFont *CUIFontCache::AddFontToCache( const char *filename )
{
	uiFont_t *font;
	uint64_t size;
	uint64_t hash;
	ImFontConfig config;
	union {
		void *v;
		char *b;
	} f;

	hash = Com_GenerateHashValue( filename, MAX_UI_FONTS );

	//
	// see if we already have the font in the cache
	//
	for ( font = m_FontList[hash]; font; font = font->m_pNext ) {
		if ( !N_stricmp( font->m_szName, filename ) ) {
			return font->m_pFont; // its already been loaded
		}
	}

	Con_Printf( "CUIFontCache: loading font '%s'...\n", filename );

	if ( strlen( filename ) >= MAX_GDR_PATH ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: name '%s' is too long", filename );
	}

	size = FS_LoadFile( filename, &f.v );
	if ( !size || !f.v ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: failed to load font file '%s'", filename );
	}

	font = (uiFont_t *)Hunk_Alloc( sizeof(*font), h_low );

	font->m_pNext = m_FontList[hash];
	m_FontList[hash] = font;

	config.FontDataOwnedByAtlas = false;
	config.GlyphExtraSpacing.x = -1.0f;

	N_strncpyz( font->m_szName, filename, sizeof(font->m_szName) );
	font->m_nFileSize = size;
	font->m_pFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF( f.v, size * ui->scale, 16.0f, &config );

	FS_FreeFile( f.v );

	return font->m_pFont;
}

void CUIFontCache::ListFonts_f( void ) {
	uint64_t memSize, i;
	uint64_t numFonts;
	const uiFont_t *font;

	Con_Printf( "---------- Font Cache Info ----------\n" );

	numFonts = 0;
	memSize = 0;
	for ( i = 0; i < MAX_UI_FONTS; i++ ) {
		font = g_pFontCache->m_FontList[i];

		if ( !font ) {
			continue;
		}

		Con_Printf( "[%s]\n", font->m_szName );
		Con_Printf( "File Size: %lu\n", font->m_nFileSize );

		memSize += font->m_nFileSize;
		numFonts++;
	}

	Con_Printf( "\n" );
	Con_Printf( "%-8lu total bytes in font cache\n", memSize );
	Con_Printf( "%-8lu total fonts in cache\n", numFonts );
}

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

    ui_active = Cvar_Get( "g_paused", "1", CVAR_LATCH | CVAR_TEMP );

#ifdef _NOMAD_DEBUG
	r_gpuDiagnostics = Cvar_Get( "r_gpuDiagnostics", "1", CVAR_LATCH | CVAR_SAVE );
#else
	r_gpuDiagnostics = Cvar_Get( "r_gpuDiagnostics", "0", CVAR_LATCH | CVAR_SAVE );
#endif

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

extern "C" void UI_Shutdown( void )
{
    if ( ui ) {
        ui->Shutdown();
		ui = NULL;
    }
    if ( strManager ) {
        strManager->Shutdown();
		strManager = NULL;
    }

    Cmd_RemoveCommand( "ui_cache" );
	Cmd_RemoveCommand( "fontinfo" );
}

// FIXME: call UI_Shutdown instead
void G_ShutdownUI( void ) {
    UI_Shutdown();
}

/*
* UI_GetHashString: an sgame interface for the string manager
*/
extern "C" void UI_GetHashString( const char *name, char *value ) {
	const stringHash_t *hash;

	hash = strManager->ValueForKey( name );

	N_strncpyz( value, hash->value, MAX_STRING_CHARS );
}

extern "C" void UI_Init( void )
{
    Con_Printf( "UI_Init: initializing UI...\n" );

    // register cvars
    UI_RegisterCvars();

    // init the library
    ui = (CUILib *)Hunk_Alloc( sizeof(*ui), h_low );
    ui->Init(); // we could call ::new

    // init the string manager
    strManager = (CUIStringManager *)Hunk_Alloc( sizeof(*strManager), h_low );
    strManager->Init();
    // load the language string file
    strManager->LoadFile( va( "scripts/ui_strings_%s.txt", UI_LangToString( ui_language->i ) ) );
    if ( !strManager->NumLangsLoaded() ) {
        N_Error( ERR_DROP, "UI_Init: no language loaded" );
    }

    ui->SetActiveMenu( UI_MENU_TITLE );

    UI_Cache_f();

    // add commands
    Cmd_AddCommand( "ui_cache", UI_Cache_f );
	Cmd_AddCommand( "fontinfo", CUIFontCache::ListFonts_f );
}

void Menu_Cache( void )
{
    ui->charset = re.RegisterShader( "gfx/bigchars" );
    ui->rb_on = re.RegisterShader( "gfx/rb_on" );
    ui->rb_off = re.RegisterShader( "gfx/rb_off" );

    ui->whiteShader = re.RegisterShader( "white" );
    ui->menubackShader = re.RegisterShader( "menuback" );
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

	UI_DrawDiagnositics();

	if ( !ui_active->i ) {
		ui->EscapeMenuToggle( STATE_PAUSE );
		if ( ui->GetState() != STATE_NONE ) {
			ui->SetActiveMenu( UI_MENU_PAUSE );
		}
		return;
	}

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	if ( ui->GetCurrentMenu() ) {
		if (ui->GetCurrentMenu()->fullscreen) {
            ui->DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ui->menubackShader );
		}

		if (ui->GetCurrentMenu()->Draw) {
			ui->GetCurrentMenu()->Draw();
		} else {
			Menu_Draw( ui->GetCurrentMenu() );
		}

		if( ui->GetFirstDraw() ) {
			ui->MouseEvent( 0, 0 );
			ui->SetFirstDraw( qfalse );
		}
	}
    else {
        ui->SetActiveMenu( UI_MENU_TITLE );
    }
/*
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
	*/
}


#define TRACE_FRAMES 60

typedef struct
{
	double cpuFrames[TRACE_FRAMES];
	uint32_t gpuTimes[TRACE_FRAMES];
	uint32_t gpuSamples[TRACE_FRAMES];
	uint32_t gpuPrimitives[TRACE_FRAMES];

	uint64_t virtualHeapUsed;
	uint64_t physicalHeapUsed;
	uint64_t stackMemoryUsed;

	uint32_t gpuTimeMin;
	uint32_t gpuTimeMax;
	uint32_t gpuTimeAvg;
	int32_t gpuTimeIndex;
	uint32_t gpuTimePrevious;

	uint32_t gpuSamplesMin;
	uint32_t gpuSamplesMax;
	uint32_t gpuSamplesAvg;
	int32_t gpuSamplesIndex;
	uint32_t gpuSamplesPrevious;

	uint32_t gpuPrimitivesMin;
	uint32_t gpuPrimitivesMax;
	uint32_t gpuPrimitivesAvg;
	int32_t gpuPrimitivesIndex;
	uint32_t gpuPrimitivesPrevious;

	qboolean cpuNewMin;
	qboolean cpuNewMax;
	double cpuMin;
	double cpuMax;
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

	selfStatus = fopen( "/proc/self/status", "r" );
	if ( !selfStatus ) {
		N_Error( ERR_FATAL, "Posix_GetProcessMemoryUsage: failed to open /proc/self/status" );
	}
	
	//
	// get virtual memory
	//
	*virtualMem = GetValue( "VmSize:" );
	
	//
	// get physical (resident) memory
	//
	*physicalMem = GetValue( "VmRSS:" );

	fclose( selfStatus );
	
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

static sys_stats_t *stats;

static void Sys_GetCPUStats( void )
{
	const double cpuTime = Sys_GetCPUUsage();
	double total, cpu;
	
	if ( cpuTime < stats->cpuMin ) {
		stats->cpuMin = cpuTime;
		stats->cpuNewMin = qtrue;
	} else if ( cpuTime > stats->cpuMax ) {
		stats->cpuMax = cpuTime;
		stats->cpuNewMax = qtrue;
	}
	
	stats->cpuPrevious = cpuTime;
	
	stats->cpuFrames[stats->cpuIndex % TRACE_FRAMES] = cpuTime;
	stats->cpuIndex++;
	
	if (stats->cpuIndex > TRACE_FRAMES) {
		// average multiple frames of cpu usage to smooth changes out
		for (uint32_t i = 0; i < TRACE_FRAMES; i++) {
			stats->cpuAvg += stats->cpuFrames[i];
		}
		
		stats->cpuAvg /= TRACE_FRAMES;
	}
}

static void Sys_DrawMemoryUsage( void )
{
	ImGui::SeparatorText( "Memory Usage/Stats" );
	ImGui::Text( "Blocks Currently Allocated: %i", SDL_GetNumAllocations() );
	ImGui::Text( "Total Virtual Memory Used: %lu", stats->virtualHeapUsed );	
    ImGui::Text( "Total Physical Memory Used: %lu", stats->physicalHeapUsed );
    ImGui::Text( "Total Stack Memory Remaining: %lu", Sys_StackMemoryRemaining() );
}

static void Sys_GPUStatFrame( uint32_t stat, uint32_t *min, uint32_t *max, uint32_t *avg, uint32_t *prev, uint32_t *frames, int32_t *index )
{
	if ( stat < *min ) {
		*min = stat;
	} else if ( stat > *max ) {
		*max = stat;
	}

	*prev = stat;

	frames[*index % TRACE_FRAMES] = stat;
	(*index)++;

	if ( *index > TRACE_FRAMES ) {
		// average multiple frames of stats to smooth changes out
		for ( uint32_t i = 0; i < TRACE_FRAMES; i++ ) {
			*avg += frames[i];
		}

		*avg /= TRACE_FRAMES;
	}
}

static void Sys_DrawGPUStats( void )
{
	uint32_t time, samples, primitives;

	re.GetGPUFrameStats( &time, &samples, &primitives );

	Sys_GPUStatFrame( time, &stats->gpuTimeMin, &stats->gpuTimeMax, &stats->gpuTimeAvg, &stats->gpuTimePrevious, stats->gpuTimes,
		&stats->gpuTimeIndex );
	Sys_GPUStatFrame( samples, &stats->gpuSamplesMin, &stats->gpuSamplesMax, &stats->gpuSamplesAvg, &stats->gpuSamplesPrevious,
		stats->gpuSamples, &stats->gpuSamplesIndex );
	Sys_GPUStatFrame( primitives, &stats->gpuPrimitivesMin, &stats->gpuPrimitivesMax, &stats->gpuPrimitivesAvg, &stats->gpuPrimitivesPrevious,
		stats->gpuPrimitives, &stats->gpuPrimitivesIndex );

	ImGui::SeparatorText( "GPU Frame Statistics" );
	ImGui::Text( "Time Elapsed (Average): %u", stats->gpuTimeAvg );
	ImGui::Text( "Samples Passed (Average): %u", stats->gpuSamplesAvg );
	ImGui::Text( "Primitives Generated (Average): %u", stats->gpuPrimitivesAvg );
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
    	ImGui::Text( "%03.4f", stats->cpuAvg );
    	ImGui::TableNextColumn();

		if ( stats->cpuNewMin ) {
			ImGui::TextColored( ImVec4( g_color_table[ ColorIndex( S_COLOR_GREEN ) ] ), "%03.4f", stats->cpuMin );
			stats->cpuNewMin = qfalse;
		} else {
			ImGui::Text( "%03.4f", stats->cpuMin );
		}

    	ImGui::TableNextColumn();

		if ( stats->cpuNewMax ) {
    		ImGui::TextColored( ImVec4( g_color_table[ ColorIndex( S_COLOR_RED ) ] ), "%03.4f", stats->cpuMax );
			stats->cpuNewMax = qfalse;
		} else {
			ImGui::Text( "%03.4f", stats->cpuMax );
		}
		
		ImGui::TableNextColumn();
    	ImGui::Text( "%03.4f", stats->cpuPrevious );
    }
    ImGui::EndTable();
}

void Sys_DisplayEngineStats( void )
{
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground
						| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar;
	extern ImFont *RobotoMono;
	ImVec2 windowPos;

	if ( !ui_diagnostics->i ) {
		return;
	}
    // draw the fps
	else if ( ui_diagnostics->i == 1 ) {
		UI_DrawFPS();
        return;
	}

	if ( ui->GetState() == STATE_CREDITS || ui->GetState() == STATE_LEGAL || ImGui::IsWindowCollapsed() ) {
		// pay respects, don't block the words

		// if its in the legal section, just don't draw it

		// if we're collapsing, we'll segfault when drawing CPU usage
		return;
	}
	
	ImGui::Begin( "Engine Diagnostics", NULL, windowFlags );

	if ( ui->GetConfig().vidWidth == 1024 ) {
		windowPos.x = 750 * ui->scale + ui->bias;
	} else {
		windowPos.x = 900 * ui->scale + ui->bias;
	}
	windowPos.y = 16 * ui->scale;
	ImGui::SetWindowPos( windowPos );

	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	}
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * ui->scale );

	if ( !stats ) {
		stats = (sys_stats_t *)Hunk_Alloc( sizeof(sys_stats_t), h_low );
	}

	// draw the cpu usage chart
	if ( ui_diagnostics->i == 2 ) {
		Sys_GetCPUStats();
		Sys_DrawCPUUsage();
        return;
    }
	// draw memory statistics
	else if ( ui_diagnostics->i == 3 ) {
        Sys_GetMemoryUsage( stats );

        Sys_DrawMemoryUsage();
        return;
	}

	//
	// fetch the data
	//

	Sys_GetCPUStats();
	Sys_GetMemoryUsage( stats );
	
	//
	// draw EVERYTHING
	//


	UI_DrawFPS( true );
	ImGui::Text( "Frame Number: %lu", com_frameNumber );

	Sys_DrawCPUUsage();
    Sys_DrawMemoryUsage();
	Sys_DrawGPUStats();

	ImGui::SeparatorText( "Computer Information" );
	
	ImGui::Text( "%ix%i", gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight );
	ImGui::Text( "%s", ui->GetConfig().version_string );
	ImGui::Text( "%s", ui->GetConfig().vendor_string );
	ImGui::Text( "%s", ui->GetConfig().renderer_string );
    ImGui::Text( "%s", ui_cpuString->s );
	
	ImGui::End();
}
