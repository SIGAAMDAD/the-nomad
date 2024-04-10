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

#define ID_PERFORMANCE				0
#define ID_VIDEO					1
#define ID_AUDIO					2
#define ID_CONTROLS					3
#define ID_GAMEPLAY					4
#define ID_TABLE					5

typedef struct {
	menuframework_t menu;

	menutext_t video;
	menutext_t performance;
	menutext_t audio;
	menutext_t controls;
	menutext_t gameplay;
} settings_t;

static settings_t *initial;
static settings_t *settings;
menutab_t settingsTabs;
menubutton_t settingsResetDefaults;
menubutton_t settingsSave;

static void SettingsMenu_Bar( void )
{
    if ( ImGui::BeginTabBar( "##SettingsMenuBar" ) ) {
        ImGui::PushStyleColor( ImGuiCol_Tab, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
        ImGui::PushStyleColor( ImGuiCol_TabActive, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_TabHovered, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

        if ( ImGui::BeginTabItem( "GRAPHICS" ) ) {
            ui->SetState( STATE_GRAPHICS );
            if ( settings->lastChild != STATE_GRAPHICS ) {
                ui->PlaySelected();
                settings->lastChild = STATE_GRAPHICS;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "ENGINE" ) ) {
            ui->SetState( STATE_PERFORMANCE );
            if ( settings->lastChild != STATE_PERFORMANCE ) {
                ui->PlaySelected();
                settings->lastChild = STATE_PERFORMANCE;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "AUDIO" ) ) {
            ui->SetState( STATE_AUDIO );
            if ( settings->lastChild != STATE_AUDIO ) {
                ui->PlaySelected();
                settings->lastChild = STATE_AUDIO;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "CONTROLS" ) ) {
            ui->SetState( STATE_CONTROLS );
            if ( settings->lastChild != STATE_CONTROLS ) {
                ui->PlaySelected();
                settings->lastChild = STATE_CONTROLS;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "GAMEPLAY" ) ) {
            ui->SetState( STATE_GAMEPLAY );
            if ( settings->lastChild != STATE_GAMEPLAY ) {
                ui->PlaySelected();
                settings->lastChild = STATE_GAMEPLAY;
            }
            ImGui::EndTabItem();
        }
        ImGui::PopStyleColor( 3 );
        ImGui::EndTabBar();
    }
}

static void SettingsMenu_ExitChild( menustate_t childstate )
{
	ui->EscapeMenuToggle( settings->paused ? STATE_PAUSE : STATE_MAIN );
	if ( settings->rebinding ) {
		// special condition so that we don't exit out of the settings menu when canceling a rebinding
		ui->SetState( STATE_CONTROLS );
		
		// just draw the stuff in the background
		ui->Menu_Title( "SETTINGS" );
		SettingsMenu_Bar();
		return;
	}
	
	if ( ui->GetState() != childstate ) {
		if ( settings->modified ) {
			settings->confirmation = qtrue;
		} else {
			return;
		}
	}
	else if ( ui->Menu_Title( "SETTINGS" ) ) {
		if ( settings->modified ) {
			settings->confirmation = qtrue;
		} else {
			ui->SetState( settings->paused ? STATE_PAUSE :  STATE_MAIN );
			return;
		}
	}
	SettingsMenu_Bar();
}


const char *Hunk_CopyString( const char *str, ha_pref pref ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Hunk_Alloc( len, pref );
    N_strncpyz( out, str, len );

    return out;
}

void UI_SettingsWriteBinds_f( void )
{
    fileHandle_t f;
    uint32_t i;

    f = FS_FOpenWrite( "bindings.cfg" );
    if ( f == FS_INVALID_HANDLE ) {
        N_Error( ERR_FATAL, "UI_SettingsWriteBinds_f: failed to write bindings" );
    }

    for ( i = 0; i < settings->controls.numBinds; i++ ) {
        FS_Printf( f, "\"%s\" \"%s\" %i \"%s\" \"%s\" \"%s\" \"%s\"" GDR_NEWLINE,
            settings->controls.keybinds[i].command,
            settings->controls.keybinds[i].label,
            settings->controls.keybinds[i].id,
            Key_KeynumToString( settings->controls.keybinds[i].defaultBind1 ),
            Key_KeynumToString( settings->controls.keybinds[i].defaultBind2 ),
            Key_KeynumToString( settings->controls.keybinds[i].bind1 ),
            Key_KeynumToString( settings->controls.keybinds[i].bind2 )
        );
    }

    FS_FClose( f );
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

    settings->controls.numBinds = 0;
    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );
        if ( !tok || !tok[0] ) {
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing parameter for keybind 'label'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'id'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'defaultBinding1'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'defaultBinding2'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'bind1'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'bind2'" );
            break;
        }

        settings->controls.numBinds++;
    }

    settings->controls.keybinds = (bind_t *)Hunk_Alloc( sizeof( *settings->controls.keybinds ) * settings->controls.numBinds, h_high );
    text_p = f.b;
    text = &text_p;

    bind = settings->controls.keybinds;
    for ( i = 0; i < settings->controls.numBinds; i++ ) {
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

static void SetHint( const char *label, const char *hint )
{
	const int windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMouseInputs
						| ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
	
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		ImGui::Begin( va( "##Tooltip%s", label ), NULL, windowFlags );
		ImGui::SetWindowPos( ImVec2( 900 * ui->scale, 300 * ui->scale ) );
		ImGui::SeparatorText( label );
		ImGui::TextUnformatted( hint );
		ImGui::End();
	}
}

static void SettingsMenu_Performance_Draw( void ) {
	SettingsMenu_ExitChild( STATE_PERFORMANCE );
	if ( ui->GetState() != STATE_PERFORMANCE ) {
		return;
	}
	
	performance_t *performance;
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader;
	int i;
	static const char *antiAliasing[] = {
		"None##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_0",
		"2x MSAA##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_1",
		"4x MSAA##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_2",
		"8x MSAA##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_3",
		"16x MSAA##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_4",
		"32x MSAA##AntiAliasingGraphicsPerformanceSettingsConfigSelectable_5",
	};
	static const char *anisotropicFiltering[] = {
		"None##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_0",
		"2x##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_1",
		"4x##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_2",
		"8x##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_3",
		"16x##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_4",
		"32x##AnisotropicFilteringGraphicsPerformanceSettingsConfigSelectable_5",
	};
	static const char *textureFiltering[] = {
		"Nearest##TextureFilteringGraphicsPerformanceSettingsConfigSelectable_0",
		"Bilinear##TextureFilteringGraphicsPerformanceSettingsConfigSelectable_1",
		"Linear Nearest##TextureFilteringGraphicsPerformanceSettingsConfigSelectable_2",
		"Nearest Linear##TextureFilteringGraphicsPerformanceSettingsConfigSelectable_3"
	};
	static const char *textureDetail[] = {
		"MSDOS",
		"Integrated GPU",
		"Normie",
		"Expensive Shit We've Got Here!",
		"GPU vs God"
	};
	
	performance = &settings->performance;
	
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##GraphicsPerformanceSettingsConfigNode", treeNodeFlags, "Graphics" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsGraphicsConfigTable", 4 );
		{
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Anti Aliasing" );
			SetHint( "Anti Aliasing",
				"Sets the Anti-Aliasing technique that the rendering engine will use\n"
				"The higher it is set, the less jagged polygons will appear, this however, does have an impact on performance." );
			ImGui::TableNextColumn();
			if ( ImGui::ArrowButton( "##AntiAliasingGraphicsPerformanceSettingsConfigLeft", ImGuiDir_Left ) ) {
				ui->PlaySelected();
				performance->multisampleType--;
				if ( performance->multisampleType < 0 ) {
					performance->multisampleType = arraylen( antiAliasing ) - 1;
				}
			}
			ImGui::TableNextColumn();
			if ( ImGui::BeginCombo( "##AntiAliasingGraphicsPerformanceSettingsConfigDropDown",
				antiAliasing[ performance->multisampleType ] ) )
			{
				if ( ImGui::IsItemClicked() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
					ui->PlaySelected();
				}
				for ( i = 0; i < arraylen( antiAliasing ); i++ ) {
					if ( ImGui::Selectable( antiAliasing[i], ( performance->multisampleType == i ) ) ) {
						ui->PlaySelected();
						performance->multisampleType = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::TableNextColumn();
			if ( ImGui::ArrowButton( "##AntiAliasingGraphicsPerformanceSettingsConfigRight", ImGuiDir_Right ) ) {
				ui->PlaySelected();
				performance->multisampleType++;
				if ( performance->multisampleType >= arraylen( antiAliasing ) ) {
					performance->multisampleType = 0;
				}
			}
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Anisotropic Filtering" );
			SetHint( "Anisotropic Filtering",
				"Increases the amount of filtering applied to textures at sharp angles."
				" Values higher than 8x could impact performance." );
			ImGui::TableNextColumn();
			if ( ImGui::ArrowButton( "##AnisotropicFilteringgGraphicsPerformanceSettingsConfigLeft", ImGuiDir_Left ) ) {
				ui->PlaySelected();
				performance->anisotropicFiltering--;
				if ( performance->anisotropicFiltering < 0 ) {
					performance->anisotropicFiltering = arraylen( anisotropicFiltering ) - 1;
				}
			}
			ImGui::TableNextColumn();
			if ( ImGui::BeginCombo( "##AnisotropicFilteringGraphicsPerformanceSettingsConfigDropDown",
				anisotropicFiltering[ performance->anisotropicFiltering ] ) )
			{
				if ( ImGui::IsItemClicked() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
					ui->PlaySelected();
				}
				for ( i = 0; i < arraylen( anisotropicFiltering ); i++ ) {
					if ( ImGui::Selectable( anisotropicFiltering[i], ( performance->anisotropicFiltering == i ) ) ) {
						ui->PlaySelected();
						performance->anisotropicFiltering = i;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::TableNextColumn();
			if ( ImGui::ArrowButton( "##AnisotropicFilteringGraphicsPerformanceSettingsConfigRight", ImGuiDir_Right ) ) {
				ui->PlaySelected();
				performance->anisotropicFiltering++;
				if ( performance->anisotropicFiltering >= arraylen( anisotropicFiltering ) ) {
					performance->anisotropicFiltering = 0;
				}
			}
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable HDR" );
			SetHint( "HDR (High Dynamic Range)", "Increases the color pallete that the rendering engine can use when enabled" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableHdr ? "ON##HDRGraphicsPerformanceSettingsConfigON" :
				"OFF##HDRGraphicsPerformanceSettingsConfigOFF", performance->enableHdr ) )
			{
				ui->PlaySelected();
				performance->enableHdr = !performance->enableHdr;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Bloom" );
			SetHint( "Bloom", "Makes light sources slighter brighter and produces a glow effect" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableBloom ? "ON##BloomGraphicsPerformanceSettingsConfigON" :
				"OFF##BloomGraphicsPerformanceSettingsConfigOFF", performance->enableBloom ) )
			{
				ui->PlaySelected();
				performance->enableBloom = !performance->enableBloom;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Vertex Lighting" );
			SetHint( "Vertex Lighting", "Enables per vertex lighting done in software, much faster but lesser quality" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableVertexLighting ? "ON##VertexLightingGraphicsPerformanceSettingsConfigON" :
				"OFF##VertexLightingGraphicsPerformanceSettingsConfigOFF", performance->enableVertexLighting ) )
			{
				ui->PlaySelected();
				performance->enableVertexLighting = !performance->enableVertexLighting;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Dynamic Lighting" );
			SetHint( "Dymamic Lighting", "Enables higher quality per pixel lighting done in a shader along with dynamic lights" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableDynamicLighting ? "ON##DynamicLightingGraphicsPerformanceSettingsConfigON" :
				"OFF##DynamicLightingGraphicsPerformanceSettingsConfigOFF", performance->enableDynamicLighting ) )
			{
				ui->PlaySelected();
				performance->enableDynamicLighting = !performance->enableDynamicLighting;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Shadows" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableShadows ? "ON##ShadowsGraphicsPerformanceSettingsConfigON" :
				"OFF##ShadowsGraphicsPerformanceSettingsConfigOFF", performance->enableShadows ) )
			{
				ui->PlaySelected();
				performance->enableShadows = !performance->enableShadows;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Lighting" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableLighting ? "ON##LightingGraphicsPerformanceSettingsConfigON" :
				"OFF##LightingGraphicsPerformanceSettingsConfigOFF", performance->enableLighting ) )
			{
				ui->PlaySelected();
				performance->enableLighting = !performance->enableLighting;
			}
			ImGui::TableNextColumn();

			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Fast Lighting" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->enableFastLighting ? "ON##FastLightingGraphicsPerformanceSettingsConfigON"
				: "OFF##FastLightingGraphicsPerformanceSettingsConfigOFF", performance->enableFastLighting ) )
			{
				ui->PlaySelected();
				performance->enableFastLighting = !performance->enableFastLighting;
			}
			ImGui::TableNextColumn();

			ImGui::TableNextRow();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}
		
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##AdvancedPerformanceSettingsConfigNode", treeNodeFlags, "Advanced" ) ) {
		ImGui::BeginTable( "##PerformanceSettingsAdvancedConfigTable", 4 );
		{
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Bump Maps" );
			SetHint( "Bump Maps",
				"When enabled, special textures will be used to simulate depth to reduce polygon count"
				" and increase quality. This requires slightly more GPU memory when enabled." );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->useNormalMapping ? "ON##BumpMapsGraphicsPerformanceSettingsConfigON"
				: "OFF##BumpMapsGraphicsPerformanceSettingsConfigOFF", performance->useNormalMapping ) )
			{
				ui->PlaySelected();
				performance->useNormalMapping = !performance->useNormalMapping;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Specular Maps" );
			SetHint( "Specular Maps",
				"When enabled, special textures will be used to simulate light reflected off a surface."
				" This will be faster than manually calculating color, but at the cost of some GPU memory." );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->useSpecularMapping ? "ON##SpecularMapsGraphicsPerformanceSettingsConfigON"
				: "OFF##SpecularMapsGraphicsPerformanceSettingsConfigOFF", performance->useSpecularMapping ) )
			{
				ui->PlaySelected();
				performance->useSpecularMapping = !performance->useSpecularMapping;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}
}

static void SettingsMenu_Audio_Draw( void )
{
	audio_t *audio;
	
	audio = &settings->audio;
	
	ImGui::BeginTable( "##AudioSettingsConfigTable", 4 );
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Master Volume" );
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		if ( ImGui::SliderInt( "##MasterVolumeAudioSettingsConfigSlider", &audio->masterVol, 0, 100 ) ) {
			ui->PlaySelected();
		}
		ImGui::TableNextColumn();
		
		ImGui::TableNextRow();
		
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Sound Effects Volume" );
		ImGui::TableNextColumn();
		if ( ImGui::RadioButton( audio->sfxOn ? "ON##SfxOnAudioSettingsConfigON" : "OFF##SfxOnAudioSettingsConfigOFF",
			audio->sfxOn ) )
		{
			ui->PlaySelected();
			audio->sfxOn = !audio->sfxOn;
		}
		ImGui::TableNextColumn();
		if ( ImGui::SliderInt( "##SoundEffectsVolumeAudioSettingsConfigSlider", &audio->sfxVol, 0, 100 ) ) {
			ui->PlaySelected();
		}
		ImGui::TableNextColumn();
		
		ImGui::TableNextRow();
		
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Music Volume" );
		ImGui::TableNextColumn();
		if ( ImGui::RadioButton( audio->musicOn ? "ON##MusicOnAudioSettingsConfigON" : "OFF##MusicOnAudioSettingsConfigOFF",
			audio->musicOn ) )
		{
			ui->PlaySelected();
			audio->musicOn = !audio->musicOn;
		}
		ImGui::TableNextColumn();
		if ( ImGui::SliderInt( "##MusicVolumeAudioSettingsConfigSlider", &audio->musicVol, 0, 100 ) ) {
			ui->PlaySelected();
		}
		ImGui::TableNextColumn();
	}
	ImGui::EndTable();
}

static void SettingsMenu_InitPresets( void )
{
}

void SettingsMenu_Cache( void )
{
	settings = (settings_t *)Hunk_Alloc( sizeof( *settings ), h_high );
	initial = (settings_t *)Hunk_Alloc( sizeof( *initial ), h_high );
	
	settings->menu.fullscreen = qtrue;
	settings->menu.x = 0;
	settings->menu.y = 0;
	settings->menu.width = ui->gpuConfig.vidWidth;
	settings->menu.height = ui->gpuConfig.vidHeight;
	settings->menu.name = "Settings##SettingsMenu";
	settings->menu.titleFontScale = 3.5f;
	settings->menu.textFontScale = 1.5f;

	settingsTabs.generic.type = MTYPE_TAB;
	settingsTabs.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
	settingsTabs.numitems = ID_TABLE;
	settingsTabs.items[0] = (menucommon_t *)&settings->video;
	settingsTabs.items[1] = (menucommon_t *)&settings->performance;
	settingsTabs.items[2] = (menucommon_t *)&settings->audio;
	settingsTabs.items[3] = (menucommon_t *)&settings->controls;
	settingsTabs.items[4] = (menucommon_t *)&settings->gameplay;

	Menu_AddItem( &settings->menu, &settingsTabs );
}
