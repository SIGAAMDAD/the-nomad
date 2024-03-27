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
#include "../rendercommon/imgui.h"

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
    int32_t multisamplingIndex;
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
    int cpuAffinity;
    uint32_t hunkMegs;
    uint32_t zoneMegs;
    uint32_t maxModuleStackMemory;

    int alwaysCompileModules;
    int allowModuleJIT;

    const char *cpuString;
} engine_t;

typedef struct {
    sound_t sound;
    graphics_t gfx;
    controls_t controls;
    engine_t performance;
} initialSettings_t;

typedef struct {
    qboolean confirmation;
    qboolean confirmreset;
    qboolean modified;
    qboolean paused; // did we get here from the pause menu?
    qboolean rebinding;

    int32_t presetIndex;

    eastl::array<const char *, 2> fullscreenStr;
    eastl::array<const char *, 2> advancedGraphicsStr;
    eastl::array<const char *, 2> useExtensionsStr;

    sound_t sound;
    graphics_t gfx;
    controls_t controls;
    engine_t performance;

    graphics_t *presets;

    menustate_t lastChild;

    qboolean showGPUDriverInfo;

    int totalGPUMemory;
    int availableGPUMemory;
} settingsmenu_t;

extern ImFont *PressStart2P;
extern ImFont *RobotoMono;
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
    { "None",                       AntiAlias_None },
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
    Cvar_Set( "r_hdr", va( "%i", settings.gfx.hdr ) );

    switch ( settings.gfx.multisamplingIndex ) {
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
    Cvar_Set( "r_multisampleType", va( "%i", settings.gfx.multisamplingIndex ) );

    Cvar_Set( "g_renderer", RenderAPIString2( settings.gfx.api ) );
}

static void SettingsMenu_ApplyAudioChanges( void )
{
    Cvar_Set( "snd_sfxvol", va( "%i", settings.sound.sfxVol ) );
    Cvar_Set( "snd_musicvol", va( "%i", settings.sound.musicVol ) );
    Cvar_Set( "snd_sfxon", va( "%i", settings.sound.sfxOn ) );
    Cvar_Set( "snd_musicon", va ( "%i", settings.sound.musicOn ) );
}

static void SettingsMenu_ApplyEngineChanges( void )
{
    Cvar_Set( "com_affinityMask", va( "0x%i", settings.performance.cpuAffinity ) );
    Cvar_Set( "com_hunkMegs", va( "%u", settings.performance.hunkMegs ) );
    Cvar_Set( "com_hunkMegs", va( "%u", settings.performance.zoneMegs ) );
    Cvar_Set( "ml_alwaysCompile", va( "%i", settings.performance.alwaysCompileModules ) );
    Cvar_Set( "ml_allowJIT", va( "%i", settings.performance.allowModuleJIT ) );

    if ( Cvar_VariableInteger( "g_moduleConfigUpdate" ) ) {
        g_pModuleLib->ModuleCall( sgvm, ModuleSaveConfiguration, 0 );
        g_pModuleLib->RunModules( ModuleSaveConfiguration, 0 );
    }
    Cvar_Set( "g_moduleConfigUpdate", "0" );
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
    settings.gfx.multisamplingIndex = Cvar_VariableInteger( "r_multisampleType" );
    settings.gfx.vsync = Cvar_VariableInteger( "r_swapInterval" );
    settings.gfx.gamma = Cvar_VariableFloat( "r_gammaAmount" );
    settings.gfx.hdr = Cvar_VariableInteger( "r_hdr" );
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
    settings.controls.mouseAccelerate = Cvar_VariableInteger( "g_mouseAcceleration" );
    settings.controls.mouseInvert = Cvar_VariableInteger( "g_mouseInvert" );

#ifdef USE_AFFINITY_MASK
    settings.performance.cpuAffinity = Sys_GetAffinityMask();
#endif
    settings.performance.hunkMegs = Cvar_VariableInteger( "com_hunkMegs" );
    settings.performance.zoneMegs = Cvar_VariableInteger( "com_zoneMegs" );
    settings.performance.cpuString = Cvar_VariableString( "sys_cpuString" );
    settings.performance.allowModuleJIT = Cvar_VariableInteger( "ml_allowJIT" );
    settings.performance.alwaysCompileModules = Cvar_VariableInteger( "ml_alwaysCompile" );

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
    ImGui::SetWindowPos( ImVec2( 200 * ui->scale, 340 * ui->scale ) );
    if (settings.confirmation) {
        ImGui::TextUnformatted( "You made some changes to your settings, would you like to apply them?" );
        if (ImGui::Button( "SAVE CHANGES##SETTINGSMENUPOPUP" )) {
            ui->PlaySelected();
            SettingsMenu_ApplyAudioChanges();
            SettingsMenu_ApplyGraphicsChanges();
            SettingsMenu_ApplyEngineChanges();
            settings.confirmation = qfalse;
            settings.modified = qfalse;
            Cbuf_ExecuteText( EXEC_APPEND, "writecfg glnomad.cfg\n" );
            Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
            ImGui::CloseCurrentPopup();
        }
        if ( ImGui::Button( "NO##SETTINGSMENUPOPUP" ) ) {
            ui->PlaySelected();
            settings.modified = qfalse;
            settings.confirmation = qfalse;
            SettingsMenu_SetDefault();
            Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
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
            Cbuf_ExecuteText( EXEC_APPEND, "writecfg glnomad.cfg" );
            Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
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
    ImGui::Begin( "##APPLYCHANGESWINDOWRESET", NULL, windowFlags );

    ImGui::SetWindowPos(ImVec2( 8 * ui->scale, 720 * ui->scale ));

    if (ImGui::Button( "RESET TO DEFAULT" )) {
        settings.confirmreset = qtrue;
        ImGui::OpenPopup( "Reset To Default##SETTINGSMENUPOPUP" );
        ui->PlaySelected();
    }

    ImGui::End();

    if ( !settings.modified ) {
        return;
    }

    ImGui::Begin( "##APPLYCHANGESWINDOWSAVE", NULL, windowFlags );

    ImGui::SetCursorScreenPos( ImVec2( 260 * ui->scale, 725 * ui->scale ) );
    
    if ( ImGui::Button( "APPLY CHANGES" ) ) {
        SettingsMenu_ApplyGraphicsChanges();
        SettingsMenu_ApplyAudioChanges();
        SettingsMenu_ApplyEngineChanges();
        Cbuf_ExecuteText( EXEC_APPEND, "writecfg glnomad.cfg\n" );
        Cbuf_ExecuteText( EXEC_APPEND, "snd_restart\n" );
        settings.modified = qfalse;
        ui->PlaySelected();
    }
    ImGui::End();
}

static void SettingsMenu_Update( void )
{
    settings.modified = false;

    if ( Cvar_VariableInteger( "g_moduleConfigUpdate" ) ) {
        settings.modified = true;
    }
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
    if ( initial.performance.cpuAffinity != settings.performance.cpuAffinity ) {
        settings.modified = true;
    }
    if ( initial.performance.hunkMegs != settings.performance.hunkMegs ) {
        settings.modified = true;
    }
    if ( initial.performance.zoneMegs != settings.performance.zoneMegs ) {
        settings.modified = true;
    }
    if ( initial.performance.allowModuleJIT != settings.performance.allowModuleJIT ) {
        settings.modified = true;
    }
    if ( initial.performance.alwaysCompileModules != settings.performance.alwaysCompileModules ) {
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

static void SettingsMenu_Bar( void )
{
    if ( ImGui::BeginTabBar( "##SettingsMenuBar" ) ) {
        ImGui::PushStyleColor( ImGuiCol_Tab, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
        ImGui::PushStyleColor( ImGuiCol_TabActive, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        ImGui::PushStyleColor( ImGuiCol_TabHovered, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) );

        if ( ImGui::BeginTabItem( "GRAPHICS" ) ) {
            ui->SetState( STATE_GRAPHICS );
            if ( settings.lastChild != STATE_GRAPHICS ) {
                ui->PlaySelected();
                settings.lastChild = STATE_GRAPHICS;
            }
            ImGui::EndTabItem();
        }
        if ( ImGui::BeginTabItem( "ENGINE" ) ) {
            ui->SetState( STATE_PERFORMANCE );
            if ( settings.lastChild != STATE_PERFORMANCE ) {
                ui->PlaySelected();
                settings.lastChild = STATE_PERFORMANCE;
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
        ImGui::PopStyleColor( 3 );
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
        } else {
            return;
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

static void SettingsMenuPerformance_Draw( void )
{
    SettingsMenu_ExitChild( STATE_PERFORMANCE );
    if ( ui->GetState() != STATE_PERFORMANCE ) {
        return;
    }

    ImGui::TextUnformatted( "WARNING: DO NOT MODIFY UNLESS YOU KNOW WHAT YOU ARE DOING, YOU COULD FUCK SOMETHING UP!!!" );
    ImGui::NewLine();

    ImGui::BeginTable( "##PerformanceSettings", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "CPU Core Affinity" );
        ImGui::TableNextColumn();
        if ( ImGui::ArrowButton( "##CPUAffinityConfigLeft", ImGuiDir_Left ) ) {
            switch ( settings.performance.cpuAffinity ) {
            case 2:
                settings.performance.cpuAffinity = SDL_GetCPUCount();
                break;
            default:
                settings.performance.cpuAffinity--;
                break;
            };
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::Text( "%i", settings.performance.cpuAffinity );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##CPUAffinityConfigRight", ImGuiDir_Right ) ) {
            if ( settings.performance.cpuAffinity == SDL_GetCPUCount() ) {
                settings.performance.cpuAffinity = 1;
            } else {
                settings.performance.cpuAffinity++;
            }
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Hunk Size" );
        ImGui::TableNextColumn();

        if ( ImGui::SliderInt( "##HunkSizeConfig", (int *)&settings.performance.hunkMegs, 256, 8192, "%u",
            ImGuiInputTextFlags_EnterReturnsTrue ) )
        {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Zone Size" );
        ImGui::TableNextColumn();

        if ( ImGui::SliderInt( "##ZoneSizeConfig", (int *)&settings.performance.zoneMegs, 24, 8192, "%u",
            ImGuiInputTextFlags_EnterReturnsTrue ) )
        {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Always Compile Modules" );
        ImGui::TableNextColumn();

        if ( ImGui::RadioButton( settings.performance.alwaysCompileModules ? "ON##AlwaysCompileModules" : "OFF##AlwaysCompileModules",
            settings.performance.alwaysCompileModules ) )
        {
            settings.performance.alwaysCompileModules = !settings.performance.alwaysCompileModules;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Allow Module JIT" );
        ImGui::TableNextColumn();
        
        if ( ImGui::RadioButton( settings.performance.allowModuleJIT ? "ON##ModuleJIT" : "OFF##ModuleJIT",
            settings.performance.allowModuleJIT ) )
        {
            settings.performance.allowModuleJIT = !settings.performance.allowModuleJIT;
            ui->PlaySelected();
        }
    }
    ImGui::EndTable();
}

static void SettingsMenuGraphics_Draw( void )
{
    uint64_t i;
    static const char *presetTitles[NUM_PRESETS] = {
        "Low",
        "Normal",
        "High",
        "Ultra High Quality",
        "Performance"
    };

    SettingsMenu_ExitChild( STATE_GRAPHICS );
    if ( ui->GetState() != STATE_GRAPHICS ) {
        return;
    }

    ImGui::BeginTable( "##GraphicsSettings", 2 );
    {
        const char *vidMode;

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Quality Preset" );
        ImGui::TableNextColumn();
        if ( ImGui::BeginCombo( "##GPUQualityPreset", settings.presetIndex != -1 ? presetTitles[ settings.presetIndex ] : "Custom" ) ) {
            for ( i = 0; i < NUM_PRESETS; i++ ) {
                if ( ImGui::Selectable( presetTitles[i], ( settings.presetIndex == i ) ) ) {
                    settings.presetIndex = i;
                    memcpy( &settings.gfx, &settings.presets[i], sizeof( *settings.presets ) );
                }
            }
            if ( ImGui::Selectable( "Custom", ( settings.presetIndex == -1 ) ) ) {
                settings.presetIndex = -1;
            }
            ImGui::EndCombo();
        }

        ImGui::TableNextRow();
        ImGui::TableNextRow();

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Anti-Aliasing" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##ANTIALIASLEFT", ImGuiDir_Left ) ) {
            switch ( settings.gfx.multisamplingIndex ) {
            case -1:
                settings.gfx.multisamplingIndex = AntiAlias_DSSAA;
                break;
            default:
                settings.gfx.multisamplingIndex--;
                break;
            };
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        if ( settings.gfx.multisamplingIndex == -1 ) {
            ImGui::TextUnformatted( "None" );
        } else {
            ImGui::TextUnformatted( antialiasSettings[settings.gfx.multisamplingIndex].label );
        }
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##ANTIALIASRIGHT", ImGuiDir_Right ) ) {
            switch ( settings.gfx.multisamplingIndex ) {
            case AntiAlias_DSSAA:
                settings.gfx.multisamplingIndex = -1;
                break;
            default:
                settings.gfx.multisamplingIndex++;
                break;
            };
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Filtering" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##TextureFilteringLeft", ImGuiDir_Left ) ) {
            switch ( settings.gfx.texfilter ) {
            case TexFilter_Linear:
                settings.gfx.texfilter = TexFilter_Trilinear;
                break;
            default:
                settings.gfx.texfilter = (textureFilter_t)( (unsigned)settings.gfx.texfilter - 1 );
                break;
            };
            settings.gfx.texfilterString = TexFilterString( settings.gfx.texfilter );
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( settings.gfx.texfilterString );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##TextureFilteringRight", ImGuiDir_Right ) ) {
            switch ( settings.gfx.texfilter ) {
            case TexFilter_Trilinear:
                settings.gfx.texfilter = TexFilter_Linear;
                break;
            default:
                settings.gfx.texfilter = (textureFilter_t)( (unsigned)settings.gfx.texfilter + 1 );
                break;
            };
            settings.gfx.texfilterString = TexFilterString( settings.gfx.texfilter );
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Texture Detail" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##TextureDetailLeft", ImGuiDir_Left ) ) {
            switch ( settings.gfx.texdetail ) {
            case TexDetail_MSDOS:
                settings.gfx.texdetail = TexDetail_GPUvsGod;
                break;
            default:
                settings.gfx.texdetail = (textureDetail_t)( (unsigned)settings.gfx.texdetail - 1 );
                break;
            };
            settings.gfx.texdetailString = TexDetailString( settings.gfx.texdetail );
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( settings.gfx.texdetailString );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##TextureDetailRight", ImGuiDir_Right ) ) {
            switch ( settings.gfx.texdetail ) {
            case TexDetail_GPUvsGod:
                settings.gfx.texdetail = TexDetail_MSDOS;
                break;
            default:
                settings.gfx.texdetail = (textureDetail_t)( (unsigned)settings.gfx.texdetail + 1 );
                break;
            };
            settings.gfx.texdetailString = TexDetailString( settings.gfx.texdetail );
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Anisotropic Filtering" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##ANIFILTER_LEFT", ImGuiDir_Left ) ) {
            settings.gfx.anisotropicFiltering = (int32_t)settings.gfx.anisotropicFiltering - 1 == -1 ? arraylen(anisotropicFilters) - 1
                : settings.gfx.anisotropicFiltering - 1;
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( anisotropicFilters[ settings.gfx.anisotropicFiltering ].label );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##ANIFILTER_RIGHT", ImGuiDir_Right ) ) {
            settings.gfx.anisotropicFiltering = settings.gfx.anisotropicFiltering + 1 >= arraylen(anisotropicFilters)
                ? 0 : settings.gfx.anisotropicFiltering + 1;
            settings.presetIndex = -1;
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
            settings.presetIndex = -1;
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
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Renderer" );
        ImGui::TableNextColumn();

        if ( ImGui::ArrowButton( "##RendererLeft", ImGuiDir_Left ) ) {
            if ( settings.gfx.api == R_SDL2 ) {
                settings.gfx.api = R_VULKAN;
            } else if ( settings.gfx.api == R_OPENGL ) {
                settings.gfx.api = R_SDL2;
            } else if ( settings.gfx.api == R_VULKAN ) {
                settings.gfx.api = R_OPENGL;
            }
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted( RenderAPIString( settings.gfx.api ) );
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##RendererRight", ImGuiDir_Right ) ) {
            if ( settings.gfx.api == R_SDL2 ) {
                settings.gfx.api = R_OPENGL;
            } else if ( settings.gfx.api == R_OPENGL ) {
                settings.gfx.api = R_VULKAN;
            } else if ( settings.gfx.api == R_VULKAN ) {
                settings.gfx.api = R_SDL2;
            }
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Fullscreen" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.fullscreenStr[settings.gfx.fullscreen], settings.gfx.fullscreen ) ) {
            settings.gfx.fullscreen = !settings.gfx.fullscreen;
            settings.presetIndex = -1;
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
            settings.presetIndex = -1;
            ui->PlaySelected();
        }
        ImGui::SameLine();
        switch ( settings.gfx.lighting ) {
        case LIGHTING_DYNAMIC:
            ImGui::TextUnformatted( "Dynamic Lighting" );
            break;
        case LIGHTING_STATIC:
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
            settings.presetIndex = -1;
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
            settings.presetIndex = -1;
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
            switch ( settings.gfx.vsync ) {
            case 1:
                settings.gfx.vsync = -1;
                break;
            default:
                settings.gfx.vsync++;
                break;
            };
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Gamma Correction" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderFloat( "##GammaCorrectionConfig", &settings.gfx.gamma, 1.0f, 5.0f ) ) {
            ui->PlaySelected();
            settings.presetIndex = -1;
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
        if ( ImGui::RadioButton( settings.gfx.hdr ? "##HDRGraphicsConfigON" : "##HDRGraphicsConfigOFF", settings.gfx.hdr ) ) {
            settings.gfx.hdr = !settings.gfx.hdr;
            settings.presetIndex = -1;
            ui->PlaySelected();
        }

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
            settings.presetIndex = -1;
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
        }
//        ImGui::TableNextRow();
//        
//        ImGui::TableNextColumn();
//        ImGui::TextUnformatted( "GPU Driver Information" );
//        ImGui::TableNextColumn();
//        if ( ImGui::RadioButton( settings.showGPUDriverInfo ? "ON##ShowGPUDriverInfo" : "OFF##ShowGPUDriverInfo", settings.showGPUDriverInfo ) ) {
//            settings.showGPUDriverInfo = !settings.showGPUDriverInfo;
//            ui->PlaySelected();
//        }
//
//        if ( settings.showGPUDriverInfo ) {
//            ImGui::TableNextRow();
//            ImGui::TextUnformatted( "GPU Extensions Count" );
//            ImGui::TableNextColumn();
//            ImGui::Text( "%u", settings.gfx.numExtensions );
//
//            ImGui::TableNextRow();
//            ImGui::TextUnformatted( "GPU Extensions List" );
//            ImGui::TableNextColumn();
//            if ( ImGui::BeginMenu( "Extensions" ) ) {
//                for ( uint32_t i = 0; i < settings.gfx.numExtensions; i++ ) {
//                    switch ( settings.gfx.api ) {
//                    case R_OPENGL: {
//                        ImVec4 color;
//                        if ( settings.gfx.GL_extended->extensions[i].enabled ) {
//                            color = ImVec4( 0.0f, 1.0f, 0.0f, 1.0f );
//                        } else {
//                            color = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
//                        }
//                        ImGui::PushStyleColor( ImGuiCol_FrameBg, color );
//                        ImGui::PushStyleColor( ImGuiCol_FrameBgActive, color );
//                        ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, color );
//                        ImGui::MenuItem( settings.gfx.GL_extended->extensions[i].name );
//                        ImGui::PopStyleColor( 3 );
//                        break; }
//                    case R_VULKAN:
//                        N_Error( ERR_FATAL, "Vulkan not implemented" );
//                        break;
//                    };
//                }
//                ImGui::EndMenu();
//            }
//        }

/*
        ImGui::Begin( "##GPUMemoryInfo", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs );
        ImGui::SetWindowPos( { (float)830 * ui->scale, (float)700 * ui->scale } );
        ImGui::SetWindowSize( { (float)500 * ui->scale, (float)100 * ui->scale } );
        ImGui::SetWindowFontScale( 1.90f );
        FontCache()->SetActiveFont( RobotoMono );

        ImGui::TextUnformatted( "Available GPU Memory" );
        ImGui::SameLine();
        ImGui::SliderInt( "##AvailableGPUMemory", &settings.availableGPUMemory, 0, settings.totalGPUMemory, "%i", ImGuiSliderFlags_NoInput );

        ImGui::End();
        FontCache()->SetActiveFont( PressStart2P );
        */
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
    if ( ImGui::SliderInt( "##MasterVolumeSlider", &settings.sound.masterVol, 0, 100 ) ) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Sound Effects" );
    if ( ImGui::RadioButton( "ON##SfxOn", settings.sound.sfxOn ) ) {
        settings.sound.sfxOn = !settings.sound.sfxOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if ( ImGui::SliderInt( "VOLUME##SfxVolume", &settings.sound.sfxVol, 0, 100 ) ) {
        ui->PlaySelected();
    }

    ImGui::SeparatorText( "Music" );
    if ( ImGui::RadioButton( "ON##MusicOn", settings.sound.musicOn ) ) {
        settings.sound.musicOn = !settings.sound.musicOn;
        ui->PlaySelected();
    }
    ImGui::SameLine();
    if ( ImGui::SliderInt( "VOLUME##MusicVolume", &settings.sound.musicVol, 0, 100 ) ) {
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

    ImGui::BeginTable( "##MouseConfiguration", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Sensitivity" );
        ImGui::TableNextColumn();
        if ( ImGui::SliderInt( "##MouseSensitivityConfiguration", &settings.controls.mouseSensitivity, 0, 100 ) ) {
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Invert" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.controls.mouseInvert ? "ON##MouseInvert" : "OFF##MouseInvert", settings.controls.mouseInvert ) ) {
            settings.controls.mouseInvert = !settings.controls.mouseInvert;
            ui->PlaySelected();
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Mouse Acceleration" );
        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( settings.controls.mouseAccelerate ? "ON##MouseAcceleration" : "OFF##MouseAcceleration", settings.controls.mouseAccelerate ) ) {
            settings.controls.mouseAccelerate = !settings.controls.mouseAccelerate;
            ui->PlaySelected();
        }
    }
    ImGui::EndTable();

    // key bindings
    ImGui::SeparatorText( "KEY BINDS" );

    ImGui::BeginTable( "##KeyBindingsTable", 2 );
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
    case STATE_PERFORMANCE:
        SettingsMenuPerformance_Draw();
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
    initial.gfx.multisamplingIndex = Cvar_VariableInteger( "r_multisampleType" );
    initial.gfx.hdr = Cvar_VariableInteger( "r_hdr" );

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

#ifdef USE_AFFINITY_MASK
    initial.performance.cpuAffinity = Sys_GetAffinityMask();
#endif
    initial.performance.hunkMegs = Cvar_VariableInteger( "com_hunkMegs" );
    initial.performance.zoneMegs = Cvar_VariableInteger( "com_zoneMegs" );
    initial.performance.cpuString = Cvar_VariableString( "sys_cpuString" );
    initial.performance.allowModuleJIT = Cvar_VariableInteger( "ml_allowJIT" );
    initial.performance.alwaysCompileModules = Cvar_VariableInteger( "ml_alwaysCompile" );

    initial.controls.keybinds = (bind_t *)Hunk_Alloc( sizeof( *initial.controls.keybinds ) * settings.controls.numBinds, h_high );
    memcpy( initial.controls.keybinds, settings.controls.keybinds, sizeof( *initial.controls.keybinds ) * settings.controls.numBinds );
}

static void SettingsMenu_InitPresets( void )
{
    uint32_t i;
    graphics_t *p;

    p = settings.presets = (graphics_t *)Hunk_Alloc( sizeof( *settings.presets ) * NUM_PRESETS, h_high );

    for ( i = 0; i < NUM_PRESETS; i++ ) {
        p[i].api = settings.gfx.api;
        p[i].fullscreen = settings.gfx.fullscreen;
        p[i].advancedGraphics = settings.gfx.advancedGraphics;
        p[i].gamma = settings.gfx.gamma;
        p[i].vsync = 1;
        p[i].customHeight = settings.gfx.customHeight;
        p[i].customWidth = settings.gfx.customWidth;
        p[i].videoMode = settings.gfx.videoMode;
        p[i].GL_extended = settings.gfx.GL_extended;
        p[i].VK_extended = settings.gfx.VK_extended;
    }

    Cvar_Get( "r_gfxPresetIndex", "1", CVAR_PROTECTED | CVAR_LATCH | CVAR_SAVE );
    settings.presetIndex = Cvar_VariableInteger( "r_gfxPresetIndex" );

    p[PRESET_LOW_QUALITY].texdetail = TexDetail_IntegratedGPU;
    p[PRESET_LOW_QUALITY].texfilter = TexFilter_Nearest;
    p[PRESET_LOW_QUALITY].bloom = true;
    p[PRESET_LOW_QUALITY].hdr = false;
    p[PRESET_LOW_QUALITY].anisotropicFiltering = 1;
    p[PRESET_LOW_QUALITY].exposure = 1.5f;
    p[PRESET_LOW_QUALITY].geometrydetail = GeomDetail_Low;
    p[PRESET_LOW_QUALITY].hdr = qfalse;
    p[PRESET_LOW_QUALITY].texquality = 1;
    p[PRESET_LOW_QUALITY].lighting = LIGHTING_STATIC;
    p[PRESET_LOW_QUALITY].multisamplingIndex = AntiAlias_2xMSAA;
    p[PRESET_LOW_QUALITY].texdetailString = TexDetailString( p[PRESET_LOW_QUALITY].texdetail );
    p[PRESET_LOW_QUALITY].texfilterString = TexFilterString( p[PRESET_LOW_QUALITY].texfilter );

    p[PRESET_NORMAL_QUALITY].texdetail = TexDetail_Normie;
    p[PRESET_NORMAL_QUALITY].texfilter = TexFilter_Nearest;
    p[PRESET_NORMAL_QUALITY].bloom = true;
    p[PRESET_NORMAL_QUALITY].hdr = true;
    p[PRESET_NORMAL_QUALITY].anisotropicFiltering = 2;
    p[PRESET_NORMAL_QUALITY].exposure = 1.5f;
    p[PRESET_NORMAL_QUALITY].geometrydetail = GeomDetail_Normal;
    p[PRESET_NORMAL_QUALITY].hdr = qtrue;
    p[PRESET_NORMAL_QUALITY].texquality = 1;
    p[PRESET_NORMAL_QUALITY].lighting = LIGHTING_STATIC;
    p[PRESET_NORMAL_QUALITY].multisamplingIndex = AntiAlias_8xMSAA;
    p[PRESET_NORMAL_QUALITY].texdetailString = TexDetailString( p[PRESET_NORMAL_QUALITY].texdetail );
    p[PRESET_NORMAL_QUALITY].texfilterString = TexFilterString( p[PRESET_NORMAL_QUALITY].texfilter );

    p[PRESET_HIGH_QUALITY].texdetail = TexDetail_ExpensiveShitWeveGotHere;
    p[PRESET_HIGH_QUALITY].texfilter = TexFilter_Nearest;
    p[PRESET_HIGH_QUALITY].bloom = true;
    p[PRESET_HIGH_QUALITY].hdr = true;
    p[PRESET_HIGH_QUALITY].anisotropicFiltering = 3;
    p[PRESET_HIGH_QUALITY].exposure = 1.5f;
    p[PRESET_HIGH_QUALITY].geometrydetail = GeomDetail_High;
    p[PRESET_HIGH_QUALITY].hdr = qtrue;
    p[PRESET_HIGH_QUALITY].texquality = 1;
    p[PRESET_HIGH_QUALITY].lighting = LIGHTING_DYNAMIC;
    p[PRESET_HIGH_QUALITY].multisamplingIndex = AntiAlias_16xMSAA;
    p[PRESET_HIGH_QUALITY].texdetailString = TexDetailString( p[PRESET_HIGH_QUALITY].texdetail );
    p[PRESET_HIGH_QUALITY].texfilterString = TexFilterString( p[PRESET_HIGH_QUALITY].texfilter );

    p[PRESET_PERFORMANCE].texdetail = TexDetail_IntegratedGPU;
    p[PRESET_PERFORMANCE].texfilter = TexFilter_Nearest;
    p[PRESET_PERFORMANCE].bloom = false;
    p[PRESET_PERFORMANCE].hdr = false;
    p[PRESET_PERFORMANCE].anisotropicFiltering = 0;
    p[PRESET_PERFORMANCE].exposure = 1.5f;
    p[PRESET_PERFORMANCE].geometrydetail = GeomDetail_VeryLow;
    p[PRESET_PERFORMANCE].hdr = qtrue;
    p[PRESET_PERFORMANCE].texquality = 1;
    p[PRESET_PERFORMANCE].lighting = LIGHTING_STATIC;
    p[PRESET_PERFORMANCE].multisamplingIndex = -1;
    p[PRESET_PERFORMANCE].texdetailString = TexDetailString( p[PRESET_PERFORMANCE].texdetail );
    p[PRESET_PERFORMANCE].texfilterString = TexFilterString( p[PRESET_PERFORMANCE].texfilter );

    p[PRESET_ULTRA_HIGH_QUALITY].texdetail = TexDetail_GPUvsGod;
    p[PRESET_ULTRA_HIGH_QUALITY].texfilter = TexFilter_Nearest;
    p[PRESET_ULTRA_HIGH_QUALITY].bloom = true;
    p[PRESET_ULTRA_HIGH_QUALITY].hdr = true;
    p[PRESET_ULTRA_HIGH_QUALITY].anisotropicFiltering = 3;
    p[PRESET_ULTRA_HIGH_QUALITY].exposure = 1.5f;
    p[PRESET_ULTRA_HIGH_QUALITY].geometrydetail = GeomDetail_VeryHigh;
    p[PRESET_ULTRA_HIGH_QUALITY].hdr = qtrue;
    p[PRESET_ULTRA_HIGH_QUALITY].texquality = 1;
    p[PRESET_ULTRA_HIGH_QUALITY].lighting = LIGHTING_DYNAMIC;
    p[PRESET_ULTRA_HIGH_QUALITY].multisamplingIndex = AntiAlias_32xMSAA;
    p[PRESET_ULTRA_HIGH_QUALITY].texdetailString = TexDetailString( p[PRESET_ULTRA_HIGH_QUALITY].texdetail );
    p[PRESET_ULTRA_HIGH_QUALITY].texfilterString = TexFilterString( p[PRESET_ULTRA_HIGH_QUALITY].texfilter );
}

void SettingsMenu_Cache( void ) {
    int32_t numExtensions;
    int32_t i;
    uint64_t len;

//    settings = (settingsmenu_t *)Hunk_Alloc( sizeof(*settings), h_high );

    memset( &settings, 0, sizeof( settings ) );

    settings.gfx.api = StringToRenderAPI( Cvar_VariableString( "g_renderer" ) );
    Cvar_Get( "g_moduleConfigUpdate", "0", CVAR_TEMP );

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

        renderImport.glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &settings.totalGPUMemory );
    }

    N_strncpyz( settings.gfx.extensionsMenuStr, va( "%u Extensions", settings.gfx.numExtensions ), sizeof( settings.gfx.extensionsMenuStr ) );

    SettingsMenu_SetDefault();
    SettingsMenu_InitPresets();
    SettingsMenu_GetInitial();

    settings.confirmation = qfalse;
    settings.modified = qfalse;
    settings.paused = Cvar_VariableInteger( "sg_paused" );
}
