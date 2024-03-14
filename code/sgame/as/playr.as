#include "entity.as"
#include "level.as"
#include "game.as"

namespace TheNomad::SGame {
	// player specific cvars
	ConVar@ sgame_QuickShotMaxTargets;
	ConVar@ sgame_QuickShotTime;
	ConVar@ sgame_QuickShotMaxRange;
	ConVar@ sgame_MaxPlayerWeapons;
	
	shared class Enchantment {
		Enchantment() {
		
		}
		
		//
		// CheckActivated: checks if the boon has been awakened yet
		//
		void CheckActivated() {
			
		}
		
		private bool m_bActive;
		private bool m_bAwakened;
		
	};
	
	void Enchantment_AddDamage_f() {
	
	}
	
	shared class QuickShot {
		QuickShot() {
			m_Targets.resize( uint( TheNomad::Engine::CvarVariableInteger( "sgame_QuickShotMaxTargets" ) ) );
			m_nTimeBetweenAcquire = TheNomad::Engine::CvarVariableInteger( "sgame_QuickShotTime" );
			m_nMaxRange = TheNomad::Engine::CvarVariableFloat( "sgame_QuickShotMaxRange" );
		}
		
		void Think() {
			if ( m_nLastTargetTime < m_nTimeBetweenAcquire ) {
				m_nLastTargetTime++;
				return;
			}
			
			DebugPrint( "QuickShot thinking...\n" );
			m_nLastTargetTime = 0;
			
			// NOTE: this might be a little bit slow depending on how many mobs are in the area
			for ( uint i = 0; i < EntityManager.NumEntities(); i++ ) {
				if ( m_Targets.find( EntityManager.GetEntity( i ) ) == -1 ) {
					// make sure we aren't adding a duplicate
					m_Targets[m_nTargetsFound] = EntityManager.GetEntity( i );
					DebugPrint( "QuickShot added entity " + formatUInt( i ) + "\n" );
					m_nTargetsFound = ;
				}
			}
		}
		
		void Clear() {
			DebugPrint( "QuickShot cleared.\n" );
			m_nLastTargetTime = 0;
			m_nTargetsFound = 0;
		}
		
		private array<EntityObject@> m_Targets;
		private uint m_nTargetsFound;
		private uint m_nTimeBetweenAcquire;
		private uint m_nLastTargetTime;
		private float m_nMaxRange;
	};
	
	shared class KeyBind {
		KeyBind() {
			down[0] = down[1] = 0;
			downtime = 0;
			msec = 0;
			active = false;
		}
		
		void Down() {
			int8[] c( MAX_TOKEN_CHARS );
			int k;
			
			TheNomad::Engine::CmdArgv( c, 1 );
			if ( c[0] != 0 ) {
				k = StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == 0 ) {
				down[0] = k;
			} else if ( down[1] == 0 ) {
				down[1] = k;
			} else {
				ConsolePrint( "Three keys down for a button!\n" );
				return;
			}
			
			if ( active ) {
				return; // still down
			}
			
			// save the timestamp for partial frame summing
			TheNomad::Engine::CmdArgv( c, 2 );
			downtime = StringToInt( c );
			
			active = true;
		}
		
		void Up() {
			int8[] c( MAX_TOKEN_CHARS );
			uint uptime;
			int k;
			
			TheNomad::Engine::CmdArgv( c, 1 );
			if ( c[0] != 0 ) {
				k = StringToInt( c );
			} else {
				return;
			}
			
			if ( down[0] == k ) {
				down[0] = 0;
			} else if ( down[1] == k ) {
				down[1] = 0;
			} else {
				return; // key up without corresponding down (menu pass through)
			}
			
			active = false;
			
			// save timestamp for partial frame summing
			TheNomad::Engine::CmdArgv( c, 2 );
			uptime = StringToInt( c );
			if ( uptime > 0 ) {
				msec += uptime - downtime;
			} else {
				msec += TheNomad::GameSystem::GameManager.FrameMsec() / 2;
			}
			
			active = false;
		}
		
		uint[] down( 2 );
		uint downtime;
		uint msec;
		bool active;
	};
	
	shared class PlayrObject : EntityObject, EntityData {
		PlayrObject() {
			m_WeaponSlots.resize( uint( TheNomad::Engine::CvarVariableInteger( "sgame_MaxPlayerWeapons" ) ) );
		}
		
		//
		// controls
		//
		void MoveNorth_Up_f() { key_MoveNorth.Up(); }
		void MoveNorth_Down_f() { key_MoveNorth.Down(); }
		void MoveSouth_Up_f() { key_MoveSouth.Up(); }
		void MoveSouth_Down_f() { key_MoveSouth.Down(); }
		void Jump_Down_f() { key_Jump.Down(); }
		void Jump_Up_f() { key_Jump.Up(); }
		
		void Quickshot_Down_f() {
			m_Flags |= PF_QUICKSHOT;
			m_QuickShot.Clear();
			
			TheNomad::Engine::SoundSystem::PlaySfx( quickshotBeginSfx );
		}
		
		void Quickshot_Up_f() {
			// TODO: perhaps add a special animation for putting the guns away?
			if ( m_QuickShot.Empty() ) {
				TheNomad::Engine::SoundSystem::PlaySfx( quickshotEndEmptySfx );
				return;
			}
			
			m_QuickShot.Activate();
			m_Flags &= ~PF_QUICKSHOT;
		}
		
		void Melee_Down_f() {
			m_nParryBoxWidth = 0.0f;
			SetState( StateNum::ST_PLAYR_MELEE );
		}
		
		void NextWeapon_f() {
			m_CurrentWeapon++;
			if ( m_CurrentWeapon >= m_WeaponSlots.size() ) {
				m_CurrentWeapon = 0;
			}
		}
		void PrevWeapon_f() {
			m_CurrentWeapon--;
			if ( m_CurrentWeapon <= 0 ) {
				m_CurrentWeapon = m_WeaponSlots.size() - 1;
			}
		}
		void SetWeapon_f() {
			
		}
		void Emote_f() {
			m_bEmoting = true;
		}
		
		
		void Damage( uint nAmount ) override {
			if ( m_bEmoting ) {
				return; // god has blessed thy soul...
			}
			
			
		}
		
		uint GetFlags() const {
			return m_Flags;
		}
		
		void Think() override {
			if ( m_Flags & PF_PARRY ) {
				ParryThink();
			} else if ( m_Flags & PF_QUICKSHOT ) {
				m_QuickShot.Think();
			}
			
			switch ( m_State.GetID() ) {
			case StateNum::ST_PLAYR_IDLE:
				IdleThink();
				break; // NOTE: maybe let the player move in combat? (that would require more sprites for the dawgs)
			case StateNum::ST_PLAYR_COMBAT:
				CombatThink();
				break;
			};
		}
		
		void CheckParry( EntityObject@ ent ) {
			if ( TheNomad::Util::BoundsIntersect( ent->GetBounds(), m_ParryBox ) ) {
				switch ( ent->GetType() ) {
				case TheNomad::GameSystem::EntityType::Projectile:
					// simply invert the direction and double the speed
					ent->SetVelocity( ent->GetVelocity() * 2 );
					ent->SetDirection( TheNomad::GameSystem::GameManager.InverseDirs[ ent->GetDirection() ] );
					break;
				case TheNomad::GameSystem::EntityType::Mob:
					if ( ent->GetFlags() & EntityFlags::Dead ) {
						ent->SetDirection( TheNomad::GameSystem::GameManager.InverseDirs[ ent->GetDirection() ] );
						ent->();
					} else { // wtf?
						DebugPrint( "Parry checker triggered on not a flying corpse...\n" );
					}
					break;
				};
			} else if ( ent->GetType() == TheNomad::GameSystem::Mob ) {
				// just a normal counter
				MobObject@ mob = cast<MobObject>( ent->GetData() );
				
				if ( !mob->CurrentAttack().IsParryable() ) {
					// unblockable, deal damage
					EntityManager.DamageEntity( ent, m_Entity );
				} else {
					
				}
			}
		}
		
		private void MakeSound() {
			if ( m_State.GetID() == StateNum::ST_PLAYR_CROUCH ) {
				return;
			}
			
			SoundData ;
			
			const float noiseScaleX = m_Velocity.x;
			if ( noiseScaleX < 1 ) {
				noiseScaleX = 1;
			}
			const float noiseScaleY = m_Velocity.y;
			if ( noiseScaleY < 1 ) {
				noiseScaleY = 1;
			}
			
			const uint distanceX = 2 * noiseScaleX / 2;
			const uint distanceY = 2 * noiseScaleY / 2;
			
			const ivec2 start( floor( m_Origin.x ) - distanceX, floor( m_Origin.y ) - distanceY );
			const ivec2 end( floor( m_Origin.x ) + distanceX, floor( m_Origin.y ) + distanceY );
			
			for ( int y = start.y; y != end.y; y++ ) {
				for ( int x = start.x; x != end.x; x++ ) {
					
				}
			}
		}
		
		private void IdleThink() {
			
		}
		private void CombatThink() {
			if ( m_Flags & key_Melee.active ) {
				// check for a parry
				
			}
		}
		private void ParryThink() {
			m_ParryBox.m_nWidth = 2.5f + m_nParryBoxWidth;
			m_ParryBox.m_nHeight = 1.0f;
			m_ParryBox.MakeBounds( vec3( m_Origin.x, m_Origin.y + ( m_Bounds.m_nWidth / 2.0f ) ) );
			
			m_nParryBoxWidth += 0.5f;
		}
		
		const uint PF_QUICKSHOT  = 0x00000001;
		const uint PF_DOUBLEJUMP = 0x00000002;
		const uint PF_PARRY      = 0x00000004;
		
		EntityObject@ m_Entity;
		
		private KeyBind key_MoveNorth, key_MoveSouth, key_MoveEast, key_MoveWest;
		private KeyBind key_Jump, key_Melee;
		
		private TheNomad::GameSystem::BBox m_ParryBox;
		private float m_nParryBoxWidth;
		
		private array<Weapon@> m_WeaponSlots;
		private QuickShot m_QuickShot;
		private int m_CurrentWeapon;
		private uint m_Flags;
		
		private bool m_bEmoting;
	};
};