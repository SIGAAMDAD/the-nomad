namespace TheNomad::SGame {
	class EntityState {
		EntityState() {
		}

		void SetInfo( const string& in name, uint tics, uint num, const uvec2& in spriteOffset ) {
			m_Name = name;
			m_nTics = tics;
			m_nStateNum = num;
			m_SpriteOffset = spriteOffset;
			
			m_nTicker = 0;
		}
		
		const string& GetName() const {
			return m_Name;
		}
		
		void Log() const {
			ConsolePrint( "[Entity State Report]\n" );
			ConsolePrint( "Name: " + m_Name + "\n" );
			ConsolePrint( "Tics: " + m_nTics + "\n" );
			ConsolePrint( "Id: " + m_nStateNum + m_nStateOffset + "\n" );
			ConsolePrint( "Sprite Offset: [" + m_SpriteOffset.x + ", " + m_SpriteOffset.y + "]\n" );
		}

		bool Load( json@ data ) {
			array<json@> values;
			string base;

			if ( !data.get( "Id", m_Name ) ) {
				ConsoleWarning( "invalid state info, missing variable 'Id'\n" );
				return false;
			}
			if ( !data.get( "SpriteOffset", values ) ) {
				ConsoleWarning( "invalid state info, missing variable 'SpriteOffset'\n" );
				return false;
			} else {
				if ( values.Count() != 2 ) {
					GameError( "invalid state info, variable 'SpriteOffset' is not a uvec2" );
				}
				m_SpriteOffset.x = uint( values[0] );
				m_SpriteOffset.y = uint( values[1] );
			}
			if ( !data.get( "Tics", m_nTics ) ) {
				ConsoleWarning( "invalid state info, missing variable 'Tics'\n" );
				return false;
			}
			if ( !data.get( "Entity", base ) ) {
				ConsoleWarning( "invalid state info, missing variable 'Entity'\n" );
				return false;
			} else {
				if ( Util::StrICmp( base, "player" ) == 0 ) {
					m_nStateOffset = 0;
				} else if ( !InfoSystem::InfoManager.GetMobTypes().TryGetValue( base, m_nStateOffset ) ) {
					GameError( "invalid state info, Entity \"" + base + "\" doesn't exist" );
				}
			}
			if ( data.get( "NextState", base ) ) {
				if ( Util::StrICmp( base, "this" ) == 0 ) {
					// this is a flip-flop state
					@m_NextState = @this;
				} else {
					@m_NextState = @StateManager.GetStateById( base );
					if ( @m_NextState is null ) {
						GameError( "invalid state info, variable NextState \"" + base + "\" isn't a valid state" );
					}
				}
			}
			if ( !data.get( "BaseNum", base ) ) {
				ConsoleWarning( "invalid state info, missing variable 'BaseNum'\n" );
				return false;
			} else {
				if ( !StateManager.GetBaseStateCache().TryGetValue( base, m_nStateNum ) ) {
					GameError( "invalid state info, variable BaseNum \"" + base + "\" isn't a valid state" );
				}
			}

			if ( @m_NextState is null ) {
				@m_NextState = @StateManager.GetStateForNum( ( m_nStateNum + m_nStateOffset ) + 1 );
			}
			if ( @m_NextState is null ) {
				ConsoleWarning( "EntityState::Load: next state is null\n" );
			}

			ConsolePrint( "State \"" + m_Name + "\" registered with ID " + m_nStateNum + " and offset of " + m_nStateOffset +
				", with next state \"" + m_NextState.m_Name + "\" with " + m_nTics + " tics\n" );

			return true;
		}

		uint GetID() const {
			return m_nStateNum + m_nStateOffset;
		}

		void Reset() {
			m_nTicker = m_nTics;
		}
		
		const uvec2& GetSpriteOffset() const {
			return m_SpriteOffset;
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
				return @m_NextState !is null ? @m_NextState : @StateManager.GetStateForNum( ( m_nStateNum + m_nStateOffset ) + 1 );
			}
			m_Animation.Run();
			return @this;
		}
		EntityState@ Cycle() {
			return m_NextState;
		}
		Animation@ GetAnimation() {
			return @m_Animation;
		}
		void SetAnimation( Animation@ anim ) {
			@m_Animation = @anim;
		}
		
		// static data
		private string m_Name;
		private uvec2 m_SpriteOffset = uvec2( 0 );
		private EntityState@ m_NextState = null;
		private Animation@ m_Animation = null;
		private uint m_nTics = 0;
		private uint m_nStateNum = 0;
		private uint m_nStateOffset = 0;

		// runtime data
		private uint m_nTicker = 0;
	};
};