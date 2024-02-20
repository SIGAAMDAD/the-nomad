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
#include <EASTL/array.h>

#define RADIOBUTTON_STR(x) { "OFF##" #x, "ON##" #x };

typedef struct {
    uint32_t keynum;
    char bindname[64];
    const char *keyname;
} keybind_t;

typedef struct {
    const char *name;
    qboolean enabled;
} gpu_extension_t;

typedef struct
{
    eastl::array<const char *, 2> allowSoftwareGLStr;
    eastl::array<const char *, 2> allowLegacyGLStr;

    eastl::array<const char *, 2> use_GL_ARB_vertex_buffer_object_str;
    eastl::array<const char *, 2> use_GL_ARB_vertex_array_object_str;

    gpu_extension_t *extensions;
    uint64_t numExtensions;

    bool allowLegacyGL;
    bool allowSoftwareGL;
    bool use_GL_ARB_vertex_buffer_object;
    bool use_GL_ARB_vertex_array_object;
} graphics_extended_GL_t;

typedef struct
{
} graphics_extended_VK_t;

typedef struct {
    graphics_extended_GL_t *GL_extended;
    graphics_extended_VK_t *VK_extended;

    int64_t musicVol;
    int64_t sfxVol;
    int64_t masterVol;
    int64_t mouseSensitivity;

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
    float gamma;
    renderapi_t api;

    bool musicOn;
    bool sfxOn;
    bool mouseAccelerate;
    bool mouseInvert;
    bool useExtensions;
    bool fullscreen;
    bool extensions;
} initialSettings_t;

typedef struct
{
    keybind_t keybinds[21];

    const char *texdetailString;
    const char *texfilterString;

    char extensionsMenuStr[64];
    eastl::array<const char *, 2> fullscreenStr;
    eastl::array<const char *, 2> advancedGraphicsStr;
    eastl::array<const char *, 2> useExtensionsStr;

    graphics_extended_GL_t *GL_extended;
    graphics_extended_VK_t *VK_extended;

    char **extensionStrings;

    qboolean confirmation;
    qboolean confirmreset;
    qboolean modified;
    qboolean paused; // did we get here from the pause menu?
    qboolean rebinding;

    uint32_t numExtensions;
    int32_t masterVol;
    int32_t musicVol;
    int32_t sfxVol;
    renderapi_t api;
    int32_t mouseSensitivity;
    uint32_t rebindIndex;
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
    float gamma;

    bool mouseAccelerate;
    bool mouseInvert;
    bool fullscreen;
    bool extensions;
    bool musicOn;
    bool sfxOn;
    bool useExtensions;
    bool advancedGraphics; // "stats for nerds", that kinda stuff
} settingsmenu_t;

static initialSettings_t initial;
static settingsmenu_t settings;

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

static const char *bindNames[] = {
    "button0",
    "button1",
    "button2",
    "button3",
    "button4",
    "button5",
    "button6",
    "button7",
    "button8",
    "button9",
    "forward",
    "backward",
    "right",
    "left"
};

typedef struct {
    const char *label;
    antialiasType_t type;
} antialiasSetting_t;

static const antialiasSetting_t antialiasSettings[] = {
    { "2x MSAA",                    AntiAlias_2xMSAA },
    { "4x MSAA",                    AntiAlias_4xMSAA },
    { "8x MSAA",                    AntiAlias_8xMSAA },
    { "16x MSAA",                   AntiAlias_16xMSAA, },
    { "32x MSAA",                   AntiAlias_32xMSAA, },
    { "2x SSAA",                    AntiAlias_2xSSAA },
    { "4x SSAA",                    AntiAlias_2xSSAA },
    { "Dynamic SSAA",               AntiAlias_DSSAA }
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
}

static void SettingsMenu_ApplyAudioChanges( void )
{
    Cvar_Set( "snd_sfxvol", va("%i", settings.sfxVol) );
    Cvar_Set( "snd_musicvol", va("%i", settings.musicVol) );
    Cvar_Set( "snd_sfxon", va("%i", settings.sfxOn) );
    Cvar_Set( "snd_musicon", va("%i", settings.musicOn) );
}

static void SettingsMenu_SetDefault( void )
{
    settings.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    settings.useExtensions = Cvar_VariableInteger( "r_useExtensions" );
    settings.customWidth = Cvar_VariableInteger( "r_customWidth" );
    settings.customHeight = Cvar_VariableInteger( "r_customHeight" );
    settings.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    settings.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    settings.texdetailString = TexDetailString( settings.texdetail );
    settings.texfilterString = TexFilterString( settings.texfilter );
    settings.videoMode = Cvar_VariableInteger( "r_mode" );
    settings.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    settings.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );
    settings.vsync = Cvar_VariableInteger( "r_swapInterval" );
    settings.gamma = Cvar_VariableFloat( "r_gammaAmount" );
    settings.advancedGraphics = false;
    settings.advancedGraphicsStr = RADIOBUTTON_STR( settings.advancedGraphics );
    settings.useExtensionsStr = RADIOBUTTON_STR( settings.useExtensions );
    settings.fullscreenStr = RADIOBUTTON_STR( settings.fullscreen );

    if ( settings.api == R_OPENGL ) {
        settings.GL_extended = (graphics_extended_GL_t *)Hunk_Alloc( sizeof(graphics_extended_GL_t), h_high );

        settings.GL_extended->allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
        settings.GL_extended->allowLegacyGL = Cvar_VariableInteger( "r_allowLegacy" );

        settings.GL_extended->allowLegacyGLStr =  RADIOBUTTON_STR( settings.GL_extended->allowLegacyGL );
        settings.GL_extended->allowSoftwareGLStr = RADIOBUTTON_STR( settings.GL_extended->allowSoftwareGL );

        settings.GL_extended->numExtensions = settings.numExtensions;
        settings.GL_extended->extensions = (gpu_extension_t *)Hunk_Alloc( sizeof(gpu_extension_t) * settings.numExtensions, h_high );
        for ( uint32_t i = 0; i < settings.numExtensions; i++ ) {
            settings.GL_extended->extensions->name = settings.extensionStrings[i];
        }
    }

    settings.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    settings.musicOn = Cvar_VariableInteger( "snd_musicon" );
    settings.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    settings.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    settings.masterVol = Cvar_VariableInteger( "snd_mastervol" );
    
    settings.mouseSensitivity = Cvar_VariableInteger( "g_mouseSensitivity" );
    settings.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    settings.mouseInvert = Cvar_VariableInteger("g_mouseInvert");

    for ( uint32_t i = 0; i < arraylen( bindNames ); i++ ) {
        N_strncpyz( settings.keybinds[i].bindname, bindNames[i], sizeof(settings.keybinds[i].bindname) );
        settings.keybinds[i].keynum = Key_GetKey( bindNames[i] );
        settings.keybinds[i].keyname = Key_GetBinding( settings.keybinds[i].keynum );
    }
}

static void SettingsMenuPopup( void )
{
    const char *title;
    if (settings.confirmation) {
        title = "Save Changes";
    } else if (settings.confirmreset) {
        title = "Reset To Default";
    } else if (settings.rebinding) {
        title = "Rebind Key";
    } else {
        return;
    }

    ImGui::Begin( va( "%s##SETTINGSMENUPOPUP", title ), NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                                            | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse );
    ImGui::SetWindowPos( ImVec2( 480 * ui->scale, 340 * ui->scale ) );
    if (settings.confirmation) {
        ImGui::TextUnformatted( "You made some changes to your settings, would you like to apply them?" );
        if (ImGui::Button( "SAVE CHANGES##SETTINGSMENUPOPUP" )) {
            ui->PlaySelected();
            SettingsMenu_ApplyAudioChanges();
            SettingsMenu_ApplyGraphicsChanges();
            settings.confirmation = qfalse;
            settings.modified = qfalse;
            VM_Call( sgvm, 0, SGAME_SAVE_SETTINGS );
            ImGui::CloseCurrentPopup();
        }
    }
    else if ( settings.confirmreset ) {
        ImGui::TextUnformatted( "Are you sure you want to reset all your settings to their defaults?" );
        if (ImGui::Button( "YES##SETTINGSMENUPOPUP" )) {
            SettingsMenu_SetDefault();
            settings.confirmreset = qfalse;
            settings.modified = qfalse;
            ui->PlaySelected();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button( "CANCEL##SETTINGSMENUPOPUP" )) {
            settings.confirmreset = qfalse;
            ui->PlaySelected();
            ImGui::CloseCurrentPopup();
        }
    }
    else if (settings.rebinding) {
        ImGui::TextUnformatted( "Press Any Key..." );
        for (uint32_t i = 0; i < NUMKEYS; i++) {
            if (Key_IsDown( i )) {
                settings.rebinding = qfalse;
                settings.keybinds[settings.rebindIndex].keynum = i;
                settings.keybinds[settings.rebindIndex].keyname = Key_KeynumToString( i );
                Cbuf_ExecuteText( EXEC_APPEND, va( "bind %s \"%s\"\n",
                    settings.keybinds[settings.rebindIndex].keyname, settings.keybinds[settings.rebindIndex].bindname ) );
            }
        }
        if (ImGui::Button("CANCEL##SETTINGSMENUPOPUP")) {
            settings.rebinding = qfalse;
            settings.rebindIndex = 0;
            ui->PlaySelected();
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::End();
}

static void SettingsMenu_ApplyChanges( void )
{
    ImGui::BeginChild( 0xff, ImVec2( 0, 0 ), ImGuiChildFlags_AlwaysAutoResize );

    ImGui::SetWindowPos(ImVec2( 8 * ui->scale, 720 * ui->scale ));

    if (ImGui::Button( "RESET TO DEFAULT" )) {
        settings.confirmreset = qtrue;
        ImGui::OpenPopup( "Reset To Default##SETTINGSMENUPOPUP" );
        ui->PlaySelected();
    }

    if (!settings.modified) {
        ImGui::EndChild();
        return;
    }

    ImGui::SetCursorScreenPos( ImVec2( 260 * ui->scale, 720 * ui->scale ) );
    
    if (ImGui::Button( "APPLY CHANGES" )) {
        SettingsMenu_ApplyGraphicsChanges();
        SettingsMenu_ApplyAudioChanges();
        Cbuf_ExecuteText( EXEC_APPEND, va( "writecfg %s\n", Cvar_VariableString( "com_defaultcfg" ) ) );
        Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
        settings.modified = qfalse;
        ui->PlaySelected();
    }
    ImGui::EndChild();
}

static void SettingsMenu_Update( void )
{
    settings.modified = false;

    if (initial.masterVol != settings.masterVol) {
        settings.modified = true;
    }
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

    if ( settings.api == R_OPENGL && initial.api == R_OPENGL ) {
        if ( settings.GL_extended->allowSoftwareGL != initial.GL_extended->allowSoftwareGL ) {
            settings.modified = true;
        }
        if ( settings.GL_extended->allowLegacyGL != initial.GL_extended->allowLegacyGL ) {
            settings.modified = true;
        }
    }
}


static GDR_INLINE void SettingsMenu_Bar( void )
{
    if ( ImGui::BeginTabBar( " " ) ) {
        if ( ImGui::BeginTabItem( "GRAPHICS" ) ) {
            ui->SetState( STATE_GRAPHICS );
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "AUDIO" ) ) {
            ui->SetState( STATE_AUDIO );
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "CONTROLS" ) ) {
            ui->SetState( STATE_CONTROLS );
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "GAMEPLAY" ) ) {
            ui->SetState( STATE_GAMEPLAY );
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

static GDR_INLINE void SettingsMenu_ExitChild( menustate_t childstate )
{
    ui->EscapeMenuToggle( settings.paused ? STATE_PAUSE :  STATE_MAIN );
    if (ui->GetState() != childstate) {
        if (settings.modified) {
            settings.confirmation = qtrue;
        }
        else {
            return;
        }
    }
    else if (ui->Menu_Title( "SETTINGS" )) {
        if (settings.modified) {
            settings.confirmation = qtrue;
        }
        else {
            ui->SetState( settings.paused ? STATE_PAUSE :  STATE_MAIN );
            return;
        }
    }
    SettingsMenu_Bar();
}

static void SettingsMenuGraphics_Draw( void )
{
    uint64_t i;

    SettingsMenu_ExitChild( STATE_GRAPHICS );

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
        if ( ImGui::BeginMenu( settings.texdetailString ) ) {
            for ( i = 0; i < NumTexDetails; i++ ) {
                if ( ImGui::MenuItem( TexDetailString( (textureDetail_t)( (int)TexDetail_MSDOS + i ) ) ) ) {
                    settings.texdetail = (textureDetail_t)( (int)TexDetail_MSDOS + i );
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
        if ( ImGui::ArrowButton( "##ANIFILTER_LEFT", ImGuiDir_Left ) ) {
            settings.anisotropicFiltering = (int32_t)settings.anisotropicFiltering - 1 == -1 ? arraylen(anisotropicFilters) - 1
                : settings.anisotropicFiltering - 1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( anisotropicFilters[ settings.anisotropicFiltering ].label );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##ANIFILTER_RIGHT", ImGuiDir_Right ) ) {
            settings.anisotropicFiltering = settings.anisotropicFiltering + 1 >= arraylen(anisotropicFilters) ? 0 : settings.anisotropicFiltering + 1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        switch ( settings.videoMode ) {
        case -2:
            vidMode = va( vidmodeSettings[0], gi.desktopWidth, gi.desktopHeight );
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
        if ( ImGui::BeginMenu( vidMode ) ) {
            if ( ImGui::MenuItem( va( "Native Resolution (%ix%i)", gi.desktopWidth, gi.desktopHeight ) ) ) {
                settings.videoMode = -2;
                ui->PlaySelected();
            }
            if ( ImGui::MenuItem( va( "Custom Resolution (%lix%li)", r_customWidth->i, r_customHeight->i ) ) ) {
                settings.videoMode = -1;
                ui->PlaySelected();
            }
            for ( i = 2; i < NUMVIDMODES + 2; i++ ) {
                if ( ImGui::MenuItem( r_vidModes[ i - 2 ].description ) ) {
                    settings.videoMode = i - 2;
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Renderer" );
        ImGui::TableNextColumn();
        if ( ImGui::BeginMenu( RenderAPIString( settings.api ) ) ) {
            if ( ImGui::MenuItem( RenderAPIString( R_OPENGL ) ) ) {
                settings.api = R_OPENGL;
                ui->PlaySelected();
            }
            if ( ImGui::MenuItem( RenderAPIString( R_SDL2 ) ) ) {
                settings.api = R_SDL2;
                ui->PlaySelected();
            }
            if ( ImGui::MenuItem( RenderAPIString( R_VULKAN ) ) ) {
                settings.api = R_VULKAN;
                ui->PlaySelected();
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Fullscreen" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.fullscreenStr[settings.fullscreen], settings.fullscreen ) ) {
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
        if ( ImGui::ArrowButton( "##VSYNCLEFT", ImGuiDir_Left ) ) {
            switch ( settings.vsync ) {
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
        switch ( settings.vsync ) {
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
        if ( ImGui::ArrowButton( "##VYSNCRIGHT", ImGuiDir_Right ) ) {
            switch (settings.vsync) {
            case 1:
                settings.vsync = -1;
                break;
            default:
                settings.vsync++;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Gamma Correction" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderFloat( " ", &settings.gamma, 1.0f, 5.0f ) ) {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "HDR" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON!" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Ambient Occlusion" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON!" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Stuff For The NERDS" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.advancedGraphicsStr[settings.advancedGraphics], settings.advancedGraphics ) ) {
            settings.advancedGraphics = !settings.advancedGraphics;
            ui->PlaySelected();
        }

        if ( settings.advancedGraphics && settings.api == R_OPENGL ) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow Software GL Driver" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.GL_extended->allowLegacyGLStr[settings.GL_extended->allowSoftwareGL],
                settings.GL_extended->allowSoftwareGL ) )
            {
                settings.GL_extended->allowSoftwareGL = !settings.GL_extended->allowSoftwareGL;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow GL Extensions" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.useExtensionsStr[settings.extensions], settings.extensions ) ) {
                settings.extensions = !settings.extensions;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow Legacy GL Functionality" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.GL_extended->allowLegacyGLStr[settings.GL_extended->allowLegacyGL],
                settings.GL_extended->allowLegacyGL ) )
            {
                settings.GL_extended->allowLegacyGL = !settings.GL_extended->allowLegacyGL;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "GL Extensions" );
            ImGui::TableNextColumn();
            if ( ImGui::BeginMenu( settings.extensionsMenuStr )) {
                for ( uint32_t i = 0; i < settings.numExtensions; i++ ) {
                    ImGui::MenuItem( settings.extensionStrings[i] );
                }
                ImGui::EndMenu();
            }
        }
    }
    ImGui::EndTable();
}

static void SettingsMenuAudio_Draw( void )
{
    SettingsMenu_ExitChild( STATE_AUDIO );

    ImGui::SeparatorText( "Master Volume" );
    if (ImGui::SliderInt( " ", &settings.masterVol, 0, 100 )) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Sound Effects" );
    if (ImGui::RadioButton( "ON##SfxOn", settings.sfxOn )) {
        settings.sfxOn = !settings.sfxOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##SfxVolume", &settings.sfxVol, 0, 100 )) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Music" );
    if (ImGui::RadioButton( "ON##MusicOn", settings.musicOn )) {
        settings.musicOn = !settings.musicOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##MusicVolume", &settings.musicVol, 0, 100 )) {
        ui->PlaySelected();
    }
}

static void SettingsMenuGameplay_Draw( void )
{
    SettingsMenu_ExitChild( STATE_GAMEPLAY );

    if ( sgvm ) {
        VM_Call( sgvm, 0, SGAME_DRAW_ADVANCED_SETTINGS );
    }
}

static void SettingsMenuControls_Draw( void )
{
    uint32_t i;

    SettingsMenu_ExitChild( STATE_CONTROLS );

    // mouse options
    ImGui::SeparatorText( "MOUSE" );

    ImGui::BeginTable( " ", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Sensitivity" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderInt( " ", &settings.mouseSensitivity, 0, 100 ) ) {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Invert" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.mouseInvert ? "ON" : "OFF", settings.mouseInvert ) ) {
            settings.mouseInvert = !settings.mouseInvert;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Acceleration" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.mouseAccelerate ? "ON" : "OFF", settings.mouseAccelerate ) ) {
            settings.mouseAccelerate = !settings.mouseAccelerate;
            ui->PlaySelected();
        }
    }
    ImGui::EndTable();

    // key bindings
    ImGui::SeparatorText( "KEY BINDS" );

    ImGui::BeginTable( " ", 2 );
    {
        // draw the legend
        const float font_scale = ImGui::GetFont()->Scale;
        ImGui::SetWindowFontScale( 3.5f * ui->scale );

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Key" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Binding" );
        ImGui::TableNextRow();

        ImGui::SetWindowFontScale( font_scale );

        for ( i = 0; i < arraylen(bindNames); i++ ) {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( settings.keybinds[i].keyname );
            ImGui::TableNextColumn();
            if ( ImGui::Button( settings.keybinds[i].bindname ) ) {
                settings.rebinding = qtrue;
                settings.rebindIndex = i;
                ui->PlaySelected();
            }

            if ( i != arraylen(bindNames) - 1 ) {
                ImGui::TableNextRow();
            }
        }
    }
    ImGui::EndTable();
}

void SettingsMenu_Draw( void )
{
    SettingsMenuPopup();

    switch ( ui->GetState() ) {
    case STATE_SETTINGS:
        SettingsMenu_ExitChild( STATE_SETTINGS );
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
    case STATE_GAMEPLAY:
        SettingsMenuGameplay_Draw();
        break;
    };

    SettingsMenu_Update();
    SettingsMenu_ApplyChanges();
}

static void SettingsMenu_GetInitial( void ) {
    memset( &initial, 0, sizeof(initial) );

    initial.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    initial.useExtensions = Cvar_VariableInteger( "r_useExtensions" );
    initial.customWidth = Cvar_VariableInteger( "r_customWidth" );
    initial.customHeight = Cvar_VariableInteger( "r_customHeight" );
    initial.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    initial.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    initial.videoMode = Cvar_VariableInteger( "r_mode" );
    initial.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    initial.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );
    initial.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );

    if ( initial.api == R_OPENGL ) {
        initial.GL_extended = (graphics_extended_GL_t *)Hunk_Alloc( sizeof(graphics_extended_GL_t), h_high );
        initial.GL_extended->allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
        initial.GL_extended->use_GL_ARB_vertex_array_object = Cvar_VariableInteger( "r_arb_vertex_array_object" );
    }
    initial.vsync = Cvar_VariableInteger( "r_swapInterval" );
    initial.gamma = Cvar_VariableFloat( "r_gammaAmount" );

    initial.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    initial.musicOn = Cvar_VariableInteger( "snd_musicon" );
    initial.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    initial.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    initial.masterVol = Cvar_VariableFloat( "snd_mastervol" );
    
    initial.mouseSensitivity = Cvar_VariableInteger( "g_mouseSensitivity" );
    initial.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    initial.mouseInvert = Cvar_VariableInteger("g_mouseInvert");
}

void SettingsMenu_Cache( void ) {
    int32_t numExtensions;
    int32_t i;
    uint64_t len;

//    settings = (settingsmenu_t *)Hunk_Alloc( sizeof(*settings), h_high );

    memset( &settings, 0, sizeof(settings) );

    settings.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );

    // get extensions list
    if ( settings.api == R_OPENGL ) {
        renderImport.glGetIntegerv( GL_NUM_EXTENSIONS, &numExtensions );

        settings.extensionStrings = (char **)Hunk_Alloc( sizeof(char *) * numExtensions, h_high );
        settings.numExtensions = numExtensions;

        for ( i = 0; i < numExtensions; i++ ) {
            const GLubyte *name = renderImport.glGetStringi( GL_EXTENSIONS, i );
            len = strlen( (const char *)name );

            settings.extensionStrings[i] = (char *)Hunk_Alloc( len + 1, h_high );
            strcpy( settings.extensionStrings[i], (const char *)name );
        }
    }

    N_strncpyz( settings.extensionsMenuStr, va( "%u Extensions", settings.numExtensions ), sizeof(settings.extensionsMenuStr) );

    SettingsMenu_GetInitial();
    SettingsMenu_SetDefault();

    settings.confirmation = qfalse;
    settings.modified = qfalse;
    settings.paused = Cvar_VariableInteger( "sg_paused" );
}
