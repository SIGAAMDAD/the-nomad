namespace moblib::System {
	class AICoverNode {
		AICoverNode() {
		}
		
		uvec2 m_Position;
		TheNomad::GameSystem::DirType m_Direction;
	};
	
	class AICoverTree  {
		AICoverTree() {
		}
		
		void CreateTree() {
			const array<array<uint64>>@ tiles = @TheNomad::SGame::LevelManager.GetMapData().GetTiles();
			
			for ( uint y = 0; y < tiles.Count(); y++ ) {
				for ( uint x = 0; x < tiles[y].Count(); x++ ) {
					
				}
			}
		}
		
		private array<AICoverNode> m_Nodes;
	};
};