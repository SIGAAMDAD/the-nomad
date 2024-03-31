namespace TheNomad::SGame {
    class QuickShot {
		QuickShot() {
		}
		QuickShot( const vec3& in origin ) {
			m_Targets.Reserve( sgame_QuickShotMaxTargets.GetInt() );
		}
		
		void Think() {
			if ( m_nLastTargetTime < uint( sgame_QuickShotTime.GetInt() ) ) {
				m_nLastTargetTime++;
				return;
			}
			
			DebugPrint( "QuickShot thinking...\n" );
			m_nLastTargetTime = 0;
			
			array<EntityObject@>@ EntList = @EntityManager.GetEntities();

			// NOTE: this might be a little bit slow depending on how many mobs are in the area
			for ( uint i = 0; i < EntList.Count(); i++ ) {
				if ( m_Targets.Find( @EntList[i] ) == -1 ) {
					if ( TheNomad::Util::Distance( EntList[i].GetOrigin(), m_Origin ) > sgame_QuickShotMaxRange.GetFloat() ) {
						continue; // too far away
					}
					// make sure we aren't adding a duplicate
					m_Targets.Add( @EntList[i] );
					DebugPrint( "QuickShot added entity " + formatUInt( i ) + "\n" );
				}
			}
		}
		
		void Clear() {
			DebugPrint( "QuickShot cleared.\n" );
			m_nLastTargetTime = 0;
			m_Targets.Clear();
		}

		private vec3 m_Origin;
		private array<EntityObject@> m_Targets;
		private uint m_nLastTargetTime;
	};
};