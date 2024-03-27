#include "game.as"
#include "main.as"

TheNomad::GameSystem::GameDifficulty config_GameDifficulty;
bool config_AdaptiveSoundtrack;
uint config_MaxEntities;
uint config_GfxDetail;

//
// ModuleConfigInit: initializes all sgame relevant configuration variables
//
void ModuleConfigInit() {
    config_AdaptiveSoundtrack = TheNomad::Util::IntToBool( TheNomad::Engine::CvarVariableInteger( "sgame_AdaptiveSoundTrack" ) );
    config_GameDifficulty = TheNomad::GameSystem::GameDifficulty( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) );
    config_MaxEntities = uint( TheNomad::Engine::CvarVariableInteger( "sgame_MaxEntities" ) );
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
    ImGui::Text( "Adaptive Soundtrack" );
    ImGui::TableNextColumn();

    if ( ImGui::RadioButton( config_AdaptiveSoundtrack ? "ON##AdaptiveSoundtrackConfig" : "OFF##AdaptiveSoundtrackConfig",
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
    ImGui::Text( "Gfx Detail" );
    ImGui::TableNextColumn();
    
    tmp = ImGui::SliderInt( "##GfxDetailConfig", config_GfxDetail, 0, 30 );
    if ( tmp != config_GfxDetail ) {
    	TheNomad::SGame::selectedSfx.Play();
    }
    config_GfxDetail = tmp;

    ImGui::EndTable();
    
	return 1;
}

int ModuleSaveConfiguration() {
    TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
    TheNomad::Engine::CvarSet( "sgame_AdaptiveSoundtrack", formatInt( config_AdaptiveSoundtrack ? 1 : 0 ) );
    return 1;
}