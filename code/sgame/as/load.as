namespace TheNomad {
	namespace SGame {
		interface EnityInfoParser {
			bool Parse( const TheNomad::Engine::InfoParser& in parse );
		};
		
		class MobInfoParser : EntityInfoParser {
			MobInfoParser() {
			}
			
			bool Parse( const TheNomad::Engine::InfoParser& in parse ) {
				if ( !parse.ParseString( "name", name ) ) {
					return false;
				}
				
				return true;
			}
			
			string name;
			int health;
			uint flags;
			
			int idleStateTics;
			
			int chaseStateTics;
		};
		
		void ParseMobInfos() {
			
		}
	};
};