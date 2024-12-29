namespace TheNomad::SGame {
    class KeyBind {
		KeyBind() {
		}
		
		KeyBind& opAssign( const KeyBind& in other ) {
			downtime = other.downtime;
			msec = other.msec;
			active = other.active;
			return this;
		}

		void Down() {
			if ( active ) {
				return; // still down
			}

			// save the timestamp for partial frame summing
			downtime = Convert().ToUInt( TheNomad::Engine::CmdArgv( 2 ) );
			
			active = true;
		}
		
		void Up() {
			if ( TheNomad::Engine::CmdArgv( 1 ).Length() < 0 ) {
				return;
			}

			// save timestamp for partial frame summing
			const uint uptime = Convert().ToUInt( TheNomad::Engine::CmdArgv( 2 ) );
			if ( uptime > 0 ) {
				msec += uptime - downtime;
			} else {
				msec += TheNomad::GameSystem::GameTic * 0.5f;
			}
			active = false;
		}
		
		uint downtime = 0;
		uint msec = 0;
		bool active = false;
	};
};