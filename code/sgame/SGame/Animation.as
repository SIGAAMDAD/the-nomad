namespace TheNomad::SGame {
	class Animation {
		Animation() {
			m_nOldTic = 0;
			m_nTicker = 1;
			m_bOscillate = false;
			m_bReverse = false;
			m_nTicRate = 0;
			m_nCurrentFrame = 0;
		}
		Animation( uint nTicRate, bool bOscillate, uint nFrames, bool bReverse ) {
			Load( nTicRate, bOscillate, nFrames, bReverse );
		}

		const Animation& opAssign( const Animation& in other ) {
			m_nTicRate = other.m_nTicRate;
			m_bOscillate = other.m_bOscillate;
			m_nNumFrames = other.m_nNumFrames;
			m_bReverse = other.m_bReverse;
			return this;
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
		
		bool NextFrame() const {
			return ( TheNomad::GameSystem::GameTic - m_nOldTic ) * TheNomad::GameSystem::DeltaTic > m_nTicRate;
		}
		
		void Run() {
			if ( !NextFrame() ) {
				return;
			}

			m_nOldTic = TheNomad::GameSystem::GameTic;

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
					if ( m_nCurrentFrame >= m_nNumFrames ) {
						m_nCurrentFrame = 0;
					}
				}
			}
		}

		void Load( uint nTicRate, bool bOscillate, uint nFrames, bool bReverse ) {
			m_nTicRate = nTicRate;
			m_bOscillate = bOscillate;
			m_nNumFrames = nFrames;
			m_bReverse = bReverse;
		}
		
		bool Load( json@ json ) {
			m_nTicRate = uint( json[ "TicRate" ] );
			m_bOscillate = bool( json[ "FlipFlop" ] );
			m_nNumFrames = uint( json[ "Frames" ] );
			m_bReverse = bool( json[ "Reverse" ] );

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
		private int m_nTicker = 0;
		private uint m_nCurrentFrame = 0;
		private bool m_bOscillate = false;
		private bool m_bReverse = false;
	};
};