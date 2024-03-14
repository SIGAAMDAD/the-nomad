#include "game.as"
#include "convar.as"
#include "spawn.as"
#include "checkpoint.as"

namespace TheNomad::SGame {
	shared enum LevelRank {
		RankA = 0,
		RankB,
		RankC,
		RankD,
		RankF,
		RankWereUBotting,
		
		NumRanks
	};
	
	shared class MapSoundData {
		MapSoundData( float dissonance = TheNomad::Engine::CvarVariableFloat( "sgame_SoundDissonance" ) ) {
			m_bModified = false;
			m_nDissonance = dissonance;
		}
		MapSoundData() {
		}
		
		array<float>& GetData() {
			m_bModified = true;
			return m_DataBits;
		}
		const array<float>& GetData() const {
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
		private array<float> m_DataBits;
		private bool m_bModified;
	};
	
	shared class MapData {
		MapData( const string& in mapName, uint nMapLevels ) {
			m_nWidth = 0;
			m_nHeight = 0;
			m_Name = mapName;
			
			m_SoundBits.resize( nMapLevels );
			for ( uint i = 0; i < nMapLevels; i++ ) {
				m_SoundBits[i].resize( MAX_MAP_WIDTH * MAX_MAP_HEIGHT );
			}
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
		
		//
		// SaveCheckpointCache: creates a cachefile containing spawn to checkpoint bindings
		// to reduce map loading time
		//
		void SaveMapCache() const {
			TheNomad::Engine::FileSystem::OutputStream file;
			const int mapCacheIdent = (('C'<<24)+('D'<<16)+('M'<<8)+'@');
			
			if ( file.Open( "_cache/" + m_Name + "_mcd.dat" ) is false ) {
				DebugPrint( "ERROR: failed to create a checkpoint cache for '" + m_Name + "'!\n" );
				return;
			}
			
			DebugPrint( "Saving cached checkpoint data...\n" );
			
			file.WriteInt( mapCacheIdent );
			file.WriteUInt( m_Spawns.size() );
			file.WriteUInt( m_Checkpoints.size() );
			
			for ( uint i = 0; i < m_Spawns.size(); i++ ) {
				file.WriteUInt( GetCheckpointIndex( m_Spawns[i].m_Checkpoint ) );
			}
		}
		
		bool LoadCheckpointCacheCache() {
			TheNomad::Engine::FileSystem::InputStream file;
			int ident;
			uint num, index;
			const int mapCacheIdent = (('C'<<24)+('D'<<16)+('M'<<8)+'@');
			
			if ( file.Open( "_cache/" + m_Name + "_mdc.dat" ) is false ) {
				ConsolePrint( "no map data cache found.\n" );
				return false;
			}
			
			file.ReadInt( ident );
			if ( ident != mapCacheIdent ) {
				GameError( "LoadMapCache( " + m_Name + " ): found data cache file, but wrong identifier" );
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
				@m_Spawns[i].m_Checkpoint = @m_Checkpooints[index];
			}
			
			return true;
		}
		
		void Load( int hMap ) {
			uint nCheckpoints, nSpawns, nTiles;
			uint i;
			vec3 xyz;
			
			bool success = TheNomad::GameSystem::SetActiveMap( hMap, nCheckpoints, nSpawns,
				nTiles, m_SoundBits.GetData(), EntityManager.GetFirstLink() );
			
			if ( success is false ) {
				GameError( "MapData::Load: failed to set map to " + formatInt( hMap ) + "\n" );
			}
			
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
				@spawn = MapSpawn( xyz, type, id, null );
				
				m_Spawns.push_back( spawn );
			}
			
			if ( !LoadCheckpointCache() ) {
				float dist;
				MapCheckpoint@ cp;
				
				for ( i = 0; i < nSpawns; i++ ) {
					dist = Distance( m_Spawns[i].m_Origin, m_Checkpoints[0].m_Origin );
					for ( uint c = 1; c < nCheckpoints; c++ ) {
						@cp = m_Checkpoints[c];
						if ( Distance( cp.m_Origin, m_Spawns[i].m_Origin ) < dist ) {
							dist = Distance( cp.m_Origin, m_Spawns[i].m_Origin );
						}
					}
				}
			}
		}
		
		const array<MapSoundData>& GetSoundBits() const {
			return m_SoundBits;
		}
		array<MapSoundData>& GetSoundBits() {
			return m_SoundBits;
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
	
	shared class LevelStats {
		LevelStats() {
			stylePoints = 0;
			numKills = 0;
			numDeaths = 0;
			collateralScore = 0;
			isClean = true;
		}
		
		void Draw() const {
			const ImGuiWindoFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
			
			ImGui::Begin( "##LevelStatsShow", null, windowFlags );
			ImGui::TextUnformatted( "" );
			if ( TheNomad::Engine::IsAnyKeyDown() ) {
				TheNomad::Engine::SoundSystem::PlaySfx( selectedSfx );
			}
			ImGui::End();
		}
		
		uint stylePoints;
		uint numKills;
		uint numDeaths;
		uint collateralScore;
		bool isClean;
	};
	
	shared class LevelMapData {
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
	
	shared class LevelRankData {
		LevelRankData() {
			rank = 0;
			minStyle = 0;
			minKills = 0;
			maxTime = 0;
			maxDeaths = 0;
			requireClean = false;
		}
		
		LevelRank rank;
		uint minStyle;
		uint minKills;
		uint maxTime;
		uint maxDeaths;
		bool requireClean; // no warcrimes, no innocent deaths, etc. required for perfect score
	};
	
	shared class LevelInfoData {
		LevelInfoData( const string& in name ) {
			m_Name = name;
			m_MapHandles.reserve( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
		}
		
		array<LevelMapData> m_MapHandles;
		string m_Name;
		uint m_nIndex;
		LevelRankData m_RankA;
		LevelRankData m_RankB;
		LevelRankData m_RankC;
		LevelRankData m_RankD;
		LevelRankData m_RankF;
		LevelRankData m_RankWereUBotting;
	};
	
	shared class LevelSystem : TheNomad::GameSystem::GameObject {
		LevelSystem() {
			m_nLevels = 0;
			@m_Current = null;
		}
		
		void OnRunTic() override {
			for ( uint i = 0; i < m_Data.GetMap().NumLevels(); i++ ) {
				if ( !m_Data.GetMap().SoundBitsChanged() ) {
					continue; // no need for dissonance
				}
				
			}
		}
		//!
		//!
		//!
		void OnLevelStart() override {
			const LevelInfoData@ data;
			const string@ mapname;
			int difficulty;
			
			// get the level index
			m_nIndex = TheNomad::CvarVariableInteger( "g_levelIndex" );
			difficulty = TheNomad::CvarVariableInteger( "sgame_Difficulty" );
			
			@mapname = @m_LevelInfoDatas[m_nIndex].m_MapHandles[difficulty].m_Name;
			
			ConsolePrint( "Loading level \"" + mapname + "\"...\n" );
			
			if ( int( TheNomad::CvarVariableInteger( "sgame_LevelDebugPrint" ) ) == 1 ) {
				
			}
			
			m_Stats = LevelStats();
			@m_Current = @m_LevelInfoDatas[m_nIndex];
			
			@m_MapData = MapData( s );
		}
		
		const string& GetName() const {
			return "LevelManager";
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
		
		const string& GetLevelInfoByIndex( uint nIndex ) const {
			return m_LevelInfoStrings[ nIndex ];
		}
		string& GetLevelInfoByIndex( uint nIndex ) {
			return m_LevelInfoStrings[ nIndex ];
		}
		
		uint ParseInfos( const array<int8>& in buf, string& out infos ) {
			TheNomad::Engine::InfoParser parse( buf, infos );
			return infos.size();
		}
		
		void LoadLevelsFromFile( const string& in fileName ) {
			uint64			len;
			int				f;
			array<int8>		buf;
			string			infos;
			
			len	= TheNomad::Engine::FileSystem::LoadFile( fileName, f, buf );
			if ( f == FS_INVALID_HANDLE ) {
				ConsolePrint( COLOR_RED + "ERROR: file '" + fileName + "' not found!\n" );
				return;
			}
			
			infos.resize( MAX_INFO_VALUE );
			m_nLevels += ParseInfos( buf, infos );
			m_LevelInfoStrings.push_back( infos );
		}
		
		const LevelInfoData@ GetLevelInfoByMapName( const string& in mapname ) const {
			for ( uint i = 0; i < m_LevelInfoDatas.size(); i++ ) {
				for ( uint a = 0; a < m_LevelInfoDatas[i].m_MapHandles.size(); a++ ) {
					if ( TheNomad::Util::StrICmp( m_LevelInfoDatas[i].m_MapHandles[a].m_Name, mapname ) ) {
						return m_LevelInfoDatas[i];
					}
				}
			}
			return null;
		}
		
		LevelStats& GetStats() {
			return m_Stats;
		}
		
		uint NumLevels() const {
			return m_nLevels;
		}
		uint LoadedIndex() const {
			return m_nIndex;
		}
		
		private array<string> m_LevelInfoStrings;
		private array<LevelInfoData@> m_LevelInfoDatas;
		private uint m_nIndex;
		private uint m_nLevels;
		
		private ConVar@ sgame_SoundDissonance;
		private LevelStats m_RankData;
		private LevelInfoData@ m_Current;
		private MapData@ m_MapData;
	};
	
	ConVar@ sgame_cheat_DeafMobs;
	ConVar@ sgame_cheat_BlindMobs;
	ConVar@ sgame_cheat_InfiniteAmmo;
	
	ConVar@ sgame_LevelIndex;
	ConVar@ sgame_MapName;
	ConVar@ sgame_LevelDebugPrint;
	ConVar@ sgame_LevelInfoFile;
	LevelSystem@ LevelManager;
	
	shared class SoundData {
		SoundData( float decibles, const vec2& in volume, const ivec2& in range, const ivec3& in origin, float diffusion ) {
			m_Loudness = decibles;
			m_Volume = volume;
			m_Origin = origin;
			m_Diffusion = diffusion;
			m_Range = range;
			Run();
		}
		
		void Run() {
			const float noiseScaleX = m_Volume.x < 1 ? 1 : m_Volume.x;
			const float noiseScaleY = m_Volume.y < 1 ? 1 : m_Volume.y;
			const int distanceX = m_Range.x * noiseScaleX / 2;
			const int distanceY = m_Range.y * noiseScaleY / 2;
			
			const ivec2 start( m_Origin.x - distanceX, m_Origin.y - distanceY );
			const ivec2 end( m_Origin.x + distanceX, m_Origin.y + distanceY );
			
			// NOTE: may need to redesign this bc/ of shared code bullshit
			array<array<int>>& soundBits = LevelManager.GetCurrentData().GetMap().GetSoundBits();
			
			for ( int y = start.y; y != end.y; y++ ) {
				for ( int x = start.x; x != end.x; x++ ) {
					soundBits[origin.z][y * LevelManger.GetCurrentData().GetMap().GetWidth() + x] += m_Decibles;
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
	string[] SP_DIFF_STRINGS( TheNomad::GameSystem::GameDifficulty::NumDifficulties );
	
	void InitLevels() {
		LevelInfoData@ data;
		uint i;
		string mapname;
		
		ConsolePrint( "Loading level infos...\n" );
		
		@LevelManager = cast<LevelSystem>( TheNomad::GameSystem::AddSystem( LevelSystem() ) );
		
		@sgame_LevelInfoFile = TheNomad::CvarManager.AddCvar( "sgame_LevelInfoFile", "", CVAR_INIT | CVAR_ROM, false );
		if ( sgame_LevelInfoFile.GetString().size() > 0 ) {
			LevelManager.LoadLevelsFromFile( sgame_LevelInfoFile.GetString() );
		} else {
			LevelManager.LoadLevelsFromFile( "scripts/levels.txt" );
		}
		
		ConsolePrint( formatUInt( LevelManager.NumLevels() ) + " levels parsed.\n" );
		
		// set level numbers
		for ( i = 0; i < LevelManager.NumLevels(); i++ ) {
			TheNomad::Engine::SetInfoValueForKey( LevelManager.GetLevelInfoByIndex( i ), "num", formatUInt( i ) );
		}
		
		mapname.resize( MAX_NPATH );
		for ( i = 0; i < LevelManager.NumLevels(); i++ ) {
			const string& infoString = LevelManager.GetLevelInfoByIndex( i );
			
			@data = LevelInfoData( TheNomad::Engine::GetInfoValueForKey( infoString, "name" ) );
			
			ConsolePrint( "Loaded level '" + data.m_Name + "'...\n" );
			
			for ( uint j = 0; j < data.m_MapHandles.size(); j++ ) {
				TheNomad::Engine::GetSizedInfoValueForKey( mapname, infoString, "mapname_difficulty_" + formatUInt( j ) );
				if ( TheNomad::Util::StrICmp( mapname, "none" ) ) {
					// level currently doesn't support this difficulty
					continue;
				}
				
				data.m_MapHandles.push_back( LevelMapData( @mapname ) );
				data.m_MapHandles.back().mapHandle = TheNomad::GameSystem::LoadMap( mapname );
				data.m_MapHandles.back().difficulty = TheNomad::GameSystem::GameDifficulty( j );
				
				if ( data.m_MapHandles.back().mapHandle == FS_INVALID_HANDLE ) {
					ConsoleWarning( "failed to load map '" + mapname + "' for level '" + data.m_Name + "'\n" );
				}
			}
			
			data.m_nIndex = i;
			
			data.m_RankA.rank = RankA;
			data.m_RankA.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_a_minStyle" ) );
			data.m_RankA.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_a_minKills" ) );
			data.m_RankA.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_a_maxDeaths" ) );
			data.m_RankA.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_a_requiresClean" ) ) );
			
			data.m_RankB.rank = RankB;
			data.m_RankB.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_b_minStyle" ) );
			data.m_RankB.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_b_minKills" ) );
			data.m_RankB.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_b_maxDeaths" ) );
			data.m_RankB.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_b_requiresClean" ) ) );
			
			data.m_RankC.rank = RankC;
			data.m_RankC.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_c_minStyle" ) );
			data.m_RankC.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_c_minKills" ) );
			data.m_RankC.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_c_maxDeaths" ) );
			data.m_RankC.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_c_requiresClean" ) ) );
			
			data.m_RankD.rank = RankD;
			data.m_RankD.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_d_minStyle" ) );
			data.m_RankD.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_d_minKills" ) );
			data.m_RankD.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_d_maxDeaths" ) );
			data.m_RankD.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_d_requiresClean" ) ) );
			
			data.m_RankF.rank = RankF;
			data.m_RankF.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_f_minStyle" ) );
			data.m_RankF.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_f_minKills" ) );
			data.m_RankF.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_f_maxDeaths" ) );
			data.m_RankF.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_f_requiresClean" ) ) );
			
			data.m_RankWereUBotting.rank = RankWereUBotting;
			data.m_RankWereUBotting.minStyle = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_afk_minStyle" ) );
			data.m_RankWereUBotting.minKills = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_afk_minKills" ) );
			data.m_RankWereUBotting.maxDeaths = StringToUInt( TheNomad::Engine::GetInfoValueForKey( infoString, "rank_afk_maxDeaths" ) );
			data.m_RankWereUBotting.requiresClean = bool( StringToInt( TheNomad::Engine::GetInfoValueForKey( infoString,
				"rank_afk_requiresClean" ) ) );
		}
	}
};
