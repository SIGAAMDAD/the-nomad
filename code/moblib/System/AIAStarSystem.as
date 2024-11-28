namespace moblib::System {
	class AIAStarNode {
		AIAStarNode() {
		}
		
	};
	
	class AIAStarNodeLinkedList {
		AIAStarNodeLinkedList() {
		}
		
		AIAStarNode@ m_OpenList = null;
		AIAStarNode@ m_ClosedList = null;
	};
	
	class AIAStarNavMesh {
		AIAStarNodeLinkedList@ FindPath( const vec3& in goal, const vec3& in origin ) {
			AIAStarNodeLinkedList@ path = null;
			
			path = AIAStarNodeLinkedList();
			
			path.
		}
		
		array<array<AIAStarNode@>> m_NodeTree;
	};
	
	AIAStarNavMesh@ NavMesh = null;
};