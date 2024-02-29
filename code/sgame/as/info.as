namespace TheNomad {
	namespace SGame {
		class InfoParser {
			InfoParser( const string& fileName ) {
				if ( !Parse() ) {
					ConsolePrint( "failed to load info file " + fileName + "\n" );
				}
			}
			
			const InfoParser& GetInfo( const string& name ) const {
				return m_Infos[name];
			}
			
			private void Warning( const string& msg ) {
				ConsolePrint( COLOR_YELLOW + "[WARNING] InfoParser: " + msg + "\n" );
			}
			
			private void Error( const string& msg ) {
				ConsolePrint();
			}
			
			bool Failed() const {
				return m_bFailed;
			}
			
			bool Parse( const string& fileName ) {
				Util::InfoParser src = Util::InfoParser( fileName );
				string tok;
				
				src.ParseExt( tok, true );
				if ( tok[0] !is '{' ) {
					Error( "" );
				}
				
				while ( 1 ) {
					src.ParseExt( tok, true );
					
					if ( tok.size() is 0 ) {
						Error();
						return false;
					}
					if ( tok[0] is '}' ) {
						break;
					}
					
					switch ( tok[0] ) {
					case '{':
						ParseObject( tok, src );
						break;
					case '(':
						ParseArray( tok, src );
						break;
					};
				}
				
				return true;
			}
			
			private dictionary<InfoParser> m_Infos;
			private array<string> m_Values;
			private array<dictionary<string>> m_ArrayLists;
			private array<dictionary<dictionary>> m_HashLists;
			
			private const string SCOPE_OPEN = "{";
			private const string SCOPE_CLOSE = "}";
		};
	};
};