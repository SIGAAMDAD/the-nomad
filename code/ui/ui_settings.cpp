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

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

#define PRESET_LOW         0
#define PRESET_NORMAL      1
#define PRESET_PERFORMANCE 2
#define PRESET_QUALITY     3
#define PRESET_CUSTOM      4
#define NUM_PRESETS        5

#define ID_MOVENORTH       0
#define ID_MOVEWEST        1
#define ID_MOVESOUTH       2
#define ID_MOVEEAST        3
#define ID_UPMOVE          4
#define ID_WEAPONNEXT      5
#define ID_WEAPONPREV      6
#define ID_USEWEAPON       7
#define ID_ALTUSEWEAPON    8
#define NUMKEYBINDS        9

typedef struct {
    const char *command;
    const char *label;
    int32_t id;
    int32_t defaultBind1;
    int32_t defaultBind2;
    int32_t bind1;
    int32_t bind2;
} bind_t;

typedef struct {
	const char **windowSizes;
	const char **vsyncList;

	int numWindowSizes;
	int numVSync;

	int vsync;
	int fullscreen;
	int noborder;
	int windowResolution;
	int windowWidth;
	int windowHeight;

	float gamma;
	float exposure;
	float sharpening;
} videoSettings_t;

typedef struct {
	const char **multisampleTypes;
	const char **anisotropyTypes;
	const char **textureDetails;
	const char **textureFilters;
	const char **toneMappingTypes;

	int numMultisampleTypes;
	int numAnisotropyTypes;
	int numTextureDetails;
	int numTextureFilters;
	int numToneMappingTypes;

	int multisampleType;
	int anisotropicFilter;
	int textureDetail;
	int textureFilter;

	int toneMappingType;

	int vertexLighting;
	int dynamicLighting;
	int bloom;
	int pbr;
	int hdr;
	int postProcessing;
	int normalMapping;
	int specularMapping;
	int depthMapping;
	int toneMapping;
} performanceSettings_t;

typedef struct {
	int sfxVolume;
	int musicVolume;
	int masterVolume;

	// size = uint16_t
	int sfxOn;
	int musicOn;
} audioSettings_t;

typedef struct {
	bind_t keybinds[NUMKEYBINDS];

	int mouseAcceleration;
	float mouseSensitivity;

	bind_t *rebindKey;
} controlsSettings_t;

typedef struct {
	int mouseCursor;
	int difficulty;
	int debugPrint;
} gameplaySettings_t;

typedef struct settingsMenu_s {
	menuframework_t menu;

	struct settingsMenu_s *presets;

	nhandle_t save_0;
	nhandle_t save_1;

	nhandle_t reset_0;
	nhandle_t reset_1;

	int lastChild;

	qboolean saveHovered;
	qboolean setDefaultsHovered;

	performanceSettings_t performance;
	videoSettings_t video;
	audioSettings_t audio;
	controlsSettings_t controls;

	const char *hintLabel;
	const char *hintMessage;

	qboolean modified;
} settingsMenu_t;

static settingsMenu_t *s_settingsMenu;
static settingsMenu_t *s_initial;

static const bind_t s_defaultKeybinds[NUMKEYBINDS] = {
	{ "+northmove", "move north", ID_MOVENORTH, KEY_W, -1, -1, -1 },
	{ "+westmove", "move west", ID_MOVEWEST, KEY_A, -1, -1, -1 },
	{ "+southmove", "move south", ID_MOVESOUTH, KEY_S, -1, -1, -1 },
	{ "+eastmove", "move east", ID_MOVEEAST, KEY_D, -1, -1, -1 },
	{ "+upmove", "jump", ID_UPMOVE, KEY_SPACE, -1, -1, -1 },
	{ "weaponnext", "next weapon", ID_WEAPONNEXT, KEY_WHEEL_DOWN, -1, -1, -1 },
	{ "weaponprev", "prev weapon", ID_WEAPONPREV, KEY_WHEEL_UP, -1, -1, -1 },
	{ "+useweapon", "use weapon", ID_USEWEAPON, KEY_MOUSE_LEFT, -1, -1, -1 },
	{ "+altuseweapon", "use weapon alt fire", ID_ALTUSEWEAPON, KEY_MOUSE_RIGHT, -1, -1, -1 }
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
	if ( s_settingsMenu->performance.multisampleType != s_initial->performance.multisampleType ) {
		s_settingsMenu->modified = qtrue;
	}
	if ( s_settingsMenu->performance.anisotropicFilter != s_initial->performance.anisotropicFilter ) {
		s_settingsMenu->modified = qtrue;
	}
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
	s_settingsMenu->presets = (settingsMenu_t *)Hunk_Alloc( sizeof( *s_settingsMenu->presets ) * NUM_PRESETS, h_high );
}

static void SettingsMenu_TabBar( void ) {
	if ( ImGui::BeginTabBar( "##SettingsMenuTabBar" ) ) {
		ImGui::PushStyleColor( ImGuiCol_Tab, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
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
		if ( ImGui::BeginTabItem( "Performance" ) ) {
			if ( s_settingsMenu->lastChild != ID_PERFORMANCE ) {
				s_settingsMenu->lastChild = ID_PERFORMANCE;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		if ( ImGui::BeginTabItem( "Audio" ) ) {
			if ( s_settingsMenu->lastChild != ID_AUDIO ) {
				s_settingsMenu->lastChild = ID_AUDIO;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		if ( ImGui::BeginTabItem( "Controls" ) ) {
			if ( s_settingsMenu->lastChild != ID_CONTROLS ) {
				s_settingsMenu->lastChild = ID_CONTROLS;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}
		if ( ImGui::BeginTabItem( "Gameplay" ) ) {
			if ( s_settingsMenu->lastChild != ID_GAMEPLAY ) {
				s_settingsMenu->lastChild = ID_GAMEPLAY;
				Snd_PlaySfx( ui->sfx_select );
			}
			ImGui::EndTabItem();
		}

		ImGui::PopStyleColor( 3 );
		ImGui::EndTabBar();
	}
}

static void SettingsMenu_Text( const char *name, const char *hint )
{
	ImGui::TextUnformatted( name );
	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) || ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled
		| ImGuiHoveredFlags_DelayNone ) )
	{
		s_settingsMenu->hintLabel = name;
		s_settingsMenu->hintMessage = hint;
	}
}

static void SettingsMenu_List( const char *label, const char **itemnames, int numitems, int *curitem, bool enabled )
{
	int i;

	if ( ImGui::BeginCombo( va( "##%sSettingsMenuConfigList", label ), itemnames[*curitem] ) ) {
		if ( ImGui::IsItemActivated() && ImGui::IsItemClicked() && enabled ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		for ( i = 0; i < numitems; i++ ) {
			if ( ImGui::Selectable( va( "%s##%sSettingsMenuConfigList", itemnames[i], label ), ( *curitem == i ) ) ) {
				if ( enabled ) {
					Snd_PlaySfx( ui->sfx_select );
					*curitem = i;
				}
			}
		}
		ImGui::EndCombo();
	}
	if ( !ImGui::IsItemActivated() && ImGui::IsItemClicked() && enabled ) {
		Snd_PlaySfx( ui->sfx_select );
	}
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

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
}

static void SettingsMenu_MultiSliderFloat( const char *name, const char *label, const char *hint, float *curvalue, float minvalue, float maxvalue,
	float delta )
{
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
	ImGui::SameLine();
	if ( ImGui::SliderFloat( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
	ImGui::SameLine();
	if ( ImGui::ArrowButton( va( "##%sSettingsMenuConfigRight", label ), ImGuiDir_Right ) ) {
		Snd_PlaySfx( ui->sfx_select );
		( *curvalue ) += delta;
		if ( *curvalue > maxvalue ) {
			*curvalue = maxvalue;
		}
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
	ImGui::SameLine();
	if ( ImGui::SliderInt( va( "##%sSettingsMenuConfigSlider", label ), curvalue, minvalue, maxvalue, "%d", enabled ? 0 : ImGuiSliderFlags_NoInput ) ) {
		if ( enabled ) {
			Snd_PlaySfx( ui->sfx_select );
		}
	}
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

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
}

static void SettingsMenu_RadioButton( const char *name, const char *label, const char *hint, int *curvalue, bool enabled )
{
	if ( !enabled ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}

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

	if ( !enabled ) {
		ImGui::PopStyleColor( 4 );
	}
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

	x = 256 * ui->scale;
	y = 128 * ui->scale;
	width = ( ui->gpuConfig.vidWidth * 0.5f ) - ( x * 0.5f );
	height = ( ui->gpuConfig.vidHeight * 0.5f ) - ( y * 0.5f );
	
	ImGui::Begin( "##RebindKeyPopup", NULL, MENU_DEFAULT_FLAGS & ~( ImGuiWindowFlags_NoBackground ) );
	ImGui::SetWindowFocus( "##RebindKeyPopup" );
	ImGui::SetWindowPos( ImVec2( x, y ) );
	ImGui::SetWindowSize( ImVec2( width, height ) );

	if ( Key_IsDown( KEY_ESCAPE ) ) {
		keys[KEY_ESCAPE].down = qfalse;
		s_settingsMenu->controls.rebindKey = NULL;
		Snd_PlaySfx( ui->sfx_select );
		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 3.0f ) * ui->scale );
	ImGui::TextUnformatted( "Press Any Key..." );

    for ( i = 0; i < NUMKEYS; i++ ) {
        if ( Key_IsDown( i ) ) {
            binding = Key_GetBinding( i );

            if ( binding != NULL ) {
				const int32_t index = SettingsMenu_GetBindIndex( binding );
                if ( s_settingsMenu->controls.keybinds[ index ].bind1 != -1 ) {
                    // we're overwriting a binding, warn them
                    ret = Sys_MessageBox( "WARNING",
                        va( "You are overwriting another binding, are you sure about this? (\"%s\" = \"%s\")",
                            Key_KeynumToString( s_settingsMenu->controls.keybinds[ index ].defaultBind1 ),
                            binding ),
                        true );
                    
                    if ( ret == 0 ) {
						Snd_PlaySfx( ui->sfx_select );
                    } else {
						s_settingsMenu->controls.rebindKey->defaultBind1 = i;
						Snd_PlaySfx( ui->sfx_select );
					}
					ImGui::End();
					return;
                }
				else if ( s_settingsMenu->controls.keybinds[ index ].bind2 != -1 ) {
                    // we're overwriting a binding, warn them
                    ret = Sys_MessageBox( "WARNING",
                        va( "You are overwriting another binding, are you sure about this? (\"%s\" = \"%s\")",
                            Key_KeynumToString( s_settingsMenu->controls.keybinds[ index ].defaultBind2 ),
                            binding ),
                        true );
                    
                    if ( ret == 0 ) {
						Snd_PlaySfx( ui->sfx_select );
                    } else {
						s_settingsMenu->controls.rebindKey->defaultBind2 = i;
						Snd_PlaySfx( ui->sfx_select );
					}
					ImGui::End();
					return;
                }
            }

            if ( s_settingsMenu->controls.rebindKey->defaultBind1 != -1 ) {
                Con_Printf( "setting double-binding for key \"%s\".\n",
                    Key_GetBinding( s_settingsMenu->controls.rebindKey->defaultBind1 ) );

                s_settingsMenu->controls.rebindKey->defaultBind2 = i;
            } else {
				s_settingsMenu->controls.rebindKey->defaultBind1 = i;
            }
            Cbuf_ExecuteText( EXEC_APPEND, va( "bind %s \"%s\"\n",
                Key_KeynumToString( i ),
                s_settingsMenu->controls.rebindKey->command ) );

			s_settingsMenu->controls.rebindKey = NULL;
			ImGui::End();
			return;
        }
    }
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
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.width, 100 * ui->scale ) );
	ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth - s_settingsMenu->menu.width, 256 * ui->scale ) );

	FontCache()->SetActiveFont( AlegreyaSC );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
	ImGui::TextUnformatted( s_settingsMenu->hintLabel );

	FontCache()->SetActiveFont( RobotoMono );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.0f ) * ui->scale );
	ImGui::TextWrapped( "%s", s_settingsMenu->hintMessage );

	ImGui::End();
}

static void ControlsMenu_Draw( void )
{
	uint64_t i;
	char bind[1024];
	char bind2[1024];

	FontCache()->SetActiveFont( RobotoMono );
	
	ImGui::BeginTable( "##ControlsSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_RadioButton( "Mouse Acceleration", "MouseAcceleration",
			"Toggles mouse acceleration",
			&s_settingsMenu->controls.mouseAcceleration, true );
	
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "Mouse Sensitivity", "MouseSensitivity",
			"Sets the speed of the mouse",
			&s_settingsMenu->controls.mouseSensitivity, 1.0f, 50.0f, 1.0f );

		ImGui::TableNextRow();

		ImGui::Separator();

		ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 3.5f ) * ui->scale );
		
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Binding" );
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Key" );

		ImGui::TableNextRow();

		ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );

		for ( i = 0; i < arraylen( s_defaultKeybinds ); i++ ) {
			ImGui::TableNextColumn();
			if ( s_settingsMenu->controls.keybinds[i].bind1 == -1 ) {
				strcpy( bind, "???" );
			}
			else {
				strcpy( bind, Key_KeynumToString( s_settingsMenu->controls.keybinds[i].bind1 ) );
				
				if ( s_settingsMenu->controls.keybinds[i].bind2 != - 1 ) {
					strcpy( bind2, Key_KeynumToString( s_settingsMenu->controls.keybinds[i].bind2 ) );
					N_strcat( bind, sizeof( bind ) - 1, " or " );
					N_strcat( bind, sizeof( bind ) - 1, bind2 );
				}
			}
			ImGui::TextUnformatted( s_settingsMenu->controls.keybinds[i].label );
			ImGui::TableNextColumn();
			if ( ImGui::Button( bind ) ) {
				s_settingsMenu->controls.rebindKey = &s_settingsMenu->controls.keybinds[i];
			}
			if ( i != NUMKEYBINDS - 1 ) {
				ImGui::TableNextRow();
			}
		}
	}
	ImGui::EndTable();
}

static void PerformanceMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::SetWindowSize( ImVec2( s_settingsMenu->menu.width * 0.90f, s_settingsMenu->menu.height ) );
	ImGui::BeginTable( "##PerformanceSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiAdjustable( "Anti-Aliasing", "AntiAliasing",
			"Sets anti-aliasing technique used by the engine",
			s_settingsMenu->performance.multisampleTypes, s_settingsMenu->performance.numMultisampleTypes,
			&s_settingsMenu->performance.multisampleType,
			s_settingsMenu->performance.postProcessing );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "Anisotropic Filtering", "AnisotropicFiltering",
			"",
			s_settingsMenu->performance.anisotropyTypes, s_settingsMenu->performance.numAnisotropyTypes,
			&s_settingsMenu->performance.anisotropicFilter, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "Texture Quality", "TextureQuality",
			"Sets the quality of textures rendered, may effect performance",
			s_settingsMenu->performance.textureDetails, s_settingsMenu->performance.numTextureDetails,
			&s_settingsMenu->performance.textureDetail, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "Texture Filtering", "TextureFiltering",
			"Sets the type of texture filtering",
			s_settingsMenu->performance.textureFilters, s_settingsMenu->performance.numTextureFilters,
			&s_settingsMenu->performance.textureFilter, true );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "HDR", "HDR",
			"Enables HDR (High Dynamic Range) texture/framebuffer usage, uses more GPU memory but allows for much more "
			"range in rendered color palette",
			&s_settingsMenu->performance.hdr, s_settingsMenu->performance.postProcessing );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "PBR", "PBR",
			"Enables Physically Based Rendering (PBR) for a more realistic texture look.",
			&s_settingsMenu->performance.pbr, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Bloom", "Bloom",
			"Enables bloom to make light sources stand out more in an environment",
			&s_settingsMenu->performance.bloom, s_settingsMenu->performance.postProcessing );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Vertex Lighting", "VertexLighting",
			"Enables per-vertex software lighting",
			&s_settingsMenu->performance.vertexLighting, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Dynamic Lighting", "DynamicLighting",
			"Enables per-pixel hardware accelerated lighting, slower than vertex lighting, but much higher quality",
			&s_settingsMenu->performance.dynamicLighting, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Tone Mapping", "ToneMapping",
			"Enables a more diverse range of colors when applying lighting to a scene",
			&s_settingsMenu->performance.toneMapping, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "Tone Mapping Type", "ToneMappingType",
			"Sets the desired tone mapping type.\n"
			"NOTE: Reinhard uses a fixed range, and makes darker spots less detailed, Exposure uses an adjustable level",
			s_settingsMenu->performance.toneMappingTypes, s_settingsMenu->performance.numToneMappingTypes,
			&s_settingsMenu->performance.toneMappingType, s_settingsMenu->performance.toneMapping );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Bump/Normal Mapping", "BumpMapping",
			"Toggles usage of normal maps",
			&s_settingsMenu->performance.normalMapping, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Specular Mapping", "SpecularMapping",
			"Toggles usage of specular maps",
			&s_settingsMenu->performance.specularMapping, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Depth (Parallax) Mapping", "ParallaxMapping",
			"Toggles usage of parallax maps",
			&s_settingsMenu->performance.depthMapping, true );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Post Processing", "PostProcessing",
			"Toggles multiple framebuffers being used to apply special affects to a frame",
			&s_settingsMenu->performance.postProcessing, true );
	}
	ImGui::EndTable();
	ImGui::SetWindowSize( ImVec2( s_settingsMenu->menu.width, s_settingsMenu->menu.height ) );

}

static void AudioMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##AudioSettingsMenuConfigTable", 2 );
	{
		SettingsMenu_MultiSliderInt( "Master Volume", "MasterVolume",
			"Sets overall volume",
			&s_settingsMenu->audio.masterVolume, 0, 100, 1,
			s_settingsMenu->audio.musicOn && s_settingsMenu->audio.sfxOn );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderInt( "Music Volume", "MusicVolume",
			"Sets the music volume",
			&s_settingsMenu->audio.musicVolume, 0, 100, 1, s_settingsMenu->audio.musicOn );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderInt( "Sound Effects Volume", "SoundEffectsVolume",
			"Sets the sound effects volume",
			&s_settingsMenu->audio.sfxVolume, 0, 100, 1, s_settingsMenu->audio.sfxOn );
		
		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Music On", "MusicOn",
			"Toggles music",
			&s_settingsMenu->audio.musicOn, true );

		ImGui::TableNextRow();

		SettingsMenu_RadioButton( "Sound Effects On", "SoundEffectsOn",
			"Toggles sound effects",
			&s_settingsMenu->audio.sfxOn, true );
	}
	ImGui::EndTable();
}

static void VideoMenu_Draw( void )
{
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::BeginTable( "##VideoSettingsMenuConfigTable", 2 );
	{

		SettingsMenu_RadioButton( "Fullscreen", "Fullscreen",
			"Sets the game's window mode to fullscreen",
			&s_settingsMenu->video.fullscreen, true );

		ImGui::TableNextRow();
	
		SettingsMenu_RadioButton( "Borderless", "Borderless",
			"Sets the game's window mode to bordless",
			&s_settingsMenu->video.noborder, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "Window Resoluiton", "Resolution",
			"Sets the game window's size",
			s_settingsMenu->video.windowSizes, s_settingsMenu->video.numWindowSizes, &s_settingsMenu->video.windowResolution, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiAdjustable( "VSync", "VSync",
			"Toggles when a frame will be rendered, vertical tearing may occur if disabled.\n"
			"NOTE: setting this to \"Enabled\" will force a maximum of your moniter's refresh rate.",
			s_settingsMenu->video.vsyncList, s_settingsMenu->video.numVSync, &s_settingsMenu->video.vsync, true );

		ImGui::TableNextRow();

		SettingsMenu_MultiSliderFloat( "Gamma", "Gamma",
			"Sets gamma linear light correction factor",
			&s_settingsMenu->video.gamma, 0.5f, 3.0f, 0.10f );

		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "Exposure", "Exposure",
			"Sets exposure level when rendered in a scene",
			&s_settingsMenu->video.exposure, 0.10f, 10.0f, 1.0f );
		
		ImGui::TableNextRow();
		
		SettingsMenu_MultiSliderFloat( "Sharpening", "ImageSharpening",
			"Sets the amount of sharpening applied to a rendered texture",
			&s_settingsMenu->video.sharpening, 0.5f, 5.0f, 0.1f );
	}
	ImGui::EndTable();
}

static void VideoMenu_Save( void )
{
	Cvar_SetIntegerValue( "r_fullscreen", s_settingsMenu->video.fullscreen );
	Cvar_SetIntegerValue( "r_noborder", s_settingsMenu->video.noborder );
	Cvar_SetIntegerValue( "r_customWidth", s_settingsMenu->video.windowWidth );
	Cvar_SetIntegerValue( "r_customHeight", s_settingsMenu->video.windowHeight );
	Cvar_SetIntegerValue( "r_mode", s_settingsMenu->video.windowResolution - 2 );
	Cvar_SetFloatValue( "r_imageSharpenAmount", s_settingsMenu->video.sharpening );

	Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

static void PerformanceMenu_Save( void )
{
	Cvar_SetIntegerValue( "r_multisampleType", s_settingsMenu->performance.multisampleType );
	switch ( s_settingsMenu->performance.multisampleType ) {
	case AntiAlias_None:
		Cvar_Set( "r_multisampleAmount", "0" );
		break;
	case AntiAlias_TAA:
	case AntiAlias_SMAA:
	case AntiAlias_FXAA:
		break;
	case AntiAlias_2xMSAA:
	case AntiAlias_2xSSAA:
		Cvar_Set( "r_multisampleAmount", "2" );
		break;
	case AntiAlias_4xMSAA:
	case AntiAlias_4xSSAA:
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

	Cvar_Set( "r_textureMode", s_settingsMenu->performance.textureFilters[ s_settingsMenu->performance.textureFilter ] );
	Cvar_SetIntegerValue( "r_textureDetail", s_settingsMenu->performance.textureDetail );
	Cvar_SetIntegerValue( "r_normalMapping", s_settingsMenu->performance.normalMapping );
	Cvar_SetIntegerValue( "r_pbr", s_settingsMenu->performance.pbr );
	Cvar_SetIntegerValue( "r_bloom", s_settingsMenu->performance.bloom );
	Cvar_SetIntegerValue( "r_postProcess", s_settingsMenu->performance.postProcessing );
	Cvar_SetIntegerValue( "r_vertexLight", s_settingsMenu->performance.vertexLighting );
	Cvar_SetIntegerValue( "r_dynamiclight", s_settingsMenu->performance.dynamicLighting );
	Cvar_SetIntegerValue( "r_toneMap", s_settingsMenu->performance.toneMapping );
	Cvar_SetIntegerValue( "r_toneMapType", s_settingsMenu->performance.toneMappingType );

	Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

static void AudioMenu_Save( void )
{
	Cvar_SetIntegerValue( "snd_mastervol", s_settingsMenu->audio.masterVolume );
	Cvar_SetIntegerValue( "snd_sfxvol", s_settingsMenu->audio.sfxVolume );
	Cvar_SetIntegerValue( "snd_musicvol", s_settingsMenu->audio.musicVolume );
	Cvar_SetIntegerValue( "snd_sfxon", s_settingsMenu->audio.sfxOn );
	Cvar_SetIntegerValue( "snd_musicon", s_settingsMenu->audio.musicOn );
}

static void ControlsMenu_Save( void )
{
	Cvar_SetIntegerValue( "g_mouseAcceleration", s_settingsMenu->controls.mouseAcceleration );
	Cvar_SetFloatValue( "g_mouseSensitivity", s_settingsMenu->controls.mouseSensitivity );
}

static void PerformanceMenu_SetDefault( void )
{
	int i;
	const char *textureMode;

	switch ( Cvar_VariableInteger( "r_arb_texture_max_anisotropy" ) ) {
	case 0:
		s_settingsMenu->performance.anisotropicFilter = 0;
		break;
	case 2:
		s_settingsMenu->performance.anisotropicFilter = 1;
		break;
	case 4:
		s_settingsMenu->performance.anisotropicFilter = 2;
		break;
	case 8:
		s_settingsMenu->performance.anisotropicFilter = 3;
		break;
	case 16:
		s_settingsMenu->performance.anisotropicFilter = 4;
		break;
	case 32:
		s_settingsMenu->performance.anisotropicFilter = 5;
		break;
	};

	s_settingsMenu->performance.multisampleType = Cvar_VariableInteger( "r_multisampleType" );
	s_settingsMenu->performance.depthMapping = Cvar_VariableInteger( "r_parallaxMapping" );
	s_settingsMenu->performance.specularMapping = Cvar_VariableInteger( "r_specularMapping" );
	s_settingsMenu->performance.normalMapping = Cvar_VariableInteger( "r_normalMapping" );
	s_settingsMenu->performance.postProcessing = Cvar_VariableInteger( "r_postProcess" );
	s_settingsMenu->performance.toneMappingType = Cvar_VariableInteger( "r_toneMapType" );
	s_settingsMenu->performance.toneMapping = Cvar_VariableInteger( "r_toneMap" );
	s_settingsMenu->performance.textureDetail = Cvar_VariableInteger( "r_textureDetail" );
	s_settingsMenu->performance.bloom = Cvar_VariableInteger( "r_bloom" );
	s_settingsMenu->performance.hdr = Cvar_VariableInteger( "r_hdr" );
	s_settingsMenu->performance.pbr = Cvar_VariableInteger( "r_pbr" );
	
	textureMode = Cvar_VariableString( "r_textureMode" );
	for ( i = 0; i < s_settingsMenu->performance.numTextureDetails; i++ ) {
		if ( !N_stricmp( textureMode, s_settingsMenu->performance.textureDetails[i] ) ) {
			s_settingsMenu->performance.textureFilter = i;
			break;
		}
	}
}

static void VideoMenu_SetDefault( void )
{
	s_settingsMenu->video.windowWidth = Cvar_VariableInteger( "r_customWidth" );
	s_settingsMenu->video.windowHeight = Cvar_VariableInteger( "r_customHeight" );
	s_settingsMenu->video.windowResolution = Cvar_VariableInteger( "r_mode" ) + 2;
	s_settingsMenu->video.vsync = Cvar_VariableInteger( "r_swapInterval" ) + 1;
	s_settingsMenu->video.gamma = Cvar_VariableFloat( "r_gammaAmount" );
	s_settingsMenu->video.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
	s_settingsMenu->video.noborder = Cvar_VariableInteger( "r_noborder" );
	s_settingsMenu->video.sharpening = Cvar_VariableFloat( "r_imageSharpenAmount" );
}

static void AudioMenu_SetDefault( void )
{
	s_settingsMenu->audio.masterVolume = Cvar_VariableInteger( "snd_mastervol" );
	s_settingsMenu->audio.musicVolume = Cvar_VariableInteger( "snd_musicvol" );
	s_settingsMenu->audio.sfxVolume = Cvar_VariableInteger( "snd_sfxvol" );
	s_settingsMenu->audio.musicOn = Cvar_VariableInteger( "snd_musicon" );
	s_settingsMenu->audio.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
}

static void ControlsMenu_SetDefault( void )
{
	s_settingsMenu->controls.mouseAcceleration = Cvar_VariableInteger( "g_mouseAcceleration" );
	s_settingsMenu->controls.mouseSensitivity = Cvar_VariableFloat( "g_mouseSensitivity" );

	memcpy( s_settingsMenu->controls.keybinds, s_defaultKeybinds, sizeof( s_defaultKeybinds ) );
}

static void SettingsMenu_Draw( void )
{
	const int windowFlags = MENU_DEFAULT_FLAGS;

	if ( s_settingsMenu->controls.rebindKey ) {
		SettingsMenu_Rebind();
	}

	SettingsMenu_DrawHint();

	ImGui::Begin( "SettingsMenu##MainMenuSettingsConfigThingy", NULL, windowFlags );
	ImGui::SetWindowSize( ImVec2( s_settingsMenu->menu.width, s_settingsMenu->menu.height ) );
	ImGui::SetWindowPos( ImVec2( s_settingsMenu->menu.x, s_settingsMenu->menu.y ) );

	UI_EscapeMenuToggle();
	if ( UI_MenuTitle( "Settings" ) ) {
		UI_PopMenu();

		ImGui::End();
		return;
	}

	ImGui::SetWindowFontScale( ( 1.2f * ImGui::GetFont()->Scale ) * ui->scale );
	SettingsMenu_TabBar();
	ImGui::SetWindowFontScale( ( 1.5f * ImGui::GetFont()->Scale ) * ui->scale );

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
		break;
	};

	//
	// draw the other widgets (save/setdefaults)
	//
	ImGui::SetCursorScreenPos( ImVec2( 256 * ui->scale, 680 * ui->scale ) );
	ImGui::Image( (ImTextureID)(uintptr_t)( s_settingsMenu->saveHovered ? s_settingsMenu->save_1 : s_settingsMenu->save_0 ),
		ImVec2( 256 * ui->scale, 72 * ui->scale ) );
	s_settingsMenu->saveHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
		Snd_PlaySfx( ui->sfx_select );
		Cbuf_ExecuteText( EXEC_APPEND, "writecfg " NOMAD_CONFIG "\n" );
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
			break;
		};
	}

	ImGui::SetCursorScreenPos( ImVec2( 528 * ui->scale, 680 * ui->scale ) );
	ImGui::Image( (ImTextureID)(uintptr_t)( s_settingsMenu->setDefaultsHovered ? s_settingsMenu->reset_1 : s_settingsMenu->reset_0 ),
		ImVec2( 256 * ui->scale, 72 * ui->scale ) );
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
			break;
		};
	}

	ImGui::End();
}

void SettingsMenu_Cache( void )
{
	static const char *s_multisampleTypes[] = {
		"None",
	    "2x MSAA",
	    "4x MSAA",
	    "8x MSAA",
	    "16x MSAA",
	    "32x MSAA",
	    "2x SSAA",
	    "4x SSAA",
//		"TAA",
//		"SMAA",
//		"FXAA"
	};
	static const char *s_anisotropyTypes[] = {
	    "2x",
	    "4x",
	    "8x",
	    "16x",
	    "32x"
	};
	static const char *s_textureDetail[] = {
	    "MS-DOS",
	    "Integrated GPU",
	    "Normie",
	    "Expensive Shit We've Got Here!",
	    "GPU vs GOD"
	};
	static const char *s_textureFilters[] = {
	    "Bilinear",
	    "Nearest",
	    "Linear Nearest",
	    "Nearest Linear"
	};
	static const char *s_toneMappingTypes[] = {
	    "Reinhard",
	    "Exposure"
	};
	static const char *s_windowSizes[] = {
		"Native Resolution",
		"Custom Resolution",
		"1024x768",
	    "1280x720",
	    "1280x800",
	    "1280x1024",
	    "1440x900",
	    "1440x960",
	    "1600x900",
	    "1600x1200",
	    "1600x1050",
	    "1920x800",
	    "1920x1080",
	    "1920x1200",
	    "1920x1280",
	    "2560x1080",
	    "2560x1440",
	    "2560x1600",
	    "2880x1620",
	    "3200x1800",
	    "3840x1600",
	    "3840x2160"
	};
	static const char *s_vsync[] = {
		"Adaptive",
		"Disabled",
		"Enabled",
	};

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
	s_settingsMenu->menu.width = ui->gpuConfig.vidWidth * 0.60f;
	s_settingsMenu->menu.height = ui->gpuConfig.vidHeight;
	s_settingsMenu->menu.titleFontScale = 3.5f;
	s_settingsMenu->menu.textFontScale = 1.5f;
	s_settingsMenu->lastChild = ID_VIDEO;

	s_settingsMenu->performance.toneMappingTypes = s_toneMappingTypes;
	s_settingsMenu->performance.multisampleTypes = s_multisampleTypes;
	s_settingsMenu->performance.anisotropyTypes = s_anisotropyTypes;
	s_settingsMenu->performance.textureDetails = s_textureDetail;
	s_settingsMenu->performance.textureFilters = s_textureFilters;

	s_settingsMenu->video.vsyncList = s_vsync;
	s_settingsMenu->video.windowSizes = s_windowSizes;

	s_settingsMenu->video.numVSync = arraylen( s_vsync );
	s_settingsMenu->video.numWindowSizes = arraylen( s_windowSizes );

	s_settingsMenu->performance.numMultisampleTypes = arraylen( s_multisampleTypes );
	s_settingsMenu->performance.numAnisotropyTypes = arraylen( s_anisotropyTypes );
	s_settingsMenu->performance.numTextureDetails = arraylen( s_textureDetail );
	s_settingsMenu->performance.numTextureFilters = arraylen( s_textureFilters );
	s_settingsMenu->performance.numToneMappingTypes = arraylen( s_toneMappingTypes );

	SettingsMenu_GetInitial();

	PerformanceMenu_SetDefault();
	VideoMenu_SetDefault();
	AudioMenu_SetDefault();
	ControlsMenu_SetDefault();

	s_settingsMenu->save_0 = re.RegisterShader( "menu/save_0" );
	s_settingsMenu->save_1 = re.RegisterShader( "menu/save_1" );

	s_settingsMenu->reset_0 = re.RegisterShader( "menu/reset_0" );
	s_settingsMenu->reset_1 = re.RegisterShader( "menu/reset_1" );
}

void UI_SettingsMenu( void ) {
	SettingsMenu_Cache();
	UI_PushMenu( &s_settingsMenu->menu );
}
