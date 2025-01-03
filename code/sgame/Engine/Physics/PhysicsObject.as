#include "Engine/Physics/Bounds.as"

namespace TheNomad::Engine::Physics {
	enum WaterType {
		None,
		Water,
		Lava,
		Acid
	};

	const float MAX_JUMP_HEIGHT = 4.5f;

    class PhysicsObject {
        PhysicsObject() {
        }

        void Init( TheNomad::SGame::EntityObject@ ent ) {
            @m_EntityData = @ent;
        }
        void Shutdown() {
        }

        float GetAngle() const {
            return m_nAngle;
        }
		float GetWeight() const {
			return m_nWeight;
		}
		void SetWeight( float nWeight ) {
			m_nWeight = nWeight;
		}
        void SetAngle( float nAngle ) {
            m_nAngle = nAngle;
        }
        void SetAcceleration( const vec3& in accel ) {
            m_Acceleration = accel;
        }
        void SetVelocity( const vec3& in vel ) {
            m_Velocity = vel;
        }
        void SetWaterLevel( int nWaterLevel ) {
        	m_nWaterLevel = nWaterLevel;
        }
        vec3& GetAcceleration() {
            return m_Acceleration;
        }
        const vec3& GetAcceleration() const {
            return m_Acceleration;
        }
        vec3& GetVelocity() {
            return m_Velocity;
        }
        const vec3& GetVelocity() const {
            return m_Velocity;
        }
        int GetWaterLevel() const {
            return m_nWaterLevel;
        }

        private void ClipBounds() {
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_NoClip" ) == 1 ) {
				return;
			}

			vec3 origin = m_EntityData.GetOrigin();

			/*
            if ( origin.x < 0.0f ) {
                m_EntityData.SetOrigin( vec3( 0.0f, origin.y, origin.z ) );
            }
            if ( origin.y < 0.0f ) {
                m_EntityData.SetOrigin( vec3( origin.x, 0.0f, origin.z ) );
		    }
			if ( origin.x > TheNomad::SGame::MapWidth - 1 ) {
				m_EntityData.SetOrigin( vec3( TheNomad::SGame::MapWidth - 1, origin.y, origin.z ) );
			}
			if ( origin.y > TheNomad::SGame::MapHeight - 1 ) {
				m_EntityData.SetOrigin( vec3( origin.x, TheNomad::SGame::MapHeight - 1, origin.z ) );
			}
			*/
		}
		
		void ApplyFriction() {
			float frictionConstant = TheNomad::Engine::CvarVariableFloat( "sgame_Friction" );
			if ( m_EntityData.GetOrigin().z > 0.0f ) {
				frictionConstant = TheNomad::Engine::CvarVariableFloat( "sgame_AirFriction" );
			}
			if ( m_nWaterLevel > 0 ) {
				frictionConstant = TheNomad::Engine::CvarVariableFloat( "sgame_WaterFriction" );
			}

			const float friction = frictionConstant * TheNomad::GameSystem::DeltaTic;

			if ( m_Velocity.x > 0.0f ) {
				m_Velocity.x -= friction;
			} else if ( m_Velocity.x < 0.0f ) {
				m_Velocity.x += friction;
			}
			if ( m_Velocity.y > 0.0f ) {
				m_Velocity.y -= friction;
			} else if ( m_Velocity.y < 0.0f ) {
				m_Velocity.y += friction;
			}
			if ( m_Velocity.z < 0.0f && m_EntityData.GetOrigin().z <= 0.0f ) {
				m_Velocity.z = 0.0f;
			} else if ( m_EntityData.GetOrigin().z >= MAX_JUMP_HEIGHT ) {
				m_Velocity.z = TheNomad::Util::Clamp(
					m_Velocity.z - ( TheNomad::Engine::CvarVariableFloat( "sgame_Gravity" ) * TheNomad::GameSystem::DeltaTic ),
					-TheNomad::Engine::CvarVariableFloat( "sgame_Gravity" ), m_Velocity.z );
			}
		}
		
		void SetWaterLevel() {			
			//
			// get waterlevel, accounting for ducking
			//
			m_nWaterLevel = 0;
			m_nWaterType = WaterType::None;
			
			const vec3 point = m_EntityData.GetOrigin();
			TheNomad::Engine::Physics::Bounds bounds = m_EntityData.GetBounds();
			uint64 tile = TheNomad::SGame::GetTile( point, bounds );
			
			if ( ( tile & SURFACEPARM_WATER ) != 0 || ( tile & SURFACEPARM_LAVA ) != 0 ) {
				m_nWaterType = WaterType( tile );
				m_nWaterLevel = 1; // just walking in a puddle
				
				// check the level below us
				bounds.m_nMins.z--;
				bounds.m_nMaxs.z--;
				tile = TheNomad::SGame::GetTile( point, bounds );
				if ( ( tile & SURFACEPARM_WATER ) != 0 || ( tile & SURFACEPARM_LAVA ) != 0 ) {
					m_nWaterLevel = 2; // swimming now
					
					// check the level above us
					bounds.m_nMins.z += 2;
					bounds.m_nMaxs.z += 2;
					tile = TheNomad::SGame::GetTile( point, bounds );
					if ( ( tile & SURFACEPARM_WATER ) != 0 || ( tile & SURFACEPARM_LAVA ) != 0 ) {
						m_nWaterLevel = 3; // fully submerged
					}
				}
			}
        }

		private TheNomad::GameSystem::DirType CalcMoveDir() const {
			if ( m_Velocity.x < 0.0f ) {
				if ( m_Velocity.y < 0.0f ) {
					return TheNomad::GameSystem::DirType::NorthWest;
				} else if ( m_Velocity.y > 0.0f ) {
					return TheNomad::GameSystem::DirType::SouthWest;
				} else {
					return TheNomad::GameSystem::DirType::West;
				}
			} else if ( m_Velocity.x > 0.0f ) {
				if ( m_Velocity.y < 0.0f ) {
					return TheNomad::GameSystem::DirType::NorthEast;
				} else if ( m_Velocity.y > 0.0f ) {
					return TheNomad::GameSystem::DirType::SouthEast;
				} else {
					return TheNomad::GameSystem::DirType::East;
				}
			} else if ( m_Velocity.y < 0.0f ) {
				if ( m_Velocity.x < 0.0f ) {
					return TheNomad::GameSystem::DirType::NorthWest;
				} else if ( m_Velocity.x > 0.0f ) {
					return TheNomad::GameSystem::DirType::NorthEast;
				} else {
					return TheNomad::GameSystem::DirType::North;
				}
			}  else if ( m_Velocity.y > 0.0f ) {
				if ( m_Velocity.x < 0.0f ) {
					return TheNomad::GameSystem::DirType::SouthWest;
				} else if ( m_Velocity.x > 0.0f ) {
					return TheNomad::GameSystem::DirType::SouthEast;
				} else {
					return TheNomad::GameSystem::DirType::South;
				}
			}
			return TheNomad::GameSystem::DirType::Inside;
		}
		
		void OnRunTic() {
		#if _NOMAD_DEBUG
			ProfileBlock block( "PhysicsObject::OnRunTic" );
		#endif
			
			// clip it
			ClipBounds();
			SetWaterLevel();
			
			vec3 origin = m_EntityData.GetOrigin();
			bool inAir = false;
			
			// calculate velocity
			m_Velocity.x = m_Acceleration.x;
			m_Velocity.y = m_Acceleration.y;
			m_Velocity.z += m_Acceleration.z;
			if ( origin.z > 0.0f ) {
				inAir = true;
			}
			
			ApplyFriction();

			vec3 tmp = origin;
			const TheNomad::GameSystem::DirType dir = CalcMoveDir();
			switch ( dir ) {
			case TheNomad::GameSystem::DirType::North:
				tmp.y -= m_EntityData.GetHalfHeight();
				break;
			case TheNomad::GameSystem::DirType::NorthEast:
				tmp.y -= m_EntityData.GetHalfHeight();
				tmp.x += m_EntityData.GetHalfWidth();
				break;
			case TheNomad::GameSystem::DirType::East:
				tmp.x += m_EntityData.GetHalfWidth();
				break;
			case TheNomad::GameSystem::DirType::SouthEast:
				tmp.y += m_EntityData.GetHalfHeight();
				tmp.x += m_EntityData.GetHalfWidth();
				break;
			case TheNomad::GameSystem::DirType::South:
				tmp.y += m_EntityData.GetHalfHeight();
				break;
			case TheNomad::GameSystem::DirType::SouthWest:
				tmp.y += m_EntityData.GetHalfHeight();
				tmp.x -= m_EntityData.GetHalfWidth();
				break;
			case TheNomad::GameSystem::DirType::West:
				tmp.x -= m_EntityData.GetHalfWidth();
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
				tmp.y -= m_EntityData.GetHalfHeight();
				tmp.x -= m_EntityData.GetHalfWidth();
				break;
			};

			TheNomad::Engine::Physics::Bounds bounds;
			bounds.m_nWidth = m_EntityData.GetBounds().m_nWidth;
			bounds.m_nHeight = m_EntityData.GetBounds().m_nHeight;
			bounds.MakeBounds( tmp );

			TheNomad::SGame::EntityObject@ active = @TheNomad::SGame::EntityManager.GetActiveEnts();
			TheNomad::SGame::EntityObject@ ent = null;
			for ( @ent = @active.m_Next; @ent !is @active; @ent = @ent.m_Next ) {
				if ( bounds.IntersectsBounds( ent.GetBounds() ) && @m_EntityData !is @ent ) {
					if ( ent.GetType() == TheNomad::GameSystem::EntityType::Weapon || ent.GetType() == TheNomad::GameSystem::EntityType::Item ) {
						m_EntityData.PickupItem( @ent );
						break;
					}
					else if ( ent.GetType() == TheNomad::GameSystem::EntityType::Wall ) {
						if ( m_EntityData.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
							cast<TheNomad::SGame::PlayrObject@>( @m_EntityData ).PassCheckpoint( @ent );
						}
						break;
					}
					m_Velocity = 0.0f;
					m_Acceleration = 0.0f;
					if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob &&
						( m_Velocity.x > 2.25f && m_Velocity.y > 2.25f ) )
					{
						// damage
						TheNomad::SGame::EntityManager.DamageEntity( @ent, @m_EntityData );
					}
					return; // clip
				}
			}
			
			if ( !( ( tmp.x < 0.0f || tmp.x >= TheNomad::SGame::MapWidth ) || ( tmp.y < 0.0f || tmp.y >= TheNomad::SGame::MapHeight ) ) ) {
				if ( TheNomad::GameSystem::CheckWallHit( tmp, dir ) ) {
					m_Acceleration = vec3( 0.0f );

					const float z = m_Velocity.z;
					m_Velocity = 0.0f;
					if ( origin.z > 0.0f ) {
						m_Velocity.z = z;
						origin.z += ( m_Velocity.z * TheNomad::GameSystem::DeltaTic );
					}
					return;
				}
			}
			
			origin.x += ( m_Velocity.x * TheNomad::GameSystem::DeltaTic );
			origin.y += ( m_Velocity.y * TheNomad::GameSystem::DeltaTic );
			origin.z += ( m_Velocity.z * TheNomad::GameSystem::DeltaTic );
			// apply gravity
			if ( origin.z < 0.0f ) {
				origin.z = 0.0f;
			}
			m_EntityData.SetOrigin( origin );

			if ( inAir && origin.z <= 0.0f ) {
				if ( m_nWaterLevel > 0 ) {
					m_EntityData.EmitSound(
						TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/water_land_" + ( Util::PRandom() & 2 ) ),
						10.0f, 0xff );
					
					TheNomad::SGame::GfxManager.AddWaterWake( origin, 800 );
				} else {
					m_EntityData.EmitSound(
						TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/land_" + ( ( Util::PRandom() & 3 ) + 1 ) ),
						10.0f, 0xff );

					//
					// add a little dust effect for that extra IMPACT
					//

					TheNomad::SGame::GfxManager.AddLanding( origin );
				}
			}
			
			// clip it
			ClipBounds();

			m_Acceleration = vec3( 0.0f );
        }
        
        private TheNomad::SGame::EntityObject@ m_EntityData = null;
        private vec3 m_Velocity = vec3( 0.0f );
        private vec3 m_Acceleration = vec3( 0.0f );
        private float m_nAngle = 0.0f;
		private float m_nWeight = 0.0f;
        private int m_nWaterLevel = 0;
		private WaterType m_nWaterType = WaterType::None;
    };
};