namespace TheNomad::SGame {
    enum StateNum {
		ST_MOB_IDLE,
		ST_MOB_SEARCH,
		ST_MOB_CHASE,
		ST_MOB_FIGHT,
		ST_MOB_FIGHT_MELEE,
		ST_MOB_FIGHT_MISSILE,
		ST_MOB_FLEE,
		ST_MOB_DEAD,
		
		ST_PLAYR_IDLE,
		ST_PLAYR_CROUCH,
		ST_PLAYR_SLIDE,
		ST_PLAYR_MELEE,
		ST_PLAYR_COMBAT,
		ST_PLAYR_DEAD,
		ST_PLAYR_LEGS_IDLE_GROUND,
		ST_PLAYR_LEGS_ASCENDING,
		ST_PLAYR_LEGS_IDLE_AIR,
		ST_PLAYR_LEGS_FALL_AIR,
		
		NumStates
	};
	
	class EntityState {
		EntityState() {
		}

		void SetInfo( const string& in name, uint tics, uint num, uint spriteOffset ) {
			m_Name = name;
			m_nTics = tics;
			m_nStateNum = num;
			m_nSpriteOffset = spriteOffset;
			
			m_nTicker = 0;
		}
		
		const string& GetName() const {
			return m_Name;
		}
		
		void Log() const {
			ConsolePrint( "[Entity State Report]\n" );
			ConsolePrint( "Name: " + m_Name + "\n" );
			ConsolePrint( "Tics: " + formatUInt( m_nTics ) + "\n" );
			ConsolePrint( "Id: " + formatUInt( m_nStateNum ) + "\n" );
			ConsolePrint( "Sprite Offset: " + formatUInt( m_nSpriteOffset ) + "\n" );
		}
		uint GetID() const {
			return m_nStateNum;
		}

		void Reset() {
			m_nTicker = m_nTics;
		}
		
		uint SpriteOffset() const {
			return m_nSpriteOffset;
		}
		bool Done() const {
			return m_nTicker == m_nTics;
		}
		uint GetTics() const {
			return m_nTicker;
		}
		EntityState@ Run() {
			m_nTicker += TheNomad::GameSystem::GameManager.GetDeltaTics();
			if ( m_nTicker >= m_nTics ) {
				return @m_NextState !is null ? @m_NextState : @StateManager.GetStateForNum( m_nStateNum + 1 );
			}
			m_Animation.Run();
			return @this;
		}
		EntityState@ Cycle() {
			return m_NextState;
		}
		void SetAnimation( Animation@ anim ) {
			@m_Animation = @anim;
		}
		
		// runtime data
		private uint m_nTicker = 0;
		
		// static data
		private string m_Name;
		private uint m_nTics = 0;
		private uint m_nStateNum = 0;
		private uint m_nSpriteOffset = 0;
		private EntityState@ m_NextState = null;
		private Animation@ m_Animation = null;
	};
};