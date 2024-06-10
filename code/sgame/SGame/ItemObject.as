namespace TheNomad::SGame {
    class ItemObject : EntityObject {
		ItemObject() {
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

		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
            return true;
		}

		void Think() const {
			if ( @m_Owner !is null ) {
				return;
			}
		}
		void Spawn( uint id, const vec3& in origin ) override {
			@m_Info = @InfoSystem::InfoManager.GetItemInfo( id );
			if ( @m_Info is null ) {
				GameError( "ItemObject::Spawn: invalid item id " + id );
			}

			m_Link.m_Origin = origin;
			m_Link.m_Bounds.m_nWidth = m_Info.width;
			m_Link.m_Bounds.m_nHeight = m_Info.height;
			m_Link.m_Bounds.MakeBounds( origin );
		}

		private InfoSystem::ItemInfo@ m_Info = null;
		private EntityObject@ m_Owner = null; // for applying effects
	};
};