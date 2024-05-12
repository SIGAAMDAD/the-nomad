namespace TheNomad::SGame {
	class Animation {
		Animation() {
		}

        const string& GetName() const {
            return m_Name;
        }
		
		int GetFrame() const {
			return m_nCurrentFrame;
		}
		
		void Run() {
			if ( uint( m_nOldTic + m_nNumFrames ) > TheNomad::GameSystem::GameManager.GetGameTic() ) {
				return;
			}
			
			m_nOldTic = TheNomad::GameSystem::GameManager.GetGameTic();
			m_nCurrentFrame += m_nTicRate;
			
			if ( m_bOscillate ) {
				if ( m_nTicRate > 0 ) {
					if ( m_nCurrentFrame >= m_nTicRate ) {
						m_nTicRate = -m_nTicRate;
					}
				} else {
					if ( m_nCurrentFrame <= 0 ) {
						m_nTicRate = -m_nTicRate;
					}
				}
			} else {
				if ( m_nCurrentFrame >= m_nNumFrames ) {
					m_nCurrentFrame = 0;
				}
			}
		}
		
		bool Load( json@ json ) {
			string state;
			
			if ( !json.get( "State", state ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'State'\n" );
				return false;
			}
			if ( !json.get( "TicRate", m_nTicRate ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'TicRate'\n" );
				return false;
			}
			if ( !json.get( "FlipFlop", m_bOscillate ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'FlipFlop'\n" );
				return false;
			}
			if ( !json.get( "Frames", m_nNumFrames ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'Frames'\n" );
				return false;
			}
			
			@m_State = @StateManager.GetStateById( state );
			if ( @m_State is null ) {
				ConsoleWarning( "invalid animation info, State \"" + state + "\" isn't valid\n" );
				return false;
			}
			m_State.SetAnimation( @this );
			
			return true;
		}
		
        private string m_Name;
		private int64 m_nOldTic = 0;
		private int m_nNumFrames = 1;
		private int m_nTicRate = 35;
		private int m_nCurrentFrame = 0;
		private bool m_bOscillate = false;
		private EntityState@ m_State = null;
	};
};