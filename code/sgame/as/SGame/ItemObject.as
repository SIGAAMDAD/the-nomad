namespace TheNomad::SGame {
    class ItemObject : EntityObject {
		ItemObject() {
		}
		ItemObject( uint type ) {
			m_nType = type;
		}
		
		uint GetItemType() const {
			return m_nType;
		}
		void SetItemType( uint id ) {
			m_nType = id;
		}

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
            return true;
		}

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin ) override {
			@m_InfoData = cast<InfoSystem::InfoLoader>( @InfoSystem::InfoManager.GetItemInfo( id ) );
			m_nType = id;
		}

		private uint m_nType = 0;
		private InfoSystem::ItemInfo@ m_Info = null;
	};
};