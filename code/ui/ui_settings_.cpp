#include "ui_lib.h"

typedef struct {
	// graphics
	int multisampleType;
	int anisotropicFiltering;
	qboolean enablePostProcessing;
	qboolean enableBloom;
	qboolean enableHdr;
	qboolean enableShadows;
	qboolean enableVertexLighting;
	qboolean enableDynamicLighting;
	qboolean enableSSAO;
	qboolean enableToneMapping;
    qboolean enablePbr;
	int toneMappingType;
	
	// advanced graphics
	qboolean useNormalMapping;
	qboolean useSpecularMapping;
	uint32_t maxPolys;
	uint32_t maxEntities;
	uint32_t maxDLights;
	
	// gameplay
	uint32_t maxParticles;
	uint32_t maxCorpses;
	uint32_t maxEntities;
	
	
	// sound
	uint32_t maxSoundChannels;
} performance_t;

typedef struct {
	int windowMode;
	int screenWidth;
	int screenHeight;
	int vsync;
	int api;
	int maxfps;
	float exposure;
	float gamma;
} video_t;

typedef struct {
	int masterVol;
	int sfxVol;
	int musicVol;
	qboolean sfxOn;
	qboolean musicOn;
} audio_t;

typedef struct {
	performance_t performance;
	video_t video;
	audio_t audio;
} settings_t;

static settings_t *initial;
static settings_t *settings;

static void SettingsMenu_GetInitial( void )
{
	settings->performance.enableHdr = Cvar_VariableInteger( "r_hdr" );
	settings->performance.enablePbr = Cvar_VariableInteger( "r_pbr" );
	settings->performance.enableBloom = Cvar_VariableInteger( "r_bloom" );
}

static void SettingsMenu_SetInitial( void )
{
}

static void SettingsMenu_SaveVideo( void )
{
	if ( settings->video.vsync != initial->video.vsync ) {
		Cvar_Set( "r_swapInterval", va( "%i", settings->video.vsync ) );
	}
}

static void SettingsMenu_SavePerformance( bool needRestart )
{
	bool restartFramebuffer;
	
	if ( settings->performance.enableHdr != initial->performance.enableHdr ) {
		restartFramebuffer = true;
	}
	if ( settings->performance.enablePbr != initial->performance.enablePbr ) {
		restartFramebuffer = true;
	}
	
	if ( !needRestart && restartFramebuffer ) {
		Cbuf_ExecuteText( EXEC_APPEND, "renderlib.restart_framebuffer\n" );
	}
}

static void SettingsMenu_Save( void )
{
	bool needRestart;
	
	needRestart = false;
	
	// we'll need a restart if we're changing anything physical
	if ( settings->video.windowMode != initial->video.windowMode ) {
		needRestart = true;
	}
	if ( settings->video.api != initial->video.api ) {
		needRestart = true;
	}
	if ( settings->video.screenWidth != initial->video.screenWdith
		|| settings->video.screenHeight != initial->video.screenHeight )
	{
		needRestart = true;
	}
	
	SettingsMenu_SaveVideo();
	SettingsMenu_SavePerformance( needRestart );
	
	SettingsMenu_GetInitial();
	SettingsMenu_SetInitial();
	
	if ( needRestart ) {
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
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

static void SetHint( const char *label, const char *hint )
{
	const int windowFlags = ;
	
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		ImGui::Begin( va( "##Tooltip%s", label ), NULL, windowFlags );
		ImGui::End();
	}
}

static void SettingsMenu_Performance_Draw( void ) {
	SettingsMenu_ExitChild( STATE_PERFORMANCE );
	if ( ui->GetState() != STATE_PERFORMANCE ) {
		return;
	}
	
	perfomance_t *performance;
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
	
	performance = &settings->performance;
	
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##GraphicsPerformanceSettingsConfigNode", "Graphics", treeNodeFlags ) {
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
			if ( ImGui::RadioButton( performance->vertexLighting ? "ON##VertexLightingGraphicsPerformanceSettingsConfigON" :
				"OFF##VertexLightingGraphicsPerformanceSettingsConfigOFF", performance->vertexLighting ) )
			{
				ui->PlaySelected();
				performance->vertexLighting = !performance->vertexLighting;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Dynamic Lighting" );
			SetHint( "Dymamic Lighting", "Enables higher quality per pixel lighting done in a shader along with dynamic lights" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->dynamicLighting ? "ON##DynamicLightingGraphicsPerformanceSettingsConfigON" :
				"OFF##DynamicLightingGraphicsPerformanceSettingsConfigOFF", performance->dynamicLighting ) )
			{
				ui->PlaySelected();
				performance->dynamicLighting = !performance->dynamicLighting;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Enable Shadows" );
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();
			if ( ImGui::RadioButton( performance->shadows ? "ON##ShadowsGraphicsPerformanceSettingsConfigON" :
				"OFF##ShadowsGraphicsPerformanceSettingsConfigOFF", performance->shadows ) )
			{
				ui->PlaySelected();
				performance->shadows = !performance->shadows;
			}
			ImGui::TableNextColumn();
			
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted( "Shadow Quality" );
			ImGui::TableNextColumn();
			
		}
		ImGui::EndTable();
		ImGui::TreePop();
	}
		
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##AdvancedPerformanceSettingsConfigNode", "Advanced", treeNodeFlags ) {
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
	
	SettingsMenu_InitPresets();
	SettingsMenu_GetInitial();
	SettingsMenu_GetDefault();
}