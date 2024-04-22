#include "SGame/MapCheckpoint.as"
#include "SGame/MapSpawn.as"
#include "SGame/MapData.as"
#include "SGame/LevelRankData.as"
#include "SGame/LevelInfoData.as"
#include "SGame/LevelStats.as"
#include "SGame/LevelMapData.as"
#include "SGame/InfoSystem/InfoDataManager.as"
#include "Engine/SoundSystem/SoundEffect.as"
#include "SGame/Sprite.as"
#include "SGame/SpriteSheet.as"
#include "SGame/GfxSystem.as"

namespace TheNomad::SGame {
	// hellbreaker add-on, toggled with "sgame_hellbreaker"
    import void HellbreakerInit() from "hellbreaker";
	import bool HellbreakerRunTic() from "hellbreaker";

    enum LevelRank {
		RankS,
		RankA,
		RankB,
		RankC,
		RankD,
		RankF,
		RankWereUBotting,
		
		NumRanks
	};

    enum GameState {
		Inactive,
		InLevel,
		EndOfLevel,
		StatsMenu,
		DeathMenu,
	};

    class LevelSystem : TheNomad::GameSystem::GameObject {
		LevelSystem() {
		}
		
		void OnInit() {
			LevelInfoData@ data;
			json@ rankS, rankA, rankB, rankC, rankD, rankF, rankU;
			uint i;
			string str;
			string mapname;
			string levelName;

			ConsolePrint( "Loading level infos...\n" );

			for ( i = 0; i < sgame_ModList.Count(); i++ ) {
				LoadLevelsFromFile( sgame_ModList[i] );
			}

			@m_Current = null;

			ConsolePrint( formatUInt( NumLevels() ) + " levels parsed.\n" );

			str.reserve( MAX_STRING_CHARS );
			for ( i = 0; i < NumLevels(); i++ ) {
				json@ info = @m_LevelInfos[ i ];
				if ( !info.get( "Name", levelName ) ) {
					ConsoleWarning( "invalid level info, Name variable is missing.\n" );
					continue;
				}

				@data = LevelInfoData( levelName );

				ConsolePrint( "Loaded level '" + data.m_Name + "'...\n" );

				for ( uint j = 0; j < TheNomad::GameSystem::GameDifficulty::NumDifficulties; j++ ) {
					if ( !info.get( "MapNameDifficulty_" + formatUInt( j ), mapname ) ) {
						ConsoleWarning( "invalid level map info for \"" + levelName
							+ "\", missing value for 'MapNameDifficulty_" + j + "', skipping.\n" );
						continue;
					}

					LevelMapData MapData = LevelMapData( mapname );
					MapData.mapHandle = TheNomad::GameSystem::LoadMap( mapname );
					MapData.difficulty = TheNomad::GameSystem::GameDifficulty( j );

					if ( MapData.mapHandle == FS_INVALID_HANDLE ) {
						ConsoleWarning( "failed to load map '" + mapname + "' for level '" + data.m_Name + "'\n" );
						continue;
					}
					data.m_MapHandles.Add( MapData );
				}

				data.m_nIndex = i;

				@rankS = @info["RankS"];
				@rankA = @info["RankA"];
				@rankB = @info["RankB"];
				@rankC = @info["RankC"];
				@rankD = @info["RankD"];
				@rankF = @info["RankF"];
				@rankU = @info["RankU"];

				//
				// all rank infos must be present
				//
				if ( @rankS is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankS'.\n" );
					continue;
				}
				if ( @rankA is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankA'.\n" );
					continue;
				}
				if ( @rankB is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankB'.\n" );
					continue;
				}
				if ( @rankC is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankC'.\n" );
					continue;
				}
				if ( @rankD is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankD'.\n" );
					continue;
				}
				if ( @rankF is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankF'.\n" );
					continue;
				}
				if ( @rankU is null ) {
					ConsoleWarning( "invalid level info file, missing object 'RankU'.\n" );
					continue;
				}

				data.m_RankS.rank = LevelRank::RankS;
				if ( !LoadLevelRankData( str, "RankS", @data.m_RankS, rankS ) ) {
					continue;
				}
				data.m_RankA.rank = LevelRank::RankA;
				if ( !LoadLevelRankData( str, "RankA", @data.m_RankA, rankA ) ) {
					continue;
				}
				data.m_RankB.rank = LevelRank::RankB;
				if ( !LoadLevelRankData( str, "RankB", @data.m_RankB, rankB ) ) {
					continue;
				}
				data.m_RankC.rank = LevelRank::RankC;
				if ( !LoadLevelRankData( str, "RankC", @data.m_RankC, rankC ) ) {
					continue;
				}
				data.m_RankD.rank = LevelRank::RankD;
				if ( !LoadLevelRankData( str, "RankD", @data.m_RankD, rankD ) ) {
					continue;
				}
				data.m_RankF.rank = LevelRank::RankF;
				if ( !LoadLevelRankData( str, "RankF", @data.m_RankF, rankF ) ) {
					continue;
				}
				data.m_RankU.rank = LevelRank::RankWereUBotting;
				if ( !LoadLevelRankData( str, "RankU", @data.m_RankU, rankU ) ) {
					continue;
				}

				m_LevelInfoDatas.Add( @data );
			}
			m_LevelInfos.Clear();
		}
		void OnShutdown() {
		}
		void OnRunTic() {
			//
			// checkpoint updates
			//
			if ( EntityManager.NumEntities() == 0 && m_MapData.GetCheckpoints().Count() != 0 ) {
				if ( m_CurrentCheckpoint >= m_MapData.GetCheckpoints().Count() ) {
					GlobalState = GameState::EndOfLevel;
					return;
				}
				DebugPrint( "Setting checkpoint " + m_CurrentCheckpoint + " to completed.\n" );
				m_MapData.GetCheckpoints()[ m_CurrentCheckpoint ].m_bPassed = true;
				m_CurrentCheckpoint++;
			}

			for ( uint i = 0; i < m_MapData.GetCheckpoints().Count(); i++ ) {

			}
		}
		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		void OnLevelEnd() {
			@m_MapData = null;
			@m_Current = null;
		}
		void OnSave() const {
		}
		void OnLoad() {
		}
		const string& GetName() const {
			return "LevelSystem";
		}
		//!
		//!
		//!
		void OnLevelStart() {
			int difficulty;
			
			// get the level index
			m_nIndex = TheNomad::Engine::CvarVariableInteger( "g_levelIndex" );
			difficulty = sgame_Difficulty.GetInt();
			m_CurrentCheckpoint = 0;

			DebugPrint( "Initializing level at " + m_nIndex + ", difficulty set to \"" + SP_DIFF_STRINGS[ difficulty ] + "\".\n" );
			
			ConsolePrint( "Loading level \"" + m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].m_Name + "\"...\n" );
			
			m_RankData = LevelStats();
			@m_Current = @m_LevelInfoDatas[m_nIndex];
			@m_MapData = MapData();
			m_MapData.Init( m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].m_Name, 1 );
			m_MapData.Load( m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].mapHandle );

			switch ( TheNomad::GameSystem::GameDifficulty( difficulty ) ) {
			case TheNomad::GameSystem::GameDifficulty::VeryEasy:
				m_nDifficultyScale = 0.5f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Easy:
				m_nDifficultyScale = 1.0f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Normal:
				m_nDifficultyScale = 1.5f;
				break;
			case TheNomad::GameSystem::GameDifficulty::Hard:
				m_nDifficultyScale = 1.90f;
				break;
			case TheNomad::GameSystem::GameDifficulty::VeryHard:
				m_nDifficultyScale = 2.5f;
				break;
			case TheNomad::GameSystem::GameDifficulty::TryYourBest:
				m_nDifficultyScale = 5.0f; // ... ;)
				break;
			};
		}
		
		void AddLevelData( LevelInfoData@ levelData ) {
			m_LevelInfoDatas.push_back( levelData );
		}
		
		const LevelInfoData@ GetLevelDataByIndex( uint nIndex ) const {
			return m_LevelInfoDatas[ nIndex ];
		}
		LevelInfoData@ GetLevelDataByIndex( uint nIndex ) {
			return m_LevelInfoDatas[ nIndex ];
		}
		
		const LevelInfoData@ GetLevelInfoByMapName( const string& in mapname ) const {
			for ( uint i = 0; i < m_LevelInfoDatas.Count(); i++ ) {
				for ( uint a = 0; a < m_LevelInfoDatas[i].m_MapHandles.Count(); a++ ) {
					if ( TheNomad::Util::StrICmp( m_LevelInfoDatas[i].m_MapHandles[a].m_Name, mapname ) == 0 ) {
						return @m_LevelInfoDatas[i];
					}
				}
			}
			return null;
		}

		private array<json@>@ LoadJSonFile( const string& in modName ) {
			string path;
			array<json@> values;
			json@ data;
			
			path = "modules/" + modName + "/scripts/levels.json";
			
			@data = json();
			if ( !data.ParseFile( path ) ) {
				ConsoleWarning( "failed to load level info file '" + path + "', skipping.\n" );
				return null;
			}

			if ( !data.get( "LevelInfo", values ) ) {
				ConsoleWarning( "level info file found, but no level infos found.\n" );
				return null;
			}
			
			return @values;
		}

		private void LoadLevelsFromFile( const string& in modName ) {
			array<json@>@ levels;

			@levels = @LoadJSonFile( modName );
			if ( @levels is null ) {
				return;
			}

			ConsolePrint( "Got " + levels.Count() + " level infos from \"" + modName + "\"\n" );
			m_nLevels += levels.Count();
			for ( uint i = 0; i < levels.Count(); i++ ) {
				m_LevelInfos.Add( @levels[i] );
			}
		}

		bool LoadLevelRankData( string& in str, const string& in rankName, LevelRankData@ data, json@ src ) {
			if ( !src.get( "MinKills", data.minKills ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MinKills'.\n" );
				return false;
			}
			if ( !src.get( "MinStyle", data.minStyle ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MinStyle'.\n" );
				return false;
			}
			if ( !src.get( "MaxDeaths", data.maxDeaths ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MaxDeaths'.\n" );
				return false;
			}
			if ( !src.get( "MaxCollateral", data.maxCollateral ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MaxCollateral'.\n" );
				return false;
			}
			if ( !src.get( "RequiresClean", data.requiresClean ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'RequiresClean'.\n" );
				return false;
			}

			return true;
		}

		float GetDifficultyScale() const {
			return m_nDifficultyScale;
		}
		
		LevelStats& GetStats() {
			return m_RankData;
		}
		
		uint NumLevels() const {
			return m_nLevels;
		}
		uint LoadedIndex() const {
			return m_nIndex;
		}

		const MapData@ GetMapData() const {
			return @m_MapData;
		}
		MapData@ GetMapData() {
			return @m_MapData;
		}

		const LevelInfoData@ GetCurrentData() const {
			return @m_Current;
		}
		LevelInfoData@ GetCurrentData() {
			return @m_Current;
		}
		
		private uint m_CurrentCheckpoint;
		private array<json@> m_LevelInfos;
		private array<LevelInfoData@> m_LevelInfoDatas;
		private uint m_nIndex = 0;
		private uint m_nLevels = 0;

		private float m_nDifficultyScale = 1.0f;
		
		private LevelStats m_RankData = LevelStats();
		private LevelInfoData@ m_Current = null;
		private MapData@ m_MapData = null;
	};

	class SoundData {
		SoundData( float decibles, const vec2& in volume, const ivec2& in range, const ivec3& in origin, float diffusion ) {
		}
		
		void Run() {
		}
	};

	uint GetMapLevel( float elevation ) {
		uint e;

		e = uint( floor( elevation ) );

		return 0;
	}

	array<string> sgame_ModList;

	GameState GlobalState;

	LevelSystem@ LevelManager;

	TheNomad::Engine::SoundSystem::SoundEffect selectedSfx;
	string[] SP_DIFF_STRINGS( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
	string[] sgame_RankStrings( LevelRank::NumRanks );
	vec4[] sgame_RankStringColors( LevelRank::NumRanks );
};