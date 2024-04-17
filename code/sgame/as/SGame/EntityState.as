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
		
		NumStates
	};
	
	class EntityState {
		EntityState() {
		}

		void SetInfo( const string& in name, uint tics, uint num, uint spriteOffset ) {
			m_Name = name;
			m_nTics = tics;
			m_nStateNum = StateNum( num );
			m_nSpriteOffset = spriteOffset;
			
			m_Ticker = 0;
		}
		
		const string& GetName() const {
			return m_Name;
		}
		
		void Log() const {
			ConsolePrint( "[Entity State Report]\n" );
			ConsolePrint( "Name: " + m_Name + "\n" );
			ConsolePrint( "Tics: " + formatUInt( m_nTics ) + "\n" );
			ConsolePrint( "Id: " + formatUInt( uint( m_nStateNum ) ) + "\n" );
			ConsolePrint( "Sprite Offset: " + formatUInt( m_nSpriteOffset ) + "\n" );
		}
		StateNum GetID() const {
			return m_nStateNum;
		}
		
		uint SpriteOffset() const {
			return m_nSpriteOffset;
		}
		bool Done() const {
			return m_Ticker == m_nTics;
		}
		uint GetTics() const {
			return m_Ticker;
		}
		void Run() {
			m_Ticker++;
		}
		void Loop() {
			m_Ticker = 0;
		}
		EntityState@ Cycle() {
			return m_NextState;
		}
		
		// runtime data
		private uint m_Ticker;
		
		// static data
		private string m_Name;
		private uint m_nTics;
		private StateNum m_nStateNum;
		private uint m_nSpriteOffset;
		private EntityState@ m_NextState;
	};
};