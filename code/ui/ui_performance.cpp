#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

#define ID_MULTISAMPLETYPE         0
#define ID_ANISOTROPICFILTER       1
#define ID_TEXTUREDETAIL           2
#define ID_TEXTUREFILTER           3
#define ID_SPECULARMAPS            4
#define ID_NORMALMAPS              5
#define ID_AMBIENTOCCLUSION        6
#define ID_BLOOM                   7
#define ID_SSAO                    8
#define ID_HDR                     9
#define ID_PBR                     10
#define ID_ENABLETONEMAPPING       11
#define ID_DYNAMICLIGHTING         12
#define ID_VERTEXLIGHTING          13
#define ID_TONEMAPPINGTYPE         14
#define ID_MAXSOUNDCHANNELS        15
#define ID_PARTICLEDETAIL          16
#define ID_MAXCORPSES              17
#define ID_MAXDLIGHTS              18

typedef struct {
    menuframework_t menu;

    menutable_t graphicsTable;
    menutable_t soundTable;

    menutext_t multisampleType;
    menutext_t anisotropicFiltering;
    menutext_t textureDetail;
    menutext_t textureFilter;
    menutext_t enableAmbientOcclusion;
    menutext_t enableSSAO;
    menutext_t enableHDR;
    menutext_t enablePBR;
    menutext_t enableBloom;
    menutext_t enableSpecularMaps;
    menutext_t enableNormalMaps;
    menutext_t enableToneMapping;
    menutext_t enableDynamicLighting;
    menutext_t enableVertexLighting;
    menutext_t toneMappingType;
    menutext_t maxCorpses;
    menutext_t maxDLights;

    menutext_t maxSoundChannels;

    menuarrow_t multisampleLeft;
    menuarrow_t multisampleRight;
    menuarrow_t anisotropyLeft;
    menuarrow_t anisotropyRight;
    menuarrow_t maxCorpsesLeft;
    menuarrow_t maxCorpsesRight;
    menuarrow_t maxDLightsLeft;
    menuarrow_t maxDLightsRight;

    menuarrow_t maxSoundChannelsLeft;
    menuarrow_t maxSoundChannelsRight;

    menulist_t multisampleList;
    menulist_t anisotropyList;
    menulist_t textureDetailList;
    menulist_t textureFilterList;
    menulist_t toneMappingTypeList;

    menuswitch_t enableSpecularMapsButton;
    menuswitch_t enableNormalMapsButton;
    menuswitch_t enableAmbientOcclusionButton;
    menuswitch_t enableSSAOButton;
    menuswitch_t enableHDRButton;
    menuswitch_t enablePBRButton;
    menuswitch_t enableBloomButton;
    menuswitch_t enableToneMappingButton;
    menuswitch_t enableVertexLightingButton;
    menuswitch_t enableDynamicLightinButton;
    menuslider_t maxCorpsesSlider;
    menuslider_t maxDLightSlider;

    menuslider_t maxSoundChannelsSlider;
} performanceOptionsInfo_t;

static performanceOptionsInfo_t *s_performanceOptionsInfo;
extern menutab_t settingsTabs;
extern menubutton_t settingsResetDefaults;
extern menubutton_t settingsSave;

static void PerformanceSettingsMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_VIDEO:
        UI_PopMenu();
        UI_VideoSettingsMenu();
        break;
    case ID_PERFORMANCE:
        break;
    case ID_AUDIO:
        UI_PopMenu();
        UI_AudioSettingsMenu();
        break;
    case ID_CONTROLS:
        UI_PopMenu();
        UI_ControlsSettingsMenu();
        break;
    case ID_GAMEPLAY:
        UI_PopMenu();
        UI_GameplaySettingsMenu();
        break;
    default:
        break;
    };
}

void PerformanceSettingsMenu_Save( void )
{
    Cvar_SetIntegerValue( "r_multisampleType", s_performanceOptionsInfo->multisampleList.curitem );

    switch ( s_performanceOptionsInfo->multisampleList.curitem ) {
    case AntiAlias_2xMSAA:
        Cvar_SetIntegerValue( "r_multisampleAmount", 2 );
        break;
    case AntiAlias_4xMSAA:
        Cvar_SetIntegerValue( "r_multisampleAmount", 4 );
        break;
    case AntiAlias_8xMSAA:
        Cvar_SetIntegerValue( "r_multisampleAmount", 8 );
        break;
    case AntiAlias_16xMSAA:
        Cvar_SetIntegerValue( "r_multisampleAmount", 16 );
        break;
    case AntiAlias_32xMSAA:
        Cvar_SetIntegerValue( "r_multisampleAmount", 32 );
        break;
    };

    Cvar_Set( "r_arb_texture_filter_anisotropic", va( "%i", s_performanceOptionsInfo->anisotropyList.curitem ? 1 : 0 ) );
    switch ( s_performanceOptionsInfo->anisotropyList.curitem ) {
    case 0:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 0 );
        break;
    case 2:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 2 );
        break;
    case 4:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 4 );
        break;
    case 8:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 8 );
        break;
    case 16:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 16 );
        break;
    case 32:
        Cvar_SetIntegerValue( "r_arb_texture_max_anisotropy", 32 );
        break;
    };

    Cvar_SetIntegerValue( "r_normalMapping", s_performanceOptionsInfo->enableNormalMapsButton.curvalue );
    Cvar_SetIntegerValue( "r_specularMapping", s_performanceOptionsInfo->enableSpecularMapsButton.curvalue );
    Cvar_SetIntegerValue( "r_bloom", s_performanceOptionsInfo->enableBloomButton.curvalue );
    Cvar_Set( "r_textureMode", s_performanceOptionsInfo->textureDetailList.itemnames[ s_performanceOptionsInfo->textureFilterList.curitem ] );
    Cvar_SetIntegerValue( "r_textureFiltering", s_performanceOptionsInfo->textureFilterList.curitem );
}

void PerformanceSettingsMenu_SetDefaults( void )
{
}

void PerformanceSettingsMenu_Cache( void )
{
    static const char *multisampleTypes[] = {
        "2x MSAA",
        "4x MSAA",
        "8x MSAA",
        "16x MSAA",
        "32x MSAA"
    };
    static const char *anisotropyTypes[] = {
        "2x",
        "4x",
        "8x",
        "16x",
        "32x"
    };
    static const char *textureDetail[] = {
        "MS-DOS",
        "Integrated GPU",
        "Normie",
        "Expensive Shit We've Got Here!",
        "GPU vs GOD"
    };
    static const char *textureFilters[] = {
        "Bilinear",
        "Nearest",
        "Linear Nearest",
        "Nearest Linear"
    };
    static const char *toneMappingTypes[] = {
        "Reinhard",
        "Exposure"
    };

    if ( !ui->uiAllocated ) {
        s_performanceOptionsInfo = (performanceOptionsInfo_t *)Hunk_Alloc( sizeof( *s_performanceOptionsInfo ), h_high );
    }
    memset( s_performanceOptionsInfo, 0, sizeof( *s_performanceOptionsInfo ) );

    s_performanceOptionsInfo->menu.fullscreen = qtrue;
    s_performanceOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_performanceOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_performanceOptionsInfo->menu.titleFontScale = 3.5f;
    s_performanceOptionsInfo->menu.textFontScale = 1.5f;
    s_performanceOptionsInfo->menu.x = 0;
    s_performanceOptionsInfo->menu.y = 0;
    s_performanceOptionsInfo->menu.name = "Performance##SettingsMenu";
    s_performanceOptionsInfo->menu.flags = MENU_DEFAULT_FLAGS;

    s_performanceOptionsInfo->multisampleType.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->multisampleType.generic.id = ID_MULTISAMPLETYPE;

    Menu_AddItem( &s_performanceOptionsInfo->menu, &settingsTabs );

    Menu_AddItem( &s_performanceOptionsInfo->menu, &settingsSave );
    Menu_AddItem( &s_performanceOptionsInfo->menu, &settingsResetDefaults );
}

void UI_PerformanceSettingsMenu( void )
{
    PerformanceSettingsMenu_Cache();
    UI_PushMenu( &s_performanceOptionsInfo->menu );
}
