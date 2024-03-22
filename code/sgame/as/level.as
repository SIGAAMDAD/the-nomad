#include "game.as"
#include "convar.as"
#include "spawn.as"
#include "checkpoint.as"
#include "sfx.as"

namespace TheNomad::SGame {
	import void HellbreakerInit() from "hellbreaker";

	shared enum GameState {
		Inactive,
		InLevel,
		StatsMenu,
		DeathMenu,
	};

	shared enum LevelRank {
		RankS = 0,
		RankA,
		RankB,
		RankC,
		RankD,
		RankF,
		RankWereUBotting,
		
		NumRanks
	};
	
	class MapSoundData {
		MapSoundData( float dissonance ) {
			m_bModified = false;
			m_nDissonance = dissonance;
//			m_DataBits.resize( MAX_MAP_WIDTH * MAX_MAP_HEIGHT );
		}
		MapSoundData() {
		}

		float[] GetCacheData() {
			return m_DataBits;
		}
		float[] GetData() {
			m_bModified = true;
			return m_DataBits;
		}
		const float[] GetData() const {
			return m_DataBits;
		}
		bool Changed() const {
			return m_bModified;
		}
		void Reset() {
			uint modifiedCount = 0;
			
			if ( !m_bModified ) {
				return;
			}
			
			for ( uint i = 0; i < m_DataBits.size(); i++ ) {
				if ( m_DataBits[i] <= 0.0f ) {
					modifiedCount++;
					continue;
				}
				m_DataBits[i] -= m_nDissonance;
				if ( m_DataBits[i] < 0.0f ) {
					m_DataBits[i] = 0.0f;
				}
			}
			
			m_bModified = modifiedCount < m_DataBits.size();
		}
		
		private float m_nDissonance;
		private float[] m_DataBits( MAX_MAP_WIDTH * MAX_MAP_HEIGHT );
		private bool m_bModified;
	};
	
	class MapData {
		MapData() {
		}

		void Init( const string& in mapName, uint nMapLevels ) {
			m_nWidth = 0;
			m_nHeight = 0;
			m_Name = mapName;

			m_SoundBits.resize( nMapLevels );
		}
		
		private uint GetCheckpointIndex( const MapCheckpoint@ cp ) const {
			for ( uint i = 0; i < m_Checkpoints.size(); i++ ) {
				if ( @cp == @m_Checkpoints[i] ) {
					return i;
				}
			}
			GameError( "GetCheckpointIndex: invalid checkpoint" );
			return 0;
		}
		
		/*
		//
		// SaveCheckpointCache: creates a cachefile containing spawn to checkpoint bindings
		// to reduce map loading time
		//
		void SaveMapCache() const {
			TheNomad::Engine::FileSystem::OutputStream file;
			const uint mapCacheMagic = 0xffa53dfe;
			
			if ( file.Open( "_cache/" + m_Name + "_mcd.dat" ) == false ) {
				DebugPrint( "ERROR: failed to create a checkpoint cache for '" + m_Name + "'!\n" );
				return;
			}
			
			DebugPrint( "Saving cached checkpoint data...\n" );
			
			file.WriteUInt( mapCacheMagic );
			file.WriteUInt( m_Spawns.size() );
			file.WriteUInt( m_Checkpoints.size() );
			
			for ( uint i = 0; i < m_Spawns.size(); i++ ) {
				file.WriteUInt( GetCheckpointIndex( m_Spawns[i].m_Checkpoint ) );
			}
		}
		
		bool LoadCheckpointCache() {
			TheNomad::Engine::FileSystem::InputStream file;
			uint magic;
			uint num, index;
			const uint mapCacheMagic = 0xffa53dfe;
			
			if ( file.Open( "_cache/" + m_Name + "_mdc.dat" ) == false ) {
				ConsolePrint( "no map data cache found.\n" );
				return false;
			}
			
			file.ReadUInt( magic );
			if ( magic != mapCacheMagic ) {
				ConsoleWarning( "LoadCheckpointCache( " + m_Name + " ): found data cache file, but wrong magic! refusing to load.\n" );
				return false;
			}
			
			file.ReadUInt( num );
			if ( num != m_Spawns.size() ) {
				ConsolePrint( "spawn count in cache file isn't the same as the one loaded, outdated? refusing to load.\n" );
				return false;
			}
			file.ReadUInt( num );
			if ( num != m_Checkpoints.size() ) {
				ConsolePrint( "checkpoint count in cache file isn't the same as the one loaded, outdated? refusing to load.\n" );
				return false;
			}
			
			for ( uint i = 0; i < m_Spawns.size(); i++ ) {
				file.ReadUInt( index );
				if ( index >= m_Checkpoints.size() ) {
					ConsolePrint( "checkpoint index in data cache file isn't valid, corruption? refusing to load.\n" );
					return false;
				}
				@m_Spawns[i].m_Checkpoint = @m_Checkpoints[index];
			}
			
			return true;
		}
		*/

		void Load( int hMap ) {
			uint nCheckpoints, nSpawns, nTiles;
			uint i;
			uvec3 xyz;
			
			TheNomad::GameSystem::SetActiveMap( hMap, nCheckpoints, nSpawns,
				nTiles, m_SoundBits[0].GetCacheData(), EntityManager.GetEntities()[0].GetLink() );

			// only supporting 1 level rn
			m_TileData.resize( 1 );
			m_TileData[0].resize( nTiles );
			TheNomad::GameSystem::GetTileData( m_TileData[0] );

			m_Checkpoints.reserve( nCheckpoints );
			
			//
			// load the checkpoints
			//
			for ( i = 0; i < nCheckpoints; i++ ) {
				MapCheckpoint@ cp;
				
				TheNomad::GameSystem::GetCheckpointData( xyz, i );
				@cp = MapCheckpoint( xyz );
				
				m_Checkpoints.push_back( cp );
			}
			
			//
			// load in spawns
			//
			for ( i = 0; i < nSpawns; i++ ) {
				MapSpawn@ spawn;
				uint id, type;
				
				TheNomad::GameSystem::GetSpawnData( xyz, type, id, i );
				@spawn = MapSpawn( xyz, id, TheNomad::GameSystem::EntityType( type ) );
				@spawn.m_Checkpoint = null;
				
				m_Spawns.push_back( spawn );
			}
			
//			if ( !LoadCheckpointCache() ) {
				// generate the indexes manually

				float dist;
				MapCheckpoint@ cp;
				
				for ( i = 0; i < nSpawns; i++ ) {
					dist = TheNomad::Util::Distance( m_Spawns[i].m_Origin, m_Checkpoints[0].m_Origin );
					for ( uint c = 1; c < nCheckpoints; c++ ) {
						@cp = @m_Checkpoints[c];
						if ( TheNomad::Util::Distance( cp.m_Origin, m_Spawns[i].m_Origin ) < dist ) {
							dist = TheNomad::Util::Distance( cp.m_Origin, m_Spawns[i].m_Origin );
						}
					}

					if ( cp is null ) {
						DebugPrint( "spawn without a checkpoint, discarding.\n" );
						continue;
					}
					cp.AddSpawn( @m_Spawns[i] );
				}
//			}
		}
		
		const array<MapSoundData>& GetSoundBits() const {
			return m_SoundBits;
		}
		array<MapSoundData>& GetSoundBits() {
			return m_SoundBits;
		}
		const array<uint>& GetTiles() const {
			return m_TileData[0];
		}
		array<uint>& GetTiles() {
			return m_TileData[0];
		}

		bool SoundBitsChanged() const {
			for ( uint i = 0; i < m_SoundBits.size(); i++ ) {
				if ( m_SoundBits[i].Changed() ) {
					return true;
				}
			}
			return false;
		}
		
		uint NumLevels() const {
			return m_TileData.size();
		}

		const array<MapSpawn@>& GetSpawns() const {
			return m_Spawns;
		}
		const array<MapCheckpoint@>& GetCheckpoints() const {
			return m_Checkpoints;
		}
		int GetWidth() const {
			return m_nWidth;
		}
		int GetHeight() const {
			return m_nHeight;
		}
		
		private string m_Name;
		private array<MapSpawn@> m_Spawns;
		private array<MapCheckpoint@> m_Checkpoints;
		private array<array<uint>> m_TileData;
		private array<MapSoundData> m_SoundBits;
		private int m_nWidth;
		private int m_nHeight;
	};
	
	class LevelStats {
		LevelStats() {
			stylePoints = 0;
			numKills = 0;
			numDeaths = 0;
			collateralScore = 0;
			isClean = true;
			drawString.reserve( MAX_STRING_CHARS );
			time.Run();
		}
		LevelStats() {
		}
		
		void Draw( bool endOfLevel ) {
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove | ImGuiWindowFlags::NoResize );

			if ( !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::Key_Tab ) ) {
				return;
			}
			
			ImGui::Begin( "##LevelStatsShow", null, windowFlags );
			if ( endOfLevel ) {
				if ( TheNomad::Engine::IsAnyKeyDown() ) {
					selectedSfx.Play();
				}
			}
			
			ImGui::BeginTable( "##LevelStatsNumbers", 2 );
			
			//
			// time
			//
			ImGui::TableNextColumn();
			ImGui::Text( "TIME:" );
			ImGui::TableNextColumn();
			
			drawString = time.ElapsedMinutes();
			drawString += ":";
			drawString += time.ElapsedSeconds();
			drawString += ".";
			drawString += time.ElapsedMilliseconds();
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ time_Rank ], sgame_RankStrings[ time_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// kills
			//
			ImGui::TableNextColumn();
			ImGui::Text( "KILLS:" );
			ImGui::TableNextColumn();
			drawString = numKills;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ kills_Rank ], sgame_RankStrings[ kills_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// deaths
			//
			ImGui::TableNextColumn();
			ImGui::Text( "DEATHS:" );
			ImGui::TableNextColumn();
			drawString = numDeaths;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ deaths_Rank ], sgame_RankStrings[ deaths_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// style
			//
			ImGui::TableNextColumn();
			ImGui::Text( "STYLE:" );
			ImGui::TableNextColumn();
			drawString = stylePoints;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ style_Rank ], sgame_RankStrings[ style_Rank ] );
			
			ImGui::EndTable();
			
			ImGui::End();
		}
		
		private string drawString;
		TheNomad::Engine::Timer time;
		uint stylePoints;
		uint numKills;
		uint numDeaths;
		uint collateralScore;
		bool isClean;

		LevelRank time_Rank;
		LevelRank style_Rank;
		LevelRank kills_Rank;
		LevelRank deaths_Rank;
	};
	
	class LevelMapData {
		LevelMapData() {
		}
		LevelMapData( const string& in name ) {
			m_Name = name;
		}
		
		LevelStats highStats;
		
		string m_Name;
		TheNomad::GameSystem::GameDifficulty difficulty;
		int mapHandle;
	};
	
	class LevelRankData {
		LevelRankData() {
			rank = LevelRank::RankS;
			minStyle = 0;
			minKills = 0;
			minTime = 0;
			maxDeaths = 0;
			requiresClean = false;
		}
		
		LevelRank rank;
		uint minStyle;
		uint minKills;
		uint minTime;
		uint maxDeaths;
		uint maxCollateral;
		bool requiresClean; // no warcrimes, no innocent deaths, etc. required for perfect score
	};
	
	class LevelInfoData {
		LevelInfoData( const string& in name ) {
			m_Name = name;
			m_MapHandles.reserve( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
		}
		
		array<LevelMapData> m_MapHandles;
		string m_Name;
		uint m_nIndex;
		LevelRankData m_RankS;
		LevelRankData m_RankA;
		LevelRankData m_RankB;
		LevelRankData m_RankC;
		LevelRankData m_RankD;
		LevelRankData m_RankF;
		LevelRankData m_RankU;
	};
	
	class LevelSystem : TheNomad::GameSystem::GameObject {
		LevelSystem() {
			LevelInfoData@ data;
			json@ rankS, rankA, rankB, rankC, rankD, rankF, rankU;
			uint i;
			string str;
			string mapname;
			string levelName;

			ConsolePrint( "Loading level infos...\n" );

			for ( i = 0; i < sgame_ModList.size(); i++ ) {
				LoadLevelsFromFile( sgame_ModList[i] );
			}

			m_nLevels = 0;
			@m_Current = null;

			ConsolePrint( formatUInt( NumLevels() ) + " levels parsed.\n" );

			// set level numbers
			for ( i = 0; i < NumLevels(); i++ ) {
				GetLevelInfoByIndex( i )["num"] = i;
			}

			str.reserve( MAX_STRING_CHARS );
			for ( i = 0; i < NumLevels(); i++ ) {
				json@ info = @GetLevelInfoByIndex( i );
				if ( !info.get( "Name", levelName ) ) {
					ConsoleWarning( "invalid level info, Name variable is missing.\n" );
					continue;
				}

				@data = LevelInfoData( levelName );

				ConsolePrint( "Loaded level '" + data.m_Name + "'...\n" );

				for ( uint j = 0; j < data.m_MapHandles.size(); j++ ) {
					if ( !info.get( "MapNameDifficulty_" + formatUInt( j ), mapname ) ) {
						// level currently doesn't support this difficulty
						continue;
					}
					if ( mapname == "none" ) {
						// level currently doesn't support this difficulty
						continue;
					}

					LevelMapData MapData = LevelMapData( mapname );
					MapData.mapHandle = TheNomad::GameSystem::LoadMap( mapname );
					MapData.difficulty = TheNomad::GameSystem::GameDifficulty( j );

					if ( MapData.mapHandle == FS_INVALID_HANDLE ) {
						ConsoleWarning( "failed to load map '" + mapname + "' for level '" + data.m_Name + "'\n" );
						continue;
					}
					data.m_MapHandles.push_back( MapData );
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
			}
		}
		
		void OnRunTic() {
			for ( uint i = 0; i < m_MapData.NumLevels(); i++ ) {
				if ( !m_MapData.SoundBitsChanged() ) {
					continue; // no need for dissonance
				}
				
			}
		}
		void OnConsoleCommand() {
		}
		void OnLevelEnd() {
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
			
			ConsolePrint( "Loading level \"" + m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].m_Name + "\"...\n" );
			
			if ( sgame_LevelDebugPrint.GetInt() == 1 ) {
				
			}
			
			m_RankData = LevelStats();
			@m_Current = @m_LevelInfoDatas[m_nIndex];
			@m_MapData = MapData();
			m_MapData.Init( m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].m_Name, 1 );

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

		const json@ GetLevelInfoByIndex( uint nIndex ) const {
			return m_LevelInfos[ nIndex ];
		}
		json@ GetLevelInfoByIndex( uint nIndex ) {
			return m_LevelInfos[ nIndex ];
		}
		
		const LevelInfoData@ GetLevelDataByIndex( uint nIndex ) const {
			return m_LevelInfoDatas[ nIndex ];
		}
		LevelInfoData@ GetLevelDataByIndex( uint nIndex ) {
			return m_LevelInfoDatas[ nIndex ];
		}
		
		const LevelInfoData@ GetLevelInfoByMapName( const string& in mapname ) const {
			for ( uint i = 0; i < m_LevelInfoDatas.size(); i++ ) {
				for ( uint a = 0; a < m_LevelInfoDatas[i].m_MapHandles.size(); a++ ) {
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
			
			path.reserve( MAX_NPATH );
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

		void LoadLevelsFromFile( const string& in modName ) {
			array<json@>@ levels;

			@levels = @LoadJSonFile( modName );
			if ( @levels is null ) {
				return;
			}

			m_LevelInfos.reserve( levels.size() );
			for ( uint i = 0; i < levels.size(); i++ ) {
				m_LevelInfos.push_back( @levels[i] );
				m_nLevels++;
			}
		}

		bool LoadLevelRankData( string& in str, const string& in rankName, LevelRankData@ data, json@ json ) {
			if ( !json.get( "MinKills", data.minKills ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MinKills'.\n" );
				return false;
			}
			if ( !json.get( "MinStyle", data.minStyle ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MinStyle'.\n" );
				return false;
			}
			if ( !json.get( "MaxDeaths", data.maxDeaths ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MaxDeaths'.\n" );
				return false;
			}
			if ( !json.get( "MaxCollateral", data.maxCollateral ) ) {
				ConsoleWarning( "invalid level info object '" + rankName + "', no variable 'MaxCollateral'.\n" );
				return false;
			}
			if ( !json.get( "RequiresClean", data.requiresClean ) ) {
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
		
		private array<json@> m_LevelInfos;
		private array<LevelInfoData@> m_LevelInfoDatas;
		private uint m_nIndex;
		private uint m_nLevels;

		private float m_nDifficultyScale;
		
		private LevelStats m_RankData;
		private LevelInfoData@ m_Current;
		private MapData@ m_MapData;
	};
	
	class SoundData {
		SoundData( float decibles, const vec2& in volume, const ivec2& in range, const ivec3& in origin, float diffusion ) {
			m_Decibles = decibles;
			m_Volume = volume;
			m_Origin = origin;
			m_Diffusion = diffusion;
			m_Range = range;
			Run();
		}
		
		void Run() {
			const float noiseScaleX = m_Volume.x < 1 ? 1 : m_Volume.x;
			const float noiseScaleY = m_Volume.y < 1 ? 1 : m_Volume.y;
			const int distanceX = m_Range.x * int( floor( noiseScaleX ) ) / 2;
			const int distanceY = m_Range.y * int( floor( noiseScaleY ) ) / 2;
			
			const ivec2 start( m_Origin.x - distanceX, m_Origin.y - distanceY );
			const ivec2 end( m_Origin.x + distanceX, m_Origin.y + distanceY );
			
			float[]@ soundBits = LevelManager.GetMapData().GetSoundBits()[m_Origin.z].GetData();
			
			for ( int y = start.y; y != end.y; y++ ) {
				for ( int x = start.x; x != end.x; x++ ) {
					soundBits[y * LevelManager.GetMapData().GetWidth() + x] += m_Decibles;
					m_Decibles -= m_Diffusion;
				}
			}
		}
		
		private ivec3 m_Origin;
		private vec2 m_Volume;
		private ivec2 m_Range;
		private float m_Diffusion;
		private float m_Decibles;
	};

	array<string> sgame_ModList;

	ConVar@ sgame_GfxDetail;
	ConVar@ sgame_QuickShotMaxTargets;
	ConVar@ sgame_QuickShotTime;
	ConVar@ sgame_QuickShotMaxRange;
	ConVar@ sgame_MaxPlayerWeapons;
	ConVar@ sgame_PlayerHealBase;
	ConVar@ sgame_Difficulty;
	ConVar@ sgame_SaveName;
	ConVar@ sgame_AdaptiveSoundtrack;
	ConVar@ sgame_DebugMode;
	ConVar@ sgame_cheat_BlindMobs;
	ConVar@ sgame_cheat_DeafMobs;
	ConVar@ sgame_cheat_InfiniteAmmo;
	ConVar@ sgame_LevelDebugPrint;
	ConVar@ sgame_LevelIndex;
	ConVar@ sgame_LevelInfoFile;
	ConVar@ sgame_SoundDissonance;
	ConVar@ sgame_MapName;
	ConVar@ sgame_MaxEntities;
	ConVar@ sgame_NoRespawningMobs;
	ConVar@ sgame_HellbreakerActive;
	ConVar@ sgame_HellbreakerOn;
	ConVar@ sgame_MaxFps;

	const array<TheNomad::GameSystem::DirType> InverseDirs = {
		TheNomad::GameSystem::DirType::South,
		TheNomad::GameSystem::DirType::SouthWest,
		TheNomad::GameSystem::DirType::West,
		TheNomad::GameSystem::DirType::NorthWest,
		TheNomad::GameSystem::DirType::North,
		TheNomad::GameSystem::DirType::NorthEast,
		TheNomad::GameSystem::DirType::East,
		TheNomad::GameSystem::DirType::SouthEast,
	};

	GameState GlobalState;

	LevelSystem@ LevelManager;

	TheNomad::Engine::SoundSystem::SoundEffect selectedSfx;
	string[] SP_DIFF_STRINGS( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
	string[] sgame_RankStrings( TheNomad::SGame::LevelRank::NumRanks );
	vec4[] sgame_RankStringColors( TheNomad::SGame::LevelRank::NumRanks );
};