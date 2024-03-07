#include "game.as"

TheNomad::GameSystem::GameDifficulty config_GameDifficulty;

int ModuleDrawConfiguration() {
    ImGui::BeginTable( "##GameDifficultyConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( "##GameDifficultyLeft", ImGuiDir_Left ) ) {
        switch ( config_GameDifficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryEasy:
            config_GameDifficulty = TheNomad::GameSystem::GameDifficulty::TryYourBest;
            break;
        default:
            config_GameDifficulty--;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( UI_SELECTED_SOUND );
	}
    ImGui::SameLine();
    ImGui::SameLine();

    ImGui::EndTable();
	return 1;
}

int ModuleSaveConfiguration() {
    return 1;
}
