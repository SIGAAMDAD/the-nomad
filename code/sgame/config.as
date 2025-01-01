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
	TheNomad::Engine::UserInterface::ConfigListVar@ sgame_GfxDetail =
		TheNomad::Engine::UserInterface::ConfigListVar( "Gfx Detail", "sgame_GfxDetail", { "None", "Low", "Medium", "High" }, 2 );
	
	TheNomad::Engine::UserInterface::ConfigButtonVar@ sgame_SaveLastUsedWeaponModes =
		TheNomad::Engine::UserInterface::ConfigButtonVar( "Save Last Used Weapon Mode", "sgame_SaveLastUsedWeaponModes",
			TheNomad::Engine::CvarVariableInteger( "sgame_SaveLastUsedWeaponModes" ) == 1 );

	sgame_Config.AddVar( @sgame_GfxDetail );
	sgame_Config.AddVar( @sgame_SaveLastUsedWeaponModes );
}

int ModuleDrawConfiguration() {
	ImGui::BeginTable( "##SGameConfigTable", 2 );
	sgame_Config.Draw();
	ImGui::EndTable();

	sgame_Config.Save();

	return 1;
}

int ModuleSaveConfiguration() {
	return 1;
}

};