#include "game.as"
#include "main.as"

TheNomad::GameSystem::GameDifficulty config_GameDifficulty;
bool config_AdaptiveSoundtrack;

//
// ModuleConfigInit: initializes all sgame relevant configuration variables
//
void ModuleConfigInit() {
    config_AdaptiveSoundtrack = TheNomad::Util::IntToBool( TheNomad::Engine::CvarVariableInteger( "sgame_AdaptiveSoundTrack" ) );
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
    ImGui::BeginTable( "##SGameConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
    if ( config_GameDifficulty != TheNomad::GameSystem::GameDifficulty::TryYourBest ) {
        AdjustDifficulty();
    } else {
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

    ImGui::EndTable();
	return 1;
}

int ModuleSaveConfiguration() {
    TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
    TheNomad::Engine::CvarSet( "sgame_AdaptiveSoundtrack", formatInt( config_AdaptiveSoundtrack ? 1 : 0 ) );
    return 1;
}
