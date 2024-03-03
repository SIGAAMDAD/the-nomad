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

    qboolean allowLegacyGL;
    qboolean allowSoftwareGL;
    qboolean use_GL_ARB_vertex_buffer_object;
    qboolean use_GL_ARB_vertex_array_object;
} graphics_extended_GL_t;

typedef struct
{
} graphics_extended_VK_t;

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
    const char *texdetailString;
    const char *texfilterString;

    char extensionsMenuStr[64];

    graphics_extended_GL_t *GL_extended;
    graphics_extended_VK_t *VK_extended;

    char **extensionStrings;

    uint32_t numExtensions;
    renderapi_t api;

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
    float exposure;

    qboolean bloom;
    qboolean hdr;
    qboolean fullscreen;
    qboolean extensions;
    qboolean useExtensions;
    qboolean advancedGraphics; // "stats for nerds", that kinda stuff
} graphics_t;

typedef struct {
    int32_t masterVol;
    int32_t musicVol;
    int32_t sfxVol;
    qboolean sfxOn;
    qboolean musicOn;
} sound_t;

typedef struct {
    bind_t *keybinds;
    uint32_t numBinds;

    int32_t mouseSensitivity;
    uint32_t rebindIndex;

    qboolean mouseAccelerate;
    qboolean mouseInvert;
} controls_t;

typedef struct {
    sound_t sound;
    graphics_t gfx;
    controls_t controls;
} initialSettings_t;

typedef struct {
    qboolean confirmation;
    qboolean confirmreset;
    qboolean modified;
    qboolean paused; // did we get here from the pause menu?
    qboolean rebinding;

    eastl::array<const char *, 2> fullscreenStr;
    eastl::array<const char *, 2> advancedGraphicsStr;
    eastl::array<const char *, 2> useExtensionsStr;

    sound_t sound;
    graphics_t gfx;
    controls_t controls;

    graphics_t *presets;

    menustate_t lastChild;
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

#define PRESET_LOW_QUALITY          0
#define PRESET_NORMAL_QUALITY       1
#define PRESET_HIGH_QUALITY         2
#define PRESET_ULTRA_HIGH_QUALITY   3
#define PRESET_PERFORMANCE          4
#define NUM_PRESETS                 5

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
    Cvar_Set( "r_mode", va( "%i", settings.gfx.videoMode ) );
    Cvar_Set( "r_fullscreen", va( "%i", settings.gfx.fullscreen ) );
    switch ( settings.gfx.videoMode ) {
    case -2:
        Cvar_Set( "r_customWidth", va( "%i", gi.desktopWidth ) );
        Cvar_Set( "r_customHeight", va( "%i", gi.desktopHeight ) );
        break;
    case -1:
        Cvar_Set( "r_customWidth", va( "%i", settings.gfx.customWidth ) );
        Cvar_Set( "r_customHeight", va( "%i", settings.gfx.customHeight ) );
        break;
    default:
        Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ settings.gfx.videoMode ].width ) );
        Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ settings.gfx.videoMode ].height ) );
        break;
    };
    Cvar_Set( "r_anisotropicFiltering", va( "%i", settings.gfx.anisotropicFiltering ) );
    Cvar_Set( "r_textureDetail", va( "%i", settings.gfx.texdetail ) );
    Cvar_Set( "r_textureFiltering", va( "%i", settings.gfx.texfilter ) );
    Cvar_Set( "r_gammaAmount", va( "%f", settings.gfx.gamma ) );

    Cvar_Set( "g_renderer", RenderAPIString2( settings.gfx.api ) );
}

static void SettingsMenu_ApplyAudioChanges( void )
{
    Cvar_Set( "snd_sfxvol", va( "%i", settings.sound.sfxVol ) );
    Cvar_Set( "snd_musicvol", va( "%i", settings.sound.musicVol ) );
    Cvar_Set( "snd_sfxon", va( "%i", settings.sound.sfxOn ) );
    Cvar_Set( "snd_musicon", va ( "%i", settings.sound.musicOn ) );
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

    settings.controls.numBinds = 0;
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

        settings.controls.numBinds++;
    }

    settings.controls.keybinds = (bind_t *)Hunk_Alloc( sizeof( *settings.controls.keybinds ) * settings.controls.numBinds, h_high );
    text_p = f.b;
    text = &text_p;

    bind = settings.controls.keybinds;
    for ( i = 0; i < settings.controls.numBinds; i++ ) {
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

static void SettingsMenu_SetDefault( void )
{
    settings.gfx.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    settings.gfx.useExtensions = Cvar_VariableInteger( "r_useExtensions" );
    settings.gfx.customWidth = Cvar_VariableInteger( "r_customWidth" );
    settings.gfx.customHeight = Cvar_VariableInteger( "r_customHeight" );
    settings.gfx.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    settings.gfx.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    settings.gfx.texdetailString = TexDetailString( settings.gfx.texdetail );
    settings.gfx.texfilterString = TexFilterString( settings.gfx.texfilter );
    settings.gfx.videoMode = Cvar_VariableInteger( "r_mode" );
    settings.gfx.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    settings.gfx.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );
    settings.gfx.vsync = Cvar_VariableInteger( "r_swapInterval" );
    settings.gfx.gamma = Cvar_VariableFloat( "r_gammaAmount" );
    settings.gfx.advancedGraphics = false;
    settings.advancedGraphicsStr = RADIOBUTTON_STR( settings.gfx.advancedGraphics );
    settings.useExtensionsStr = RADIOBUTTON_STR( settings.gfx.useExtensions );
    settings.fullscreenStr = RADIOBUTTON_STR( settings.gfx.fullscreen );

    if ( settings.gfx.api == R_OPENGL ) {
        settings.gfx.GL_extended = (graphics_extended_GL_t *)Hunk_Alloc( sizeof( graphics_extended_GL_t ), h_high );

        settings.gfx.GL_extended->allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
        settings.gfx.GL_extended->allowLegacyGL = Cvar_VariableInteger( "r_allowLegacy" );

        settings.gfx.GL_extended->allowLegacyGLStr =  RADIOBUTTON_STR( settings.gfx.GL_extended->allowLegacyGL );
        settings.gfx.GL_extended->allowSoftwareGLStr = RADIOBUTTON_STR( settings.gfx.GL_extended->allowSoftwareGL );

        settings.gfx.GL_extended->numExtensions = settings.gfx.numExtensions;
        settings.gfx.GL_extended->extensions = (gpu_extension_t *)Hunk_Alloc( sizeof( gpu_extension_t ) * settings.gfx.numExtensions, h_high );
        for ( uint32_t i = 0; i < settings.gfx.numExtensions; i++ ) {
            settings.gfx.GL_extended->extensions->name = settings.gfx.extensionStrings[i];
        }
    }

    settings.sound.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    settings.sound.musicOn = Cvar_VariableInteger( "snd_musicon" );
    settings.sound.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    settings.sound.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    settings.sound.masterVol = Cvar_VariableInteger( "snd_mastervol" );
    
    settings.controls.mouseSensitivity = Cvar_VariableInteger( "g_mouseSensitivity" );
    settings.controls.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    settings.controls.mouseInvert = Cvar_VariableInteger("g_mouseInvert");

    SettingsMenu_LoadBindings();
}

static void SettingsMenu_Rebind( void )
{
    int32_t bind;
    uint32_t i;
    int ret;
    const char *binding;

    ImGui::TextUnformatted( "PRESS ESCAPE TO CANCEL" );
    if ( Key_IsDown( KEY_ESCAPE ) ) {
        settings.rebinding = qfalse;
        settings.controls.rebindIndex = 0;
        ui->PlaySelected();
        ImGui::CloseCurrentPopup();
        return;
    }

    ImGui::TextUnformatted( "Press Any Key..." );
    for ( i = 0; i < NUMKEYS; i++ ) {
        if ( Key_IsDown( i ) ) {
            settings.rebinding = qfalse;
            binding = Key_GetBinding( i );

            if ( binding != NULL ) {
                if ( settings.controls.keybinds[ Key_GetKey( binding ) ].bind1 != -1 ) {
                    // we're overwriting a binding, warn them
                    ret = Sys_MessageBox( "WARNING",
                        va( "You are overwriting another binding, are you sure about this? (\"%s\" = \"%s\")",
                            Key_KeynumToString( settings.controls.keybinds[ Key_GetKey( binding ) ].bind1 ),
                            binding ),
                        true );
                    
                    if ( ret == 0 ) {
                        settings.rebinding = qfalse;
                        settings.controls.rebindIndex = 0;
                        ui->PlaySelected();
                        ImGui::CloseCurrentPopup();
                        return;
                    }
                }
            }

            if ( settings.controls.keybinds[settings.controls.rebindIndex].bind1 != -1 ) {
                Con_Printf( "setting double-binding for key \"%s\".\n",
                    Key_GetBinding( settings.controls.keybinds[settings.controls.rebindIndex].bind1 ) );

                settings.controls.keybinds[settings.controls.rebindIndex].bind2 = i;
            } else {
                settings.controls.keybinds[settings.controls.rebindIndex].bind1 = i;
            }
            Cbuf_ExecuteText( EXEC_APPEND, va( "bind %s \"%s\"\n",
                Key_KeynumToString( i ),
                settings.controls.keybinds[settings.controls.rebindIndex].command ) );
            
            settings.modified = qtrue;
            settings.rebinding = qfalse;
            settings.controls.rebindIndex = 0;
            ImGui::CloseCurrentPopup();
        }
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
            g_pModuleLib->ModuleCall( sgvm, ModuleSaveConfiguration, 0 );
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
        SettingsMenu_Rebind();
    }
    ImGui::End();
}

static void SettingsMenu_ApplyChanges( void )
{
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin( "##APPLYCHANGESWINDOW", NULL, windowFlags );

    ImGui::SetWindowPos(ImVec2( 8 * ui->scale, 720 * ui->scale ));

    if (ImGui::Button( "RESET TO DEFAULT" )) {
        settings.confirmreset = qtrue;
        ImGui::OpenPopup( "Reset To Default##SETTINGSMENUPOPUP" );
        ui->PlaySelected();
    }

    if ( !settings.modified ) {
        ImGui::End();
        return;
    }

    ImGui::SetCursorScreenPos( ImVec2( 260 * ui->scale, 720 * ui->scale ) );
    
    if ( ImGui::Button( "APPLY CHANGES" ) ) {
        SettingsMenu_ApplyGraphicsChanges();
        SettingsMenu_ApplyAudioChanges();
        Cbuf_ExecuteText( EXEC_APPEND, va( "writecfg %s\n", Cvar_VariableString( "com_defaultcfg" ) ) );
        Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
        settings.modified = qfalse;
        ui->PlaySelected();
    }
    ImGui::End();
}

static void SettingsMenu_Update( void )
{
    settings.modified = false;

    if ( initial.gfx.anisotropicFiltering != settings.gfx.anisotropicFiltering ) {
        settings.modified = true;
    }
    if ( initial.gfx.api != settings.gfx.api ) {
        settings.modified = true;
    }
    if ( initial.gfx.customHeight != settings.gfx.customHeight ) {
        settings.modified = true;
    }
    if ( initial.gfx.customWidth != settings.gfx.customWidth ) {
        settings.modified = true;
    }
    if ( initial.gfx.fullscreen != settings.gfx.fullscreen ) {
        settings.modified = true;
    }
    if ( initial.gfx.gamma != settings.gfx.gamma ) {
        settings.modified = true;
    }
    if ( initial.gfx.vsync != settings.gfx.vsync ) {
        settings.modified = true;
    }
    if ( initial.gfx.videoMode != settings.gfx.videoMode ) {
        settings.modified = true;
    }
    if ( initial.gfx.lighting != settings.gfx.lighting ) {
        settings.modified = true;
    }
    if ( initial.gfx.texdetail != settings.gfx.texdetail ) {
        settings.modified = true;
    }
    if ( initial.gfx.texfilter != settings.gfx.texfilter ) {
        settings.modified = true;
    }
    if ( initial.gfx.texquality != settings.gfx.texquality ) {
        settings.modified = true;
    }
    if ( initial.gfx.hdr != settings.gfx.hdr ) {
        settings.modified = true;
    }
    if ( initial.gfx.bloom != settings.gfx.bloom ) {
        settings.modified = true;
    }
    if ( initial.gfx.exposure != settings.gfx.exposure ) {
        settings.modified = true;
    }
    if ( initial.gfx.geometrydetail != settings.gfx.geometrydetail ) {
        settings.modified = true;
    }
    if ( initial.gfx.multisamplingIndex != settings.gfx.multisamplingIndex ) {
        settings.modified = true;   
    }
    if ( initial.controls.mouseAccelerate != settings.controls.mouseAccelerate ) {
        settings.modified = true;
    }
    if ( initial.controls.mouseInvert != settings.controls.mouseInvert ) {
        settings.modified = true;
    }
    if ( initial.controls.mouseSensitivity != settings.controls.mouseSensitivity ) {
        settings.modified = true;
    }
    if ( memcmp( initial.controls.keybinds, settings.controls.keybinds,
        sizeof( *initial.controls.keybinds ) * settings.controls.numBinds ) != 0 )
    {
        settings.modified = true;
    }
    if ( initial.sound.masterVol != settings.sound.masterVol ) {
        settings.modified = true;
    }
    if ( initial.sound.musicVol != settings.sound.musicVol ) {
        settings.modified = true;
    }
    if ( initial.sound.sfxVol != settings.sound.sfxVol ) {
        settings.modified = true;
    }
    if ( initial.sound.musicOn != settings.sound.musicOn ) {
        settings.modified = true;
    }

    if ( settings.gfx.api == R_OPENGL && initial.gfx.api == R_OPENGL ) {
        if ( settings.gfx.GL_extended->allowSoftwareGL != initial.gfx.GL_extended->allowSoftwareGL ) {
            settings.modified = true;
        }
        if ( settings.gfx.GL_extended->allowLegacyGL != initial.gfx.GL_extended->allowLegacyGL ) {
            settings.modified = true;
        }
    }
}

static GDR_INLINE void SettingsMenu_Bar( void )
{
    if ( ImGui::BeginTabBar( " " ) ) {
        if ( ImGui::BeginTabItem( "GRAPHICS" ) ) {
            ui->SetState( STATE_GRAPHICS );
            if ( settings.lastChild != STATE_GRAPHICS ) {
                ui->PlaySelected();
                settings.lastChild = STATE_GRAPHICS;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "AUDIO" ) ) {
            ui->SetState( STATE_AUDIO );
            if ( settings.lastChild != STATE_AUDIO ) {
                ui->PlaySelected();
                settings.lastChild = STATE_AUDIO;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "CONTROLS" ) ) {
            ui->SetState( STATE_CONTROLS );
            if ( settings.lastChild != STATE_CONTROLS ) {
                ui->PlaySelected();
                settings.lastChild = STATE_CONTROLS;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "GAMEPLAY" ) ) {
            ui->SetState( STATE_GAMEPLAY );
            if ( settings.lastChild != STATE_GAMEPLAY ) {
                ui->PlaySelected();
                settings.lastChild = STATE_GAMEPLAY;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

static GDR_INLINE void SettingsMenu_ExitChild( menustate_t childstate )
{
    ui->EscapeMenuToggle( settings.paused ? STATE_PAUSE : STATE_MAIN );
    if ( settings.rebinding ) {
        // special condition so that we don't exit out of the settings menu when canceling a rebinding
        ui->SetState( STATE_CONTROLS );

        // just draw the stuff in the background
        ui->Menu_Title( "SETTINGS" );
        SettingsMenu_Bar();
        return;
    }

    if ( ui->GetState() != childstate ) {
        if ( settings.modified ) {
            settings.confirmation = qtrue;
        }
    }
    else if ( ui->Menu_Title( "SETTINGS" ) ) {
        if ( settings.modified ) {
            settings.confirmation = qtrue;
        } else {
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
    if ( ui->GetState() != STATE_GRAPHICS ) {
        return;
    }

    ImGui::BeginTable( "##GraphicsSettings", 2 );
    {
        const char *vidMode;
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Anti-Aliasing" );
        ImGui::TableNextColumn();

        if (ImGui::BeginMenu( antialiasSettings[ settings.gfx.multisamplingIndex ].label )) {
            for (uint32_t a = 0; a < arraylen(antialiasSettings); a++) {
                if (ImGui::MenuItem( antialiasSettings[a].label )) {
                    settings.gfx.multisamplingIndex = a;
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Filtering" );
        ImGui::TableNextColumn();
        if (ImGui::BeginMenu( settings.gfx.texfilterString )) {
            for (i = 0; i < NumTexFilters; i++) {
                if (ImGui::MenuItem( TexFilterString( (textureFilter_t)((int)TexFilter_Linear + i) ) )) {
                    settings.gfx.texfilter = (textureFilter_t)((int)TexFilter_Linear + i);
                    settings.gfx.texfilterString = TexFilterString( settings.gfx.texfilter );
                    ui->PlaySelected();
                }
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Detail" );
        ImGui::TableNextColumn();
        if ( ImGui::BeginMenu( settings.gfx.texdetailString ) ) {
            for ( i = 0; i < NumTexDetails; i++ ) {
                if ( ImGui::MenuItem( TexDetailString( (textureDetail_t)( (int)TexDetail_MSDOS + i ) ) ) ) {
                    settings.gfx.texdetail = (textureDetail_t)( (int)TexDetail_MSDOS + i );
                    settings.gfx.texdetailString = TexDetailString( settings.gfx.texdetail );
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
            settings.gfx.anisotropicFiltering = (int32_t)settings.gfx.anisotropicFiltering - 1 == -1 ? arraylen(anisotropicFilters) - 1
                : settings.gfx.anisotropicFiltering - 1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( anisotropicFilters[ settings.gfx.anisotropicFiltering ].label );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##ANIFILTER_RIGHT", ImGuiDir_Right ) ) {
            settings.gfx.anisotropicFiltering = settings.gfx.anisotropicFiltering + 1 >= arraylen(anisotropicFilters)
                ? 0 : settings.gfx.anisotropicFiltering + 1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        switch ( settings.gfx.videoMode ) {
        case -2:
            vidMode = va( vidmodeSettings[0], gi.desktopWidth, gi.desktopHeight );
            break;
        case -1:
            vidMode = va( vidmodeSettings[1], r_customWidth->i, r_customHeight->i );
            break;
        default:
            vidMode = r_vidModes[ settings.gfx.videoMode ].description;
            break;
        };
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Video Mode" );
        ImGui::TableNextColumn();
        if ( ImGui::ArrowButton( "##VideoModeLeft", ImGuiDir_Left ) ) {
            switch ( settings.gfx.videoMode ) {
            case -2:
                settings.gfx.videoMode = NUMVIDMODES - 1;
                break;
            default:
                settings.gfx.videoMode--;
                break;
            };
            ui->PlaySelected();
        }
        ImGui::SameLine();
        switch ( settings.gfx.videoMode ) {
        case -2:
            ImGui::Text( "Desktop Resolution (%ix%i)", gi.desktopWidth, gi.desktopHeight );
            break;
        case -1:
            ImGui::Text( "Custom Resolution (%lix%li)", r_customWidth->i, r_customHeight->i );
            break;
        default:
            ImGui::TextUnformatted( r_vidModes[ settings.gfx.videoMode ].description );
            break;
        };
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##VideoModeRight", ImGuiDir_Right ) ) {
            switch ( settings.gfx.videoMode ) {
            case NUMVIDMODES - 1:
                settings.gfx.videoMode = -2;
                break;
            default:
                settings.gfx.videoMode++;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Renderer" );
        ImGui::TableNextColumn();
        if ( ImGui::BeginMenu( RenderAPIString( settings.gfx.api ) ) ) {
            if ( ImGui::MenuItem( RenderAPIString( R_OPENGL ) ) ) {
                settings.gfx.api = R_OPENGL;
                ui->PlaySelected();
            }
            if ( ImGui::MenuItem( RenderAPIString( R_SDL2 ) ) ) {
                settings.gfx.api = R_SDL2;
                ui->PlaySelected();
            }
            if ( ImGui::MenuItem( RenderAPIString( R_VULKAN ) ) ) {
                settings.gfx.api = R_VULKAN;
                ui->PlaySelected();
            }
            ImGui::EndMenu();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Fullscreen" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.fullscreenStr[settings.gfx.fullscreen], settings.gfx.fullscreen ) ) {
            settings.gfx.fullscreen = !settings.gfx.fullscreen;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Lighting" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##LightingLeft", ImGuiDir_Left ) ) {
            switch ( settings.gfx.lighting ) {
            case 0:
                settings.gfx.lighting = 1;
                break;
            default:
                settings.gfx.lighting--;
                break;
            };
            ui->PlaySelected();
        }
        ImGui::SameLine();
        switch ( settings.gfx.lighting ) {
        case 0:
            ImGui::TextUnformatted( "Dynamic Lighting" );
            break;
        case 1:
            ImGui::TextUnformatted( "Static Lighting" );
            break;
        };
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##LightingRight", ImGuiDir_Right ) ) {
            switch ( settings.gfx.lighting ) {
            case 1:
                settings.gfx.lighting = 0;
                break;
            default:
                settings.gfx.lighting++;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "VSync" );
        ImGui::TableNextColumn();
        if ( ImGui::ArrowButton( "##VSYNCLEFT", ImGuiDir_Left ) ) {
            switch ( settings.gfx.vsync ) {
            case -1:
                settings.gfx.vsync = 1;
                break;
            default:
                settings.gfx.vsync--;
                break;
            };
            ui->PlaySelected();
        }
        ImGui::SameLine();
        switch ( settings.gfx.vsync ) {
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
            switch (settings.gfx.vsync) {
            case 1:
                settings.gfx.vsync = -1;
                break;
            default:
                settings.gfx.vsync++;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Gamma Correction" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderFloat( " ", &settings.gfx.gamma, 1.0f, 5.0f ) ) {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Bloom" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON! :)" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "HDR" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON! :)" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Ambient Occlusion" );
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "COMING SOON! :)" );

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Stuff For The NERDS" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.advancedGraphicsStr[settings.gfx.advancedGraphics], settings.gfx.advancedGraphics ) ) {
            settings.gfx.advancedGraphics = !settings.gfx.advancedGraphics;
            ui->PlaySelected();
        }

        if ( settings.gfx.advancedGraphics && settings.gfx.api == R_OPENGL ) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow Software GL Driver" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.gfx.GL_extended->allowLegacyGLStr[settings.gfx.GL_extended->allowSoftwareGL],
                settings.gfx.GL_extended->allowSoftwareGL ) )
            {
                settings.gfx.GL_extended->allowSoftwareGL = !settings.gfx.GL_extended->allowSoftwareGL;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow GL Extensions" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.useExtensionsStr[settings.gfx.extensions], settings.gfx.extensions ) ) {
                settings.gfx.extensions = !settings.gfx.extensions;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Allow Legacy GL Functionality" );
            ImGui::TableNextColumn();
            if ( ImGui::RadioButton( settings.gfx.GL_extended->allowLegacyGLStr[settings.gfx.GL_extended->allowLegacyGL],
                settings.gfx.GL_extended->allowLegacyGL ) )
            {
                settings.gfx.GL_extended->allowLegacyGL = !settings.gfx.GL_extended->allowLegacyGL;
                ui->PlaySelected();
            }

            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "GL Extensions" );
            ImGui::TableNextColumn();
            if ( ImGui::BeginMenu( settings.gfx.extensionsMenuStr )) {
                for ( uint32_t i = 0; i < settings.gfx.numExtensions; i++ ) {
                    ImGui::MenuItem( settings.gfx.extensionStrings[i] );
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
    if ( ui->GetState() != STATE_AUDIO ) {
        return;
    }

    ImGui::SeparatorText( "Master Volume" );
    if (ImGui::SliderInt( " ", &settings.sound.masterVol, 0, 100 )) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Sound Effects" );
    if (ImGui::RadioButton( "ON##SfxOn", settings.sound.sfxOn )) {
        settings.sound.sfxOn = !settings.sound.sfxOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##SfxVolume", &settings.sound.sfxVol, 0, 100 )) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Music" );
    if (ImGui::RadioButton( "ON##MusicOn", settings.sound.musicOn )) {
        settings.sound.musicOn = !settings.sound.musicOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt( "VOLUME##MusicVolume", &settings.sound.musicVol, 0, 100 )) {
        ui->PlaySelected();
    }
}

static void SettingsMenuGameplay_Draw( void )
{
    SettingsMenu_ExitChild( STATE_GAMEPLAY );
    if ( ui->GetState() != STATE_GAMEPLAY ) {
        return;
    }

    if ( sgvm ) {
        g_pModuleLib->ModuleCall( sgvm, ModuleDrawConfiguration, 0 );
        g_pModuleLib->RunModules( ModuleDrawConfiguration, 0 );
    }
}

static void SettingsMenuControls_Draw( void )
{
    uint32_t i;
    char bind[1024];
    char bind2[1024];

    SettingsMenu_ExitChild( STATE_CONTROLS );
    if ( ui->GetState() != STATE_CONTROLS ) {
        return;
    }

    // mouse options
    ImGui::SeparatorText( "MOUSE" );

    ImGui::BeginTable( " ", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Sensitivity" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderInt( " ", &settings.controls.mouseSensitivity, 0, 100 ) ) {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Invert" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.controls.mouseInvert ? "ON" : "OFF", settings.controls.mouseInvert ) ) {
            settings.controls.mouseInvert = !settings.controls.mouseInvert;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Acceleration" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.controls.mouseAccelerate ? "ON" : "OFF", settings.controls.mouseAccelerate ) ) {
            settings.controls.mouseAccelerate = !settings.controls.mouseAccelerate;
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

        for ( i = 0; i < settings.controls.numBinds; i++ ) {
            ImGui::TableNextColumn();
            if ( settings.controls.keybinds[i].bind1 == -1 ) {
                strcpy( bind, "???" );
            } else {
                strcpy( bind, Key_KeynumToString( settings.controls.keybinds[i].bind1 ) );
                N_strupr( bind );

                if ( settings.controls.keybinds[i].bind2 != -1 ) {
                    strcpy( bind2, Key_KeynumToString( settings.controls.keybinds[i].bind2 ) );
                    N_strupr( bind2 );

                    strcat( bind, " or " );
                    strcat( bind, bind2 );
                }
            }
            ImGui::TextUnformatted( bind );
            ImGui::TableNextColumn();
            if ( ImGui::Button( settings.controls.keybinds[i].label ) ) {
                settings.rebinding = qtrue;
                settings.controls.rebindIndex = i;
                ui->PlaySelected();
            }

            if ( i != settings.controls.numBinds - 1 ) {
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
    default:
        return;
    };

    SettingsMenu_Update();
    SettingsMenu_ApplyChanges();
}

static void SettingsMenu_GetInitial( void ) {
    memset( &initial, 0, sizeof( initial ) );

    initial.gfx.anisotropicFiltering = Cvar_VariableInteger( "r_anisotropicFiltering" );
    initial.gfx.useExtensions = Cvar_VariableInteger( "r_useExtensions" );
    initial.gfx.customWidth = Cvar_VariableInteger( "r_customWidth" );
    initial.gfx.customHeight = Cvar_VariableInteger( "r_customHeight" );
    initial.gfx.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    initial.gfx.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    initial.gfx.videoMode = Cvar_VariableInteger( "r_mode" );
    initial.gfx.fullscreen = Cvar_VariableInteger( "r_fullscreen" );
    initial.gfx.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );
    initial.gfx.multisamplingIndex = Cvar_VariableInteger( "r_multisample" );

    if ( initial.gfx.api == R_OPENGL ) {
        initial.gfx.GL_extended = (graphics_extended_GL_t *)Hunk_Alloc( sizeof( graphics_extended_GL_t ), h_high );
        initial.gfx.GL_extended->allowSoftwareGL = Cvar_VariableInteger( "r_allowSoftwareGL" );
        initial.gfx.GL_extended->use_GL_ARB_vertex_array_object = Cvar_VariableInteger( "r_arb_vertex_array_object" );
    }
    initial.gfx.vsync = Cvar_VariableInteger( "r_swapInterval" );
    initial.gfx.gamma = Cvar_VariableFloat( "r_gammaAmount" );

    initial.sound.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    initial.sound.musicOn = Cvar_VariableInteger( "snd_musicon" );
    initial.sound.sfxVol = Cvar_VariableInteger( "snd_sfxvol" );
    initial.sound.musicVol = Cvar_VariableInteger( "snd_musicvol" );
    initial.sound.masterVol = Cvar_VariableFloat( "snd_mastervol" );
    
    initial.controls.mouseSensitivity = Cvar_VariableInteger( "g_mouseSensitivity" );
    initial.controls.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    initial.controls.mouseInvert = Cvar_VariableInteger("g_mouseInvert");

    initial.controls.keybinds = (bind_t *)Hunk_Alloc( sizeof( *initial.controls.keybinds ) * settings.controls.numBinds, h_high );
    memcpy( initial.controls.keybinds, settings.controls.keybinds, sizeof( *initial.controls.keybinds ) * settings.controls.numBinds );
}

static void SettingsMenu_InitPresets( void )
{
    uint32_t i;
    graphics_t *p;

    p = settings.presets = (graphics_t *)Hunk_Alloc( sizeof( *settings.presets ) * NUM_PRESETS, h_high );

    p[PRESET_LOW_QUALITY].fullscreen = true;
}

void SettingsMenu_Cache( void ) {
    int32_t numExtensions;
    int32_t i;
    uint64_t len;

//    settings = (settingsmenu_t *)Hunk_Alloc( sizeof(*settings), h_high );

    memset( &settings, 0, sizeof( settings ) );

    settings.gfx.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );

    // get extensions list
    if ( settings.gfx.api == R_OPENGL ) {
        renderImport.glGetIntegerv( GL_NUM_EXTENSIONS, &numExtensions );

        settings.gfx.extensionStrings = (char **)Hunk_Alloc( sizeof( char * ) * numExtensions, h_high );
        settings.gfx.numExtensions = numExtensions;

        for ( i = 0; i < numExtensions; i++ ) {
            const GLubyte *name = renderImport.glGetStringi( GL_EXTENSIONS, i );
            len = strlen( (const char *)name );

            settings.gfx.extensionStrings[i] = (char *)Hunk_Alloc( len + 1, h_high );
            strcpy( settings.gfx.extensionStrings[i], (const char *)name );
        }
    }

    N_strncpyz( settings.gfx.extensionsMenuStr, va( "%u Extensions", settings.gfx.numExtensions ), sizeof( settings.gfx.extensionsMenuStr ) );

    SettingsMenu_SetDefault();
    SettingsMenu_GetInitial();

    settings.confirmation = qfalse;
    settings.modified = qfalse;
    settings.paused = Cvar_VariableInteger( "sg_paused" );
}
