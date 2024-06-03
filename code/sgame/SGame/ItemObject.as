namespace TheNomad::SGame {
    class ItemObject : EntityObject {
		ItemObject() {
		}
		ItemObject( uint type ) {
			m_nType = type;
		}

		void Pickup( EntityObject@ ent ) {
			switch ( ent.GetType() ) {
			case GameSystem::EntityType::Item:
			case GameSystem::EntityType::Weapon:
				GameError( "ItemObject::Pickup: invalid pickup entity (item/weapon)" );
			default:
				break;
			};

			@m_Owner = @ent;
			DebugPrint( "Item " + m_Link.m_nEntityNumber + " now owned by " + ent.GetEntityNum() + ".\n" );

			// TODO: in the future, mobs will be able to pick up items just like the player
			m_Info.pickupSfx.Play();
		}
		void Use() {
			m_Info.useSfx.Play();
			Engine::CmdExecuteCommand( m_Info.effect + " " + m_Owner.GetEntityNum() );
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
		private EntityObject@ m_Owner = null; // for applying effects
	};
};