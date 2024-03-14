#include "game.as"

TheNomad::GameSystem::GameDifficulty config_GameDifficulty = TheNomad::GameSystem::GameDifficulty( TheNomad::SGame::sgame_Difficulty.GetInt() );
bool config_AdaptiveSoundtrack = true;

int ModuleDrawConfiguration() {
    ImGui::BeginTable( "##GameDifficultyConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( "##GameDifficultyLeft", ImGuiDir_Left ) ) {
        switch ( config_GameDifficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryEasy:
            config_GameDifficulty = TheNomad::GameSystem::GameDifficulty::VeryHard;
            break;
        default:
            config_GameDifficulty--;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( TheNomad::SGame::selectedSfx );
	}
    ImGui::SameLine();
    ImGui::Text( TheNomad::SGame::SP_DIFF_STRINGS[ config_GameDifficulty ] );
    ImGui::SameLine();
    if ( ImGui::ArrowButton( "##GameDifficultyRight", ImGuiDir_Right ) ) {
        switch ( config_GameDifficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryHard:
            config_GameDifficulty = TheNomad::GameSystem::GameDifficulty::VeryEasy;
            break;
        default:
            config_GameDifficulty++;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( TheNomad::SGame::selectedSfx );
    }

    ImGui::TableNextRow();
    
    ImGui::TableNextColumn();
    ImGui:Text( "Adaptive Soundtrack" );
    ImGui::TableNextColumn();

    if ( ImGui::RadioButton( config_AdaptiveSoundtrack ? "ON##AdaptiveSoundtrackConfig" : "OFF##AdaptiveSoundtrackConfig",
        config_AdaptiveSoundtrack ) )
    {
        TheNomad::Engine::SoundSystem::PlaySfx( TheNomad::SGame::selectedSfx );
    }

	return 1;
}

int ModuleSaveConfiguration() {
    TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
    TheNomad::Engine::CvarSet( "sgame_AdaptiveSoundtrack", formatInt( int( uint( config_AdaptiveSoundtrack ) ) ) );
    return 1;
}
