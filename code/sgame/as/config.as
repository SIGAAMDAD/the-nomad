#include "game.as"
#include "main.as"

shared class ModuleConfig {
    ModuleConfig( ModuleObject@ main ) {
        config_GameDifficulty = TheNomad::GameSystem::GameDifficulty( main.sgame_Difficulty.GetInt() );
        config_AdaptiveSoundtrack = true;
        @ModObject = @main;
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
            ModObject.selectedSfx.Play();
    	}
        ImGui::SameLine();
        ImGui::Text( ModObject.SP_DIFF_STRINGS[ config_GameDifficulty ] );
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
            ModObject.selectedSfx.Play();
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
            ModObject.selectedSfx.Play();
        }

        ImGui::EndTable();

    	return 1;
    }

    int ModuleSaveConfiguration() {
        TheNomad::Engine::CvarSet( "sgame_Difficulty", formatInt( int( config_GameDifficulty ) ) );
        TheNomad::Engine::CvarSet( "sgame_AdaptiveSoundtrack", formatInt( config_AdaptiveSoundtrack ? 1 : 0 ) );
        return 1;
    }

    TheNomad::GameSystem::GameDifficulty config_GameDifficulty;
    bool config_AdaptiveSoundtrack;

    private ModuleObject@ ModObject;
};
