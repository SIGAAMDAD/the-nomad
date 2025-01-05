#include "SGame/PlayrObject.as"
#include "Engine/Renderer/RenderEntity.as"

namespace TheNomad::SGame {
	//
	// ArmData:
	// you've got arms, y'know, use 'em
	// used to just be primitives in the PlayrObject but now
	// a dedicated separated object to make handling dual-wielding
	// easier
	//
	class ArmData {
		ArmData() {
		}
		
		void Link( PlayrObject@ base, int nArmIndex, const uvec2& in sheetSize, const uvec2& in spriteSize ) {
			@m_EntityData = base;

			@m_SpriteSheet = @TheNomad::Engine::ResourceCache.GetSpriteSheet( "skins/" + base.GetSkin(), 1024, 1024, 32, 32 );

			@m_State = StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_IDLE );
			if ( m_State is null ) {
				GameError( "ArmData::Link: failed to load idle arms state" );
			}

			m_nArmIndex = nArmIndex;
			Facing = FACING_RIGHT;
			m_nTicker = 0;

			const string id = ( nArmIndex == 0 ? "right" : "left" );
			@m_IdleState = StateManager.GetStateById( "player_arms_idle_" + id );
			if ( m_IdleState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_idle_" + id + "\"" );
			}

			@m_MeleeState = StateManager.GetStateById( "player_arms_melee_" + id );
			if ( m_MeleeState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_melee_" + id + "\"" );
			}

			@m_MoveState = StateManager.GetStateById( "player_arms_move_" + id );
			if ( m_MoveState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_move_" + id + "\"" );
			}

			@m_SlideState = StateManager.GetStateById( "player_arms_slide_" + id );
			if ( m_SlideState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_slide_" + id + "\"" );
			}

			@m_StealthCrawlState = StateManager.GetStateById( "player_arms_stealth_crawl_" + id );
			if ( m_StealthCrawlState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_stealth_crawl_" + id + "\"" );
			}

			@m_StealthReadyState = StateManager.GetStateById( "player_arms_stealth_ready_" + id );
			if ( m_StealthReadyState is null ) {
				GameError( "ArmData::Link: couldn't find state \"player_arms_stealth_ready_" + id + "\"" );
			}
		}
		
		private SpriteSheet@ CalcState() {
			if ( m_State is m_MeleeState ) {
				// if we're in melee state, lock in
				return m_SpriteSheet;
			}
			
			if ( m_nWeaponSlot != uint( -1 ) ) {
				// assign a weapon specific state
				WeaponObject@ weapon = m_EntityData.GetInventory().GetSlot( m_nWeaponSlot ).GetData();

				@m_State = weapon.GetState();
				
				return weapon.GetSpriteSheet();
			}
			else if ( m_EntityData.GetPhysicsObject().GetVelocity() != Vec3Origin && m_EntityData.GetOrigin().z == 0.0f ) {
				if ( ( m_EntityData.Flags & PF_SLIDING ) != 0 ) {
					@m_State = m_SlideState;
				}
				else if ( ( m_EntityData.Flags & PF_CROUCHING ) != 0 ) {
					// get a specific stealth state
					if ( m_EntityData.GetFacing() == m_nArmIndex ) {
						@m_State = m_StealthCrawlState;
					} else {
						@m_State = m_StealthReadyState;
					}
				}
				else {
					@m_State = m_MoveState;
				}
			}
			else {
				@m_State = m_IdleState;
			}
			
			return m_SpriteSheet;
		}
		void Think() {
			@m_State = m_State.Run( m_nTicker );
		}
		void Draw() {
			SpriteSheet@ sheet = CalcState();
			
			TheNomad::Engine::Renderer::RenderEntity refEntity;
			refEntity.origin = m_EntityData.GetOrigin();
			refEntity.sheetNum = sheet.GetShader();
			refEntity.spriteId = TheNomad::Engine::Renderer::GetSpriteId( sheet, m_State );
			if ( sheet !is m_SpriteSheet ) {
				// drawing a weapon, don't mess with the sprite direction
				WeaponObject@ weapon = m_EntityData.GetInventory().GetSlot( m_nWeaponSlot ).GetData();
				refEntity.scale = weapon.GetWeaponInfo().size;
			} else {
				vec2 scale = TheNomad::Engine::Renderer::GetFacing( Facing );
				if ( ( m_EntityData.Flags & PF_SLIDING ) != 0 ) {
					scale.x = -scale.x;
				}
				refEntity.scale = scale;
			}
			if ( Facing == FACING_RIGHT ) {
				refEntity.rotation = Util::RAD2DEG( -m_EntityData.GetArmAngle() );
			} else {
				refEntity.rotation = Util::RAD2DEG( m_EntityData.GetArmAngle() );
			}
			refEntity.Draw();
		}
		
		SpriteSheet@ GetSpriteSheet() {
			return m_SpriteSheet;
		}
		const SpriteSheet@ GetSpriteSheet() const {
			return m_SpriteSheet;
		}
		uint GetEquippedWeapon() const {
			return m_nWeaponSlot;
		}
		EntityState@ GetState() {
			return m_State;
		}
		const EntityState@ GetState() const {
			return m_State;
		}
		int GetFacing() const {
			return Facing;
		}
		uint& GetTicker() {
			return m_nTicker;
		}
		uint GetTicker() const {
			return m_nTicker;
		}
		
		void SetEquippedSlot( uint nSlot ) {
			m_nWeaponSlot = nSlot;
		}
		void SetState( EntityState@ state ) {
			@m_State = state;
		}
		void SetState( StateNum state ) {
			@m_State = StateManager.GetStateForNum( state );
		}
		void SetFacing( int nFacing ) {
			Facing = nFacing;
		}

		private EntityState@ m_IdleState = null;
		private EntityState@ m_MeleeState = null;
		private EntityState@ m_MoveState = null;
		private EntityState@ m_SlideState = null;
		private EntityState@ m_StealthCrawlState = null;
		private EntityState@ m_StealthReadyState = null;
		
		private PlayrObject@ m_EntityData = null;
		private SpriteSheet@ m_SpriteSheet = null;
		private EntityState@ m_State = null;
		private uint m_nWeaponSlot = uint( -1 );
		int Facing = FACING_RIGHT;
		private int m_nArmIndex = 0;
		private uint m_nTicker = 0;
	};
};