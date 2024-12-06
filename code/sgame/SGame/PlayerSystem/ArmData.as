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
			@m_EntityData = @base;

			@m_SpriteSheet = @TheNomad::Engine::ResourceCache.GetSpriteSheet(
				"skins/" + base.GetSkin() + "/arms_" + ( nArmIndex == 0 ? "right" : "left" ),
				sheetSize.x, sheetSize.y, spriteSize.x, spriteSize.y );
			if ( @m_SpriteSheet is null ) {
				GameError( "ArmData::Link: failed to load arms sprite sheet" );
			}

			@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_IDLE );
			if ( @m_State is null ) {
				GameError( "ArmData::Link: failed to load idle arms state" );
			}

			m_nArmIndex = nArmIndex;
			m_nFacing = FACING_RIGHT;
			m_nTicker = 0;
		}
		
		private SpriteSheet@ CalcState() {
			if ( @m_State is @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_MELEE ) ) {
				// if we're in melee state, lock in
				return @m_SpriteSheet;
			}
			
			if ( @m_Equipped !is null ) {
				// assign a weapon specific state
				@m_State = @m_Equipped.GetState();
				
				return @m_Equipped.GetSpriteSheet();
			}
			else if ( m_EntityData.GetPhysicsObject().GetVelocity() != Vec3Origin ) {
				if ( m_EntityData.IsSliding() ) {
					@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_SLIDE );
				}
				else if ( m_EntityData.IsCrouching() ) {
					// get a specific stealth state
					if ( m_EntityData.GetFacing() == m_nArmIndex ) {
						@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_STEALTH_CRAWL );
					} else {
						@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_STEALTH_READY );
					}
				}
				else {
					@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_MOVE );
				}
			}
			else {
				@m_State = @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_IDLE );
			}
			
			return @m_SpriteSheet;
		}
		void Think() {
			@m_State = @m_State.Run( m_nTicker );
		}
		void Draw() {
			TheNomad::Engine::Renderer::RenderEntity refEntity;
			
			// TODO: calculate sprite direction here
			
			SpriteSheet@ sheet = @CalcState();
			
			refEntity.origin = m_EntityData.GetOrigin();
			refEntity.sheetNum = sheet.GetShader();
			refEntity.spriteId = TheNomad::Engine::Renderer::GetSpriteId( @sheet, @m_State );
			refEntity.scale = TheNomad::Engine::Renderer::GetFacing( m_nFacing );
			refEntity.rotation = m_EntityData.GetArmAngle();
			refEntity.Draw();
		}
		
		SpriteSheet@ GetSpriteSheet() {
			return @m_SpriteSheet;
		}
		const SpriteSheet@ GetSpriteSheet() const {
			return @m_SpriteSheet;
		}
		WeaponObject@ GetEquippedWeapon() {
			return @m_Equipped;
		}
		const WeaponObject@ GetEquippedWeapon() const {
			return @m_Equipped;
		}
		InfoSystem::WeaponProperty GetMode() const {
			return m_nMode;
		}
		EntityState@ GetState() {
			return @m_State;
		}
		const EntityState@ GetState() const {
			return @m_State;
		}
		int GetFacing() const {
			return m_nFacing;
		}
		uint& GetTicker() {
			return m_nTicker;
		}
		uint GetTicker() const {
			return m_nTicker;
		}
		
		void SetEquippedWeapon( WeaponObject@ weapon ) {
			@m_Equipped = @weapon;
		}
		void SetMode( InfoSystem::WeaponProperty nMode ) {
			m_nMode = nMode;
		}
		void SetState( EntityState@ state ) {
			@m_State = @state;
		}
		void SetState( StateNum state ) {
			@m_State = @StateManager.GetStateForNum( state );
		}
		void SetFacing( int nFacing ) {
			m_nFacing = nFacing;
		}
		
		private PlayrObject@ m_EntityData = null;
		private WeaponObject@ m_Equipped = null;
		private SpriteSheet@ m_SpriteSheet = null;
		private EntityState@ m_State = null;
		private InfoSystem::WeaponProperty m_nMode = InfoSystem::WeaponProperty::None;
		private int m_nFacing = FACING_RIGHT;
		private int m_nArmIndex = 0;
		private uint m_nTicker = 0;
	};
};