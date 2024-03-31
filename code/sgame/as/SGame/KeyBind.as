namespace TheNomad::SGame {
    class KeyBind {
		KeyBind() {
			down[0] = down[1] = 0;
			downtime = 0;
			msec = 0;
			active = false;
		}
		KeyBind() {
		}
		
		KeyBind& opAssign( const KeyBind& in other ) {
			down[0] = other.down[0];
			down[1] = other.down[1];
			downtime = other.downtime;
			msec = other.msec;
			active = other.active;
			return this;
		}

		void Down() {
			int8[] c( MAX_TOKEN_CHARS );
			int k;
			
			TheNomad::Engine::CmdArgvFixed( c, 1 );
			if ( c[0] != 0 ) {
				k = TheNomad::Util::StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == 0 ) {
				down[0] = k;
			} else if ( down[1] == 0 ) {
				down[1] = k;
			} else {
				ConsolePrint( "Three keys down for a button!\n" );
				return;
			}
			
			if ( active ) {
				return; // still down
			}
			
			// save the timestamp for partial frame summing
			TheNomad::Engine::CmdArgvFixed( c, 2 );
			downtime = TheNomad::Util::StringToInt( c );
			
			active = true;
		}
		
		void Up() {
			int8[] c( MAX_TOKEN_CHARS );
			uint uptime;
			uint k;
			
			TheNomad::Engine::CmdArgvFixed( c, 1 );
			if ( c[0] != 0 ) {
				k = TheNomad::Util::StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == k ) {
				down[0] = 0;
			} else if ( down[1] == k ) {
				down[1] = 0;
			} else {
				return; // key up without corresponding down (menu pass through)
			}
			
			active = false;
			
			// save timestamp for partial frame summing
			TheNomad::Engine::CmdArgvFixed( c, 2 );
			uptime = TheNomad::Util::StringToInt( c );
			if ( uptime > 0 ) {
				msec += uptime - downtime;
			} else {
				msec += TheNomad::GameSystem::GameManager.GetGameMsec() / 2;
			}
			
			active = false;
		}
		
		uint[] down( 2 );
		uint downtime;
		uint msec;
		bool active;
	};
};