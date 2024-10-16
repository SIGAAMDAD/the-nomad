namespace TheNomad::SGame {
	class Animation {
		Animation() {
			m_nOldTic = 0;
			m_nTicker = 1;
		}
		
		int GetFrame() const {
			return m_nCurrentFrame;
		}
		uint NumFrames() const {
			return m_nNumFrames;
		}
		uint GetTicRate() const {
			return m_nTicRate;
		}
		bool IsFlipFlop() const {
			return m_bOscillate;
		}

		void Log() const {
			ConsolePrint( "TicRate: " + m_nTicRate + "\n" );
			ConsolePrint( "Frames: " + m_nNumFrames + "\n" );
			ConsolePrint( "FlipFlop: " + m_bOscillate + "\n" );
		}
		
		void Run() {
			if ( TheNomad::GameSystem::GameManager.GetGameTic() - m_nOldTic > m_nTicRate ) {
				m_nOldTic = TheNomad::GameSystem::GameManager.GetGameTic();

				if ( m_bReverse ) {
					m_nCurrentFrame--;
					if ( m_nCurrentFrame < 0 ) {
						m_nCurrentFrame = m_nNumFrames - 1;
					}
				} else {
					m_nCurrentFrame += m_nTicker;
					if ( m_bOscillate ) {
						if ( m_nCurrentFrame >= m_nNumFrames ) {
							m_nTicker = -1;
						} else if ( m_nCurrentFrame < 0 ) {
							m_nCurrentFrame = 0;
							m_nTicker = 1;
						}
					} else {
						if ( m_nCurrentFrame >= int( m_nNumFrames ) ) {
							m_nCurrentFrame = 0;
						}
					}
				}
			}
		}
		
		bool Load( json@ json ) {
			string base;

			m_nTicRate = uint( json[ "TicRate" ] );
			m_bOscillate = bool( json[ "FlipFlop" ] );
			m_nNumFrames = uint( json[ "Frames" ] );
			m_bReverse = bool( json[ "Reverse" ] );

		/*
			if ( !json.get( "TicRate", base ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'TicRate'\n" );
				return false;
			}
			m_nTicRate = Convert().ToInt( base );
			if ( !json.get( "FlipFlop", base ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'FlipFlop'\n" );
				return false;
			}
			m_bOscillate = Convert().ToBool( base );
			if ( !json.get( "Frames", base ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'Frames'\n" );
				return false;
			}
			m_nNumFrames = Convert().ToUInt( base );
			if ( !json.get( "Reverse", base ) ) {
				ConsoleWarning( "invalid animation info, missing variable 'Reverse'\n" );
				return false;
			}
			m_bReverse = Convert().ToBool( base );
		*/

			if ( m_bReverse ) {
				m_nTicker = -1;
			}
			if ( m_nNumFrames == 0 ) {
				m_nTicker = 0;
			}

			return true;
		}

		void SetState( EntityState@ state ) {
			@m_State = @state;
		}
		EntityState@ GetState() {
			return @m_State;
		}

		private EntityState@ m_State = null;
		
		private uint m_nOldTic = 0;
		private uint m_nNumFrames = 1;
		private uint m_nTicRate = 1;
		private uint m_nTicker = 0;
		private uint m_nCurrentFrame = 0;
		private bool m_bOscillate = false;
		private bool m_bReverse = false;
	};
};