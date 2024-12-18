#include "Engine/UserInterface/ConfigSet.as"
#include "Engine/UserInterface/ConfigVar.as"
#include "Engine/UserInterface/ConfigSliderVar.as"
#include "Engine/UserInterface/ConfigButtonVar.as"
#include "Engine/UserInterface/ConfigListVar.as"
#include "main.as"

TheNomad::Engine::UserInterface::ConfigSet sgame_Config( "SGame", null );

namespace nomadmain {

//
// ConfigInit: initializes all sgame relevant configuration variables
//
void ConfigInit() {
	TheNomad::Engine::UserInterface::ConfigButtonVar@ sgame_AdaptiveSoundtrack = null;
	
	TheNomad::Engine::UserInterface::ConfigListVar@ sgame_GfxDetail =
		TheNomad::Engine::UserInterface::ConfigListVar( "Gfx Detail", "sgame_GfxDetail", { "None", "Low", "Medium", "High" }, 2 );
	
	TheNomad::Engine::UserInterface::ConfigButtonVar@ sgame_HUD_ShowStatusBars =
		TheNomad::Engine::UserInterface::ConfigButtonVar( "[HUD] Show Status Bars", "sgame_HUD_ShowStatusBars", true );

//    @sgame_AdaptiveSoundtrack =
//        TheNomad::Engine::UserInterface::ConfigButtonVar( "Adaptive Soundtrack", "sgame_AdaptiveSoundtrack", true, CVAR_SAVE );

	sgame_Config.AddVar( @sgame_GfxDetail );
	sgame_Config.AddVar( @sgame_HUD_ShowStatusBars );
}

int ModuleDrawConfiguration() {
	sgame_Config.Draw();
	return 1;
}

int ModuleSaveConfiguration() {
	return 1;
}

};