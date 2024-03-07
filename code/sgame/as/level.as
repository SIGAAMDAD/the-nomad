#include "game.as"

namespace TheNomad {
	namespace SGame {
		string[] SP_DIFF_STRINGS( TheNomad::GameSystem::NumDifficulties );

		ConVar@ sgame_LevelIndex;
		ConVar@ sgame_MapName;
		ConVar@ sgame_LevelDebugPrint;
		ConVar@ sgame_SaveName;
		ConVar@ sgame_Difficulty;
		ConVar@ sgame_DebugMode;

		int selectedSfx;
		
		shared class MapSpawnData {
			MapSpawnData( const vec3& in origin, int nEntityId, TheNomad::GameSystem::EntityType nEntityType ) {
				m_Origin = origin;
				m_nEntityId = nEntityId;
				m_nEntityType = nEntityType;
			}
			MapSpawnData() {
			}
			
//			MapSpawn@ m_Base;
			vec3 m_Origin;
			int m_nEntityId;
			TheNomad::GameSystem::EntityType m_nEntityType;
		};
		
		shared class MapData : TheNomad::GameSystem::GameObject
		{
			MapData() {
				m_nWidth = 0;
				m_nHeight = 0;
			}
			
			void Load( const string& in mapName ) {
				TheNomad::Engine::CvarSet( "sgame_MapName", mapName );
				
//				TheNomad::Engine::Renderer::RE_LoadWorldMap( mapName );
			}
			
			void OnLoad() {
				
			}
			void OnSave() const {
				
			}
			
			const array<MapSpawnData@>& GetSpawns() const {
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
			
			private array<MapSpawnData@> m_Spawns;
			private array<vec3> m_Checkpoints;
			private int m_nWidth;
			private int m_nHeight;
		};
		
		shared class LevelInfoData {
			LevelInfoData( TheNomad::GameSystem::GameDifficulty difficulty, int nMinKills, int nMinStyle, int nMaxCollateral, int nMinTime ) {
				m_Difficulty = difficulty;
//				m_Rank = rank;
				m_nMinKills = nMinKills;
				m_nMinStyle = nMinStyle;
				m_nMaxCollateral = nMaxCollateral;
				m_nMinTime = nMinTime;
			}
			
			TheNomad::GameSystem::GameDifficulty m_Difficulty;
//			LevelRank m_Rank;
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
				TheNomad::GameSystem::BeginSaveSection( GetName() );
				
//				TheNomad::GameSystem::SaveInt( "BestRank", m_nBestRank );
				TheNomad::GameSystem::SaveInt( "BestDifficulty", m_nBestDifficulty );
				TheNomad::GameSystem::SaveInt( "MostKills", m_nBestKills );
				TheNomad::GameSystem::SaveInt( "MaxColleteral", m_nBestCollateral );
				TheNomad::GameSystem::SaveInt( "BestTime", m_nBestTime );
				
				TheNomad::GameSystem::EndSaveSection();
			}
			const string& GetName() const {
				return "LevelData";
			}
			
			void LoadInfo( ) {
				
			}
			
			void Start( int nIndex, const string& in name ) {
				ConsolePrint( "Starting level " + name + "...\n" );
				
//				if ( g_DebugPrint.GetInt() is 1 ) {
//					ConsolePrint(
//						"[Level Info] ->\n"
//						"Name: " + name + "\n"
//						"Index: " + nIndex + "\n"
//					);
//				}
				
//				m_MapData.Load(  );
			}
			void End() {
				ConsolePrint( "Ending level...\n" );
			}
			
			MapData m_MapData;
			
			// level info data
//			array<LevelInfoData> m_RankingData;
			
			// ranking data
			string m_Name;
			
//			LevelRank m_nBestRank;
			TheNomad::GameSystem::GameDifficulty m_nBestDifficulty;
			int m_nBestKills;
			int m_nBestTime;
			int m_nBestCollateral;
			int m_nBestStyle;
		};
	};
};
