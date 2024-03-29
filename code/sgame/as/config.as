#include "game.as"
#include "main.as"

TheNomad::GameSystem::GameDifficulty config_GameDifficulty;
bool config_AdaptiveSoundtrack;
bool config_DebugPrint;
uint config_MaxSoundChannels;
uint config_MaxEntities;
uint config_GfxDetail;

//
// ModuleConfigInit: initializes all sgame relevant configuration variables
//
void ModuleConfigInit() {
    config_AdaptiveSoundtrack = TheNomad::Util::IntToBool( TheNomad::Engine::CvarVariableInteger( "sgame_AdaptiveSoundTrack" ) );
    config_DebugPrint = TheNomad::Util::IntToBool( TheNomad::Engine::CvarVariableInteger( "sgame_DebugMode" ) );
    config_GameDifficulty = TheNomad::GameSystem::GameDifficulty( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) );
    config_MaxEntities = uint( TheNomad::Engine::CvarVariableInteger( "sgame_MaxEntities" ) );
    config_MaxSoundChannels = uint( TheNomad::Engine::CvarVariableInteger( "sgame_MaxSoundChannels" ) );
    config_GfxDetail = uint( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) );
}

void AdjustDifficulty() {
    if ( ImGui::ArrowButton( "##GameDifficultyLeft", ImGuiDir::Left ) ) {
        switch ( config_GameDifficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryEasy:
            config_GameDifficulty = TheNomad::GameSystem::GameDifficulty::VeryHard;
            break;
        default:
            config_GameDifficulty--;
            break;
        };
        TheNomad::SGame::selectedSfx.Play();
    }
    ImGui::SameLine();
    ImGui::Text( TheNomad::SGame::SP_DIFF_STRINGS[ config_GameDifficulty ] );
    ImGui::SameLine();
    if ( ImGui::ArrowButton( "##GameDifficultyRight", ImGuiDir::Right ) ) {
        switch ( config_GameDifficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryHard:
            config_GameDifficulty = TheNomad::GameSystem::GameDifficulty::VeryEasy;
            break;
        default:
            config_GameDifficulty++;
            break;
        };
        TheNomad::SGame::selectedSfx.Play();
    }
}

int ModuleDrawConfiguration() {
    uint tmp;
    return 0;

    ImGui::BeginTable( "##SGameConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
    if ( config_GameDifficulty != TheNomad::GameSystem::GameDifficulty::TryYourBest
    	|| TheNomad::SGame::GlobalState != TheNomad::SGame::GameState::InLevel )
    {
        AdjustDifficulty();
    } else {
    	// remember, no pussy
        ImGui::Text( "You CHOSE This, No Pullin' Out Now!" );
    }

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text( "Debug Messages" );
    ImGui::TableNextColumn();

    if ( ImGui::RadioButton( config_DebugPrint ? "##ONDebugPrintConfig" : "##OFFDebugPrintConfig", config_DebugPrint ) ) {
        config_DebugPrint = !config_DebugPrint;
        TheNomad::SGame::selectedSfx.Play();
    }

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text( "Adaptive Soundtrack" );
    ImGui::TableNextColumn();

    if ( ImGui::RadioButton( config_AdaptiveSoundtrack ? "##ONAdaptiveSoundtrackConfig" : "##OFFAdaptiveSoundtrackConfig",
        config_AdaptiveSoundtrack ) )
    {
        config_AdaptiveSoundtrack = !config_AdaptiveSoundtrack;
        TheNomad::SGame::selectedSfx.Play();
    }
    
    ImGui::TableNextRow();
    
    ImGui::TableNextColumn();
    ImGui::Text( "Max Entities" );
    ImGui::TableNextColumn();
    
    tmp = ImGui::SliderInt( "##MaxEntitiesConfig", config_MaxEntities, 100, 1000 );
    if ( tmp != config_MaxEntities ) {
    	TheNomad::SGame::selectedSfx.Play();
    }
    config_MaxEntities = tmp;

    ImGui::TableNextRow();
    
    ImGui::TableNextColumn();
    ImGui::Text( "Max Sound Channels" );
    ImGui::TableNextColumn();
    
    tmp = ImGui::SliderInt( "##MaxSoundChannelsConfig", config_MaxSoundChannels, 100, 1000 );
    if ( tmp != config_MaxSoundChannels ) {
    	TheNomad::SGame::selectedSfx.Play();
    }
    config_MaxSoundChannels = tmp;
    
    ImGui::TableNextRow();
    
    ImGui::TableNextColumn();
    ImGui::Text( "Gfx Detail" );
    ImGui::TableNextColumn();
    
    tmp = ImGui::SliderInt( "##GfxDetailConfig", config_GfxDetail, 0, 30 );
    if ( tmp != config_GfxDetail ) {
    	TheNomad::SGame::selectedSfx.Play();
    }
    config_GfxDetail = tmp;

    if ( config_AdaptiveSoundtrack != TheNomad::Util::UIntToBool( uint( TheNomad::SGame::sgame_AdaptiveSoundtrack.GetInt() ) ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }
    if ( config_DebugPrint != TheNomad::Util::UIntToBool( uint( TheNomad::SGame::sgame_DebugMode.GetInt() ) ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }
    if ( config_MaxEntities != uint( TheNomad::SGame::sgame_MaxEntities.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }
    if ( config_MaxSoundChannels != uint( TheNomad::SGame::sgame_MaxSoundChannels.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }
    if ( config_GfxDetail != uint( TheNomad::SGame::sgame_GfxDetail.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }
    if ( config_GameDifficulty != TheNomad::GameSystem::GameDifficulty( TheNomad::SGame::sgame_Difficulty.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "g_moduleConfigUpdate", "1" );
    }

    ImGui::EndTable();
    
	return 1;
}

int ModuleSaveConfiguration() {
    if ( config_MaxEntities != uint( TheNomad::SGame::sgame_MaxEntities.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "sgame_MaxEntities", formatUInt( config_MaxEntities ) );
    }
    if ( config_MaxSoundChannels != uint( TheNomad::SGame::sgame_MaxSoundChannels.GetInt() ) ) {
        TheNomad::Engine::CvarSet( "sgame_MaxSoundChannels", formatUInt( config_MaxSoundChannels ) );
    }
    TheNomad::Engine::CvarSet( "sgame_DebugMode", formatInt( config_DebugPrint ? 1 : 0 )  );
    TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
    TheNomad::Engine::CvarSet( "sgame_AdaptiveSoundtrack", formatInt( config_AdaptiveSoundtrack ? 1 : 0 ) );
    return 1;
}