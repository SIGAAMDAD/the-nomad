class SGameConfig
{
    TheNomad::GameSystem::GameDifficulty difficulty;
};

SGameConfig config;

int ModuleDrawConfig() {
    ImGui::BeginTable( "##GameDifficultyConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
	if ( ImGui::ArrowButton( "##GameDifficultyLeft", ImGuiDir_Left ) ) {
        switch ( config.difficulty ) {
        case TheNomad::GameSystem::GameDifficulty::VeryEasy:
            config.difficulty = TheNomad::GameSystem::GameDifficulty::TryYourBest;
            break;
        default:
            config.difficulty--;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( UI_SELECTED_SOUND );
	}
	if ( ImGui::ArrowButton( "##GameDifficultyRight", ImGuiDir_Right ) ) {

	}
	return 1;
}

int ModuleSaveConfig() {
    return 1;
}
