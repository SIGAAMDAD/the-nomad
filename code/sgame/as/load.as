namespace TheNomad {
	namespace SGame {
		
		shared class PowerupInfoParser : EntityInfoParser {
			PowerupInfoParser() {
			}
			
			bool Parse( const TheNomad::Engine::InfoParser& in parse ) {
				if ( !parse.ParseString( "name", name ) ) {
					return false;
				}
				if ( !parse.ParseString( "effect", effect ) ) {
					return false;
				}
				if ( !parse.ParseInt( "useSfx", hUseSfx ) ) {
					return false;
				}
			}
			
			string name;
			string effect;
			int hUseSfx;
			int hPickupSfx;
			int hIconShader;
		};
		
		shared class ItemInfoParser : EntityInfoParser {
			ItemInfoParser() {
			}
			
			bool Parse( const TheNomad::Engine::InfoParser& in parse ) {
				if ( !parse.ParseString( "name", name ) ) {
					return false;
				}
				if ( !parse.ParseUInt( "flags", flags ) ) {
					return false;
				}
				if ( !parse.ParseInt( "iconShader", hIconShader ) ) {
					return false;
				}
				if ( !parse.ParseInt( "pickupSfx", hPickupSfx ) ) {
					return false;
				}
				if ( !parse.ParseInt( "useSfx", hUseSfx ) ) {
					return false;
				}
				
				return true;
			}
			
			string name;
			uint flags;
			int hIconShader;
			int hPickupSfx;
			int hUseSfx;
		};
		
		void ParseItemInfos() {
			
		}
		
		shared class MobInfoParser : EntityInfoParser {
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