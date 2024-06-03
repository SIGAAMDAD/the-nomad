namespace TheNomad::SGame {
    class LevelInfoData {
		LevelInfoData( const string& in name ) {
			m_Name = name;
		}
		
		array<LevelMapData> m_MapHandles;
		string m_Name;
		
		// music data
		string m_;

		uint m_nIndex = 0;
		LevelRankData m_RankS;
		LevelRankData m_RankA;
		LevelRankData m_RankB;
		LevelRankData m_RankC;
		LevelRankData m_RankD;
		LevelRankData m_RankF;
		LevelRankData m_RankU;
	};
};