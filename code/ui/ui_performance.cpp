#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SAVECONFIG   6
#define ID_SETDEFAULTS  7

#define ID_MULTISAMPLETYPE         0
#define ID_ANISOTROPICFILTERING    1
#define ID_TEXTUREDETAIL           2
#define ID_TEXTUREFILTER           3
#define ID_SPECULARMAPS            4
#define ID_NORMALMAPS              5
#define ID_BLOOM                   6
#define ID_AMBIENTOCCLUSION        7
#define ID_HDR                     8
#define ID_PBR                     9
#define ID_TONEMAPPING             10
#define ID_DYNAMICLIGHTING         11
#define ID_VERTEXLIGHTING          12
#define ID_TONEMAPPINGTYPE         13
#define ID_MAXSOUNDCHANNELS        14
#define ID_PARTICLEDETAIL          15
#define ID_MAXCORPSES              16
#define ID_MAXDLIGHTS              17
#define ID_POSTPROCESS             18

static const char *s_itemHints[] = {
	"",
};

typedef struct {
    menuframework_t menu;

    menucommon_t *focusItem;
    
    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;
    
    menutab_t tabs;
    
    menubutton_t save;
    menubutton_t setDefaults;

    menutable_t graphicsTable;
    menutable_t soundTable;

    menutext_t multisampleType;
    menutext_t anisotropicFiltering;
    menutext_t textureDetail;
    menutext_t textureFilter;
    menutext_t enableHDR;
    menutext_t enableAmbientOcclusion;
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
    menutext_t enablePostProcessing;

    menutext_t maxSoundChannels;

    menuarrow_t multisampleLeft;
    menuarrow_t multisampleRight;
    menuarrow_t anisotropyLeft;
    menuarrow_t anisotropyRight;
    menuarrow_t textureDetailLeft;
    menuarrow_t textureDetailRight;
    menuarrow_t textureFilterLeft;
    menuarrow_t textureFilterRight;
    menuarrow_t maxCorpsesLeft;
    menuarrow_t maxCorpsesRight;
    menuarrow_t maxDLightsLeft;
    menuarrow_t maxDLightsRight;
    menuarrow_t toneMappingTypeLeft;
    menuarrow_t toneMappingTypeRight;

    menuarrow_t maxSoundChannelsLeft;
    menuarrow_t maxSoundChannelsRight;

    menulist_t multisampleList;
    menulist_t anisotropyList;
    menulist_t textureDetailList;
    menulist_t textureFilterList;
    menulist_t toneMappingTypeList;
    menulist_t ambientOcclusionList;

	menuswitch_t enablePostProcessingButton;
	menuswitch_t enableAmbientOcclusionButton;
    menuswitch_t enableSpecularMapsButton;
    menuswitch_t enableNormalMapsButton;
    menuswitch_t enableSSAOButton;
    menuswitch_t enableHDRButton;
    menuswitch_t enablePBRButton;
    menuswitch_t enableBloomButton;
    menuswitch_t enableToneMappingButton;
    menuswitch_t enableVertexLightingButton;
    menuswitch_t enableDynamicLightinButton;
    menuslider_t maxCorpsesSlider;
    menuslider_t maxDLightsSlider;

    menuslider_t maxSoundChannelsSlider;
} performanceOptionsInfo_t;

static performanceOptionsInfo_t *s_performanceOptionsInfo;

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
    case ID_SAVECONFIG:
    	PerformanceSettingsMenu_Save();
    	break;
   	case ID_SETDEFAULTS:
   		PerformanceSettingsMenu_SetDefaults();
   		break;
    };
}

void PerformanceSettingsMenu_SetDefaults( void )
{
    Cvar_Reset( "r_hdr" );
}

void PerformanceSettingsMenu_Save( void )
{
	if ( s_performanceOptionsInfo->enablePostProcessingButton.curvalue ) {
		Cvar_SetIntegerValue( "r_multisampleType", s_performanceOptionsInfo->multisampleList.curitem );
		
		switch ( s_performanceOptionsInfo->multisampleList.curitem ) {
		case AntiAlias_None:
            Cvar_Set( "r_multisampleAmount", "0" );
            break;
		case AntiAlias_2xSSAA:
		case AntiAlias_2xMSAA:
			Cvar_Set( "r_multisampleAmount", "2" );
			break;
		case AntiAlias_4xSSAA:
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
		default:
			N_Error( ERR_DROP, "PerformanceSettingsMenu_Save: invalid multisample state" );
		};
		
		Cvar_SetIntegerValue( "r_ssao", s_performanceOptionsInfo->enableAmbientOcclusionButton.curvalue );
	} else {
		// force disable everything that relies on a disabled framebuffer
		Cvar_Set( "r_ssao", "0" );
		Cvar_Set( "r_multisampleType", "0" );
	}
	Cvar_SetIntegerValue( "r_hdr", s_performanceOptionsInfo->enableHDRButton.curvalue );
	Cvar_SetIntegerValue( "r_postProcess", s_performanceOptionsInfo->enablePostProcessingButton.curvalue );
	
	// only the gaussian blur requires a special framebuffer, not tone mapping itself
	Cvar_SetIntegerValue( "r_toneMap", s_performanceOptionsInfo->enableToneMappingButton.curvalue );
	Cvar_SetIntegerValue( "r_toneMapType", s_performanceOptionsInfo->toneMappingTypeList.curitem );
	
	Cvar_SetIntegerValue( "r_normalMapping", s_performanceOptionsInfo->enableNormalMapsButton.curvalue );
	Cvar_SetIntegerValue( "r_specularMapping", s_performanceOptionsInfo->enableSpecularMapsButton.curvalue );
	Cvar_Set( "r_textureMode", s_performanceOptionsInfo->textureDetailList.itemnames[s_performanceOptionsInfo->textureDetailList.curitem] );
	Cvar_SetIntegerValue( "r_textureFilter", s_performanceOptionsInfo->textureFilterList.curitem );
	
	switch ( s_performanceOptionsInfo->anisotropyList.curitem ) {
	case 0:
		Cvar_Set( "r_arb_texture_max_anisotropy", "0" );
		break;
	case 1:
		Cvar_Set( "r_arb_texture_max_anisotropy", "2" );
		break;
	case 2:
		Cvar_Set( "r_arb_texture_max_anisotropy", "4" );
		break;
	case 3:
		Cvar_Set( "r_arb_texture_max_anisotropy", "8" );
		break;
	case 4:
		Cvar_Set( "r_arb_texture_max_anisotropy", "16" );
		break;
	case 5:
		Cvar_Set( "r_arb_texture_max_anisotropy", "32" );
		break;
	default:
		N_Error( ERR_DROP, "PerformanceSettingsMenu_Save: invalid anisotropy state" );
	};
}

static void PerformanceMenu_Update( void )
{
	int i;
	bool noFocus;
	
	Menu_Draw( &s_performanceOptionsInfo->menu );
	
	if ( s_performanceOptionsInfo->focusItem ) {
//		s_itemHintPopup->message.text = s_itemHints[ s_performanceOptionsInfo->focusItem->id ];
//		Menu_Draw( &s_itemHintPopup->menu );
	}
}

static void PerformanceMenu_SliderLeft( void *ptr, int event )
{
    float value;

    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    value = ( (menuslider_t *)ptr )->curvalue - 1;
    if ( value < ( (menuslider_t *)ptr )->minvalue ) {
        value = ( (menuslider_t *)ptr )->minvalue;
    }
    ( (menuslider_t *)ptr )->curvalue = value;
}

static void PerformanceMenu_SliderRight( void *ptr, int event )
{
    float value;

    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    value = ( (menuslider_t *)ptr )->curvalue + 1;
    if ( value < ( (menuslider_t *)ptr )->minvalue ) {
        value = ( (menuslider_t *)ptr )->minvalue;
    }
    ( (menuslider_t *)ptr )->curvalue = value;
}

static void PerformanceMenu_SetHint( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	
	if ( ( (menucommon_t *)ptr )->type != MTYPE_TEXT ) {
		N_Error( ERR_DROP, "PerformanceMenu_SetHint: bad data, not a text object" );
	}
	
	s_performanceOptionsInfo->focusItem = (menucommon_t *)ptr;
}

static void PerformanceSettingsMenu_InitTheNerdyShit( void );

void PerformanceSettingsMenu_Cache( void )
{
	PROFILE_FUNCTION();
	
	static menucommon_t empty;
    static const char *multisampleTypes[] = {
        "2x MSAA",
        "4x MSAA",
        "8x MSAA",
        "16x MSAA",
        "32x MSAA",
        "2x SSAA",
        "4x SSAA"
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
    memset( &empty, 0, sizeof( empty ) );
    
    RobotoMono = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    AlegreyaSC = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    
    s_performanceOptionsInfo->save.generic.type = MTYPE_BUTTON;
    s_performanceOptionsInfo->save.generic.id = ID_SAVECONFIG;
    s_performanceOptionsInfo->save.generic.font = AlegreyaSC;
    
    s_performanceOptionsInfo->video.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->video.generic.id = ID_VIDEO;
    s_performanceOptionsInfo->video.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->video.generic.font = AlegreyaSC;
    s_performanceOptionsInfo->video.text = "Video##VideoSettingsMenuTabBar";
    s_performanceOptionsInfo->video.color = color_white;
    
    s_performanceOptionsInfo->performance.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->performance.generic.id = ID_PERFORMANCE;
    s_performanceOptionsInfo->performance.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->performance.generic.font = AlegreyaSC;
    s_performanceOptionsInfo->performance.text = "Performance##PerformanceSettingsMenuTabBar";
    s_performanceOptionsInfo->performance.color = color_white;
    
    s_performanceOptionsInfo->audio.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->audio.generic.id = ID_AUDIO;
    s_performanceOptionsInfo->audio.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->audio.generic.font = AlegreyaSC;
    s_performanceOptionsInfo->audio.text = "Audio##AudioSettingsMenuTabBar";
    s_performanceOptionsInfo->audio.color = color_white;
    
    s_performanceOptionsInfo->controls.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->controls.generic.id = ID_CONTROLS;
    s_performanceOptionsInfo->controls.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->controls.generic.font = AlegreyaSC;
    s_performanceOptionsInfo->controls.text = "Controls##ControlsSettingsMenuTabBar";
    s_performanceOptionsInfo->controls.color = color_white;
    
    s_performanceOptionsInfo->gameplay.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->gameplay.generic.id = ID_GAMEPLAY;
    s_performanceOptionsInfo->gameplay.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->gameplay.generic.font = AlegreyaSC;
    s_performanceOptionsInfo->gameplay.text = "Gameplay##GameplaySettingsMenuTabBar";
    s_performanceOptionsInfo->gameplay.color = color_white;
    
    s_performanceOptionsInfo->menu.fullscreen = qtrue;
    s_performanceOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_performanceOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_performanceOptionsInfo->menu.titleFontScale = 3.5f;
    s_performanceOptionsInfo->menu.textFontScale = 1.5f;
    s_performanceOptionsInfo->menu.draw = PerformanceMenu_Update;
    s_performanceOptionsInfo->menu.x = 0;
    s_performanceOptionsInfo->menu.y = 0;
    s_performanceOptionsInfo->menu.name = "Performance";

    s_performanceOptionsInfo->multisampleType.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->multisampleType.generic.id = ID_MULTISAMPLETYPE;
    s_performanceOptionsInfo->multisampleType.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->multisampleType.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->multisampleType.generic.font = RobotoMono;
    s_performanceOptionsInfo->multisampleType.text = "Anti-Aliasing";
    s_performanceOptionsInfo->multisampleType.color = color_white;
    
    s_performanceOptionsInfo->anisotropicFiltering.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->anisotropicFiltering.generic.id = ID_ANISOTROPICFILTERING;
    s_performanceOptionsInfo->anisotropicFiltering.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->anisotropicFiltering.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->anisotropicFiltering.generic.font = RobotoMono;
    s_performanceOptionsInfo->anisotropicFiltering.text = "Anisotropy Amount";
    s_performanceOptionsInfo->anisotropicFiltering.color = color_white;
    
    s_performanceOptionsInfo->textureDetail.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->textureDetail.generic.id = ID_TEXTUREDETAIL;
    s_performanceOptionsInfo->textureDetail.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->textureDetail.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->textureDetail.generic.font = RobotoMono;
    s_performanceOptionsInfo->textureDetail.text = "Texture Detail";
    s_performanceOptionsInfo->textureDetail.color = color_white;
    
    s_performanceOptionsInfo->textureFilter.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->textureFilter.generic.id = ID_TEXTUREFILTER;
    s_performanceOptionsInfo->textureFilter.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->textureFilter.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->textureFilter.generic.font = RobotoMono;
    s_performanceOptionsInfo->textureFilter.text = "Texture Filtering";
    s_performanceOptionsInfo->textureFilter.color = color_white;
    
    s_performanceOptionsInfo->enablePostProcessing.generic.type = MTYPE_BUTTON;
    s_performanceOptionsInfo->enablePostProcessing.generic.id = ID_POSTPROCESS;
    s_performanceOptionsInfo->enablePostProcessing.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enablePostProcessing.generic.eventcallback = PerformanceMenu_SetHint;
    
    s_performanceOptionsInfo->enableAmbientOcclusion.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableAmbientOcclusion.generic.id = ID_AMBIENTOCCLUSION;
    s_performanceOptionsInfo->enableAmbientOcclusion.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableAmbientOcclusion.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableAmbientOcclusion.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableAmbientOcclusion.text = "Ambient Occlusion";
    s_performanceOptionsInfo->enableAmbientOcclusion.color = color_white;
    
    s_performanceOptionsInfo->enableHDR.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableHDR.generic.id = ID_HDR;
    s_performanceOptionsInfo->enableHDR.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableHDR.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableHDR.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableHDR.text = "HDR";
    s_performanceOptionsInfo->enableHDR.color = color_white;
    
    s_performanceOptionsInfo->enablePBR.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enablePBR.generic.id = ID_PBR;
    s_performanceOptionsInfo->enablePBR.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enablePBR.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enablePBR.generic.font = RobotoMono;
    s_performanceOptionsInfo->enablePBR.text = "PBR";
    s_performanceOptionsInfo->enablePBR.color = color_white;
    
    s_performanceOptionsInfo->enableBloom.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableBloom.generic.id = ID_BLOOM;
    s_performanceOptionsInfo->enableBloom.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableBloom.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableBloom.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableBloom.text = "Bloom";
    s_performanceOptionsInfo->enableBloom.color = color_white;
    
    s_performanceOptionsInfo->enableToneMapping.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableToneMapping.generic.id = ID_TONEMAPPING;
    s_performanceOptionsInfo->enableToneMapping.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableToneMapping.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableToneMapping.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableToneMapping.text = "Tone Mapping";
    s_performanceOptionsInfo->enableToneMapping.color = color_white;
    
    s_performanceOptionsInfo->enableDynamicLighting.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableDynamicLighting.generic.id = ID_DYNAMICLIGHTING;
    s_performanceOptionsInfo->enableDynamicLighting.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableDynamicLighting.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableDynamicLighting.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableDynamicLighting.text = "Dynamic Lighting";
    s_performanceOptionsInfo->enableDynamicLighting.color = color_white;
    
    s_performanceOptionsInfo->enableVertexLighting.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableVertexLighting.generic.id = ID_VERTEXLIGHTING;
    s_performanceOptionsInfo->enableVertexLighting.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableVertexLighting.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableVertexLighting.generic.font = RobotoMono;
    s_performanceOptionsInfo->enableVertexLighting.text = "Vertex Lighting";
    s_performanceOptionsInfo->enableVertexLighting.color = color_white;
    
    s_performanceOptionsInfo->toneMappingType.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->toneMappingType.generic.id = ID_TONEMAPPINGTYPE;
    s_performanceOptionsInfo->toneMappingType.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->toneMappingType.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->toneMappingType.generic.font = RobotoMono;
    s_performanceOptionsInfo->toneMappingType.text = "Tone Mapping Type";
    s_performanceOptionsInfo->toneMappingType.color = color_white;
    
    s_performanceOptionsInfo->maxCorpses.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->maxCorpses.generic.id = ID_MAXCORPSES;
    s_performanceOptionsInfo->maxCorpses.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->maxCorpses.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->maxCorpses.generic.font = RobotoMono;
    s_performanceOptionsInfo->maxCorpses.text = "Max Corpses";
    s_performanceOptionsInfo->maxCorpses.color = color_white;
    
    s_performanceOptionsInfo->maxDLights.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->maxDLights.generic.id = ID_MAXDLIGHTS;
    s_performanceOptionsInfo->maxDLights.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->maxDLights.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->maxDLights.generic.font = RobotoMono;
    s_performanceOptionsInfo->maxDLights.text = "Max DLights";
    s_performanceOptionsInfo->maxDLights.color = color_white;
    
    s_performanceOptionsInfo->maxSoundChannels.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->maxSoundChannels.generic.id = ID_MAXSOUNDCHANNELS;
    s_performanceOptionsInfo->maxSoundChannels.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->maxSoundChannels.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->maxSoundChannels.text = "Max Sound Channels";
    s_performanceOptionsInfo->maxSoundChannels.color = color_white;
    
    s_performanceOptionsInfo->multisampleLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->multisampleLeft.generic.id = ID_MULTISAMPLETYPE;
    s_performanceOptionsInfo->multisampleLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_performanceOptionsInfo->multisampleLeft.generic.name = "##AntiAliasingPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->multisampleLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->multisampleLeft.color = color_white;
    s_performanceOptionsInfo->multisampleLeft.data = (menucommon_t *)&s_performanceOptionsInfo->multisampleList;
    
    s_performanceOptionsInfo->multisampleRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->multisampleRight.generic.id = ID_MULTISAMPLETYPE;
    s_performanceOptionsInfo->multisampleRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_performanceOptionsInfo->multisampleRight.generic.name = "##AntiAliasingPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->multisampleRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->multisampleRight.color = color_white;
    s_performanceOptionsInfo->multisampleRight.data = (menucommon_t *)&s_performanceOptionsInfo->multisampleRight;
    
    s_performanceOptionsInfo->toneMappingTypeLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->toneMappingTypeLeft.generic.id = ID_TONEMAPPINGTYPE;
    s_performanceOptionsInfo->toneMappingTypeLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_performanceOptionsInfo->toneMappingTypeLeft.generic.name = "##ToneMappingTypePerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->toneMappingTypeLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->toneMappingTypeLeft.color = color_white;
    s_performanceOptionsInfo->toneMappingTypeLeft.data = (menucommon_t *)&s_performanceOptionsInfo->toneMappingTypeList;
    
    s_performanceOptionsInfo->toneMappingTypeRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->toneMappingTypeRight.generic.id = ID_TONEMAPPINGTYPE;
    s_performanceOptionsInfo->toneMappingTypeRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_performanceOptionsInfo->toneMappingTypeRight.generic.name = "##ToneMappingTypePerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->toneMappingTypeRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->toneMappingTypeRight.color = color_white;
    s_performanceOptionsInfo->toneMappingTypeRight.data = (menucommon_t *)&s_performanceOptionsInfo->toneMappingTypeList;
   	
    s_performanceOptionsInfo->anisotropyLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->anisotropyLeft.generic.id = ID_ANISOTROPICFILTERING;
    s_performanceOptionsInfo->anisotropyLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_performanceOptionsInfo->anisotropyLeft.generic.name = "##AnisotropicFilteringPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->anisotropyLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->anisotropyLeft.color = color_white;
    s_performanceOptionsInfo->anisotropyLeft.data = (menucommon_t *)&s_performanceOptionsInfo->anisotropyList;
    
    s_performanceOptionsInfo->anisotropyRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->anisotropyRight.generic.id = ID_ANISOTROPICFILTERING;
    s_performanceOptionsInfo->anisotropyRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_performanceOptionsInfo->anisotropyRight.generic.name = "##AnisotropicFilteringPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->anisotropyRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->anisotropyRight.color = color_white;
    s_performanceOptionsInfo->anisotropyRight.data = (menucommon_t *)&s_performanceOptionsInfo->anisotropyList;
    
    s_performanceOptionsInfo->textureDetailLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->textureDetailLeft.generic.id = ID_TEXTUREDETAIL;
    s_performanceOptionsInfo->textureDetailLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_performanceOptionsInfo->textureDetailLeft.generic.name = "##TextureDetailPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->textureDetailLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->textureDetailLeft.color = color_white;
    s_performanceOptionsInfo->textureDetailLeft.data = (menucommon_t *)&s_performanceOptionsInfo->textureDetailList;
    
    s_performanceOptionsInfo->textureDetailRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->textureDetailRight.generic.id = ID_TEXTUREDETAIL;
    s_performanceOptionsInfo->textureDetailRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_performanceOptionsInfo->textureDetailRight.generic.name = "##TextureDetailPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->textureDetailRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->textureDetailRight.color = color_white;
    s_performanceOptionsInfo->textureDetailRight.data = (menucommon_t *)&s_performanceOptionsInfo->textureDetailList;
    
    s_performanceOptionsInfo->textureFilterLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->textureFilterLeft.generic.id = ID_TEXTUREFILTER;
    s_performanceOptionsInfo->textureFilterLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_performanceOptionsInfo->textureFilterLeft.generic.name = "##TextureFilteringPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->textureFilterLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->textureFilterLeft.color = color_white;
    s_performanceOptionsInfo->textureFilterLeft.data = (menucommon_t *)&s_performanceOptionsInfo->textureFilterList;
    
    s_performanceOptionsInfo->textureFilterRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->textureFilterRight.generic.id = ID_TEXTUREFILTER;
    s_performanceOptionsInfo->textureFilterRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_performanceOptionsInfo->textureFilterRight.generic.name = "##TextureFilteringPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->textureFilterRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->textureFilterRight.color = color_white;
    s_performanceOptionsInfo->textureFilterRight.data = (menucommon_t *)&s_performanceOptionsInfo->textureFilterList;
    
    s_performanceOptionsInfo->maxCorpsesLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->maxCorpsesLeft.generic.id = ID_MAXCORPSES;
    s_performanceOptionsInfo->maxCorpsesLeft.generic.eventcallback = PerformanceMenu_SliderLeft;
    s_performanceOptionsInfo->maxCorpsesLeft.generic.name = "##MaxCorpsesPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->maxCorpsesLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->maxCorpsesLeft.color = color_white;
    s_performanceOptionsInfo->maxCorpsesLeft.data = (menucommon_t *)&s_performanceOptionsInfo->maxCorpsesSlider;
    
    s_performanceOptionsInfo->maxCorpsesRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->maxCorpsesRight.generic.id = ID_MAXCORPSES;
    s_performanceOptionsInfo->maxCorpsesRight.generic.eventcallback = PerformanceMenu_SliderRight;
    s_performanceOptionsInfo->maxCorpsesRight.generic.name = "##MaxCorpsesPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->maxCorpsesRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->maxCorpsesRight.color = color_white;
    s_performanceOptionsInfo->maxCorpsesRight.data = (menucommon_t *)&s_performanceOptionsInfo->maxCorpsesSlider;
    
    s_performanceOptionsInfo->maxDLightsLeft.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->maxDLightsLeft.generic.id = ID_MAXDLIGHTS;
    s_performanceOptionsInfo->maxDLightsLeft.generic.eventcallback = PerformanceMenu_SliderLeft;
    s_performanceOptionsInfo->maxDLightsLeft.generic.name = "##MaxDLightsPerformanceSettingsMenuConfigLeft";
    s_performanceOptionsInfo->maxDLightsLeft.direction = ImGuiDir_Left;
    s_performanceOptionsInfo->maxDLightsLeft.color = color_white;
    s_performanceOptionsInfo->maxDLightsLeft.data = (menucommon_t *)&s_performanceOptionsInfo->maxDLightsSlider;
    
    s_performanceOptionsInfo->maxDLightsRight.generic.type = MTYPE_ARROW;
    s_performanceOptionsInfo->maxDLightsRight.generic.id = ID_MAXCORPSES;
    s_performanceOptionsInfo->maxDLightsRight.generic.eventcallback = PerformanceMenu_SliderRight;
    s_performanceOptionsInfo->maxDLightsRight.generic.name = "##MaxDLightsPerformanceSettingsMenuConfigRight";
    s_performanceOptionsInfo->maxDLightsRight.direction = ImGuiDir_Right;
    s_performanceOptionsInfo->maxDLightsRight.color = color_white;
    s_performanceOptionsInfo->maxDLightsRight.data = (menucommon_t *)&s_performanceOptionsInfo->maxDLightsSlider;
    
    s_performanceOptionsInfo->textureDetailList.generic.type = MTYPE_LIST;
    s_performanceOptionsInfo->textureDetailList.generic.id = ID_TEXTUREDETAIL;
    s_performanceOptionsInfo->textureDetailList.generic.name = "##TextureDetailPerformanceSettingsMenuConfigList";
    s_performanceOptionsInfo->textureDetailList.numitems = arraylen( textureDetail );
    s_performanceOptionsInfo->textureDetailList.itemnames = textureDetail;
    
    s_performanceOptionsInfo->toneMappingTypeList.generic.type = MTYPE_LIST;
    s_performanceOptionsInfo->toneMappingTypeList.generic.id = ID_TONEMAPPINGTYPE;
    s_performanceOptionsInfo->toneMappingTypeList.generic.name = "##ToneMappingTypePerformanceSettingsMenuConfigList";
    s_performanceOptionsInfo->toneMappingTypeList.numitems = arraylen( textureDetail );
    s_performanceOptionsInfo->toneMappingTypeList.itemnames = toneMappingTypes;
    
    s_performanceOptionsInfo->textureFilterList.generic.type = MTYPE_LIST;
    s_performanceOptionsInfo->textureFilterList.generic.id = ID_TEXTUREFILTER;
    s_performanceOptionsInfo->textureFilterList.generic.name = "##TextureFiltersPerformanceSettingsMenuConfigList";
    s_performanceOptionsInfo->textureFilterList.numitems = arraylen( textureFilters );
    s_performanceOptionsInfo->textureFilterList.itemnames = textureFilters;
    
    s_performanceOptionsInfo->multisampleList.generic.type = MTYPE_LIST;
    s_performanceOptionsInfo->multisampleList.generic.id = ID_MULTISAMPLETYPE;
    s_performanceOptionsInfo->multisampleList.generic.name = "##AntiAliasingPerformanceSettingsMenuConfigList";
    s_performanceOptionsInfo->multisampleList.numitems = arraylen( multisampleTypes );
    s_performanceOptionsInfo->multisampleList.itemnames = multisampleTypes;
    
    s_performanceOptionsInfo->anisotropyList.generic.type = MTYPE_LIST;
    s_performanceOptionsInfo->anisotropyList.generic.id = ID_MULTISAMPLETYPE;
    s_performanceOptionsInfo->anisotropyList.generic.name = "##AnisotropicFilteringPerformanceSettingsMenuConfigList";
    s_performanceOptionsInfo->anisotropyList.numitems = arraylen( anisotropyTypes );
    s_performanceOptionsInfo->anisotropyList.itemnames = anisotropyTypes;
    
    PerformanceSettingsMenu_InitTheNerdyShit();
    
    s_performanceOptionsInfo->tabs.generic.type = MTYPE_TAB;
    s_performanceOptionsInfo->tabs.generic.id = ID_TABLE;
    s_performanceOptionsInfo->tabs.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo->tabs.generic.name = "##SettingsMenuTabBar";
    s_performanceOptionsInfo->tabs.numitems = ID_TABLE;
    s_performanceOptionsInfo->tabs.items[0] = (menucommon_t *)&s_performanceOptionsInfo->video;
	s_performanceOptionsInfo->tabs.items[1] = (menucommon_t *)&s_performanceOptionsInfo->performance;
	s_performanceOptionsInfo->tabs.items[2] = (menucommon_t *)&s_performanceOptionsInfo->audio;
	s_performanceOptionsInfo->tabs.items[3] = (menucommon_t *)&s_performanceOptionsInfo->controls;
	s_performanceOptionsInfo->tabs.items[4] = (menucommon_t *)&s_performanceOptionsInfo->gameplay;
	
	s_performanceOptionsInfo->graphicsTable.generic.type = MTYPE_TABLE;
	s_performanceOptionsInfo->graphicsTable.generic.name = "##PerformanceSettingsMenuConfigTable";
	s_performanceOptionsInfo->graphicsTable.columns = 4;
	
	{
		PROFILE_SCOPE( "Performance Menu Register" );
		
	    Menu_AddItem( &s_performanceOptionsInfo->menu, &s_performanceOptionsInfo->tabs );
	    
	    Menu_AddItem( &s_performanceOptionsInfo->menu, &s_performanceOptionsInfo->graphicsTable );
	    
	    Table_AddRow( &s_performanceOptionsInfo->graphicsTable );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->multisampleType );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->multisampleLeft );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->multisampleList );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->multisampleRight );
	    
	    Table_AddRow( &s_performanceOptionsInfo->graphicsTable );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->anisotropicFiltering );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->anisotropyLeft );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->anisotropyList );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->anisotropyRight );
		
	    Table_AddRow( &s_performanceOptionsInfo->graphicsTable );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->textureFilter );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->textureFilterLeft );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->textureFilterList );
	    Table_AddItem( &s_performanceOptionsInfo->graphicsTable, &s_performanceOptionsInfo->textureFilterRight );
	
	    Menu_AddItem( &s_performanceOptionsInfo->menu, &s_performanceOptionsInfo->save );
	    Menu_AddItem( &s_performanceOptionsInfo->menu, &s_performanceOptionsInfo->setDefaults );
	}
}

void UI_PerformanceSettingsMenu( void )
{
    UI_PushMenu( &s_performanceOptionsInfo->menu );
}

static void PerformanceSettingsMenu_InitTheNerdyShit( void )
{
	s_performanceOptionsInfo->enableSpecularMaps.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableSpecularMaps.generic.id = ID_SPECULARMAPS;
    s_performanceOptionsInfo->enableSpecularMaps.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableSpecularMaps.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableSpecularMaps.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_performanceOptionsInfo->enableSpecularMaps.text = "Specular Mapping";
    s_performanceOptionsInfo->enableSpecularMaps.color = color_white;
    
    s_performanceOptionsInfo->enableNormalMaps.generic.type = MTYPE_TEXT;
    s_performanceOptionsInfo->enableNormalMaps.generic.id = ID_NORMALMAPS;
    s_performanceOptionsInfo->enableNormalMaps.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_performanceOptionsInfo->enableNormalMaps.generic.eventcallback = PerformanceMenu_SetHint;
    s_performanceOptionsInfo->enableNormalMaps.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_performanceOptionsInfo->enableNormalMaps.text = "Normal Maps";
    s_performanceOptionsInfo->enableNormalMaps.color = color_white;
}