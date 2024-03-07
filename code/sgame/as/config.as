#include "game.as"

enum SoundTrack {
    SoundTrack_MGR,
    SoundTrack_BlackFlag,
    SoundTrack_DOOM,
    SoundTrack_TheNomad,

    NumSoundTracks
};

TheNomad::GameSystem::GameDifficulty config_GameDifficulty;
SoundTrack config_SoundTrack;

array<string> soundTrackTitles = {
    "MGR",
    "Assassin's Creed IV: Black Flag",
    "DOOM",
    "The Nomad"
};

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
    ImGui::Text( "Soundtrack" );
    ImGui::TableNextColumn();

    if ( ImGui::ArrowButton( "##SoundtrackLeft", ImGuiDir_Left ) ) {
        switch ( config_SoundTrack ) {
        case SoundTrack::SoundTrack_MGR:
            config_SoundTrack = SoundTrack::SoundTrack_TheNomad;
            break;
        default:
            config_SoundTrack--;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( TheNomad::SGame::selectedSfx );
    }
    ImGui::SameLine();
    ImGui::Text( soundTrackTitles[ config_SoundTrack ] );
    ImGui::SameLine();
    if ( ImGui::ArrowButton( "##SoundtrackRight", ImGuiDir_Right ) ) {
        switch ( config_SoundTrack ) {
        case SoundTrack::SoundTrack_TheNomad:
            config_SoundTrack = SoundTrack::SoundTrack_MGR;
            break;
        default:
            config_SoundTrack++;
            break;
        };
        TheNomad::Engine::SoundSystem::PlaySfx( TheNomad::SGame::selectedSfx );
    }

    ImGui::EndTable();
	return 1;
}

int ModuleSaveConfiguration() {
    TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
    return 1;
}
