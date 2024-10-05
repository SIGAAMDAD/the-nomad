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
#include <fmod/fmod.h>

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

typedef int quality_t;
typedef int toggle_t;
typedef int select_t;

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
	int maxFPS;
	int performanceMonitor;

	float gamma;
	float exposure;
	float sharpening;
} videoSettings_t;

typedef struct {
	const char **multisampleTypes;
	const char **textureDetails;
	const char **textureFilters;
	const char **onoff;
	const char **qualityTypes;

	int numMultisampleTypes;
	int numTextureDetails;
	int numTextureFilters;
	int numQualities;

	select_t textureFilter;
	select_t multisampleType;
	
	quality_t multisampleQuality;
	quality_t textureDetail;
	quality_t lightingQuality;
	quality_t effectsQuality;
	float fixedResolutionScaling;

	toggle_t loadTexturesOnDemand;
	toggle_t particles;
	toggle_t autoExposure;
	toggle_t dynamicLighting;
	toggle_t bloom;
	toggle_t fixedRendering;
} performanceSettings_t;

typedef struct {
	performanceSettings_t basic;
} preset_t;

typedef struct {
	const char **speakermodeTypes;

	int numSpeakermodeTypes;

	int sfxVolume;
	int musicVolume;
	int masterVolume;

	// size = uint16_t
	int sfxOn;
	int musicOn;

	int speakerMode;
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
	videoSettings_t video;
	audioSettings_t audio;
	controlsSettings_t controls;
	gameplaySettings_t gameplay;

	uint64_t currentModSettings;

	GLint availableGPUMemory;
	GLint totalGPUMemory;

	const char *hintLabel;
	const char *hintMessage;

	const void *focusedItem;
	const void *currentItem;

	qboolean playedClickSound;
	qboolean playedHoverSound;
	qboolean modified;

	gpuMemory_t memUsage;
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
	if ( memcmp( &s_settingsMenu->audio, &s_initial->audio, sizeof( audioSettings_t ) ) != 0 ) {
		s_settingsMenu->modified = qtrue;
	}
}

static float PerformanceMenu_CalcScore( void )
{
	float score;

	score = 100.0f;
	
	if ( s_settingsMenu->performance.bloom ) {
		score -= 45.0f;
	}
	switch ( s_settingsMenu->performance.multisampleType ) {
	case AntiAlias_None:
		score += 50.0f;
		break;
	case AntiAlias_SSAA:
		score -= 6.0f * s_settingsMenu->performance.multisampleQuality;;
		break;
	case AntiAlias_MSAA:
		score -= 10.0f * s_settingsMenu->performance.multisampleQuality;
		break;
	case AntiAlias_FXAA:
		score -= 2.0f * s_settingsMenu->performance.multisampleQuality;;
		break;
	};
	
	if ( s_settingsMenu->performance.dynamicLighting ) {
		score -= 16.0f;
	} else {
		score += 16.0f;
	}

	if ( s_settingsMenu->performance.lightingQuality == 0 ) {
		score -= 5.0f;
	} else if ( s_settingsMenu->performance.lightingQuality == 1 ) {
		score -= 15.0f;
	} else if ( s_settingsMenu->performance.lightingQuality == 2 ) {
		score -= 20.0f;
	}

	return score;
}

const char *Hunk_CopyString( const char *str, ha_pref pref ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Hunk_Alloc( len, pref );
    N_strncpyz( out, str, len );

    return out;
}

static void SettingsMenu_InitPresets( void ) {
	/*
	for reference:

	bloom = frame killer
	lighting quality = anything higher than 1 is a frame killer
	multisampling = MSAA > 4 is a frame killer
	*/

	s_settingsMenu->presets = (preset_t *)Hunk_Alloc( sizeof( *s_settingsMenu->presets ) * NUM_PRESETS, h_high );
	s_settingsMenu->preset = PRESET_NORMAL;

	memset( s_settingsMenu->presets, 0, sizeof( performanceSettings_t ) * NUM_PRESETS );

	// some quality but more optimized just for playability
	s_settingsMenu->presets[ PRESET_LOW ].basic.multisampleType = AntiAlias_FXAA;
	s_settingsMenu->presets[ PRESET_LOW ].basic.multisampleQuality = 0;
	s_settingsMenu->presets[ PRESET_LOW ].basic.textureDetail = TexDetail_IntegratedGPU;
	s_settingsMenu->presets[ PRESET_LOW ].basic.textureFilter = TEXFILTER_ANISOTROPY2;
	s_settingsMenu->presets[ PRESET_LOW ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_LOW ].basic.bloom = qfalse;
	s_settingsMenu->presets[ PRESET_LOW ].basic.lightingQuality = 0;

	s_settingsMenu->presets[ PRESET_NORMAL ].basic.multisampleType = AntiAlias_MSAA;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.multisampleQuality = 1;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.textureDetail = TexDetail_Normie;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.textureFilter = TEXFILTER_ANISOTROPY4;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.bloom = qfalse;
	s_settingsMenu->presets[ PRESET_NORMAL ].basic.lightingQuality = 1;

	s_settingsMenu->presets[ PRESET_HIGH ].basic.multisampleType = AntiAlias_MSAA;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.multisampleQuality = 2;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.textureDetail = TexDetail_ExpensiveShitWeveGotHere;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.textureFilter = TEXFILTER_ANISOTROPY8;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.bloom = qtrue;
	s_settingsMenu->presets[ PRESET_HIGH ].basic.lightingQuality = 2;

	// highest quality rendering, no care for performance
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.multisampleType = AntiAlias_MSAA;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.multisampleQuality = 2;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.textureDetail = TexDetail_GPUvsGod;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.textureFilter = TEXFILTER_ANISOTROPY16;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.dynamicLighting = qtrue;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.bloom = qtrue;
	s_settingsMenu->presets[ PRESET_QUALITY ].basic.lightingQuality = 2;
	
	// looks the worst but gets the best framerate and much less memory consumption
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.multisampleType = AntiAlias_None;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.multisampleQuality = 0;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.textureDetail = TexDetail_MSDOS;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.textureFilter = TEXFILTER_ANISOTROPY2;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.dynamicLighting = qfalse;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.bloom = qfalse;
	s_settingsMenu->presets[ PRESET_PERFORMANCE ].basic.lightingQuality = 0;
}

static void SettingsMenu_SetPreset( const preset_t *preset )
{
	s_settingsMenu->performance.bloom = preset->basic.bloom;
	s_settingsMenu->performance.dynamicLighting = preset->basic.dynamicLighting;
	s_settingsMenu->performance.multisampleType = preset->basic.multisampleType;
	s_settingsMenu->performance.textureFilter = preset->basic.textureFilter;
	s_settingsMenu->performance.textureDetail = preset->basic.textureDetail;
	s_settingsMenu->performance.lightingQuality = preset->basic.lightingQuality;
	s_settingsMenu->performance.multisampleQuality = preset->basic.multisampleQuality;
}

static inline void SfxFocused( const void *item ) {
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		if ( s_settingsMenu->focusedItem != item ) {
			s_settingsMenu->focusedItem = item;
//			Snd_PlaySfx( ui->sfx_move );
		}
	}
}

static void SettingsMenu_GetGPUMemoryInfo( void )
{
	re.GetGPUMemStats( &s_settingsMenu->memUsage );
	switch ( s_settingsMenu->gpuMemInfoType ) {
	case GPU_MEMINFO_NVX: {
		GLint dedicatedMemory, availableMemory, totalMemory;
		float realDedicated, realAvailable, realTotal;

		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedicatedMemory );
		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory );
		renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemory );

		s_settingsMenu->availableGPUMemory = availableMemory;
		s_settingsMenu->totalGPUMemory = totalMemory;
		break; }
	case GPU_MEMINFO_ATI: {
		GLint vboMemory, textureMemory, renderbufferMemory;
		float realVbo, realTexture, realRenderbuffer;

		renderImport.glGetIntegerv( GL_VBO_FREE_MEMORY_ATI, &vboMemory );
		renderImport.glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, &textureMemory );
		renderImport.glGetIntegerv( GL_RENDERBUFFER_FREE_MEMORY_ATI, &renderbufferMemory );

		// the ATI memory usage estimation will always be very rough

		s_settingsMenu->totalGPUMemory = s_settingsMenu->memUsage.estTextureMemUsed + s_settingsMenu->memUsage.estBufferMemUsed
			+ s_settingsMenu->memUsage.estRenderbufferMemUsed + vboMemory + textureMemory + renderbufferMemory;
		s_settingsMenu->availableGPUMemory = vboMemory + textureMemory + renderbufferMemory;
		break; }
	case GPU_MEMINFO_NONE: {
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
	const char *name;
	
	name = va( "##%sSettingsMenuConfigList", label );

	ImGui::PushStyleColor( ImGuiCol_Text, colorLimeGreen );
	if ( ImGui::BeginCombo( name, itemnames[*curitem] ) ) {
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
	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) && !s_settingsMenu->playedClickSound ) {
		Snd_PlaySfx( ui->sfx_select );
		s_settingsMenu->playedClickSound = qtrue;
	} else {
		s_settingsMenu->playedClickSound = qfalse;
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
			Snd_PlaySfx( ui->sfx_select );
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
	float delta, bool enabled )
{
	if ( !enabled ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}

	ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	/*
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigLeft", label ), ImGuiDir_Left ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curvalue ) -= delta;
		if ( *curvalue < minvalue ) {
			*curvalue = minvalue;
		}
	}
	ImGui::SameLine();
	*/
	ImGui::SliderFloat( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue, "%.3f", enabled ? 0 : ImGuiSliderFlags_NoInput );
	/*
	ImGui::SameLine();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigRight", label ), ImGuiDir_Right ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curvalue ) += delta;
		if ( *curvalue > maxvalue ) {
			*curvalue = maxvalue;
		}
	}
	*/
	ImGui::PopStyleColor();

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
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

	ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	ImGui::TableNextColumn();
	SettingsMenu_Text( name, hint );
	ImGui::TableNextColumn();
	/*
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigLeft", label ), ImGuiDir_Left ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select );
			( *curvalue ) -= delta;
			if ( *curvalue < minvalue ) {
				*curvalue = minvalue;
			}
		}
	}
	ImGui::SameLine();
	*/
	ImGui::SliderInt( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue, "%d", enabled ? 0 : ImGuiSliderFlags_NoInput );
	/*
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
	ImGui::PopStyleColor();
	*/

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

static void SettingsMenu_GetNewBindings( void )
{
	int i;

	for ( i = 0; i < NUMKEYBINDS; i++ ) {
		s_settingsMenu->controls.keybinds[i].bind1 = Key_GetKey( s_settingsMenu->controls.keybinds[i].command );
	}
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
						s_settingsMenu->controls.rebindKey = NULL;
						s_settingsMenu->controls.rebindIndex = 0;
                    } else {
						s_settingsMenu->controls.rebindKey->bind1 = i;
						s_settingsMenu->controls.keybinds[ index ].bind1 = -1;
						Snd_PlaySfx( ui->sfx_select );
						s_settingsMenu->controls.rebindKey = NULL;
						s_settingsMenu->controls.rebindIndex = 0;
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
						s_settingsMenu->controls.rebindKey = NULL;
						s_settingsMenu->controls.rebindIndex = 0;
                    } else {
						s_settingsMenu->controls.rebindKey->bind2 = i;
						s_settingsMenu->controls.keybinds[ index ].bind2 = -1;
						Snd_PlaySfx( ui->sfx_select );
						s_settingsMenu->controls.rebindKey = NULL;
						s_settingsMenu->controls.rebindIndex = 0;
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

			s_settingsMenu->controls.rebindKey = NULL;
			s_settingsMenu->controls.rebindIndex = 0;

			SettingsMenu_GetNewBindings();
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
	float score;

	score = PerformanceMenu_CalcScore();

	flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	ImGui::Begin( "##SettingsMenuHintWindow", NULL, flags );
	ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth - s_settingsMenu->menu.width, 472 * ui->scale ) );
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.width, 100 * ui->scale ) );

	const ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	if ( s_settingsMenu->lastChild == ID_PERFORMANCE ) {
		ImGui::SetCursorScreenPos( ImVec2( ImGui::GetWindowPos().x, ( 100 * ui->scale ) + 300 * ui->scale ) );
		FontCache()->SetActiveFont( RobotoMono );
		ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
		ImGui::TextUnformatted( "Est. Performance Score:" );
		ImGui::Text( "%0.02f", score );
	}

	if ( !s_settingsMenu->hintLabel || !s_settingsMenu->hintMessage ) {
		ImGui::End();
		return;
	}

	ImGui::SetCursorScreenPos( cursorPos );

	FontCache()->SetActiveFont( AlegreyaSC );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
	ImGui::TextUnformatted( s_settingsMenu->hintLabel );

	FontCache()->SetActiveFont( RobotoMono );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.0f ) * ui->scale );
	ImGui::TextWrapped( "%s", s_settingsMenu->hintMessage );

	ImGui::End();
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
		if ( ImGui::Button( va( "%s##Binding%i", bind, i ) ) ) {
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
		SettingsMenu_RadioButton( strManager->ValueForKey( "GAMEUI_MOUSEACCEL" )->value, "MouseAcceleration",
			"Toggles mouse acceleration", &s_settingsMenu->controls.mouseAcceleration, true );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( strManager->ValueForKey( "GAMEUI_MOUSESENSITIVITY" )->value, "MouseSensitivity",
			"Sets the speed of the mouse",
			&s_settingsMenu->controls.mouseSensitivity, 1.0f, 50.0f, 1.0f, true );

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
		true );

	ImGui::TableNextRow();

	SettingsMenu_MultiAdjustable( "ANTIALIASING QUALITY", "AntiAliasingQuality",
		"Sets anti-aliasing quality",
		s_settingsMenu->performance.qualityTypes + 1, s_settingsMenu->performance.numQualities - 2,
		&s_settingsMenu->performance.multisampleQuality, true );
	
	ImGui::TableNextRow();
	
	SettingsMenu_MultiAdjustable( "TEXTURE QUALITY", "TextureQuality",
		"Sets the quality of textures rendered, may effect performance",
		s_settingsMenu->performance.textureDetails, s_settingsMenu->performance.numTextureDetails,
		&s_settingsMenu->performance.textureDetail, true );

	ImGui::TableNextRow();

//	ImGui::TableNextRow();
//
//	SettingsMenu_MultiAdjustable( "TEXTURE FILTERING", "TextureFiltering",
//		"Sets the type of texture filtering",
//		s_settingsMenu->advancedPerformance.anisotropyTypes, s_settingsMenu->advancedPerformance.numAnisotropyTypes,
//		&s_settingsMenu->performance.textureFilter, true );

	SettingsMenu_MultiAdjustable( "LIGHTING QUALITY", "LightingQuality",
		"Sets lighting quality", s_settingsMenu->performance.qualityTypes + 1, s_settingsMenu->performance.numQualities - 2,
		&s_settingsMenu->performance.lightingQuality, true );

	ImGui::TableNextRow();

	SettingsMenu_RadioButton( "FIXED RENDERING", "FixedRendering",
		"Forces the engine to render at a fixed virtual resolution (based on \"FIXED RESOLUTION SCALE\")."
		"This might increase performance on lower end systems", &s_settingsMenu->performance.fixedRendering, true );

	ImGui::TableNextRow();

	SettingsMenu_MultiSliderFloat( "FIXED RESOLUTION SCALE", "FixedResolutionScaling",
		"Sets the fixed resolution scaling.", &s_settingsMenu->performance.fixedResolutionScaling, 0.0f, 1.0f, 0.01f,
		s_settingsMenu->performance.fixedRendering );

	ImGui::TableNextRow();
	
	SettingsMenu_RadioButton( "BLOOM", "Bloom",
		"Enables bloom to make light sources stand out more in an environment",
		&s_settingsMenu->performance.bloom, true );

	ImGui::TableNextRow();

	SettingsMenu_RadioButton( "DYNAMIC LIGHTING", "DynamicLighting",
		"Enables per-pixel dynamic lighting", &s_settingsMenu->performance.dynamicLighting, true );
	
	ImGui::EndTable();
}

static void PerformanceMenu_Draw( void )
{
	int preset;

	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##PerformanceSettingsMenuConfigTable", 2 );
	{
		preset = s_settingsMenu->preset;
		SettingsMenu_MultiAdjustable( "PRESET", "GraphicsPresets",
			"",
			s_settingsMenu->presetNames, NUM_PRESETS, &preset, true );
		
		if ( preset != s_settingsMenu->preset ) {
			SettingsMenu_SetPreset( &s_settingsMenu->presets[ preset ] );
			s_settingsMenu->preset = preset;
		}

		ImGui::EndTable();

		PerformanceMenu_DrawBasic();
	}

	ImGui::Begin( "##GPUMemoryInfo", NULL, MENU_DEFAULT_FLAGS );
	ImGui::SetWindowSize( ImVec2( 390 * ui->scale + ui->bias, 100 * ui->scale ) );
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.width + 50 * ui->scale + ui->bias, 600 * ui->scale ) );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 2.5f );

	ImGui::End();
}

static void AudioMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##AudioSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiAdjustable( "SPEAKER MODE", "SpeakerMode",
			"Sets the speaker configuration",
			s_settingsMenu->audio.speakermodeTypes, s_settingsMenu->audio.numSpeakermodeTypes,
			&s_settingsMenu->audio.speakerMode, true );

		ImGui::TableNextRow();

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

		SettingsMenu_RadioButton( "MUSIC ON", "MusicOn",
			"Toggles Music", &s_settingsMenu->audio.musicOn, true );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "SFX ON", "SfxOn",
			"Toggles Sound Effects", &s_settingsMenu->audio.sfxOn, true );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiSliderInt( "MAX SOUND CHANNELS", "MaxSoundChannels",
			"Sets the maximum amount of channels that can processed at a time, will increase CPU load.",
			&s_settingsMenu->audio.maxSoundChannels, 64, 512, 1, true );
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

		SettingsMenu_MultiSliderFloat( "GAMMA", "Gamma",
			"Sets gamma linear light correction factor",
			&s_settingsMenu->video.gamma, 0.5f, 3.0f, 0.10f, true );

		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "BRIGHTNESS", "Exposure",
			"Sets brightness in a rendered scene",
			&s_settingsMenu->video.exposure, 0.10f, 10.0f, 1.0f, true );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "IMAGE SHARPENING", "ImageSharpening",
			"Sets the amount of sharpening applied to a rendered texture",
			&s_settingsMenu->video.sharpening, 0.5f, 20.0f, 0.1f, true );
		
		ImGui::TableNextRow();

		SettingsMenu_MultiSliderInt( "FRAME LIMITER", "FrameLimiter",
			"Sets the maximum amount of frames the game can render per second.",
			&s_settingsMenu->video.maxFPS, 0, 1000, 1, true );
		
		/*
		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "PERFORMANCE MONITOR", "PerfomanceMonitor",
			"", s_settingsMenu->performance.qualityTypes, s_settingsMenu->performance.numQualities,
			&s_settingsMenu->video.performanceMonitor, true );
		*/
	}
	ImGui::EndTable();
}

static void GameplayMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##GameSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_RadioButton( "TOGGLE HUD", "ToggleHUD",
			"Toggles Heads-Up-Display (HUD). Turn this off if you want a more immersive experience",
			&s_settingsMenu->gameplay.toggleHUD, true );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "DEBUG MODE", "DebugMode",
			"Toggles debug messages from SGame",
			&s_settingsMenu->gameplay.debugPrint, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "STOP GAME ON FOCUS LOST", "StopGameOnFocusLost",
			"If on the game will pause when the window is unfocused",
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

	switch ( s_settingsMenu->video.windowResolution ) {
	case 0:
		Cvar_SetIntegerValue( "r_mode", -2 );
		break;
	default:
		Cvar_SetIntegerValue( "r_mode", s_settingsMenu->video.windowResolution - 1 );
		break;
	};

	Cvar_SetIntegerValue( "r_fullscreen", s_settingsMenu->video.windowMode >= WINDOWMODE_FULLSCREEN );
	Cvar_SetIntegerValue( "r_noborder", s_settingsMenu->video.windowMode % 2 != 0 );
	Cvar_SetIntegerValue( "r_customWidth", s_settingsMenu->video.windowWidth );
	Cvar_SetIntegerValue( "r_customHeight", s_settingsMenu->video.windowHeight );
	Cvar_SetIntegerValue( "r_swapInterval", s_settingsMenu->video.vsync - 1 );
	Cvar_SetFloatValue( "r_imageSharpenAmount", s_settingsMenu->video.sharpening );
	Cvar_SetFloatValue( "r_autoExposure", s_settingsMenu->video.exposure );
	Cvar_SetFloatValue( "r_gammaAmount", s_settingsMenu->video.gamma );

	if ( !N_stricmp( g_renderer->s, "opengl" ) ) {
		SDL_GL_SetSwapInterval( s_settingsMenu->video.vsync - 1 );
	}
	if ( s_settingsMenu->video.windowResolution != s_initial->video.windowResolution
		|| s_settingsMenu->video.windowMode != s_initial->video.windowMode
		|| ( s_settingsMenu->video.windowWidth != s_initial->video.windowWidth
		|| s_settingsMenu->video.windowHeight != s_initial->video.windowHeight )
	)
	{
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart keep_context\n" );
	}
/*
	switch ( s_settingsMenu->video.windowMode ) {
	case WINDOWMODE_BORDERLESS_FULLSCREEN:
	case WINDOWMODE_BORDERLESS_WINDOWED:
		SDL_SetWindowBordered( SDL_window, SDL_FALSE );
		break;
	case WINDOWMODE_WINDOWED:
	case WINDOWMODE_FULLSCREEN:
		SDL_SetWindowBordered( SDL_window, SDL_TRUE );
		break;
	};
*/
//	SDL_SetWindowPosition( SDL_window, vid_xpos->i, vid_ypos->i );
}

static bool PerformanceMenu_FBO_Save( void )
{
	bool restartFBO = false, restartVid = false;

	Cvar_SetIntegerValue( "r_antialiasQuality", s_settingsMenu->performance.multisampleQuality );
	Cvar_SetIntegerValue( "r_bloom", s_settingsMenu->performance.bloom );

	if ( s_settingsMenu->performance.bloom ) {
		Cvar_Set( "r_hdr", "1" );
	}

	if ( s_settingsMenu->performance.multisampleType != s_initial->performance.multisampleType ) {
		restartFBO = true;
	}
	if ( s_settingsMenu->performance.multisampleQuality != s_initial->performance.multisampleQuality ) {
		restartFBO = true;
	}
	if ( s_settingsMenu->performance.bloom != s_initial->performance.bloom ) {
		restartFBO = true;
	}
	if ( ( s_settingsMenu->performance.multisampleType == AntiAlias_SSAA && s_initial->performance.multisampleType != AntiAlias_SSAA )
		|| ( s_settingsMenu->performance.multisampleType != AntiAlias_SSAA && s_initial->performance.multisampleType == AntiAlias_SSAA ) )
	{
		restartVid = true;
		restartFBO = false;
	}

	if ( restartFBO && !restartVid ) {
		Cvar_SetIntegerValue( "r_multisampleType", s_settingsMenu->performance.multisampleType );
		Cbuf_ExecuteText( EXEC_APPEND, "fbo_restart\n" );
	}
	return restartVid;
}

static void SettingsMenu_PerformanceRestartConfirm( qboolean action )
{
	if ( action ) {
		Cvar_SetIntegerValue( "r_multisampleType", s_settingsMenu->performance.multisampleType );
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart fast\n" );
	}
}

static void PerformanceMenu_Save( void )
{
	bool needRestart = false;

	if ( s_settingsMenu->performance.textureDetail != s_initial->performance.textureDetail ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.fixedRendering != s_initial->performance.fixedRendering ) {
		needRestart = true;
	}
	if ( s_settingsMenu->performance.fixedResolutionScaling != s_initial->performance.fixedResolutionScaling ) {
		needRestart = true;
	}

	Cvar_Set( "r_textureMode", s_settingsMenu->performance.textureFilters[ s_settingsMenu->performance.textureFilter ] );
	Cvar_SetIntegerValue( "r_dynamiclight", s_settingsMenu->performance.dynamicLighting );
	Cvar_SetIntegerValue( "r_textureDetail", s_settingsMenu->performance.textureDetail );
	Cvar_SetIntegerValue( "r_dynamiclight", s_settingsMenu->performance.dynamicLighting );
	Cvar_SetIntegerValue( "r_lightingQuality", s_settingsMenu->performance.lightingQuality );
	Cvar_SetIntegerValue( "r_fixedRendering", s_settingsMenu->performance.fixedRendering );
	Cvar_SetFloatValue( "r_fixedResolutionScaling", s_settingsMenu->performance.fixedResolutionScaling );

	if ( needRestart || PerformanceMenu_FBO_Save() ) {
		UI_ConfirmMenu( "Some settings that you have changed require a restart to take effect, apply them?",
			NULL, SettingsMenu_PerformanceRestartConfirm );
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
	Cvar_SetIntegerValue( "snd_speakerMode", s_settingsMenu->audio.speakerMode );
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
			Key_SetBinding( bind->bind1, bind->command );

			if ( bind->bind2 != -1 ) {
				Key_SetBinding( bind->bind2, bind->command );
			}
		}
	}
}

static void GameplayMenu_Save( void )
{
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

	s_settingsMenu->performance.dynamicLighting = Cvar_VariableInteger( "r_dynamiclight" );
	s_settingsMenu->performance.multisampleType = Cvar_VariableInteger( "r_multisampleType" );
	s_settingsMenu->performance.textureDetail = Cvar_VariableInteger( "r_textureDetail" );
	s_settingsMenu->performance.bloom = Cvar_VariableInteger( "r_bloom" );
	s_settingsMenu->performance.lightingQuality = Cvar_VariableInteger( "r_lightingQuality" );
	s_settingsMenu->performance.multisampleQuality = Cvar_VariableInteger( "r_antialiasQuality" );
	s_settingsMenu->performance.fixedRendering = Cvar_VariableInteger( "r_fixedRendering" );
	s_settingsMenu->performance.fixedResolutionScaling = Cvar_VariableFloat( "r_fixedResolutionScaling" );
	
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
	if ( Cvar_VariableInteger( "r_mode" ) == -2 ) {
		s_settingsMenu->video.windowResolution = 0;
	} else {
		s_settingsMenu->video.windowResolution = Cvar_VariableInteger( "r_mode" ) + 1;
	}
	s_settingsMenu->video.vsync = Cvar_VariableInteger( "r_swapInterval" ) + 1;
	s_settingsMenu->video.gamma = Cvar_VariableFloat( "r_gammaAmount" );
	s_settingsMenu->video.windowMode = Cvar_VariableInteger( "r_fullscreen" ) + Cvar_VariableInteger( "r_noborder" );
	s_settingsMenu->video.sharpening = Cvar_VariableFloat( "r_imageSharpenAmount" );
	s_settingsMenu->video.exposure = Cvar_VariableFloat( "r_autoExposure" );
	s_settingsMenu->video.maxFPS = Cvar_VariableInteger( "com_maxfps" );
}

static void AudioMenu_SetDefault( void )
{
	s_settingsMenu->audio.masterVolume = Cvar_VariableInteger( "snd_masterVolume" );
	s_settingsMenu->audio.musicVolume = Cvar_VariableInteger( "snd_musicVolume" );
	s_settingsMenu->audio.sfxVolume = Cvar_VariableInteger( "snd_effectsVolume" );
	s_settingsMenu->audio.musicOn = Cvar_VariableInteger( "snd_musicOn" );
	s_settingsMenu->audio.sfxOn = Cvar_VariableInteger( "snd_effectsOn" );
	s_settingsMenu->audio.maxSoundChannels = Cvar_VariableInteger( "snd_maxSoundChannels" );
	s_settingsMenu->audio.speakerMode = Cvar_VariableInteger( "snd_speakerMode" );
}

static void Controls_GetKeyAssignment( const char *command, int *twokeys )
{
	int count;
	int j;
	const char *b;

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < 256; j++ ) {
		b = Key_GetBinding( j );
		if ( !b || !*b ) {
			continue;
		}
		if ( !N_stricmp( b, command ) ) {
			twokeys[ count ] = j;
			count++;
			if ( count == 2 ) {
				break;
			}
		}
	}
}

static void ControlsMenu_SetDefault( void )
{
	int i;
	int twokeys[2];

	s_settingsMenu->controls.mouseAcceleration = Cvar_VariableInteger( "g_mouseAcceleration" );
	s_settingsMenu->controls.mouseSensitivity = Cvar_VariableFloat( "g_mouseSensitivity" );

	memcpy( s_settingsMenu->controls.keybinds, s_defaultKeybinds, sizeof( s_defaultKeybinds ) );
	for ( i = 0; i < NUMKEYBINDS; i++ ) {
		s_settingsMenu->controls.keybinds[i].bind1 = s_settingsMenu->controls.keybinds[i].defaultBind1;
		s_settingsMenu->controls.keybinds[i].bind2 = s_settingsMenu->controls.keybinds[i].defaultBind2;

		Controls_GetKeyAssignment( s_settingsMenu->controls.keybinds[i].command, twokeys );

		s_settingsMenu->controls.keybinds[i].bind1 = twokeys[0];
		s_settingsMenu->controls.keybinds[i].bind2 = twokeys[1];
	}
}

static void GameplayMenu_SetDefault( void )
{
	s_settingsMenu->gameplay.mouseCursor = Cvar_VariableInteger( "sgame_CursorType" );
	s_settingsMenu->gameplay.debugPrint = Cvar_VariableInteger( "sgame_DebugMode" );
	s_settingsMenu->gameplay.toggleHUD = Cvar_VariableInteger( "sgame_ToggleHUD" );
	s_settingsMenu->gameplay.pauseUnfocused = Cvar_VariableInteger( "com_pauseUnfocused" );
}

static qboolean SettingsMenu_PresetCustom( void )
{
	const preset_t *p;

	p = &s_settingsMenu->presets[ s_settingsMenu->preset ];

	if ( s_settingsMenu->performance.bloom != p->basic.bloom ) {
		return qtrue;
	}
	if ( s_settingsMenu->performance.dynamicLighting != p->basic.dynamicLighting ) {
		return qtrue;
	}
	if ( s_settingsMenu->performance.textureFilter != p->basic.textureFilter ) {
		return qtrue;
	}
	if ( s_settingsMenu->performance.textureDetail != p->basic.textureDetail ) {
		return qtrue;
	}
	if ( s_settingsMenu->performance.multisampleType != p->basic.multisampleType ) {
		return qtrue;
	}

	return qfalse;
}

static void SettingsMenu_ExitModified( qboolean action )
{
	if ( action ) {
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
	if ( ui->activemenu != &s_settingsMenu->menu && ui->menustate == UI_MENU_PAUSE ) {
		UI_SetActiveMenu( UI_MENU_PAUSE );
	} else {
		UI_PopMenu();
	}
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
		if ( s_settingsMenu->modified ) {
			UI_ConfirmMenu( "You have made some changes to your settings,\nwould you like to save now?", NULL, SettingsMenu_ExitModified );
		} else {
			UI_SetActiveMenu( UI_MENU_PAUSE );
		}
	}
	if ( UI_MenuTitle( "Settings" ) ) {
		if ( s_settingsMenu->modified ) {
			UI_ConfirmMenu( "You have made some changes to your settings,\nwould you like to save now?", NULL, SettingsMenu_ExitModified );
		} else {
			if ( ui->menustate == UI_MENU_PAUSE ) {
				UI_SetActiveMenu( UI_MENU_PAUSE );
			} else {
				UI_PopMenu();
			}
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

	if ( SettingsMenu_PresetCustom() ) {
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

#define NUM_ANTIALIAS_TYPES 4
#define NUM_ANISOTROPY_TYPES 5
#define NUM_HUD_OPTIONS 4
#define NUM_VSYNC_TYPES 3
#define NUM_WINDOW_MODES 4
#define NUM_WINDOW_SIZES NUMVIDMODES - 1
#define NUM_TEXTURE_FILTERS 4
#define NUM_TEXTURE_DETAILS 5

void SettingsMenu_Cache( void )
{
	char str[MAXPRINTMSG];
	char *p;

	static const char *s_multisampleTypes[ NUM_ANTIALIAS_TYPES ];
	static const char *s_anisotropyTypes[ NUM_ANISOTROPY_TYPES ];
	static const char *s_textureDetail[ NUM_TEXTURE_DETAILS ];
	static const char *s_textureFilters[ NUM_TEXTURE_FILTERS ];
	static const char *s_windowSizes[ NUM_WINDOW_SIZES ];
	static const char *s_vsync[ NUM_VSYNC_TYPES ];
	static const char *difficulties[ NUMDIFS - 1 ];
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
	static const char *s_qualityTypes[] = {
		"VERY LOW",
		"LOW",
		"NORMAL",
		"HIGH",
		"VERY HIGH",
//		"INSANE"
	};
	static const char *s_speakerModes[] = {
		"Default",
    	"Multichannel",
    	"1 Speaker",
    	"2 Speakers",
    	"4 Speakers",
    	"Surround Sound 5.0",
    	"Surround Sound 5.1",
    	"Surround Sound 7.1",
    	"Surround Sound 7.1.4"
	};
	static const char *s_windowModes[ NUM_WINDOW_MODES ];

	s_multisampleTypes[0] = strManager->ValueForKey( "GAMEUI_NONE" )->value;
	s_multisampleTypes[1] = strManager->ValueForKey( "GAMEUI_MSAA" )->value;
	s_multisampleTypes[2] = strManager->ValueForKey( "GAMEUI_SSAA" )->value;
	s_multisampleTypes[3] = strManager->ValueForKey( "GAMEUI_FXAA" )->value;

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
	s_windowSizes[1] = strManager->ValueForKey( "GAMEUI_WINDOW_1280X720" )->value;
	s_windowSizes[2] = strManager->ValueForKey( "GAMEUI_WINDOW_1600X1200" )->value;
	s_windowSizes[3] = strManager->ValueForKey( "GAMEUI_WINDOW_1600X1050" )->value;
	s_windowSizes[4] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1080" )->value;
	s_windowSizes[5] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1200" )->value;
	s_windowSizes[6] = strManager->ValueForKey( "GAMEUI_WINDOW_1920X1280" )->value;
	s_windowSizes[7] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1080" )->value;
	s_windowSizes[8] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1440" )->value;
	s_windowSizes[9] = strManager->ValueForKey( "GAMEUI_WINDOW_2560X1600" )->value;
	s_windowSizes[10] = strManager->ValueForKey( "GAMEUI_WINDOW_2880X1620" )->value;
	s_windowSizes[11] = strManager->ValueForKey( "GAMEUI_WINDOW_3200X1800" )->value;
	s_windowSizes[12] = strManager->ValueForKey( "GAMEUI_WINDOW_3840X1600" )->value;
	s_windowSizes[13] = strManager->ValueForKey( "GAMEUI_WINDOW_3840X2160" )->value;

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
	
	s_settingsMenu->menu.track = Snd_RegisterSfx( "event:/music/main_theme" );
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
	s_settingsMenu->performance.multisampleTypes = s_multisampleTypes;
	s_settingsMenu->performance.textureDetails = s_textureDetail;
	s_settingsMenu->performance.textureFilters = s_textureFilters;
	s_settingsMenu->presetNames = s_presetLabels;
	s_settingsMenu->performance.qualityTypes = s_qualityTypes;

	s_settingsMenu->audio.speakermodeTypes = s_speakerModes;

	s_settingsMenu->gameplay.difficultyNames = difficulties;

	s_settingsMenu->video.vsyncList = s_vsync;
	s_settingsMenu->video.windowSizes = s_windowSizes;
	s_settingsMenu->video.windowModes = s_windowModes;

	s_settingsMenu->video.numVSync = arraylen( s_vsync );
	s_settingsMenu->video.numWindowSizes = arraylen( s_windowSizes );
	s_settingsMenu->video.numWindowModes = arraylen( s_windowModes );

	s_settingsMenu->audio.numSpeakermodeTypes = arraylen( s_speakerModes );

	s_settingsMenu->performance.numQualities = arraylen( s_qualityTypes );
	s_settingsMenu->performance.numMultisampleTypes = arraylen( s_multisampleTypes );
	s_settingsMenu->performance.numTextureDetails = arraylen( s_textureDetail );
	s_settingsMenu->performance.numTextureFilters = arraylen( s_textureFilters );

	s_settingsMenu->gameplay.numDifficultyTypes = arraylen( difficulties );

	if ( strstr( ui->gpuConfig.renderer_string, "NVIDIA" ) ) {
		s_settingsMenu->gpuMemInfoType = GPU_MEMINFO_NVX;
	} else if ( strstr( ui->gpuConfig.renderer_string, "ATI" ) || strstr( ui->gpuConfig.renderer_string, "AMD" ) ) {
		s_settingsMenu->gpuMemInfoType = GPU_MEMINFO_ATI;
	} else {
		s_settingsMenu->gpuMemInfoType = GPU_MEMINFO_NONE;
	}

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