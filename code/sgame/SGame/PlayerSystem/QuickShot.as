namespace TheNomad::SGame {
    class LockShot {
		LockShot() {
		}
		LockShot( const vec3& in origin ) {
			m_Targets.Resize( sgame_LockShotMaxTargets.GetInt() );
		}
		
		void Think() {
			if ( m_nLastTargetTime < uint( sgame_LockShotTime.GetInt() ) ) {
				m_nLastTargetTime++;
				return;
			}
			
			DebugPrint( "LockShot thinking...\n" );
			m_nLastTargetTime = 0;

			// NOTE: this might be a little bit slow depending on how many mobs are in the area
			EntityManager.ForEachEntity( function( ref@ thisObject, EntityObject@ ent ) {
				LockShot@ this = cast<LockShot>( @thisObject );
				if ( this.m_Targets.Find( @ent ) == -1 ) {
					if ( TheNomad::Util::Distance( ent.GetOrigin(), this.m_Origin ) > sgame_LockShotMaxRange.GetFloat() ) {
						return; // too far away
					}
					// make sure we aren't adding a duplicate
					if ( this.m_nTargetCount >= this.m_Targets.Count() ) {
						this.m_nTargetCount = 0;
					}
					@this.m_Targets[ this.m_nTargetCount++ ] = @ent;
					DebugPrint( "LockShot added entity\n" );
				}
			}, @this );
		}
		
		void Clear() {
			DebugPrint( "LockShot cleared.\n" );
			m_nLastTargetTime = 0;
			m_nTargetCount = 0;
			m_Targets.Clear();
		}

		vec3 m_Origin = vec3( 0.0f );
		array<EntityObject@> m_Targets;
		uint m_nTargetCount = 0;
		uint m_nLastTargetTime = 0;
	};
};