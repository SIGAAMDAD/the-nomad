

namespace TheNomad {
	namespace SGame {
		class EntityInfo {
			EntityInfo() {
			}
			
			const EntityInfo& GetInfo( const string& name ) const {
				return m_Infos[name];
			}
			
			bool Parse( const string& fileName ) {
				Util::InfoParser src = Util::InfoParser( fileName );
				
				
				while ( 1 ) {
				}
				
				return true;
			}
			
			private dictionary<EntityInfo> m_Infos;
			private array<string> m_Values;
			private array<dictionary<string>> m_ArrayLists;
			private array<dictionary<dictionary>> m_HashLists;
		};
	};
};