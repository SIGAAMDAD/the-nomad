namespace TheNomad::SGame {
    class KeyBind {
		KeyBind() {
		}
		
		KeyBind& opAssign( const KeyBind& in other ) {
			down0 = other.down0;
			down1 = other.down1;
			downtime = other.downtime;
			msec = other.msec;
			active = other.active;
			return this;
		}

		void Down() {
			uint k;
			string c;
			
			c = TheNomad::Engine::CmdArgv( 1 );
			if ( c[0] != 0 ) {
				k = Convert().ToUInt( c );
			} else {
				return;
			}
			
			if ( down0 == 0 ) {
				down0 = k;
			} else if ( down1 == 0 ) {
				down1 = k;
			} else {
				ConsolePrint( "Three keys down for a button!\n" );
				return;
			}
			
			if ( active ) {
				return; // still down
			}
			
			// save the timestamp for partial frame summing
			c = TheNomad::Engine::CmdArgv( 2 );
			downtime = Convert().ToUInt( c );
			
			active = true;
		}
		
		void Up() {
			uint uptime;
			uint k;
			string c;

			c = TheNomad::Engine::CmdArgv( 1 );
			if ( c[0] != 0 ) {
				k = Convert().ToUInt( c );
			} else {
				return;
			}
			
			if ( down0 == k ) {
				down0 = 0;
			} else if ( down1 == k ) {
				down1 = 0;
			} else {
				return; // key up without corresponding down (menu pass through)
			}
			
			active = false;
			
			// save timestamp for partial frame summing
			c = TheNomad::Engine::CmdArgv( 2 );
			uptime = Convert().ToUInt( c );
			if ( uptime > 0 ) {
				msec += uptime - downtime;
			} else {
				msec += TheNomad::GameSystem::GameManager.GetGameMsec() / 2;
			}
			
			active = false;
		}
		
		uint down0 = 0;
		uint down1 = 0;
		uint downtime = 0;
		uint msec = 0;
		bool active = false;
	};
};