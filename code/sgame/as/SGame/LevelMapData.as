namespace TheNomad::SGame {
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
};