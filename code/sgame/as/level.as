#include "sg_game.as"

namespace TheNomad::SGame {
	ConVar g_LevelIndex;
	ConVar g_MapName;
	
	shared class MapSpawn
	{
		MapSpawn( const vec3& origin, int nEntityId, SGEntityType nEntityType ) {
			m_Origin = origin;
			m_nEntityId = nEntityId;
			m_nEntityType = nEntityType;
		}
		
		vec3 m_Origin;
		int m_nEntityId;
		SGEntityType m_nEntityType;
	};
	
	shared class MapData : TheNomad::GameSystem::GameObject
	{
		MapData() {
			m_nWidth = 0;
			m_nHeight = 0;
		}
		
		void Load( const string& mapName ) {
			g_MapName.Set( mapName );
			
			TheNomad::Engine::Renderer::RE_LoadWorldMap( mapName );
		}
		
		void OnLoad() {
			
		}
		void OnSave() const {
			
		}
		
		const array<MapSpawn>& GetSpawns() const {
			return m_Spawns;
		}
		const array<vec3>& GetCheckpoints() const {
			return m_Checkpoints;
		}
		int GetWidth() const {
			return m_nWidth;
		}
		int GetHeight() const {
			return m_nHeight;
		}
		
		private array<MapSpawn> m_Spawns;
		private array<vec3> m_Checkpoints;
		private int m_nWidth;
		private int m_nHeight;
	};
	
	shared class LevelInfoData {
		LevelInfoData( const string& difficulty, LevelRank rank, int nMinKills, int nMinStyle, int nMaxCollateral, int nMinTime ) {
			m_Difficulty = difficulty;
			m_Rank = rank;
			m_nMinKills = nMinKills;
			m_nMinStyle = nMinStyle;
			m_nMaxCollateral = nMaxCollateral;
			m_nMinTime = nMinTime;
		}
		
		string m_Difficulty;
		LevelRank m_Rank;
		int m_nMinKills;
		int m_nMinStyle;
		int m_nMaxCollateral;
		int m_nMinTime;
	};
	
	shared class LevelData : TheNomad::GameSystem::GameObject
	{
		LevelData() {
		}
		
		void OnLoad() {
			
		}
		void OnSave() const {
			TheNomad::Game::Archive::BeginSaveSection( "LevelData" );
			
			TheNomad::Game::Archive::EndSaveSection();
		}
		
		//void LoadInfo( const TheNomad::Util::InfoParser& parse ) {
		//	
		//}
		
		void Start( int nIndex, const string& name ) {
			ConsolePrint( "Starting level " + name + "...\n" );
			
			if ( g_DebugPrint.GetInt() is 1 ) {
				TheNomad::ConsolePrint(
					"[Level Info] ->\n"
					"Name: " + name + "\n"
					"Index: " + nIndex + "\n"
				);
			}
			
			m_MapData.Load(  );
		}
		void End() {
			ConsolePrint( "Ending level...\n" );
		}
		
		MapData m_MapData;
		
		// level info data
		dictionary<LevelInfoData> m_RankingData;
		
		// ranking data
		GameDifficulty m_nBestDifficulty;
		int m_nBestKills;
		int m_nBestTime;
		int m_nBestCollateral;
		int m_nBestStyle;
	};
	
	ConVar@ LevelDebugPrint;
};
