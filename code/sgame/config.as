#include "Engine/UserInterface/ConfigSet.as"
#include "Engine/UserInterface/ConfigVar.as"
#include "Engine/UserInterface/ConfigSliderVar.as"
#include "Engine/UserInterface/ConfigButtonVar.as"
#include "Engine/UserInterface/ConfigListVar.as"
#include "main.as"

TheNomad::Engine::UserInterface::ConfigSet sgame_Config( "SGame", null );
TheNomad::Engine::UserInterface::ConfigButtonVar@ sgame_AdaptiveSoundtrack;
TheNomad::Engine::UserInterface::ConfigListVar@ sgame_GfxDetail;
TheNomad::Engine::UserInterface::ConfigListVar@ sgame_Difficulty;

//
// ConfigInit: initializes all sgame relevant configuration variables
//
void ConfigInit() {
    @sgame_AdaptiveSoundtrack =
        TheNomad::Engine::UserInterface::ConfigButtonVar( "Adaptive Soundtrack", "sgame_AdaptiveSoundtrack", true, CVAR_SAVE );
    @sgame_GfxDetail =
        TheNomad::Engine::UserInterface::ConfigListVar( "Gfx Detail", "sgame_GfxDetail", 2, { "None", "Low", "Medium", "High" }, CVAR_SAVE );
    @sgame_Difficulty =
        TheNomad::Engine::UserInterface::ConfigListVar( "Difficulty", "sgame_Difficulty", 2, {}, CVAR_SAVE );
    
    sgame_Config.AddVar( @sgame_AdaptiveSoundtrack );
    sgame_Config.AddVar( @sgame_MaxGfx );
    sgame_Config.AddVar( @sgame_Difficulty );
}

int ModuleDrawConfiguration() {
    /*
    uint tmp;
    return 0;

    ImGui::BeginTable( "##SGameConfig", 2 );

	ImGui::TableNextColumn();
	ImGui::Text( "Game Difficulty" );
	ImGui::TableNextColumn();
    if ( config_GameDifficulty != TheNomad::GameSystem::GameDifficulty::TryYourBest
    	|| TheNomad::SGame::GlobalState != TheNomad::SGame::GameState::InLevel )
    {
    } else {
    	// remember, no pussy
        ImGui::Text( "You CHOSE This, No Pullin' Out Now!" );
    }
    */
    return 1;
}

int ModuleSaveConfiguration() {
    return 1;
}