//
// InitResources: caches all important SGame resources
//
void InitResources() {
	string str;

	ConsolePrint( "Initializing SGame Resources...\n" );

	TheNomad::Engine::Timer timer;

	timer.Start();

	TheNomad::SGame::InfoSystem::InfoManager.LoadMobInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadItemInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadAmmoInfos();
	TheNomad::SGame::InfoSystem::InfoManager.LoadWeaponInfos();

	str = TheNomad::Engine::CvarVariableString( "skin" );
	TheNomad::Engine::ResourceCache.GetShader( "sprites/players/" + str + "_torso" );
	TheNomad::Engine::ResourceCache.GetShader( "sprites/players/" + str + "_legs" );
	TheNomad::Engine::ResourceCache.GetShader( "sprites/players/" + str + "_arms" );

	// NOTE: always load the sprite sheets after the info sprites
	// doing it there causes a weird bug where the sprite's texture doesn't
	// render correctly
	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_torso", 512, 512, 32, 32 );
	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_legs", 512, 512, 32, 32 );
	TheNomad::Engine::Renderer::RegisterSpriteSheet( "sprites/players/" + str + "_arms", 512, 512, 32, 32 );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/misc/passCheckpoint.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/mobs/detect.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/mobs/detectMeme.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/death1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/death2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/death3.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/pain2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/slide1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/dash.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel3.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal0.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal1.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal2.ogg" );
	TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal3.ogg" );

	TheNomad::Engine::ResourceCache.GetSfx( "music/levels/darkofthenight_combat.ogg" );

	//
	// register strings
	//
	{
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryEasy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_EASY", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Easy] );
		TheNomad::GameSystem::GetString( "SP_DIFF_MEDIUM", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Normal] );
		TheNomad::GameSystem::GetString( "SP_DIFF_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::Hard] );
		TheNomad::GameSystem::GetString( "SP_DIFF_VERY_HARD", TheNomad::SGame::SP_DIFF_STRINGS[TheNomad::GameSystem::GameDifficulty::VeryHard] );

		TheNomad::GameSystem::GetString( "SP_RANK_S_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankS] );
		TheNomad::GameSystem::GetString( "SP_RANK_A_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankA] );
		TheNomad::GameSystem::GetString( "SP_RANK_B_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankB] );
		TheNomad::GameSystem::GetString( "SP_RANK_C_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankC] );
		TheNomad::GameSystem::GetString( "SP_RANK_D_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankD] );
		TheNomad::GameSystem::GetString( "SP_RANK_F_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankF] );
		TheNomad::GameSystem::GetString( "SP_RANK_U_TITLE", TheNomad::SGame::sgame_RankStrings[TheNomad::SGame::LevelRank::RankWereUBotting] );

		TheNomad::GameSystem::GetString( "SP_RANK_S_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankS ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_A_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankA ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_B_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankB ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_C_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankC ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_D_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankD ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_F_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankF ] = TheNomad::Util::StringToColor( str );
		TheNomad::GameSystem::GetString( "SP_RANK_U_COLOR", str );
		TheNomad::SGame::sgame_RankStringColors[ TheNomad::SGame::LevelRank::RankWereUBotting ] = TheNomad::Util::StringToColor( str );
	}

	timer.Stop();
	ConsolePrint( "InitResources: " + timer.ElapsedMilliseconds() + "ms\n" );
}
