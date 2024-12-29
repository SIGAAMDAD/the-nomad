namespace TheNomad::SGame {
    class ItemObject : EntityObject {
		ItemObject() {
		}
		~ItemObject() {
			@m_ScriptData = null;
		}

		void SetOwner( EntityObject@ ent ) {
			@m_Owner = @ent;
		}
		EntityObject@ GetOwner() {
			return @m_Owner;
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
			EmitSound( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/iteraction/pickup_item" ), 2.5f, 0xff );
		}
		void Use() {
			EmitSound( m_Info.useSfx, 2.5f, 0xff );
		}

		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			section.SaveBool( "hasOwner", m_Owner !is null );
			if ( @m_Owner !is null ) {
				section.SaveUInt( "owner", m_Owner.GetEntityNum() );
			}
		}
		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			if ( section.LoadBool( "hasOwner" ) ) {
				@m_Owner = @EntityManager.GetEntityForNum( section.LoadUInt( "owner" ) );
			}

			Spawn( m_Link.m_nEntityId, m_Link.m_Origin );

            return true;
		}

		void LinkScript( itemlib::ItemScript@ script ) {
			@m_ScriptData = @script;
			DebugPrint( "Linked item script for \"" + m_Info.name + "\" at entity '" + m_Link.m_nEntityNumber + "''\n" );
		}

		void Think() override {
			if ( @m_Owner !is null ) {
				return;
			}

			@m_State = @StateManager.GetNullState();
		}
		void Draw() override {
			if ( @m_Owner !is null || Util::Distance( EntityManager.GetActivePlayer().GetOrigin(), m_Link.m_Origin ) > 16.0f ) {
				return;
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.sheetNum = -1;
			refEntity.spriteId = m_Info.iconShader;
			refEntity.origin = m_Link.m_Origin;
			refEntity.scale = m_Info.size;
			refEntity.Draw();
		}
		void Spawn( uint id, const vec3& in origin ) override {
			@m_Info = @InfoSystem::InfoManager.GetItemInfo( id );
			if ( @m_Info is null ) {
				GameError( "ItemObject::Spawn: invalid item id " + id );
			}

			m_hShader = m_Info.iconShader;
			m_Link.m_nEntityId = id;
			m_Link.m_Origin = origin;
			m_Bounds.m_nWidth = m_Info.size.x;
			m_Bounds.m_nHeight = m_Info.size.y;
			m_Bounds.MakeBounds( origin );

			@m_State = @StateManager.GetNullState();

			m_Name = m_Info.name;

			itemlib::AllocScript( @this );
		}

		itemlib::ItemScript@ GetScript() {
			return @m_ScriptData;
		}

		const InfoSystem::ItemInfo@ GetItemInfo() const {
			return @m_Info;
		}
		InfoSystem::ItemInfo@ GetItemInfo() {
			return @m_Info;
		}

		private InfoSystem::ItemInfo@ m_Info = null;
		protected itemlib::ItemScript@ m_ScriptData = null;
		protected EntityObject@ m_Owner = null; // for applying effects
	};
};