#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

enum
{
    kbMoveForward,

    NUMBINDS
};

typedef struct {
    uint32_t key;
    const char *binding;
} keybind_t;

typedef struct {
    renderapi_t api;
    bool fullscreen;
    bool extensions;
    textureFilter_t texfilter;
    textureDetail_t texdetail;
    uint32_t texquality;
    uint32_t geometrydetail;
    int32_t videoMode;
    uint32_t lighting;
    int32_t customWidth;
    int32_t customHeight;
    uint32_t multisamplingIndex;
    uint32_t anisotropicFilteringTmp;
    uint32_t anisotropicFiltering;
    int32_t vsync;
    bool allowSoftwareGL;
    float gamma;

    bool mouseAccelerate;
    bool mouseInvert;
    int32_t mouseSensitivity;
    bool musicOn;
    bool sfxOn;
    int32_t musicVol;
    int32_t sfxVol;
} initialSettings_t;

typedef struct
{
    const char *texdetailString;
    const char *texfilterString;

    // temp cvar data
    keybind_t keybinds[NUMBINDS];
    bool mouseAccelerate;
    bool mouseInvert;
    int32_t mouseSensitivity;
    uint32_t rebindIndex;
    qboolean rebinding;
    bool musicOn;
    bool sfxOn;
    int32_t musicVol;
    int32_t sfxVol;
    renderapi_t api;
    bool fullscreen;
    bool extensions;
    textureFilter_t texfilter;
    textureDetail_t texdetail;
    uint32_t texquality;
    uint32_t geometrydetail;
    int32_t videoMode;
    uint32_t lighting;
    int32_t customWidth;
    int32_t customHeight;
    uint32_t multisamplingIndex;
    uint32_t anisotropicFilteringTmp;
    uint32_t anisotropicFiltering;
    int32_t vsync;
    bool allowSoftwareGL;
    bool gammaOn;
    float gamma;

    uint32_t newLineCount;

    bool advancedGraphics; // "stats for nerds", that kinda stuff
    qboolean confirmation;
    qboolean confirmreset;
    qboolean modified;
} settingsmenu_t;

static initialSettings_t initial;
static settingsmenu_t settings;


static const char *KeyToString( uint32_t key )
{
    switch (key) {
    case KEY_LSHIFT: return "Left-Shift";
    case KEY_RSHIFT: return "Right-Shift";
    default:
        break;
    };
    Con_Printf("WARNING: unknown key %i\n", key);
    return "";
}

typedef struct {
    const char *label;
    const vidmode_t *mode;
} vidmodeSetting_t;

static const char *vidmodeSettings[NUMVIDMODES+2] = {
    { "Native Resolution (%ix%i)",  }, //NULL },
    { "Custom Resolution (%ix%i)",  }, //NULL },
//    { "320x240",                    }, //&r_vidModes[ VIDMODE_320x240 ] },
//    { "640x480",                    }, //&r_vidModes[ VIDMODE_640x480 ] },
//    { "800x600",                    }, //&r_vidModes[ VIDMODE_800x600 ] },
    { "1024x768",                   }, //&r_vidModes[ VIDMODE_1024x768 ] },
    { "2048x1536",                  }, //&r_vidModes[ VIDMODE_2048x1536 ] },
    { "1280x720",                   }, //&r_vidModes[ VIDMODE_1280x720 ] },
    { "1600x900",                   }, //&r_vidModes[ VIDMODE_1600x900 ] },
    { "1920x1080",                  }, //&r_vidModes[ VIDMODE_1920x1080 ] },
    { "3840x2160",                  }, //&r_vidModes[ VIDMODE_3840x2160 ] },
};

typedef struct {
    const char *label;
    uint32_t amount;
} anistropicSetting_t;

static const anistropicSetting_t anisotropicFilters[] = {
    { "2x",     2},
    { "4x",     4},
    { "8x",     8},
    { "16x",    16},
    { "32x",    32}
};

typedef struct {
    const char *label;
    antialiasType_t type;
} antialiasSetting_t;

static const antialiasSetting_t antialiasSettings[] = {
    { "2x MSAA",                     AntiAlias_2xMSAA },
    { "4x MSAA",                     AntiAlias_4xMSAA },
    { "8x MSAA",                     AntiAlias_8xMSAA },
    { "16x MSAA",                    AntiAlias_16xMSAA, },
    { "32x MSAA",                    AntiAlias_32xMSAA, },
    { "2x SSAA",                                    AntiAlias_2xSSAA },
    { "4x SSAA",                                    AntiAlias_2xSSAA },
    { "Dynamic SSAA",      AntiAlias_DSSAA }
};

static renderapi_t StringToRenderAPI( const char *str )
{
    if (!N_stricmp( str, "opengl" )) {
        return R_OPENGL;
    } else if (!N_stricmp( str, "vulkan" )) {
        return R_VULKAN;
    } else if (!N_stricmp( str, "sdl2" )) {
        return R_SDL2;
    }

    N_Error( ERR_FATAL, "Bad render api string '%s'", str );

    // quiet compiler warning
    return (renderapi_t)0;
}


const char *RenderAPIString2( renderapi_t api )
{
    switch (api) {
    case R_OPENGL:
        return "opengl";
    case R_SDL2:
        return "sdl2";
    case R_VULKAN:
        return "vulkan";
    default:
        break;
    };

    N_Error( ERR_FATAL, "Bad renderapi enum: %i", (int)api );

    // silence compiler warning
    return NULL;
}

const char *TexDetailString( textureDetail_t detail )
{
    switch (detail) {
    case TexDetail_MSDOS: return "MS-DOS (VGA)";
    case TexDetail_IntegratedGPU: return "Integrated GPU";
    case TexDetail_Normie: return "Normie";
    case TexDetail_ExpensiveShitWeveGotHere: return "Expensive Shit We've Got Here";
    case TexDetail_GPUvsGod: return "GPU vs God";
    default:
        break;
    };
    N_Error( ERR_FATAL, "Invalid texture detail %i", (int)detail );

    // silence compiler warning
    return NULL;
}

const char *TexFilterString( textureFilter_t filter )
{
    switch (filter) {
    case TexFilter_Linear: return "Linear";
    case TexFilter_Nearest: return "Nearest";
    case TexFilter_Bilinear: return "Bilinear";
    case TexFilter_Trilinear: return "Trilinear";
    default:
        break;
    };
    N_Error( ERR_FATAL, "Invalid texture filter %i", (int)filter );
    
    // silence compiler warning
    return NULL;
}

const char *RenderAPIString( renderapi_t api )
{
    switch (api) {
    case R_OPENGL:
        return "OpenGL";
    case R_SDL2:
        return "SDL2 (Software) (COMING SOON!)";
    case R_VULKAN:
        return "Vulkan (COMING SOON!)";
    default:
        break;
    };

    N_Error( ERR_FATAL, "Bad renderapi enum: %i", (int)api );

    // silence compiler warning
    return NULL;
}

static void SettingsMenu_ApplyGraphicsChanges( void )
{
    Cvar_Set( "r_mode", va( "%i", settings.videoMode ) );
    Cvar_Set( "r_fullscreen", va( "%i", settings.fullscreen ) );
    switch (settings.videoMode) {
    case -2:
        Cvar_Set( "r_customWidth", va( "%i", gi.desktopWidth ) );
        Cvar_Set( "r_customHeight", va( "%i", gi.desktopHeight ) );
        break;
    case -1:
        Cvar_Set( "r_customWidth", va( "%i", settings.customWidth ) );
        Cvar_Set( "r_customHeight", va( "%i", settings.customHeight ) );
        break;
    default:
        Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ settings.videoMode ].width ) );
        Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ settings.videoMode ].height ) );
        break;
    };
    Cvar_Set( "r_anisotropicFiltering", va( "%i", settings.anisotropicFiltering ) );
    Cvar_Set( "r_textureDetail", va( "%i", settings.texdetail ) );
    Cvar_Set( "r_textureFiltering", va( "%i", settings.texfilter ) );
    Cvar_Set( "r_gammaAmount", va( "%f", settings.gamma ) );

    Cvar_Set( "g_renderer", RenderAPIString2( settings.api ) );

    Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

static void SettingsMenu_ApplyAudioChanges( void )
{
    Cvar_Set( "snd_sfxvol", va("%i", settings.sfxVol) );
    Cvar_Set( "snd_musicvol", va("%i", settings.musicVol) );
    Cvar_Set( "snd_sfxon", va("%i", settings.sfxOn) );
    Cvar_Set( "snd_musicon", va("%i", settings.musicOn) );

    Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
}


static void SettingsMenu_DrawConfirmation( void )
{
    if (ImGui::BeginPopupModal( "Save Changes", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse )) {
        ImGui::TextUnformatted( "You made some changes to your settings, would you like to apply them?" );
        if (ImGui::Button( "SAVE CHANGES" )) {
            SettingsMenu_ApplyGraphicsChanges();
            settings.confirmation = qfalse;
            settings.modified = qfalse;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void SettingsMenu_SetDefault( void )
{
    settings.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    settings.extensions = Cvar_VariableInteger( "r_useExtensions" );
    settings.customWidth = Cvar_VariableInteger( "r_customWidth" );
    settings.customHeight = Cvar_VariableInteger( "r_customHeight" );
    settings.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    settings.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    settings.texdetailString = TexDetailString( settings.texdetail );
    settings.texfilterString = TexFilterString( settings.texfilter );
    settings.videoMode = Cvar_VariableInteger( "r_mode" );
    settings.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    settings.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );
    settings.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );
    settings.allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
    settings.vsync = Cvar_VariableInteger( "r_swapInterval" );
    settings.gamma = Cvar_VariableFloat( "r_gammeAmount" );
    settings.advancedGraphics = false;

    settings.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    settings.musicOn = Cvar_VariableInteger( "snd_musicon" );
    settings.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    settings.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    
    settings.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    settings.mouseInvert = Cvar_VariableInteger("g_mouseInvert");
}

static void SettingsMenu_ConfirmReset( void )
{
    if (ImGui::BeginPopupModal( "Confirm Reset To Default", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse )) {
        ImGui::TextUnformatted( "Are you want to reset all your settings to their defaults?" );
        if (ImGui::Button( "YES" )) {
            SettingsMenu_SetDefault();
            settings.confirmreset = qfalse;
            settings.modified = qfalse;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button( "CANCEL" )) {
            settings.confirmreset = qfalse;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void SettingsMenu_ApplyChanges( void )
{
    ImGui::BeginChild( 0xff, ImVec2( 0, 0 ), ImGuiChildFlags_AlwaysAutoResize );

    ImGui::SetWindowPos(ImVec2( 8 * ui->scale, 720 * ui->scale ));
    ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.5f * ui->scale );

    if (ImGui::Button( "RESET TO DEFAULT" )) {
        settings.confirmreset = qtrue;
        ImGui::OpenPopup( "Confirm Reset To Default" );
        ui->PlaySelected();
    }

    if (!settings.modified) {
        ImGui::EndChild();
        return;
    }

    ImGui::SameLine( 248 );
    
    if (ImGui::Button( "APPLY CHANGES" )) {
        SettingsMenu_ApplyGraphicsChanges();
        SettingsMenu_ApplyAudioChanges();
        Cbuf_ExecuteText( EXEC_APPEND, "writecfg\n" );
        settings.modified = qfalse;
        ui->PlaySelected();
    }
    ImGui::EndChild();
}

static void SettingsMenu_Update( void )
{
    settings.modified = false;

    if (initial.sfxOn != settings.sfxOn) {
        settings.modified = true;
    }
    if (initial.musicOn != settings.musicOn) {
        settings.modified = true;
    }
    if (initial.sfxVol != settings.sfxVol) {
        settings.modified = true;
    }
    if (initial.musicVol != settings.musicVol) {
        settings.modified = true;
    }
    if (initial.videoMode != settings.videoMode) {
        settings.modified = true;
    }
    if (initial.allowSoftwareGL != settings.allowSoftwareGL) {
        settings.modified = true;
    }
    if (initial.anisotropicFiltering != settings.anisotropicFiltering) {
        settings.modified = true;
    }
    if (initial.api != settings.api) {
        settings.modified = true;
    }
    if (initial.customHeight != settings.customHeight) {
        settings.modified = true;
    }
    if (initial.customWidth != settings.customWidth) {
        settings.modified = true;
    }
    if (initial.extensions != settings.extensions) {
        settings.modified = true;
    }
    if (initial.vsync != settings.vsync) {
        settings.modified = true;
    }
    if (initial.fullscreen != settings.fullscreen) {
        settings.modified = true;
    }
    if (initial.geometrydetail != settings.geometrydetail) {
        settings.modified = true;
    }
    if (initial.multisamplingIndex != settings.multisamplingIndex) {
        settings.modified = true;
    }
    if (initial.texquality != settings.texquality) {
        settings.modified = true;
    }
    if (initial.texfilter != settings.texfilter) {
        settings.modified = true;
    }
    if (initial.texdetail != settings.texdetail) {
        settings.modified = true;
    }
    if (initial.gamma != settings.gamma) {
        settings.modified = true;
    }
}

static void SettingsMenu_RebindKey( void )
{
    if (ImGui::BeginPopupModal( "Rebind Key" )) {
        ImGui::TextUnformatted( "Press Any Key..." );

        for (uint32_t i = 0; i < NUMKEYS; i++) {
            if (Key_IsDown( i )) {
                settings.rebinding = qtrue;
                settings.rebindIndex = i;
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("CANCEL")) {
            settings.rebinding = qfalse;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}


static GDR_INLINE void SettingsMenu_Bar( void )
{
    if (ImGui::BeginTabBar( " " )) {
        if (ImGui::BeginTabItem( "GRAPHICS" )) {
            ui->SetState( STATE_GRAPHICS );
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem( "AUDIO" )) {
            ui->SetState( STATE_AUDIO );
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem( "CONTROLS" )) {
            ui->SetState( STATE_CONTROLS );
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

static void SettingsMenuGraphics_Draw( void )
{
    uint64_t i;

    ui->EscapeMenuToggle( STATE_MAIN );
    if (ui->GetState() != STATE_GRAPHICS) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            return;
        }
    }
    else if (ui->Menu_Title( "SETTINGS" )) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            ui->SetState( STATE_MAIN );
            return;
        }
    }
    SettingsMenu_Bar();

    ImGui::BeginTable( " ", 2 );
    {
        const char *vidMode;
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Anti-Aliasing" );
        ImGui::TableNextColumn();

        if (ImGui::BeginMenu( antialiasSettings[ settings.multisamplingIndex ].label )) {
            for (uint32_t a = 0; a < arraylen(antialiasSettings); a++) {
                if (ImGui::MenuItem( antialiasSettings[a].label )) {
                    settings.multisamplingIndex = a;
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Filtering" );
        ImGui::TableNextColumn();
        if (ImGui::BeginMenu( settings.texfilterString )) {
            for (i = 0; i < NumTexFilters; i++) {
                if (ImGui::MenuItem( TexFilterString( (textureFilter_t)((int)TexFilter_Linear + i) ) )) {
                    settings.texfilter = (textureFilter_t)((int)TexFilter_Linear + i);
                    settings.texfilterString = TexFilterString( settings.texfilter );
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Detail" );
        ImGui::TableNextColumn();
        if (ImGui::BeginMenu( settings.texdetailString )) {
            for (i = 0; i < NumTexDetails; i++) {
                if (ImGui::MenuItem( TexDetailString( (textureDetail_t)((int)TexDetail_MSDOS + i) ) )) {
                    settings.texdetail = (textureDetail_t)((int)TexDetail_MSDOS + i);
                    settings.texdetailString = TexDetailString( settings.texdetail );
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Anisotropic Filtering" );
        ImGui::TableNextColumn();
        if (ImGui::ArrowButton( "##ANIFILTER_LEFT", ImGuiDir_Left )) {
            settings.anisotropicFiltering = (int32_t)settings.anisotropicFiltering - 1 == -1 ? arraylen(anisotropicFilters) - 1
                : settings.anisotropicFiltering - 1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( anisotropicFilters[ settings.anisotropicFiltering ].label );
        ImGui::SameLine();
        if (ImGui::ArrowButton( "##ANIFILTER_RIGHT", ImGuiDir_Right )) {
            settings.anisotropicFiltering = settings.anisotropicFiltering + 1 >= arraylen(anisotropicFilters) ? 0 : settings.anisotropicFiltering + 1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        switch (settings.videoMode) {
        case -2:
            vidMode = va( vidmodeSettings[0], r_customWidth->i, r_customHeight->i );
            break;
        case -1:
            vidMode = va( vidmodeSettings[1], r_customWidth->i, r_customHeight->i );
            break;
        default:
            vidMode = r_vidModes[ settings.videoMode ].description;
            break;
        };
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Video Mode" );
        ImGui::TableNextColumn();
        if (ImGui::BeginMenu( vidMode )) {
            if (ImGui::MenuItem( va( "Native Resolution (%ix%i)", r_customWidth->i, r_customHeight->i ) )) {
                settings.videoMode = -2;
                ui->PlaySelected();
            }
            if (ImGui::MenuItem( va( "Custom Resolution (%ix%i)", r_customWidth->i, r_customHeight->i ) )) {
                settings.videoMode = -1;
                ui->PlaySelected();
            }
            for (i = 2; i < NUMVIDMODES+2; i++) {
                if (ImGui::MenuItem( r_vidModes[ i - 2 ].description )) {
                    settings.videoMode = i - 2;
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Rendering API" );
        ImGui::TableNextColumn();
        if (ImGui::BeginMenu( RenderAPIString( settings.api ) )) {
            if (ImGui::MenuItem( RenderAPIString( R_OPENGL ) )) {
                settings.api = R_OPENGL;
                ui->PlaySelected();
            }
            if (ImGui::MenuItem( RenderAPIString( R_SDL2 ) )) {
                settings.api = R_SDL2;
                ui->PlaySelected();
            }
            if (ImGui::MenuItem( RenderAPIString( R_VULKAN ) )) {
                settings.api = R_VULKAN;
                ui->PlaySelected();
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Fullscreen" );
        ImGui::TableNextColumn();
        if (ImGui::RadioButton( settings.fullscreen ? "ON" : "OFF", settings.fullscreen )) {
            settings.fullscreen = !settings.fullscreen;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Lighting" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON!" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "VSync" );
        ImGui::TableNextColumn();
        if (ImGui::ArrowButton( "##VSYNCLEFT", ImGuiDir_Left )) {
            switch (settings.vsync) {
            case -1:
                settings.vsync = 1;
                break;
            default:
                settings.vsync--;
                break;
            };
            ui->PlaySelected();
        }
        ImGui::SameLine();
        switch (settings.vsync) {
        case -1:
            ImGui::TextUnformatted( "Adaptive" );
            break;
        case 0:
            ImGui::TextUnformatted( "Disabled" );
            break;
        case 1:
            ImGui::TextUnformatted( "Enabled" );
            break;
        default:
            N_Error( ERR_FATAL, "Invalid UI state" );
        };
        ImGui::SameLine();
        if (ImGui::ArrowButton( "##VYSNCRIGHT", ImGuiDir_Right )) {
            switch (settings.vsync) {
            case -1:
                settings.vsync = 1;
                break;
            default:
                settings.vsync--;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Gamma Correction" );
        ImGui::TableNextColumn();
        if (ImGui::SliderFloat( " ", &settings.gamma, 1.0f, 5.0f )) {
            ui->PlaySelected();
        }
    }
    ImGui::EndTable();
}

static void SettingsMenuAudio_Draw( void )
{
    ui->EscapeMenuToggle( STATE_SETTINGS );
    if (ui->GetState() != STATE_AUDIO) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            return;
        }
    }
    else if (ui->Menu_Title( "SETTINGS" )) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            ui->SetState( STATE_MAIN );
            return;
        }
    }
    SettingsMenu_Bar();

    ImGui::SeparatorText( "Sound Effects" );
    if (ImGui::RadioButton( "ON##SfxOn", settings.sfxOn )) {
        settings.sfxOn = !settings.sfxOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##SfxVolume", &settings.sfxVol, 0.0f, 100.0f )) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Music" );
    if (ImGui::RadioButton( "ON##MusicOn", settings.musicOn )) {
        settings.musicOn = !settings.musicOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##MusicVolume", &settings.musicVol, 0.0f, 100.0f )) {
        ui->PlaySelected();
    }
}

static void SettingsMenuControls_Draw( void )
{
    ui->EscapeMenuToggle( STATE_SETTINGS );
    if (ui->GetState() != STATE_CONTROLS) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            return;
        }
    }
    else if (ui->Menu_Title( "SETTINGS" )) {
        if (settings.modified) {
            ImGui::OpenPopup( "Save Changes" );
            settings.confirmation = qtrue;
        }
        else {
            ui->SetState( STATE_MAIN );
            return;
        }
    }
    SettingsMenu_Bar();

    // mouse options
    ImGui::SeparatorText( "MOUSE" );
    ImGui::BeginTable( " ", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Sensitivity" );
        ImGui::TableNextColumn();
        if (ImGui::SliderInt( " ", &settings.mouseSensitivity, 0, 100.0f )) {
            ui->PlaySelected();
        }
        
        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Invert Mouse" );
        ImGui::TableNextColumn();
        if (ImGui::RadioButton( settings.mouseInvert ? "ON" : "OFF", settings.mouseInvert )) {
            settings.mouseInvert = !settings.mouseInvert;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Acceleration" );
        ImGui::TableNextColumn();
        if (ImGui::RadioButton( settings.mouseAccelerate ? "ON" : "OFF", settings.mouseAccelerate )) {
            settings.mouseAccelerate = !settings.mouseAccelerate;
            ui->PlaySelected();
        }
    }
    ImGui::EndTable();
}

void SettingsMenu_Draw( void )
{
    if (settings.confirmreset) {
        SettingsMenu_ConfirmReset();
    }
    else if (settings.confirmation) {
        SettingsMenu_DrawConfirmation();
    }

    switch (ui->GetState()) {
    case STATE_SETTINGS:
        ui->EscapeMenuToggle( STATE_MAIN );
        if (ui->GetState() != STATE_SETTINGS) {
            break;
        }
        else if (ui->Menu_Title( "SETTINGS" )) {
            ui->SetState( STATE_MAIN );
            break;
        }
        SettingsMenu_Bar();
        break;
    case STATE_GRAPHICS:
        SettingsMenuGraphics_Draw();
        break;
    case STATE_AUDIO:
        SettingsMenuAudio_Draw();
        break;
    case STATE_CONTROLS:
        SettingsMenuControls_Draw();
        break;
    };

    SettingsMenu_Update();
    SettingsMenu_ApplyChanges();
}

static void SettingsMenu_GetInitial( void ) {
    initial.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    initial.extensions = Cvar_VariableInteger( "r_useExtensions" );
    initial.customWidth = Cvar_VariableInteger( "r_customWidth" );
    initial.customHeight = Cvar_VariableInteger( "r_customHeight" );
    initial.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    initial.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    initial.videoMode = Cvar_VariableInteger( "r_mode" );
    initial.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    initial.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );
    initial.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );
    initial.allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
    initial.vsync = Cvar_VariableInteger( "r_swapInterval" );
    initial.gamma = Cvar_VariableFloat( "r_gammeAmount" );

    initial.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    initial.musicOn = Cvar_VariableInteger( "snd_musicon" );
    initial.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    initial.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    
    initial.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    initial.mouseInvert = Cvar_VariableInteger("g_mouseInvert");
}

void SettingsMenu_Cache( void )
{
    memset(&settings, 0, sizeof(settings));

    SettingsMenu_GetInitial();
    SettingsMenu_SetDefault();

    settings.confirmation = qfalse;
    settings.modified = qfalse;
}
