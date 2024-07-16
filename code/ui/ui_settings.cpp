/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "../engine/n_allocator.h"
#include "ui_string_manager.h"
#include "ui_table.h"
#include "../rendergl/ngl.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include "../rendercommon/imgui.h"


#define DRAWMODE_IMMEDIATE 0
#define DRAWMODE_CLIENT    1
#define DRAWMODE_GPU       2
#define DRAWMODE_MAPPED    3

#define WINDOWMODE_WINDOWED					0
#define WINDOWMODE_BORDERLESS_WINDOWED		1
#define WINDOWMODE_FULLSCREEN				2
#define WINDOWMODE_BORDERLESS_FULLSCREEN	3

#define GPU_MEMINFO_NVX  0
#define GPU_MEMINFO_ATI  1
#define GPU_MEMINFO_NONE 2

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_MODS			5
#define ID_TABLE        6
#define ID_SETDEFAULTS  7
#define ID_SAVECONFIG   8

#define PRESET_LOW         0
#define PRESET_NORMAL      1
#define PRESET_HIGH        2
#define PRESET_PERFORMANCE 3
#define PRESET_QUALITY     4
#define PRESET_CUSTOM      5
#define NUM_PRESETS        6

#define ID_MOVENORTH       0
#define ID_MOVEWEST        1
#define ID_MOVESOUTH       2
#define ID_MOVEEAST        3
#define ID_UPMOVE          4
#define ID_WEAPONNEXT      5
#define ID_WEAPONPREV      6
#define ID_USEWEAPON       7
#define ID_ALTUSEWEAPON    8
#define ID_SWITCHWIELDING  9
#define ID_SWITCHMODE      10
#define ID_SWITCHHAND      11
#define ID_DASH            12
#define ID_MELEE           13
#define ID_CROUCH          14
#define ID_QUICKSHOT       15
#define NUMKEYBINDS        16

#define BINDGROUP_MOVEMENT 0
#define BINDGROUP_COMBAT   1
#define BINDGROUP_MISC     2

#define TEXFILTER_ANISOTROPY32	4
#define TEXFILTER_ANISOTROPY16	3
#define TEXFILTER_ANISOTROPY8	2
#define TEXFILTER_ANISOTROPY4	1
#define TEXFILTER_ANISOTROPY2	0

typedef struct {
    const char *command;
    const char *label;
    int id;
    int defaultBind1;
    int defaultBind2;
    int bind1;
    int bind2;
	int group;
} bind_t;

typedef struct {
	const char **windowSizes;
	const char **vsyncList;
	const char **windowModes;

	int numWindowSizes;
	int numVSync;
	int numWindowModes;

	int vsync;
	int windowMode;
	int windowResolution;
	int windowWidth;
	int windowHeight;
//	int maxFPS;

	float gamma;
	float exposure;
	float sharpening;
} videoSettings_t;

typedef struct {
	const char **anisotropyTypes;
	const char **toneMappingTypes;

	int numAnisotropyTypes;
	int numToneMappingTypes;

	int anisotropicFilter;
	int toneMappingType;

	int postProcessing;
	int normalMapping;
	int specularMapping;
	int depthMapping;
	int toneMapping;
	int pbr;
	int hdr;

	int bufferMode;
} performanceAdvancedSettings_t;

typedef struct {
	const char **multisampleTypes;
	const char **textureDetails;
	const char **textureFilters;
	const char **onoff;

	int numMultisampleTypes;
	int numTextureDetails;
	int numTextureFilters;

	int multisampleType;
	int textureDetail;
	int textureFilter;

	int vertexLighting;
	int dynamicLighting;
	int bloom;
	int ssao;
} performanceSettings_t;

typedef struct {
	performanceSettings_t basic;
	performanceAdvancedSettings_t advanced;
} preset_t;

typedef struct {
	int sfxVolume;
	int musicVolume;
	int masterVolume;

	// size = uint16_t
	int sfxOn;
	int musicOn;

	int maxSoundChannels;
} audioSettings_t;

typedef struct {
	bind_t keybinds[NUMKEYBINDS];

	int mouseAcceleration;
	float mouseSensitivity;
	int currentBindingGroup;
	int rebindIndex;

	bind_t *rebindKey;
} controlsSettings_t;

typedef struct {
	const char **difficultyNames;
	const char **mouseTypes;

	int numMouseTypes;
	int numDifficultyTypes;

	int mouseCursor;
	int difficulty;
	int debugPrint;
	int toggleHUD;

	int pauseUnfocused;
} gameplaySettings_t;

typedef struct settingsMenu_s {
	menuframework_t menu;

	const char **presetNames;

	preset_t *presets;
	int preset;

	int gpuMemInfoType;

	nhandle_t save_0;
	nhandle_t save_1;

	nhandle_t reset_0;
	nhandle_t reset_1;

	int lastChild;

	qboolean saveHovered;
	qboolean setDefaultsHovered;

	int advancedSettings;

	performanceSettings_t performance;
	performanceAdvancedSettings_t advancedPerformance;
	videoSettings_t video;
	audioSettings_t audio;
	controlsSettings_t controls;
	gameplaySettings_t gameplay;

	uint64_t currentModSettings;

	char gpuMem0[64];
	char gpuMem1[64];
	char gpuMem2[64];

	const char *hintLabel;
	const char *hintMessage;

	const void *focusedItem;

	qboolean modified;
} settingsMenu_t;

static settingsMenu_t *s_settingsMenu;
static settingsMenu_t *s_initial;

static const bind_t s_defaultKeybinds[NUMKEYBINDS] = {
	{ "+north", "forward", ID_MOVENORTH, KEY_W, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+west", "left", ID_MOVEWEST, KEY_A, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+south", "backward", ID_MOVESOUTH, KEY_S, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+east", "right", ID_MOVEEAST, KEY_D, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+jump", "jump", ID_UPMOVE, KEY_SPACE, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+weapnext", "next weapon", ID_WEAPONNEXT, KEY_WHEEL_DOWN, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+weapprev", "prev weapon", ID_WEAPONPREV, KEY_WHEEL_UP, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+useweap", "use weapon", ID_USEWEAPON, KEY_MOUSE_LEFT, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+altuseweap", "use weapon alt fire", ID_ALTUSEWEAPON, KEY_MOUSE_RIGHT, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+switchwielding", "switch weapon wielding", ID_SWITCHWIELDING, KEY_MOUSE_BUTTON_4, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+switchmode", "switch weapon mode", ID_SWITCHMODE, KEY_MOUSE_BUTTON_5, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+switchhand", "switch weapon hand", ID_SWITCHHAND, KEY_Q, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+dash", "dash", ID_DASH, KEY_SHIFT, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+melee", "melee", ID_MELEE, KEY_F, -1, -1, -1, BINDGROUP_COMBAT },
	{ "+crouch", "crouch", ID_CROUCH, KEY_CTRL, -1, -1, -1, BINDGROUP_MOVEMENT },
	{ "+quickshot", "quickshot", ID_QUICKSHOT, KEY_Q, -1, -1, -1, BINDGROUP_COMBAT }
};

static void SettingsMenu_GetInitial( void )
{
	if ( !ui->uiAllocated ) {
		s_initial = (settingsMenu_t *)Hunk_Alloc( sizeof( *s_initial ), h_high );
	}
	s_initial->controls = s_settingsMenu->controls;
	s_initial->performance = s_settingsMenu->performance;
	s_initial->advancedPerformance = s_settingsMenu->advancedPerformance;
	s_initial->video = s_settingsMenu->video;
	s_initial->audio = s_settingsMenu->audio;
}

static void SettingsMenu_CheckModified( void )
{
	int i;

	s_settingsMenu->modified = qfalse;

	if ( memcmp( &s_settingsMenu->controls, &s_initial->controls, sizeof( controlsSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( &s_settingsMenu->video, &s_initial->video, sizeof( videoSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( &s_settingsMenu->performance, &s_initial->performance, sizeof( performanceSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( &s_settingsMenu->video, &s_initial->video, sizeof( videoSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( &s_settingsMenu->advancedPerformance, &s_initial->advancedPerformance, sizeof( performanceAdvancedSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( &s_settingsMenu->audio, &s_initial->audio, sizeof( audioSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}

/*
	if ( s_settingsMenu->gameplay.debugPrint != s_initial->gameplay.debugPrint ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->gameplay.difficulty != s_initial->gameplay.difficulty ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->gameplay.mouseCursor != s_initial->gameplay.mouseCursor ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->gameplay.pauseUnfocused != s_initial->gameplay.pauseUnfocused ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->gameplay.toggleHUD != s_initial->gameplay.toggleHUD ) {
		s_settingsMenu->modified = qtrue;	
	}

	if ( s_settingsMenu->controls.mouseAcceleration != s_initial->controls.mouseAcceleration ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->controls.mouseSensitivity != s_initial->controls.mouseSensitivity ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( memcmp( s_settingsMenu->controls.keybinds, s_initial->controls.keybinds, sizeof( s_settingsMenu->controls.keybinds ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}

	if ( s_settingsMenu->audio.masterVolume != s_initial->audio.masterVolume ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->audio.sfxVolume != s_initial->audio.sfxVolume ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->audio.musicVolume != s_initial->audio.musicVolume ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->audio.sfxOn != s_initial->audio.sfxOn ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->audio.musicOn != s_initial->audio.musicOn ) {
		s_settingsMenu->modified = qtrue;
	}

	if ( s_settingsMenu->performance.multisampleType != s_initial->performance.multisampleType ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.anisotropicFilter != s_initial->advancedPerformance.anisotropicFilter ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.bloom != s_initial->performance.bloom ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.textureDetail != s_initial->performance.textureDetail ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.textureFilter != s_initial->performance.textureFilter ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.depthMapping != s_initial->advancedPerformance.depthMapping ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.specularMapping != s_initial->advancedPerformance.specularMapping ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.normalMapping != s_initial->advancedPerformance.normalMapping ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.dynamicLighting != s_initial->performance.dynamicLighting ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.vertexLighting != s_initial->performance.vertexLighting ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.hdr != s_initial->advancedPerformance.hdr ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.pbr != s_initial->advancedPerformance.pbr ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.postProcessing != s_initial->advancedPerformance.postProcessing ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.toneMapping != s_initial->advancedPerformance.toneMapping ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.bloom != s_initial->performance.bloom ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->advancedPerformance.toneMappingType != s_initial->advancedPerformance.toneMappingType ) {
		s_settingsMenu->modified = qtrue;
	}

	if ( s_settingsMenu->video.exposure != s_initial->video.exposure ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.vsync != s_initial->video.vsync ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.fullscreen != s_initial->video.fullscreen ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.gamma != s_initial->video.gamma ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.windowWidth != s_initial->video.windowWidth ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.windowHeight != s_initial->video.windowHeight ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.windowResolution != s_initial->video.windowResolution ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.noborder != s_initial->video.noborder ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.sharpening != s_initial->video.sharpening ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->video.maxFPS != s_initial->video.maxFPS ) {
		s_settingsMenu->modified = qtrue;
	}
*/
}

const char *Hunk_CopyString( const char *str, ha_pref pref ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Hunk_Alloc( len, pref );
    N_strncpyz( out, str, len );

    return out;
}

static void SettingsMenu_LoadBindings( void )
{
    union {
        void *v;
        char *b;
    } f;
    const char **text, *text_p, *tok;
    bind_t *bind;
    uint64_t i;

    FS_LoadFile( "bindings.cfg", &f.v );
    if ( !f.v ) {
        N_Error( ERR_DROP, "SettingsMenu_Cache: no bindings file" );
    }

    text_p = f.b;
    text = &text_p;

    COM_BeginParseSession( "bindings.cfg" );

    text_p = f.b;
    text = &text_p;

    bind = s_settingsMenu->controls.keybinds;
    for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
        tok = COM_ParseExt( text, qtrue );
        if ( !tok || !tok[0] ) {
            break;
        }
        bind->command = Hunk_CopyString( tok, h_high );

        tok = COM_ParseExt( text, qfalse );
        bind->label = Hunk_CopyString( tok, h_high );

        tok = COM_ParseExt( text, qfalse );
        bind->id = atoi( tok );

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->defaultBind1 = Key_StringToKeynum( tok );
        } else {
            bind->defaultBind1 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->defaultBind2 = Key_StringToKeynum( tok );
        } else {
            bind->defaultBind2 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->bind1 = Key_StringToKeynum( tok );
        } else {
            bind->bind1 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->bind2 = Key_StringToKeynum( tok );
        } else {
            bind->bind2 = -1;
        }

        if ( bind->defaultBind1 != -1 ) {
            bind->bind1 = bind->defaultBind1;
        }
        if ( bind->defaultBind2 != -1 ) {
            bind->bind2 = bind->defaultBind2;
        }

        Con_Printf( "Added keybind \"%s\": \"%s\"\n", bind->command, bind->label );

        bind++;
    }

    FS_FreeFile( f.v );
}

static void SettingsMenu_InitPresets( void ) {
	s_settingsMenu->presets = (preset_t *)Hunk_Alloc( sizeof( *s_settingsMenu->presets ) * NUM_PRESETS, h_high );
	s_settingsMenu->preset = PRESET_NORMAL;

	memset( &s_settingsMenu->presets[ PRESET_CUSTOM ], 0, sizeof( performanceSettings_t ) );

	// some quality but more optimized just for playability
	s_settingsMenu->presets[ PRESET_LOW ].basic.multisampleType = AntiAlias_2xMSAA;
	s_settingsMenu->presets[ PRESET_LOW ].basic.textureDetail = TexDetail_IntegratedGPU;
	s_settingsMenu->presets[ PRESET_LOW ].basic.textureFilter = TEXFILTER_ANISOTROPY4;
	s_settingsMenu->presets[ PRESET_LOW ].basic.vertexLighting = qtrue;
	s_settingsMenu->presets[ PRESET_LOW ].basic.dynamicLighting = qfalse;
	s_settingsMenu->presets[ PRESET_LOW ].basic.bloom = qfalse;

	s_settingsMenu->presets[ PRESET_NORMAL ].basic.multisampleType = AntiAlias_4xMSAA;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.textureDetail = TexDetail_Normie;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.textureFilter = TEXFILTER_ANISOTROPY2;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.vertexLighting = qtrue;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.dynamicLighting = qfalse;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.bloom = qtrue;

	s_settingsMenu->presets[ PRESET_HIGH ].basic.multisampleType = AntiAlias_16xMSAA;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.textureDetail = TexDetail_ExpensiveShitWeveGotHere;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.textureFilter = TEXFILTER_ANISOTROPY16;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.vertexLighting = qtrue;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.bloom = qtrue;

	// highest quality rendering, no care for performance
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.multisampleType = AntiAlias_32xMSAA;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.textureDetail = TexDetail_GPUvsGod;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.textureFilter = TEXFILTER_ANISOTROPY32;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.vertexLighting = qtrue;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.bloom = qtrue;
	
	// looks the worst but gets the best framerate
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.multisampleType = AntiAlias_None;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.textureDetail = TexDetail_MSDOS;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.textureFilter = TEXFILTER_ANISOTROPY2;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.vertexLighting = qfalse;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.dynamicLighting = qfalse;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.bloom = qfalse;
}

static void SettingsMenu_SetPreset( const preset_t *preset )
{
	s_settingsMenu->performance.multisampleType = preset->basic.multisampleType;
	s_settingsMenu->performance.textureDetail = preset->basic.textureDetail;
	s_settingsMenu->performance.textureFilter = preset->basic.textureFilter;
	s_settingsMenu->performance.vertexLighting = preset->basic.vertexLighting;
	s_settingsMenu->performance.dynamicLighting = preset->basic.dynamicLighting;
	s_settingsMenu->performance.bloom = preset->basic.bloom;
}

static inline void SfxFocused( const void *item ) {
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		if ( s_settingsMenu->focusedItem != item ) {
			s_settingsMenu->focusedItem = item;
			Snd_PlaySfx( ui->sfx_move );
		}
	}
}

static void SettingsMenu_GetGPUMemoryInfo( void )
{
	const char *memSuffix;
	auto getMemSuffix = [&]( GLint mem, float& real ) -> void {
		real = mem;
		if ( real > 1000 ) {
			real /= 1000;
			memSuffix = "Mb";
		}
		if ( real > 1000 ) {
			real /= 1000;
			memSuffix = "Gb";
		}
	};

	switch ( s_settingsMenu->gpuMemInfoType ) {
	case GPU_MEMINFO_NVX: {
		GLint dedicatedMemory, availableMemory, totalMemory;
		float realDedicated, realAvailable, realTotal;

		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicatedMemory );
		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory );
		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemory );

		getMemSuffix( dedicatedMemory, realDedicated );
		Com_snprintf( s_settingsMenu->gpuMem0, sizeof( s_settingsMenu->gpuMem0 ) - 1,
			"Dedicated GPU Memory: %0.04f%s", realDedicated, memSuffix );

		getMemSuffix( availableMemory, realAvailable );
		Com_snprintf( s_settingsMenu->gpuMem1, sizeof( s_settingsMenu->gpuMem1 ) - 1,
			"Available GPU Memory: %0.04f%s", realAvailable, memSuffix );

		getMemSuffix( totalMemory, realTotal );
		Com_snprintf( s_settingsMenu->gpuMem2, sizeof( s_settingsMenu->gpuMem2 ) - 1,
			"Total GPU Memory: %0.04f%s", realTotal, memSuffix );
		break; }
	case GPU_MEMINFO_ATI: {
		GLint vboMemory, textureMemory, renderbufferMemory;
		float realVbo, realTexture, realRenderbuffer;

		renderImport.glGetIntegerv( GL_VBO_FREE_MEMORY_ATI, &vboMemory );
		renderImport.glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, &textureMemory );
		renderImport.glGetIntegerv( GL_RENDERBUFFER_FREE_MEMORY_ATI, &renderbufferMemory );

		getMemSuffix( vboMemory, realVbo );
		Com_snprintf( s_settingsMenu->gpuMem0, sizeof( s_settingsMenu->gpuMem0 ) - 1,
			"Available Vertex Buffer Memory: %0.04f%s", realVbo, memSuffix );

		getMemSuffix( textureMemory, realTexture );
		Com_snprintf( s_settingsMenu->gpuMem1, sizeof( s_settingsMenu->gpuMem1 ) - 1,
			"Available Texture Buffer Memory: %0.04f%s", realTexture, memSuffix );

		getMemSuffix( renderbufferMemory, realRenderbuffer );
		Com_snprintf( s_settingsMenu->gpuMem2, sizeof( s_settingsMenu->gpuMem2 ) - 1,\
			"Available Render Buffer Memory: %0.04f%s", realRenderbuffer, memSuffix );
		break; }
	case GPU_MEMINFO_NONE: {
		memset( s_settingsMenu->gpuMem0, 0, sizeof( s_settingsMenu->gpuMem0 ) );
		memset( s_settingsMenu->gpuMem1, 0, sizeof( s_settingsMenu->gpuMem1 ) );
		memset( s_settingsMenu->gpuMem2, 0, sizeof( s_settingsMenu->gpuMem2 ) );
		break; }
	};
}

static void SettingsMenu_TabBar( void ) {
	if ( ImGui::BeginTabBar( "##SettingsMenuTabBar" ) ) {
		ImGui::PushStyleColor( ImGuiCol_Tab, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabActive, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabHovered, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

		FontCache()->SetActiveFont( AlegreyaSC );
		if ( ImGui::BeginTabItem( "Video" ) ) {
			if ( s_settingsMenu->lastChild != ID_VIDEO ) {
				s_settingsMenu->lastChild = ID_VIDEO;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		SfxFocused( "Video" );
		if ( ImGui::BeginTabItem( "Graphics" ) ) {
			if ( s_settingsMenu->lastChild != ID_PERFORMANCE ) {
				s_settingsMenu->lastChild = ID_PERFORMANCE;
				Snd_PlaySfx( ui->sfx_select );
			}
			SettingsMenu_GetGPUMemoryInfo();
			ImGui::EndTabItem();
		}
		SfxFocused( "Graphics" );
		if ( ImGui::BeginTabItem( "Audio" ) ) {
			if ( s_settingsMenu->lastChild != ID_AUDIO ) {
				s_settingsMenu->lastChild = ID_AUDIO;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		SfxFocused( "Audio" );
		if ( ImGui::BeginTabItem( "Controls" ) ) {
			if ( s_settingsMenu->lastChild != ID_CONTROLS ) {
				s_settingsMenu->lastChild = ID_CONTROLS;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		SfxFocused( "Controls" );
		if ( ImGui::BeginTabItem( "Gameplay" ) ) {
			if ( s_settingsMenu->lastChild != ID_GAMEPLAY ) {
				s_settingsMenu->lastChild = ID_GAMEPLAY;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		SfxFocused( "Gameplay" );
		if ( ImGui::BeginTabItem( "Mods" ) ) {
			if ( s_settingsMenu->lastChild != ID_MODS ) {
				s_settingsMenu->lastChild = ID_MODS;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		SfxFocused( "Mods" );

		ImGui::PopStyleColor( 3 );
		ImGui::EndTabBar();
	}
}

extern void Text_Draw( menutext_t *text );

static void SettingsMenu_Text( const char *name, const char *hint )
{
	ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0, 0, 0, 0 ) );
	ImGui::PushStyleColor( ImGuiCol_Text, colorCyan );
	if ( s_settingsMenu->focusedItem == name ) {
		ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
	}
	ImGui::Button( name );
//	UI_DrawText( name, &hovered, &clicked );
	ImGui::PopStyleColor( 3 );
	if ( s_settingsMenu->focusedItem == name ) {
		ImGui::PopStyleColor();
	}
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone )
		|| ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
	{
		s_settingsMenu->hintLabel = name;
		s_settingsMenu->hintMessage = hint;
	}
	ImGui::PopStyleColor();
	SfxFocused( name );
}

static void SettingsMenu_List( const char *label, const char **itemnames, int numitems, int *curitem, bool enabled )
{
	int i;

	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	if ( ImGui::BeginCombo( va( "##%sSettingsMenuConfigList", label ), itemnames[*curitem] ) ) {
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		for ( i = 0; i < numitems; i++ ) {
			if ( ImGui::Selectable( va( "%s##%sSettingsMenuConfigList", itemnames[i], label ), ( *curitem == i ) ) ) {
				if ( enabled ) {
					Snd_PlaySfx( ui->sfx_select );
					*curitem = i;
				}
			}
			SfxFocused( itemnames[i] );
		}
		ImGui::EndCombo();
	}
	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
	ImGui::PopStyleColor();
	SfxFocused( label );
}

static void SettingsMenu_MultiAdjustable( const char *name, const char *label, const char *hint, const char **itemnames, int numitems,
	int *curitem, bool enabled )
{
	if ( !enabled ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}

	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigLeft", label ), ImGuiDir_Left ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select);
			( *curitem )--;
			if ( *curitem < 0 ) {
				*curitem = 0;
			}
		}
	}
	SfxFocused( (void *)( (uintptr_t)curitem * 0xaf ) );
	ImGui::SameLine();
	SettingsMenu_List( label, itemnames, numitems, curitem, enabled );
	ImGui::SameLine();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigRight", label ), ImGuiDir_Right ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curitem )++;
		if ( *curitem > numitems - 1 ) {
			*curitem = numitems - 1;
		}
	}
	SfxFocused( (void *)( (uintptr_t)curitem * 0xfa ) );

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
	ImGui::PopStyleColor();
}

static void SettingsMenu_MultiSliderFloat( const char *name, const char *label, const char *hint, float *curvalue, float minvalue, float maxvalue,
	float delta )
{
	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigLeft", label ), ImGuiDir_Left ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curvalue ) -= delta;
		if ( *curvalue < minvalue ) {
			*curvalue = minvalue;
		}
	}
	SfxFocused( (void *)( (uintptr_t)curvalue * 0xaf ) );
	ImGui::SameLine();
	if ( ImGui::SliderFloat( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue ) ) {
		Snd_PlaySfx( ui->sfx_move );
	}
	SfxFocused( curvalue );
	ImGui::SameLine();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigRight", label ), ImGuiDir_Right ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curvalue ) += delta;
		if ( *curvalue > maxvalue ) {
			*curvalue = maxvalue;
		}
	}
	SfxFocused( (void *)( (uintptr_t)curvalue * 0xfa ) );
	ImGui::PopStyleColor();
}

static void SettingsMenu_MultiSliderInt( const char *name, const char *label, const char *hint, int *curvalue, int minvalue, int maxvalue,
	int delta, bool enabled )
{
	if ( !enabled ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}

	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigLeft", label ), ImGuiDir_Left ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select );
			( *curvalue ) -= delta;
			if ( *curvalue < minvalue ) {
				*curvalue = minvalue;
			}
		}
	}
	SfxFocused( (void *)( (uintptr_t)curvalue * 0xaf ) );
	ImGui::SameLine();
	if ( ImGui::SliderInt( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue, "%d", enabled ? 0 : ImGuiSliderFlags_NoInput ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_move );
		}
	}
	SfxFocused( curvalue );
	ImGui::SameLine();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigRight", label ), ImGuiDir_Right ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select );
			( *curvalue ) += delta;
			if ( *curvalue > maxvalue ) {
				*curvalue = maxvalue;
			}
		}
	}
	SfxFocused( (void *)( (uintptr_t)curvalue * 0xfa ) );

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
	ImGui::PopStyleColor();
}

static void SettingsMenu_RadioButton( const char *name, const char *label, const char *hint, int *curvalue, bool enabled )
{
	if ( !enabled ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}

	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	if ( ImGui::RadioButton( *curvalue ? va( "ON##%sSettingsMenuConfigButtonON", label ) : va( "OFF##%sSettingsMenuConfigButtonOFF", label ),
		*curvalue ) )
	{
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select );
			*curvalue = !*curvalue;
		}
	}
	SfxFocused( curvalue );

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
	ImGui::PopStyleColor();
}

static int32_t SettingsMenu_GetBindIndex( const char *bind )
{
	int32_t i;

	for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
		if ( !N_stricmp( s_settingsMenu->controls.keybinds[i].command, bind ) ) {
			return i;
		}
	}
	return -1;
}

static void SettingsMenu_Rebind( void )
{
    int32_t bind;
    uint32_t i;
    int ret;
    const char *binding;
	float width;
	float height;
	float x, y;
	float scale;

	x = 256 * ui->scale;
	y = 128 * ui->scale;
	width = ( ui->gpuConfig.vidWidth * 0.5f ) - ( x * 0.5f );
	height = ( ui->gpuConfig.vidHeight * 0.5f ) - ( y * 0.5f );
	
	ImGui::SetNextWindowFocus();
	ImGui::Begin( "##RebindKeyPopup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse );
	ImGui::SetWindowPos( ImVec2( x, y ) );
	ImGui::SetWindowSize( ImVec2( width, height ) );

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.25f ) * ui->scale );
	ImGui::TextUnformatted( strManager->ValueForKey( "GAMEUI_REBIND" )->value );

	if ( Key_IsDown( KEY_ESCAPE ) ) {
		keys[KEY_ESCAPE].down = qfalse;
		s_settingsMenu->controls.rebindKey = NULL;
		Snd_PlaySfx( ui->sfx_null );
		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
	ImGui::TextUnformatted( strManager->ValueForKey( "GAMEUI_PRESSKEY" )->value );

    for ( i = 0; i < NUMKEYS; i++ ) {
        if ( Key_IsDown( i ) ) {
            binding = Key_GetBinding( i );

            if ( binding != NULL ) {
				const int32_t index = SettingsMenu_GetBindIndex( binding );
                if ( s_settingsMenu->controls.keybinds[i].bind1 != -1 && s_settingsMenu->controls.rebindIndex == 1 ) {
                    // we're overwriting a binding, warn them
                    ret = Sys_MessageBox( "WARNING",
                        va( "You are overwriting another binding, are you sure about this? (\"%s\" = \"%s\")",
                            Key_KeynumToString( s_settingsMenu->controls.keybinds[ index ].bind1 ),
                            binding ),
                        true );
                    
                    if ( ret == 0 ) {
						Snd_PlaySfx( ui->sfx_select );
                    } else {
						s_settingsMenu->controls.rebindKey->bind1 = i;
						Snd_PlaySfx( ui->sfx_select );
					}
					ImGui::End();
					return;
                }
				else if ( s_settingsMenu->controls.keybinds[i].bind2 != -1 && s_settingsMenu->controls.rebindIndex == 2 ) {
                    // we're overwriting a binding, warn them
                    ret = Sys_MessageBox( "WARNING",
                        va( "You are overwriting another binding, are you sure about this? (\"%s\" = \"%s\")",
                            Key_KeynumToString( s_settingsMenu->controls.keybinds[ index ].bind2 ),
                            binding ),
                        true );
                    
                    if ( ret == 0 ) {
						Snd_PlaySfx( ui->sfx_select );
                    } else {
						s_settingsMenu->controls.rebindKey->bind2 = i;
						Snd_PlaySfx( ui->sfx_select );
					}
					ImGui::End();
					return;
                }
            }

			if ( s_settingsMenu->controls.rebindIndex == 1 ) {
				s_settingsMenu->controls.keybinds[i].bind1 = i;
			} else if ( s_settingsMenu->controls.rebindIndex == 2 ) {
				s_settingsMenu->controls.keybinds[i].bind2 = i;
			}
            Cbuf_ExecuteText( EXEC_APPEND, va( "bind \"%s\" \"%s\"\n",
                Key_KeynumToString( i ),
                s_settingsMenu->controls.rebindKey->command ) );

			s_settingsMenu->controls.rebindKey = NULL;
			s_settingsMenu->controls.rebindIndex = 0;
			ImGui::End();
			return;
        }
    }
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale );
	ImGui::End();
}

static void SettingsMenu_DrawHint( void )
{
	int flags;

	if ( !s_settingsMenu->hintLabel && !s_settingsMenu->hintMessage ) {
		return;
	}

	flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	ImGui::Begin( "##SettingsMenuHintWindow", NULL, flags );
	ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth - s_settingsMenu->menu.width, 256 * ui->scale ) );
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.width, 100 * ui->scale ) );

	if ( !s_settingsMenu->hintLabel || !s_settingsMenu->hintMessage ) {
		return;
	}

	FontCache()->SetActiveFont( AlegreyaSC );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
	ImGui::TextUnformatted( s_settingsMenu->hintLabel );

	FontCache()->SetActiveFont( RobotoMono );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.0f ) * ui->scale );
	ImGui::TextWrapped( "%s", s_settingsMenu->hintMessage );

	ImGui::End();
}

static nhandle_t GetCustomButton( int button )
{
	switch ( button ) {
	case KEY_PAD0_A: return ui->controller_a;
	case KEY_PAD0_B: return ui->controller_b;
	case KEY_PAD0_X: return ui->controller_x;
	case KEY_PAD0_Y: return ui->controller_y;
	case KEY_PAD0_LEFTTRIGGER: return ui->controller_left_trigger;
	case KEY_PAD0_RIGHTTRIGGER: return ui->controller_right_trigger;
	case KEY_PAD0_LEFTBUTTON: return ui->controller_left_button;
	case KEY_PAD0_RIGHTBUTTON: return ui->controller_right_button;
	case KEY_PAD0_LEFTSTICK_CLICK:
	case KEY_PAD0_LEFTSTICK_UP:
	case KEY_PAD0_LEFTSTICK_RIGHT:
	case KEY_PAD0_LEFTSTICK_DOWN:
	case KEY_PAD0_LEFTSTICK_LEFT:
	case KEY_PAD0_RIGHTSTICK_CLICK:
	case KEY_PAD0_RIGHTSTICK_UP:
	case KEY_PAD0_RIGHTSTICK_RIGHT:
	case KEY_PAD0_RIGHTSTICK_DOWN:
	case KEY_PAD0_RIGHTSTICK_LEFT:
	case KEY_PAD0_DPAD_UP: return ui->controller_dpad_up;
	case KEY_PAD0_DPAD_RIGHT: return ui->controller_dpad_right;
	case KEY_PAD0_DPAD_DOWN: return ui->controller_dpad_down;
	case KEY_PAD0_DPAD_LEFT: return ui->controller_dpad_left;
	default:
		break;
	};
	return FS_INVALID_HANDLE;
}

static void ControlsMenu_DrawBindings( int group )
{
	static char bind[1024];
	static char bind2[1024];
	int i;
	nhandle_t hShader;

//	ImGui::BeginTable( va( "##ControlsSettingsMenuBindingsTableGrouping%i", group ), 2 );
	ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.10f, 0.10f, 0.10f, 1.0f ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.10f, 0.10f, 0.10f, 1.0f ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.10f, 0.10f, 0.10f, 1.0f ) );
	for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
		if ( s_settingsMenu->controls.keybinds[i].group != group ) {
			continue;
		}

		ImGui::TableNextColumn();
		if ( s_settingsMenu->controls.keybinds[i].bind1 == -1 ) {
			strcpy( bind, strManager->ValueForKey( "GAMEUI_NOBIND" )->value );
		} else {
			strcpy( bind, Key_KeynumToString( s_settingsMenu->controls.keybinds[i].bind1 ) );
		}
		if ( s_settingsMenu->controls.keybinds[i].bind2 == -1 ) {
			strcpy( bind2, strManager->ValueForKey( "GAMEUI_NOBIND" )->value );
		} else {
			strcpy( bind2, Key_KeynumToString( s_settingsMenu->controls.keybinds[i].bind2 ) );
		}
		SettingsMenu_Text( s_settingsMenu->controls.keybinds[i].label, NULL );
		ImGui::TableNextColumn();
		if ( ImGui::Button( bind ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_settingsMenu->controls.rebindKey = &s_settingsMenu->controls.keybinds[i];
			s_settingsMenu->controls.rebindIndex = 1;
		}
		SfxFocused( &s_settingsMenu->controls.keybinds[i].bind1 );
		ImGui::SameLine();
		if ( ImGui::Button( bind2 ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_settingsMenu->controls.rebindKey = &s_settingsMenu->controls.keybinds[i];
			s_settingsMenu->controls.rebindIndex = 2;
		}
		SfxFocused( &s_settingsMenu->controls.keybinds[i].bind2 );
		if ( i != NUMKEYBINDS - 1 ) {
			ImGui::TableNextRow();
		}
	}
	ImGui::PopStyleColor( 3 );
//	ImGui::EndTable();
}

static void ControlsMenu_Draw( void )
{
	uint64_t i;

	FontCache()->SetActiveFont( RobotoMono );
	
	ImGui::BeginTable( "##ControlsSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiAdjustable( strManager->ValueForKey( "GAMEUI_MOUSEACCEL" )->value, "MouseAcceleration",
			"Toggles mouse acceleration", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->controls.mouseAcceleration, true );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( strManager->ValueForKey( "GAMEUI_MOUSESENSITIVITY" )->value, "MouseSensitivity",
			"Sets the speed of the mouse",
			&s_settingsMenu->controls.mouseSensitivity, 1.0f, 50.0f, 1.0f );

		ImGui::TableNextRow();

		ImGui::Separator();

		ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.5f ) * ui->scale );
		
		FontCache()->SetActiveFont( AlegreyaSC );
		
		ImGui::PushStyleColor( ImGuiCol_Text, colorCyan );
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Binding" );
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Key" );
		ImGui::PopStyleColor();

		FontCache()->SetActiveFont( RobotoMono );

		ImGui::TableNextRow();

		ImGui::TableNextColumn();

		ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );

		if ( ImGui::BeginTabBar( "##ControlsSettingsBindingSelector" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Tab, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_TabActive, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_TabHovered, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

			if ( ImGui::BeginTabItem( "Movement##ControlsSettingsBindingsMovement" ) ) {
				if ( s_settingsMenu->controls.currentBindingGroup != BINDGROUP_MOVEMENT ) {
					Snd_PlaySfx( ui->sfx_select );
				}
				s_settingsMenu->controls.currentBindingGroup = BINDGROUP_MOVEMENT;
				ImGui::EndTabItem();
			}
			SfxFocused( "Movement##ControlsSettingsBindingsMovement" );
			if ( ImGui::BeginTabItem( "Combat##ControlsSettingsBindingsCombat" ) ) {
				if ( s_settingsMenu->controls.currentBindingGroup != BINDGROUP_COMBAT ) {
					Snd_PlaySfx( ui->sfx_select );
				}
				s_settingsMenu->controls.currentBindingGroup = BINDGROUP_COMBAT;
				ImGui::EndTabItem();
			}
			SfxFocused( "Combat##ControlsSettingsBindingsCombat" );
			if ( ImGui::BeginTabItem( "General##ControlsSettingsBindingsGeneral" ) ) {
				if ( s_settingsMenu->controls.currentBindingGroup != BINDGROUP_MISC ) {
					Snd_PlaySfx( ui->sfx_select );
				}
				s_settingsMenu->controls.currentBindingGroup = BINDGROUP_MISC;
				ImGui::EndTabItem();
			}
			SfxFocused( "General##ControlsSettingsBindingsGeneral" );

			ImGui::EndTabBar();
		}

		ImGui::PopStyleColor( 3 );
		ImGui::TableNextRow();

		ControlsMenu_DrawBindings( s_settingsMenu->controls.currentBindingGroup );
	}
	ImGui::EndTable();
}

static void PerformanceMenu_DrawBasic( void )
{
	ImGui::BeginTable( "##PerformanceSettingsMenuConfigTable", 2 );

	SettingsMenu_MultiAdjustable( "ANTIALIASING", "AntiAliasing",
		"Sets anti-aliasing technique used by the engine",
		s_settingsMenu->performance.multisampleTypes, s_settingsMenu->performance.numMultisampleTypes,
		&s_settingsMenu->performance.multisampleType,
		s_settingsMenu->advancedPerformance.postProcessing );
	
	ImGui::TableNextRow();
	
	SettingsMenu_MultiAdjustable( "TEXTURE QUALITY", "TextureQuality",
		"Sets the quality of textures rendered, may effect performance",
		s_settingsMenu->performance.textureDetails, s_settingsMenu->performance.numTextureDetails,
		&s_settingsMenu->performance.textureDetail, true );
	
	ImGui::TableNextRow();

	SettingsMenu_MultiAdjustable( "TEXTURE FILTERING", "TextureFiltering",
		"Sets the type of texture filtering",
		s_settingsMenu->advancedPerformance.anisotropyTypes, s_settingsMenu->advancedPerformance.numAnisotropyTypes,
		&s_settingsMenu->performance.textureFilter, true );
	
	ImGui::TableNextRow();
	
	SettingsMenu_MultiAdjustable( "BLOOM", "Bloom",
		"Enables bloom to make light sources stand out more in an environment",
		s_settingsMenu->performance.onoff, 2,
		&s_settingsMenu->performance.bloom, s_settingsMenu->advancedPerformance.postProcessing );

	ImGui::TableNextRow();

	SettingsMenu_MultiAdjustable( "SSAO", "ScreenSpaceAmbientOcclusion",
			"Enables screen space ambient occlusion for more diverse coloration of environments",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->performance.ssao, s_settingsMenu->advancedPerformance.postProcessing );
	
	ImGui::TableNextRow();

	SettingsMenu_MultiAdjustable( "VERTEX LIGHTING", "VertexLighting",
		"Enables per-vertex software lighting",
		s_settingsMenu->performance.onoff, 2,
		&s_settingsMenu->performance.vertexLighting, true );
		
	ImGui::TableNextRow();

	SettingsMenu_MultiAdjustable( "DYNAMIC LIGHTING", "DynamicLighting",
		"Enables per-pixel hardware accelerated lighting, slower than vertex lighting, but much higher quality",
		s_settingsMenu->performance.onoff, 2,
		&s_settingsMenu->performance.dynamicLighting, true );
	
	ImGui::EndTable();
}

static void PerformanceMenu_DrawAdvanced( void )
{
	if ( ImGui::CollapsingHeader( "Framebuffer Settings" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsMenuConfigTableFramebuffer", 2 );

		SettingsMenu_MultiAdjustable( "POST PROCESSING", "PostProcessing",
			"Toggles multiple framebuffers being used to apply special affects to a frame",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.postProcessing, true );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "ANTIALIASING", "AntiAliasing",
			"Sets anti-aliasing technique used by the engine",
			s_settingsMenu->performance.multisampleTypes, s_settingsMenu->performance.numMultisampleTypes,
			&s_settingsMenu->performance.multisampleType,
			s_settingsMenu->advancedPerformance.postProcessing );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "HDR", "HDR",
			"Enables HDR (High Dynamic Range) texture/framebuffer usage, uses more GPU memory but allows for much more "
			"range in rendered color palette", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.hdr, s_settingsMenu->advancedPerformance.postProcessing );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "PBR", "PBR",
			"Enables Physically Based Rendering (PBR) for a more realistic texture look.",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.pbr, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "BLOOM", "BloomConfig",
			"Enables bloom to make light sources stand out more in an environment",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->performance.bloom, s_settingsMenu->advancedPerformance.postProcessing );
		
		SettingsMenu_MultiAdjustable( "SSAO", "SSAOConfig",
			"Enables screen space ambient occlusion for more diverse coloration of environments",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->performance.ssao, s_settingsMenu->advancedPerformance.postProcessing );

		ImGui::EndTable();
	}
	if ( ImGui::CollapsingHeader( "GPU Shader Settings" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsMenuConfigTableGPUShader", 2 );

		SettingsMenu_MultiAdjustable( "TONE MAPPING", "ToneMapping",
			"Enables a more diverse range of colors when applying lighting to a scene",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.toneMapping, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "TONE MAPPING TYPE", "ToneMappingType",
			"Sets the desired tone mapping type.\n"
			"NOTE: Reinhard uses a fixed range, and makes darker spots less detailed, Exposure uses an adjustable level",
			s_settingsMenu->advancedPerformance.toneMappingTypes, s_settingsMenu->advancedPerformance.numToneMappingTypes,
			&s_settingsMenu->advancedPerformance.toneMappingType, s_settingsMenu->advancedPerformance.toneMapping );
		
		ImGui::EndTable();
	}
	if ( ImGui::CollapsingHeader( "Texture Settings" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsMenuConfigTableTextureStuff", 2 );

		SettingsMenu_MultiAdjustable( "ANISOTROPIC FILTERING", "AnisotropicFiltering",
			"Sets the level of anisotropic filtering that is applied to textures",
			s_settingsMenu->advancedPerformance.anisotropyTypes, s_settingsMenu->advancedPerformance.numAnisotropyTypes,
			&s_settingsMenu->advancedPerformance.anisotropicFilter, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "TEXTURE QUALITY", "TextureQuality",
			"Sets the quality of textures rendered, may effect performance",
			s_settingsMenu->performance.textureDetails, s_settingsMenu->performance.numTextureDetails,
			&s_settingsMenu->performance.textureDetail, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "DEFAULT TEXTURE FILTERING", "TextureFiltering",
			"Sets the type of texture filtering",
			s_settingsMenu->performance.textureFilters, s_settingsMenu->performance.numTextureFilters,
			&s_settingsMenu->performance.textureFilter, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "NORMAL MAPPING", "BumpMapping",
			"Toggles usage of normal maps", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.normalMapping, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "SPECULAR MAPPING", "SpecularMapping",
			"Toggles usage of specular maps", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.specularMapping, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "PARALLAX MAPPING", "ParallaxMapping",
			"Toggles usage of parallax maps", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedPerformance.depthMapping, true );
		
		ImGui::EndTable();
	}
	if ( ImGui::CollapsingHeader( "Lighting" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsMenuConfigTableLighting", 2 );

		SettingsMenu_MultiAdjustable( "VERTEX LIGHTING", "VertexLighting",
			"Enables per-vertex software lighting", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->performance.vertexLighting, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "DYNAMIC LIGHTING", "DynamicLighting",
			"Enables per-pixel hardware accelerated lighting, slower than vertex lighting, but much higher quality",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->performance.dynamicLighting, true );
		
		ImGui::EndTable();
	}
}

static void PerformanceMenu_Draw( void )
{
	int preset;

	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##PerformanceSettingsMenuConfigTable", 2 );
	{
		/*
		preset = s_settingsMenu->preset;
		SettingsMenu_MultiAdjustable( "Preset", "GraphicsPresets",
			"",
			s_settingsMenu->presetNames, NUM_PRESETS, &preset, true );
		
		if ( preset != s_settingsMenu->preset ) {
			SettingsMenu_SetPreset( &s_settingsMenu->presets[ preset ] );
			s_settingsMenu->preset = preset;
		}

		ImGui::TableNextRow();
		*/

		SettingsMenu_MultiAdjustable( "ADVANCED SETTINGS", "AdvancedSettings",
			"Shows advanced settings, don't use if you aren't familiar with graphics programming.",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->advancedSettings, true );

		ImGui::EndTable();

		if ( s_settingsMenu->advancedSettings ) {
			PerformanceMenu_DrawAdvanced();
		} else {
			PerformanceMenu_DrawBasic();
		}
	}

	ImGui::Begin( "##GPUMemoryInfo", NULL, MENU_DEFAULT_FLAGS | ImGuiWindowFlags_AlwaysAutoResize & ~( ImGuiWindowFlags_NoBackground ) );
	ImGui::SetWindowPos( ImVec2( 900 * ui->scale, 600 * ui->scale ) );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 2.5f );
	ImGui::TextUnformatted( s_settingsMenu->gpuMem0 );
	ImGui::TextUnformatted( s_settingsMenu->gpuMem1 );
	ImGui::TextUnformatted( s_settingsMenu->gpuMem2 );
	ImGui::End();
}

static void AudioMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##AudioSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiSliderInt( "MASTER VOLUME", "MasterVolume",
			"Sets overall volume",
			&s_settingsMenu->audio.masterVolume, 0, 100, 1,
			s_settingsMenu->audio.musicOn && s_settingsMenu->audio.sfxOn );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderInt( "MUSIC VOLUME", "MusicVolume",
			"Sets the music volume",
			&s_settingsMenu->audio.musicVolume, 0, 100, 1, s_settingsMenu->audio.musicOn );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderInt( "SOUND EFFECTS VOLUME", "SoundEffectsVolume",
			"Sets the sound effects volume",
			&s_settingsMenu->audio.sfxVolume, 0, 100, 1, s_settingsMenu->audio.sfxOn );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "MUSIC ON", "MusicOn",
			"Toggles music", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->audio.musicOn, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "SOUND EFFECTS ON", "SoundEffectsOn",
			"Toggles sound effects", s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->audio.sfxOn, true );
	}
	ImGui::EndTable();
}

static void VideoMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##VideoSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiAdjustable( strManager->ValueForKey( "GAMEUI_WINDOWMODE" )->value, "WindowMode",
			"Sets the game's window mode to fullscreen",
			s_settingsMenu->video.windowModes, s_settingsMenu->video.numWindowModes,
			&s_settingsMenu->video.windowMode, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( strManager->ValueForKey( "GAMEUI_MODE_WINDOWRES" )->value, "WindowResolution",
			"Sets the game window's size",
			s_settingsMenu->video.windowSizes, s_settingsMenu->video.numWindowSizes, &s_settingsMenu->video.windowResolution, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "V-SYNC", "VSync",
			"Toggles when a frame will be rendered, vertical tearing may occur if disabled.\n"
			"NOTE: setting this to \"Enabled\" will force a maximum of your moniter's refresh rate.",
			s_settingsMenu->video.vsyncList, s_settingsMenu->video.numVSync, &s_settingsMenu->video.vsync, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiSliderFloat( "BRIGHTNESS", "Gamma",
			"Sets gamma linear light correction factor",
			&s_settingsMenu->video.gamma, 0.5f, 3.0f, 0.10f );

		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "EXPOSURE", "Exposure",
			"Sets exposure level when rendered in a scene",
			&s_settingsMenu->video.exposure, 0.10f, 10.0f, 1.0f );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "IMAGE SHARPENING", "ImageSharpening",
			"Sets the amount of sharpening applied to a rendered texture",
			&s_settingsMenu->video.sharpening, 0.5f, 20.0f, 0.1f );
		
		/* for now we will only really allow running at 60 fps
		ImGui::TableNextRow();

		SettingsMenu_MultiSliderInt( "FRAME LIMITER", "FrameLimiter",
			"Sets the maximum amount of frames the game can render per second.",
			&s_settingsMenu->video.maxFPS, 0, 1000, 1, true );
		*/
	}
	ImGui::EndTable();
}

static void GameplayMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##GameSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiAdjustable( "GAME DIFFICULTY", "GameDifficulty",
			"Sets the game's difficulty",
			s_settingsMenu->gameplay.difficultyNames, s_settingsMenu->gameplay.numDifficultyTypes, &s_settingsMenu->gameplay.difficulty,
			s_settingsMenu->gameplay.difficulty != DIF_HARDEST );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "TOGGLE HUD", "ToggleHUD",
			"Toggles Heads-Up-Display (HUD). Turn this off if you want a more immersive experience",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->gameplay.toggleHUD, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "DEBUG MODE", "DebugMode",
			"Toggles debug messages from SGame",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->gameplay.debugPrint, true );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "STOP GAME ON FOCUS LOST", "StopGameOnFocusLost",
			"If on the game will pause when the window is unfocused",
			s_settingsMenu->performance.onoff, 2,
			&s_settingsMenu->gameplay.pauseUnfocused, true );
	}
	ImGui::EndTable();
}

static void ModuleMenu_Draw( void )
{
	uint64_t i;
	const char *name;
	float scale;

	scale = ImGui::GetFont()->Scale;

	FontCache()->SetActiveFont( RobotoMono );

	ImGui::SetCursorScreenPos( ImVec2( 0, 168 * ui->scale ) );
    ImGui::BeginChild( ImGui::GetID( "MODSMENUEDIT" ), ImVec2( 200 * ui->scale, 460 * ui->scale ), ImGuiChildFlags_None,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus );
    FontCache()->SetActiveFont( RobotoMono );

    ImGui::SeparatorText( "MODS" );
    ImGui::SetWindowFontScale( scale );
    for ( i = 0; i < g_pModuleLib->GetModCount(); i++ ) {
//		ImGui::TextUnformatted( g_pModuleLib->m_pModList[i].info->m_szName );
		if ( ImGui::Selectable( va( "%s##ModuleSelectionSettings%lu", g_pModuleLib->m_pModList[i].info->m_szName, i ),
			( s_settingsMenu->currentModSettings == i ) ) )
		{
			Snd_PlaySfx( ui->sfx_select );
			s_settingsMenu->currentModSettings = i;
		}
    }
    ImGui::EndChild();

    ImGui::SetCursorScreenPos( ImVec2( 208 * ui->scale, 168 * ui->scale ) );
    ImGui::BeginChild( ImGui::GetID( "EntryDraw" ), ImVec2( 400 * ui->scale, 460 * ui->scale ), ImGuiChildFlags_None,
        MENU_DEFAULT_FLAGS );

	name = g_pModuleLib->m_pModList[s_settingsMenu->currentModSettings].info->m_szName;

    ImGui::SetWindowFontScale( scale * 1.5f );
    FontCache()->SetActiveFont( AlegreyaSC );
    ImGui::SeparatorText( name );
	FontCache()->SetActiveFont( RobotoMono );
	ImGui::SetWindowFontScale( scale * 1.0f );

	g_pModuleLib->ModuleCall( g_pModuleLib->m_pModList[s_settingsMenu->currentModSettings].info, ModuleDrawConfiguration, 0 );

    ImGui::EndChild();
    ImGui::SetWindowFontScale( scale );
}

static void VideoMenu_Save( void )
{
	extern SDL_Window *SDL_window;

	Cvar_SetIntegerValue( "r_fullscreen", s_settingsMenu->video.windowMode >= WINDOWMODE_FULLSCREEN );
	Cvar_SetIntegerValue( "r_noborder", s_settingsMenu->video.windowMode % 2 != 0 );
	Cvar_SetIntegerValue( "r_customWidth", s_settingsMenu->video.windowWidth );
	Cvar_SetIntegerValue( "r_customHeight", s_settingsMenu->video.windowHeight );
	Cvar_SetIntegerValue( "r_mode", s_settingsMenu->video.windowResolution - 2 );
	Cvar_SetIntegerValue( "r_swapInterval", s_settingsMenu->video.vsync - 1 );
	Cvar_SetFloatValue( "r_imageSharpenAmount", s_settingsMenu->video.sharpening );
	Cvar_SetFloatValue( "r_autoExposure", s_settingsMenu->video.exposure );
	Cvar_SetFloatValue( "r_gammaAmount", s_settingsMenu->video.gamma );

	if ( !N_stricmp( g_renderer->s, "opengl" ) ) {
		SDL_GL_SetSwapInterval( s_settingsMenu->video.vsync - 1 );
	}
	SDL_SetWindowFullscreen( SDL_window, s_settingsMenu->video.windowMode >= WINDOWMODE_FULLSCREEN
		? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );
	SDL_SetWindowSize( SDL_window, r_vidModes[ s_settingsMenu->video.windowResolution - 2 ].width,
		r_vidModes[ s_settingsMenu->video.windowResolution - 2 ].height );
	SDL_SetWindowBordered( SDL_window, (SDL_bool)( s_settingsMenu->video.windowMode % 2 != 0 ) );
//	SDL_SetWindowPosition( SDL_window, vid_xpos->i, vid_ypos->i );
}

static bool PerformanceMenu_FBO_Save( void )
{
	bool restartFBO = false, restartVid = false;

	Cvar_SetIntegerValue( "r_multisampleType", s_settingsMenu->performance.multisampleType );
	switch ( s_settingsMenu->performance.multisampleType ) {
	case AntiAlias_None:
	case AntiAlias_TAA:
	case AntiAlias_SMAA:
	case AntiAlias_FXAA:
	case AntiAlias_CSAA:
		Cvar_Set( "r_multisampleAmount", "0" );
		break;
	case AntiAlias_2xSSAA:
		Cvar_Set( "r_multisampleAmount", "2" );
		break;
	case AntiAlias_4xSSAA:
		Cvar_Set( "r_multisampleAmount", "4" );
		break;
	case AntiAlias_2xMSAA:
		Cvar_Set( "r_multisampleAmount", "2" );
		break;
	case AntiAlias_4xMSAA:
		Cvar_Set( "r_multisampleAmount", "4" );
		break;
	case AntiAlias_8xMSAA:
		Cvar_Set( "r_multisampleAmount", "8" );
		break;
	case AntiAlias_16xMSAA:
		Cvar_Set( "r_multisampleAmount", "16" );
		break;
	case AntiAlias_32xMSAA:
		Cvar_Set( "r_multisampleAmount", "32" );
		break;
	};

	// if we're switching from simple multisampling to super sampling, that'll require a full
	// vid_restart because of the resolution change
	if ( ( s_settingsMenu->performance.multisampleType <= AntiAlias_32xMSAA
		&& s_initial->performance.multisampleType >= AntiAlias_2xSSAA
		&& s_initial->performance.multisampleType <= AntiAlias_4xSSAA )
		||
		( s_settingsMenu->performance.multisampleType >= AntiAlias_2xSSAA
		&& s_settingsMenu->performance.multisampleType <= AntiAlias_4xSSAA
		&& s_initial->performance.multisampleType <= AntiAlias_32xMSAA ) )
	{
		restartVid = true;
	}

	if ( s_settingsMenu->performance.multisampleType != s_initial->performance.multisampleType ) {
		restartFBO = true;
	}
	if ( s_settingsMenu->performance.bloom != s_initial->performance.bloom ) {
		restartFBO = true;
	}
	if ( s_settingsMenu->performance.ssao != s_initial->performance.ssao ) {
		restartFBO = true;
	}
	if ( s_settingsMenu->advancedPerformance.postProcessing != s_initial->advancedPerformance.postProcessing ) {
		restartFBO = true;
	}

	Cvar_SetIntegerValue( "r_hdr", s_settingsMenu->advancedPerformance.hdr );
	Cvar_SetIntegerValue( "r_bloom", s_settingsMenu->performance.bloom );
	Cvar_SetIntegerValue( "r_postProcess", s_settingsMenu->advancedPerformance.postProcessing );
	Cvar_SetIntegerValue( "r_toneMap", s_settingsMenu->advancedPerformance.toneMapping );
	Cvar_SetIntegerValue( "r_toneMapType", s_settingsMenu->advancedPerformance.toneMappingType );
	Cvar_SetIntegerValue( "r_ssao", s_settingsMenu->performance.ssao );

	if ( restartVid ) {
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
	} else if ( restartFBO ) {
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart_fbo\n" );
	}
	return restartVid;
}

static void PerformanceMenu_Save( void )
{
	bool needRestart = false;

	// convert from simple stuff to the actual settings
	if ( !s_settingsMenu->advancedSettings ) {
		if ( s_settingsMenu->performance.textureDetail > TexDetail_Normie ) {
			s_settingsMenu->advancedPerformance.normalMapping = qtrue;
			s_settingsMenu->advancedPerformance.specularMapping = qtrue;
			s_settingsMenu->advancedPerformance.depthMapping = qtrue;
		}
		if ( s_settingsMenu->performance.textureDetail >= TexDetail_Normie ) {
			s_settingsMenu->advancedPerformance.hdr = qtrue;
			s_settingsMenu->advancedPerformance.pbr = qtrue;
			s_settingsMenu->advancedPerformance.toneMapping = qtrue;
			s_settingsMenu->advancedPerformance.postProcessing = qtrue;
		}

		switch ( s_settingsMenu->performance.textureFilter ) {
		case TEXFILTER_ANISOTROPY2:
			s_settingsMenu->advancedPerformance.anisotropicFilter = 0;
			break;
		case TEXFILTER_ANISOTROPY4:
			s_settingsMenu->advancedPerformance.anisotropicFilter = 1;
			break;
		case TEXFILTER_ANISOTROPY8:
			s_settingsMenu->advancedPerformance.anisotropicFilter = 2;
			break;
		case TEXFILTER_ANISOTROPY16:
			s_settingsMenu->advancedPerformance.anisotropicFilter = 3;
			break;
		case TEXFILTER_ANISOTROPY32:
			s_settingsMenu->advancedPerformance.anisotropicFilter = 4;
			break;
		};
	}

	if ( s_settingsMenu->advancedPerformance.normalMapping != s_initial->advancedPerformance.normalMapping ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.specularMapping != s_initial->advancedPerformance.specularMapping ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.textureDetail != s_initial->performance.textureDetail ) {
		needRestart = true;
	}

	Cvar_SetIntegerValue( "r_multisampleType", s_settingsMenu->performance.multisampleType );
	switch ( s_settingsMenu->performance.multisampleType ) {
	case AntiAlias_None:
		Cvar_Set( "r_multisampleAmount", "0" );
		break;
	case AntiAlias_TAA:
	case AntiAlias_SMAA:
	case AntiAlias_FXAA:
	case AntiAlias_CSAA:
		Cvar_Set( "r_multisampleAmount", "8" );
		break;
	case AntiAlias_2xSSAA:
		Cvar_Set( "r_multisampleAmount", "2" );
		break;
	case AntiAlias_4xSSAA:
		Cvar_Set( "r_multisampleAmount", "4" );
		break;
	case AntiAlias_2xMSAA:
		Cvar_Set( "r_multisampleAmount", "2" );
		break;
	case AntiAlias_4xMSAA:
		Cvar_Set( "r_multisampleAmount", "4" );
		break;
	case AntiAlias_8xMSAA:
		Cvar_Set( "r_multisampleAmount", "8" );
		break;
	case AntiAlias_16xMSAA:
		Cvar_Set( "r_multisampleAmount", "16" );
		break;
	case AntiAlias_32xMSAA:
		Cvar_Set( "r_multisampleAmount", "32" );
		break;
	};

	if ( s_settingsMenu->performance.multisampleType != s_initial->performance.multisampleType ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.bloom != s_initial->performance.bloom ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.ssao != s_initial->performance.ssao ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.postProcessing != s_initial->advancedPerformance.postProcessing ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.textureDetail != s_initial->performance.textureDetail ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.normalMapping != s_initial->advancedPerformance.normalMapping ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.specularMapping != s_initial->advancedPerformance.specularMapping ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.toneMappingType != s_initial->advancedPerformance.toneMappingType ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.toneMapping != s_initial->advancedPerformance.toneMapping ) {
		needRestart = true;
	}
	if ( s_settingsMenu->advancedPerformance.hdr != s_initial->advancedPerformance.hdr ) {
		needRestart = true;
	}

	switch ( s_settingsMenu->advancedPerformance.anisotropicFilter ) {
	case 0:
		Cvar_Set( "r_arb_texture_max_anisotropy", "2" );
		break;
	case 1:
		Cvar_Set( "r_arb_texture_max_anisotropy", "4" );
		break;
	case 2:
		Cvar_Set( "r_arb_texture_max_anisotropy", "8" );
		break;
	case 3:
		Cvar_Set( "r_arb_texture_max_anisotropy", "16" );
		break;
	case 4:
		Cvar_Set( "r_arb_texture_max_anisotropy", "32" );
		break;
	};

	Cvar_Set( "r_textureMode", s_settingsMenu->performance.textureFilters[ s_settingsMenu->performance.textureFilter ] );
	Cvar_SetIntegerValue( "r_vertexLight", s_settingsMenu->performance.vertexLighting );
	Cvar_SetIntegerValue( "r_dynamiclight", s_settingsMenu->performance.dynamicLighting );
	Cvar_SetIntegerValue( "r_textureDetail", s_settingsMenu->performance.textureDetail );
	Cvar_SetIntegerValue( "r_normalMapping", s_settingsMenu->advancedPerformance.normalMapping );
	Cvar_SetIntegerValue( "r_pbr", s_settingsMenu->advancedPerformance.pbr );
	Cvar_SetIntegerValue( "r_vertexLight", s_settingsMenu->performance.vertexLighting );
	Cvar_SetIntegerValue( "r_dynamiclight", s_settingsMenu->performance.dynamicLighting );

	if ( !PerformanceMenu_FBO_Save() && needRestart ) {
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
	}
}

static void AudioMenu_Save( void )
{
	Cvar_SetIntegerValue( "snd_masterVolume", s_settingsMenu->audio.masterVolume );
	Cvar_SetIntegerValue( "snd_effectsVolume", s_settingsMenu->audio.sfxVolume );
	Cvar_SetIntegerValue( "snd_musicVolume", s_settingsMenu->audio.musicVolume );
	Cvar_SetIntegerValue( "snd_effectsOn", s_settingsMenu->audio.sfxOn );
	Cvar_SetIntegerValue( "snd_musicOn", s_settingsMenu->audio.musicOn );
	Cvar_SetIntegerValue( "snd_maxSoundChannels", s_settingsMenu->audio.maxSoundChannels );
}

static void ControlsMenu_Save( void )
{
	int i;
	const bind_t *bind;

	Cvar_SetIntegerValue( "g_mouseAcceleration", s_settingsMenu->controls.mouseAcceleration );
	Cvar_SetFloatValue( "g_mouseSensitivity", s_settingsMenu->controls.mouseSensitivity );

	for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
		bind = &s_settingsMenu->controls.keybinds[i];

		if ( bind->bind1 != -1 ) {
			Cbuf_ExecuteText( EXEC_APPEND, va( "bind \"%s\" \"%s\"\n", Key_GetBinding( s_defaultKeybinds[i].bind1 ),
				s_defaultKeybinds[i].command ) );
		}
		if ( bind->bind2 != -1 ) {
			Cbuf_ExecuteText( EXEC_APPEND, va( "bind \"%s\" \"%s\"\n", Key_GetBinding( s_defaultKeybinds[i].bind2 ),
				s_defaultKeybinds[i].command ) );
		}
	}
}

static void GameplayMenu_Save( void )
{
	Cvar_SetIntegerValue( "sgame_Difficulty", s_settingsMenu->gameplay.difficulty );
	Cvar_SetIntegerValue( "sgame_CursorType", s_settingsMenu->gameplay.mouseCursor );
	Cvar_SetIntegerValue( "sgame_DebugMode", s_settingsMenu->gameplay.debugPrint );
	Cvar_SetIntegerValue( "sgame_ToggleHUD", s_settingsMenu->gameplay.toggleHUD );
}

static void ModuleMenu_Save( void )
{
	g_pModuleLib->ModuleCall( sgvm, ModuleSaveConfiguration, 0 );
	g_pModuleLib->RunModules( ModuleSaveConfiguration, 0 );
}

static void PerformanceMenu_SetDefault( void )
{
	int i;
	const char *textureMode;

	switch ( Cvar_VariableInteger( "r_arb_texture_max_anisotropy" ) ) {
	case 2:
		s_settingsMenu->advancedPerformance.anisotropicFilter = 0;
		break;
	case 4:
		s_settingsMenu->advancedPerformance.anisotropicFilter = 1;
		break;
	case 8:
		s_settingsMenu->advancedPerformance.anisotropicFilter = 2;
		break;
	case 16:
		s_settingsMenu->advancedPerformance.anisotropicFilter = 3;
		break;
	case 32:
		s_settingsMenu->advancedPerformance.anisotropicFilter = 4;
		break;
	};

	s_settingsMenu->performance.dynamicLighting = Cvar_VariableInteger( "r_dynamiclight" );
	s_settingsMenu->performance.vertexLighting = Cvar_VariableInteger( "r_vertexLight" );
	s_settingsMenu->performance.multisampleType = Cvar_VariableInteger( "r_multisampleType" );
	s_settingsMenu->performance.ssao = Cvar_VariableInteger( "r_ssao" );
	s_settingsMenu->advancedPerformance.depthMapping = Cvar_VariableInteger( "r_parallaxMapping" );
	s_settingsMenu->advancedPerformance.specularMapping = Cvar_VariableInteger( "r_specularMapping" );
	s_settingsMenu->advancedPerformance.normalMapping = Cvar_VariableInteger( "r_normalMapping" );
	s_settingsMenu->advancedPerformance.postProcessing = Cvar_VariableInteger( "r_postProcess" );
	s_settingsMenu->advancedPerformance.toneMappingType = Cvar_VariableInteger( "r_toneMapType" );
	s_settingsMenu->advancedPerformance.toneMapping = Cvar_VariableInteger( "r_toneMap" );
	s_settingsMenu->performance.textureDetail = Cvar_VariableInteger( "r_textureDetail" );
	s_settingsMenu->performance.bloom = Cvar_VariableInteger( "r_bloom" );
	s_settingsMenu->advancedPerformance.hdr = Cvar_VariableInteger( "r_hdr" );
	s_settingsMenu->advancedPerformance.pbr = Cvar_VariableInteger( "r_pbr" );
	s_settingsMenu->advancedPerformance.postProcessing = Cvar_VariableInteger( "r_postProcess" );

	s_settingsMenu->advancedPerformance.bufferMode = Cvar_VariableInteger( "r_drawMode" );

	s_settingsMenu->advancedSettings = qfalse;
	
	textureMode = Cvar_VariableString( "r_textureMode" );
	for ( i = 0; i < s_settingsMenu->performance.numTextureDetails; i++ ) {
		if ( !N_stricmp( textureMode, s_settingsMenu->performance.textureDetails[i] ) ) {
			s_settingsMenu->performance.textureFilter = i;
			break;
		}
	}

	SettingsMenu_GetGPUMemoryInfo();
}

static void VideoMenu_SetDefault( void )
{
	s_settingsMenu->video.windowWidth = Cvar_VariableInteger( "r_customWidth" );
	s_settingsMenu->video.windowHeight = Cvar_VariableInteger( "r_customHeight" );
	s_settingsMenu->video.windowResolution = Cvar_VariableInteger( "r_mode" ) + 2;
	s_settingsMenu->video.vsync = Cvar_VariableInteger( "r_swapInterval" ) + 1;
	s_settingsMenu->video.gamma = Cvar_VariableFloat( "r_gammaAmount" );
	s_settingsMenu->video.windowMode = Cvar_VariableInteger( "r_fullscreen" ) + Cvar_VariableInteger( "r_noborder" );
	s_settingsMenu->video.sharpening = Cvar_VariableFloat( "r_imageSharpenAmount" );
	s_settingsMenu->video.exposure = Cvar_VariableFloat( "r_autoExposure" );
//	s_settingsMenu->video.maxFPS = Cvar_VariableInteger( "com_maxfps" );
}

static void AudioMenu_SetDefault( void )
{
	s_settingsMenu->audio.masterVolume = Cvar_VariableInteger( "snd_masterVolume" );
	s_settingsMenu->audio.musicVolume = Cvar_VariableInteger( "snd_musicVolume" );
	s_settingsMenu->audio.sfxVolume = Cvar_VariableInteger( "snd_effectsVolume" );
	s_settingsMenu->audio.musicOn = Cvar_VariableInteger( "snd_musicOn" );
	s_settingsMenu->audio.sfxOn = Cvar_VariableInteger( "snd_effectsOn" );
	s_settingsMenu->audio.maxSoundChannels = Cvar_VariableInteger( "snd_maxSoundChannels" );
}

static void ControlsMenu_SetDefault( void )
{
	int i;

	s_settingsMenu->controls.mouseAcceleration = Cvar_VariableInteger( "g_mouseAcceleration" );
	s_settingsMenu->controls.mouseSensitivity = Cvar_VariableFloat( "g_mouseSensitivity" );

	memcpy( s_settingsMenu->controls.keybinds, s_defaultKeybinds, sizeof( s_defaultKeybinds ) );
	for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
//		s_settingsMenu->controls.keybinds[i].bind1 = s_defaultKeybinds[i].defaultBind1;
//		s_settingsMenu->controls.keybinds[i].bind2 = s_defaultKeybinds[i].defaultBind2;

		s_settingsMenu->controls.keybinds[i].bind1 = Key_GetKey( s_defaultKeybinds[i].command );
		s_settingsMenu->controls.keybinds[i].bind2 = -1;
	}
}

static void GameplayMenu_SetDefault( void )
{
	s_settingsMenu->gameplay.difficulty = Cvar_VariableInteger( "sgame_Difficulty" );
	s_settingsMenu->gameplay.mouseCursor = Cvar_VariableInteger( "sgame_CursorType" );
	s_settingsMenu->gameplay.debugPrint = Cvar_VariableInteger( "sgame_DebugMode" );
	s_settingsMenu->gameplay.toggleHUD = Cvar_VariableInteger( "sgame_ToggleHUD" );
	s_settingsMenu->gameplay.pauseUnfocused = Cvar_VariableInteger( "com_pauseUnfocused" );
}

static void SettingsMenu_Draw( void )
{
	const int windowFlags = MENU_DEFAULT_FLAGS;
	ImVec2 itemSpacing;

	if ( s_settingsMenu->controls.rebindKey ) {
		SettingsMenu_Rebind();
	}

	SettingsMenu_DrawHint();

	itemSpacing = ImGui::GetStyle().ItemSpacing;
	ImGui::GetStyle().ItemSpacing.y = 0.0f;

	ImGui::Begin( "SettingsMenu##MainMenuSettingsConfigThingy", NULL, windowFlags );
	ImGui::SetWindowSize( ImVec2( s_settingsMenu->menu.width, s_settingsMenu->menu.height ) );
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.x, s_settingsMenu->menu.y ) );

	SettingsMenu_CheckModified();
	UI_EscapeMenuToggle();

	// [SIREngine] 6/3/24
	// fixed pause menu to settings menu
	if ( ui->activemenu != &s_settingsMenu->menu && ui->menustate == UI_MENU_PAUSE ) {
		UI_SetActiveMenu( UI_MENU_PAUSE );
	}
	if ( UI_MenuTitle( "Settings" ) ) {
		if ( ui->menustate == UI_MENU_PAUSE ) {
			UI_SetActiveMenu( UI_MENU_PAUSE );
		} else {
			UI_PopMenu();
		}
		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale( ( 1.2f * ImGui::GetFont()->Scale ) * ui->scale );
	SettingsMenu_TabBar();
	ImGui::SetWindowFontScale( ( 1.75f * ImGui::GetFont()->Scale ) * ui->scale );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, colorBrown );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, colorBrass );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, colorBrass );
	ImGui::PushStyleColor( ImGuiCol_Button, colorBrown );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, colorBrass );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colorBrass );

	switch ( s_settingsMenu->lastChild ) {
	case ID_VIDEO:
		VideoMenu_Draw();
		break;
	case ID_PERFORMANCE:
		PerformanceMenu_Draw();
		break;
	case ID_AUDIO:
		AudioMenu_Draw();
		break;
	case ID_CONTROLS:
		ControlsMenu_Draw();
		break;
	case ID_GAMEPLAY:
		GameplayMenu_Draw();
		break;
	case ID_MODS:
		ModuleMenu_Draw();
		break;
	};

	if ( s_settingsMenu->modified ) {
		s_settingsMenu->preset = PRESET_CUSTOM;
	}
	if ( !ImGui::IsAnyItemHovered() ) {
		s_settingsMenu->focusedItem = NULL;
	}

	ImGui::PopStyleColor( 6 );
	ImGui::GetStyle().ItemSpacing = itemSpacing;
	ImGui::End();

	//
	// draw the other widgets (save/setdefaults)
	//
	if ( s_settingsMenu->modified ) {
		ImGui::Begin( "##SettingsMenuButtons", NULL, MENU_DEFAULT_FLAGS | ImGuiWindowFlags_AlwaysAutoResize );
		ImGui::SetWindowPos( ImVec2( 280 * ui->scale, 670 * ui->scale ) );
		ImGui::SetCursorScreenPos( ImVec2( 280 * ui->scale, 680 * ui->scale ) );
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 2.75f );
		if ( in_mode->i == 0 ) {
			ImGui::Image( (ImTextureID)(uintptr_t)( s_settingsMenu->saveHovered ? s_settingsMenu->save_1 : s_settingsMenu->save_0 ),
				ImVec2( 256 * ui->scale, 72 * ui->scale ) );
			if ( !s_settingsMenu->saveHovered && ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
				Snd_PlaySfx( ui->sfx_move );
			}
		} else {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			if ( s_settingsMenu->saveHovered ) {
				ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
			}
			ImGui::Button( "Save Settings", ImVec2( 0, ( 32 * s_settingsMenu->menu.textFontScale ) * ui->scale ) );
			ImGui::PopStyleColor( 3 );
			if ( s_settingsMenu->saveHovered ) {
				ImGui::PopStyleColor();
			}
		}
		s_settingsMenu->saveHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			Cbuf_ExecuteText( EXEC_APPEND, "writecfg " LOG_DIR "/" NOMAD_CONFIG "\n" );
			switch ( s_settingsMenu->lastChild ) {
			case ID_VIDEO:
				VideoMenu_Save();
				break;
			case ID_PERFORMANCE:
				PerformanceMenu_Save();
				break;
			case ID_AUDIO:
				AudioMenu_Save();
				break;
			case ID_CONTROLS:
				ControlsMenu_Save();
				break;
			case ID_GAMEPLAY:
				GameplayMenu_Save();
				break;
			case ID_MODS:
				ModuleMenu_Save();
				break;
			};
			SettingsMenu_GetInitial();
		}

		ImGui::SetCursorScreenPos( ImVec2( 540 * ui->scale, 680 * ui->scale ) );
		if ( in_mode->i == 0 ) {
			ImGui::Image( (ImTextureID)(uintptr_t)( s_settingsMenu->setDefaultsHovered ? s_settingsMenu->reset_1 : s_settingsMenu->reset_0 ),
				ImVec2( 256 * ui->scale, 72 * ui->scale ) );
			if ( !s_settingsMenu->setDefaultsHovered && ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled
				| ImGuiHoveredFlags_DelayNone ) )
			{
				Snd_PlaySfx( ui->sfx_move );
			}
		} else {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
			if ( s_settingsMenu->setDefaultsHovered ) {
				ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
			}
			ImGui::Button( "Set Default", ImVec2( 0, ( 32 * s_settingsMenu->menu.textFontScale ) * ui->scale ) );
			ImGui::PopStyleColor( 3 );
			if ( s_settingsMenu->setDefaultsHovered ) {
				ImGui::PopStyleColor();
			}
		}
		s_settingsMenu->setDefaultsHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			switch ( s_settingsMenu->lastChild ) {
			case ID_VIDEO:
				VideoMenu_SetDefault();
				break;
			case ID_PERFORMANCE:
				PerformanceMenu_SetDefault();
				break;
			case ID_AUDIO:
				AudioMenu_SetDefault();
				break;
			case ID_CONTROLS:
				ControlsMenu_SetDefault();
				break;
			case ID_GAMEPLAY:
				GameplayMenu_SetDefault();
				break;
			};
		}
		ImGui::End();
	}
}

qboolean R_HasExtension( const char *ext )
{
    const char *ptr = N_stristr( gi.gpuConfig.extensions_string, ext );
	if ( ptr == NULL ) {
		return qfalse;
	}
	ptr += strlen( ext );
	return ( ( *ptr == ' ' ) || ( *ptr == '\0' ) );  // verify its complete string.
}

#define NUM_ANTIALIAS_TYPES 11
#define NUM_ANISOTROPY_TYPES 5
#define NUM_HUD_OPTIONS 4
#define NUM_VSYNC_TYPES 3
#define NUM_WINDOW_MODES 4
#define NUM_WINDOW_SIZES 22
#define NUM_TEXTURE_FILTERS 4
#define NUM_TEXTURE_DETAILS 5

void SettingsMenu_Cache( void )
{
	char str[MAXPRINTMSG];
	char *p;
	int i;

	static const char *s_multisampleTypes[ NUM_ANTIALIAS_TYPES ];
	static const char *s_anisotropyTypes[ NUM_ANISOTROPY_TYPES ];
	static const char *s_textureDetail[ NUM_TEXTURE_DETAILS ];
	static const char *s_textureFilters[ NUM_TEXTURE_FILTERS ];
	static const char *s_toneMappingTypes[] = {
	    "Reinhard",
	    "Exposure"
	};
	static const char *s_windowSizes[ NUM_WINDOW_SIZES ];
	static const char *s_vsync[ NUM_VSYNC_TYPES ];
	static const char *difficulties[ NUMDIFS - 1 ];
	static const char *s_mouseTypes[] = {
		"dot",
		"circle & dot",
		"full crosshair",
		"filled crosshair"
	};
	static const char *s_bufferModeOptions[] = {
		"Immediate (Legacy, Untested)",
		"Client (Legacy, Untested)",
		"GPU Buffered (OpenGL 3.3)",
		"GPU To Client Mapping (OpenGL 4.5)"
	};
	static const char *s_hudOptions[ NUM_HUD_OPTIONS ];
	static const char *s_presetLabels[] = {
		"Low",
		"Normal",
		"High",
		"Performance",
		"Quality",
		"Custom"
	};
	static const char *s_onOff[] = {
		"OFF",
		"ON"
	};
	static const char *s_windowModes[ NUM_WINDOW_MODES ];

	s_multisampleTypes[0] = strManager->ValueForKey( "GAMEUI_NONE" )->value;
	s_multisampleTypes[1] = strManager->ValueForKey( "GAMEUI_2X_MSAA" )->value;
	s_multisampleTypes[2] = strManager->ValueForKey( "GAMEUI_4X_MSAA" )->value;
	s_multisampleTypes[3] = strManager->ValueForKey( "GAMEUI_8X_MSAA" )->value;
	s_multisampleTypes[4] = strManager->ValueForKey( "GAMEUI_16X_MSAA" )->value;
	s_multisampleTypes[5] = strManager->ValueForKey( "GAMEUI_32X_MSAA" )->value;
	s_multisampleTypes[6] = strManager->ValueForKey( "GAMEUI_2X_SSAA" )->value;
	s_multisampleTypes[7] = strManager->ValueForKey( "GAMEUI_4X_SSAA" )->value;
	s_multisampleTypes[8] = strManager->ValueForKey( "GAMEUI_TAA" )->value;
	s_multisampleTypes[9] = strManager->ValueForKey( "GAMEUI_SMAA" )->value;
	s_multisampleTypes[10] = strManager->ValueForKey( "GAMEUI_FXAA" )->value;

	s_anisotropyTypes[0] = strManager->ValueForKey( "GAMEUI_ANISOTROPIC2X" )->value;
	s_anisotropyTypes[1] = strManager->ValueForKey( "GAMEUI_ANISOTROPIC4X" )->value;
	s_anisotropyTypes[2] = strManager->ValueForKey( "GAMEUI_ANISOTROPIC8X" )->value;
	s_anisotropyTypes[3] = strManager->ValueForKey( "GAMEUI_ANISOTROPIC16X" )->value;
	s_anisotropyTypes[4] = strManager->ValueForKey( "GAMEUI_ANISOTROPIC32X" )->value;

	s_vsync[0] = strManager->ValueForKey( "GAMEUI_VSYNC_ADAPTIVE" )->value;
	s_vsync[1] = strManager->ValueForKey( "GAMEUI_DISABLED" )->value;
	s_vsync[2] = strManager->ValueForKey( "GAMEUI_ENABLED" )->value;

	s_textureFilters[0] = strManager->ValueForKey( "GAMEUI_BILINEAR" )->value;
	s_textureFilters[1] = strManager->ValueForKey( "GAMEUI_NEAREST" )->value;
	s_textureFilters[2] = strManager->ValueForKey( "GAMEUI_LINEARNEAREST" )->value;
	s_textureFilters[3] = strManager->ValueForKey( "GAMEUI_NEARESTLINEAR" )->value;

	s_textureDetail[0] = strManager->ValueForKey( "GAMEUI_TEXDETAIL_VERYLOW" )->value;
	s_textureDetail[1] = strManager->ValueForKey( "GAMEUI_TEXDETAIL_LOW" )->value;
	s_textureDetail[2] = strManager->ValueForKey( "GAMEUI_TEXDETAIL_MEDIUM" )->value;
	s_textureDetail[3] = strManager->ValueForKey( "GAMEUI_TEXDETAIL_HIGH" )->value;
	s_textureDetail[4] = strManager->ValueForKey( "GAMEUI_TEXDETAIL_ULTRA" )->value;

	s_windowSizes[0] = strManager->ValueForKey( "GAMEUI_WINDOW_NATIVE" )->value;
	s_windowSizes[1] = strManager->ValueForKey( "GAMEUI_WINDOW_CUSTOM" )->value;
	s_windowSizes[2] = strManager->ValueForKey( "GAMEUI_WINDOW_1024X768" )->value;
	s_windowSizes[3] = strManager->ValueForKey( "GAMEUI_WINDOW_1280X720" )->value;
	s_windowSizes[4] = strManager->ValueForKey( "GAMEUI_WINDOW_1280X800" )->value;
	s_windowSizes[5] = strManager->ValueForKey( "GAMEUI_WINDOW_1280X1024" )->value;
	s_windowSizes[6] = strManager->ValueForKey( "GAMEUI_WINDOW_1440X900" )->value;
	s_windowSizes[7] = strManager->ValueForKey( "GAMEUI_WINDOW_1440X960" )->value;
	s_windowSizes[8] = strManager->ValueForKey( "GAMEUI_WINDOW_1600X900" )->value;
	s_windowSizes[9] = strManager->ValueForKey( "GAMEUI_WINDOW_1600X1200" )->value;
	s_windowSizes[10] = strManager->ValueForKey( "GAMEUI_WINDOW_1600X1050" )->value;
	s_windowSizes[11] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X800" )->value;
	s_windowSizes[12] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1080" )->value;
	s_windowSizes[13] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1200" )->value;
	s_windowSizes[14] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1280" )->value;
	s_windowSizes[15] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1080" )->value;
	s_windowSizes[16] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1440" )->value;
	s_windowSizes[17] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1600" )->value;
	s_windowSizes[18] = strManager->ValueForKey( "GAMEUI_WINDOW_2880X1620" )->value;
	s_windowSizes[19] = strManager->ValueForKey( "GAMEUI_WINDOW_3200X1800" )->value;
	s_windowSizes[20] = strManager->ValueForKey( "GAMEUI_WINDOW_3840X1600" )->value;
	s_windowSizes[21] = strManager->ValueForKey( "GAMEUI_WINDOW_3840X2160" )->value;

	s_hudOptions[0] = strManager->ValueForKey( "MENU_HUD" )->value;
	s_hudOptions[1] = strManager->ValueForKey( "MENU_ADVANCED_HUD" )->value;
	s_hudOptions[2] = strManager->ValueForKey( "MENU_HUD_STYLE" )->value;
	s_hudOptions[3] = strManager->ValueForKey( "MENU_HUD_PSTATS" )->value;

	s_windowModes[0] = strManager->ValueForKey( "GAMEUI_MODE_WINDOWED" )->value;
	s_windowModes[1] = strManager->ValueForKey( "GAMEUI_MODE_BORDERLESS_WINDOWED" )->value;
	s_windowModes[2] = strManager->ValueForKey( "GAMEUI_MODE_FULLSCREEN" )->value;
	s_windowModes[3] = strManager->ValueForKey( "GAMEUI_MODE_BORDERLESS_FULLSCREEN" )->value;

	if ( !ui->uiAllocated ) {
		s_settingsMenu = (settingsMenu_t *)Hunk_Alloc( sizeof( *s_settingsMenu ), h_high );
		SettingsMenu_InitPresets();
	}

	s_settingsMenu->hintLabel = NULL;
	s_settingsMenu->hintMessage = NULL;
	
	s_settingsMenu->menu.fullscreen = qtrue;
	s_settingsMenu->menu.x = 0;
	s_settingsMenu->menu.y = 0;
	s_settingsMenu->menu.draw = SettingsMenu_Draw;
	s_settingsMenu->menu.flags = MENU_DEFAULT_FLAGS;
	s_settingsMenu->menu.width = ui->gpuConfig.vidWidth * 0.60f;
	s_settingsMenu->menu.height = ui->gpuConfig.vidHeight - ( 100 * ui->scale );
	s_settingsMenu->menu.titleFontScale = 3.5f;
	s_settingsMenu->menu.textFontScale = 1.5f;
	s_settingsMenu->lastChild = ID_VIDEO;

	s_settingsMenu->performance.onoff = s_onOff;
	s_settingsMenu->advancedPerformance.toneMappingTypes = s_toneMappingTypes;
	s_settingsMenu->performance.multisampleTypes = s_multisampleTypes;
	s_settingsMenu->advancedPerformance.anisotropyTypes = s_anisotropyTypes;
	s_settingsMenu->performance.textureDetails = s_textureDetail;
	s_settingsMenu->performance.textureFilters = s_textureFilters;
	s_settingsMenu->presetNames = s_presetLabels;

	s_settingsMenu->gameplay.difficultyNames = difficulties;

	s_settingsMenu->video.vsyncList = s_vsync;
	s_settingsMenu->video.windowSizes = s_windowSizes;
	s_settingsMenu->video.windowModes = s_windowModes;

	s_settingsMenu->video.numVSync = arraylen( s_vsync );
	s_settingsMenu->video.numWindowSizes = arraylen( s_windowSizes );
	s_settingsMenu->video.numWindowModes = arraylen( s_windowModes );

	s_settingsMenu->performance.numMultisampleTypes = arraylen( s_multisampleTypes );
	s_settingsMenu->advancedPerformance.numAnisotropyTypes = arraylen( s_anisotropyTypes );
	s_settingsMenu->performance.numTextureDetails = arraylen( s_textureDetail );
	s_settingsMenu->performance.numTextureFilters = arraylen( s_textureFilters );
	s_settingsMenu->advancedPerformance.numToneMappingTypes = arraylen( s_toneMappingTypes );

	s_settingsMenu->gameplay.numDifficultyTypes = arraylen( difficulties );

	s_settingsMenu->gpuMemInfoType = GPU_MEMINFO_NVX;

	PerformanceMenu_SetDefault();
	VideoMenu_SetDefault();
	AudioMenu_SetDefault();
	ControlsMenu_SetDefault();
	GameplayMenu_SetDefault();

	SettingsMenu_GetInitial();

	s_settingsMenu->save_0 = re.RegisterShader( "menu/save_0" );
	s_settingsMenu->save_1 = re.RegisterShader( "menu/save_1" );

	s_settingsMenu->reset_0 = re.RegisterShader( "menu/reset_0" );
	s_settingsMenu->reset_1 = re.RegisterShader( "menu/reset_1" );
}

void UI_SettingsMenu( void ) {
	SettingsMenu_Cache();
	UI_PushMenu( &s_settingsMenu->menu );
}