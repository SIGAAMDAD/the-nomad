namespace TheNomad::Engine::Physics {
	enum WaterType {
		None,
		Water,
		Lava,
		Acid
	};

	const float MAX_JUMP_HEIGHT = 3.5f;

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
			if ( SGame::sgame_NoClip.GetInt() == 1 ) {
				return;
			}

			vec3 origin = m_EntityData.GetOrigin();

            if ( origin.x < 0.0f ) {
                m_EntityData.SetOrigin( vec3( 0.0f, origin.y, origin.z ) );
            }
            if ( origin.y < 0.0f ) {
                m_EntityData.SetOrigin( vec3( origin.x, 0.0f, origin.z ) );
		    }
			if ( origin.x > TheNomad::SGame::LevelManager.GetMapData().GetWidth() - 1 ) {
				m_EntityData.SetOrigin( vec3( TheNomad::SGame::LevelManager.GetMapData().GetWidth() - 1, origin.y, origin.z ) );
			}
			if ( origin.y > TheNomad::SGame::LevelManager.GetMapData().GetHeight() - 1 ) {
				m_EntityData.SetOrigin( vec3( origin.x, TheNomad::SGame::LevelManager.GetMapData().GetHeight() - 1, origin.z ) );
			}
		}
		
		void ApplyFriction() {
			if ( m_Velocity[0] < 0.0f ) {
				m_Velocity[0] = TheNomad::Util::Clamp( m_Velocity[0] + TheNomad::SGame::sgame_Friction.GetFloat(), m_Velocity[0], 100.0f );
			} else if ( m_Velocity[0] > 0.0f ) {
				m_Velocity[0] = TheNomad::Util::Clamp( m_Velocity[0] - TheNomad::SGame::sgame_Friction.GetFloat(), -100.0f, m_Velocity[0] );
			}
			if ( m_Velocity[1] < 0.0f ) {
				m_Velocity[1] = TheNomad::Util::Clamp( m_Velocity[1] + TheNomad::SGame::sgame_Friction.GetFloat(), m_Velocity[1], 100.0f );
			} else if ( m_Velocity[1] > 0.0f ) {
				m_Velocity[1] = TheNomad::Util::Clamp( m_Velocity[1] - TheNomad::SGame::sgame_Friction.GetFloat(), -100.0f, m_Velocity[1] );
			}
			if ( m_Velocity[2] < 0.0f && m_EntityData.GetOrigin().z <= 0.0f ) {
				m_Velocity[2] = 0.0f;
			} else if ( m_Velocity[2] > 0.0f && m_EntityData.GetOrigin().z == 0.0f ) {
//				m_Velocity[2] += TheNomad::SGame::sgame_Gravity.GetFloat();
			} else if ( m_Velocity[2] > 0.0f && m_EntityData.GetOrigin().z >= MAX_JUMP_HEIGHT ) {
				m_Velocity[2] -= TheNomad::SGame::sgame_Gravity.GetFloat();
			}
		}
		
		void SetWaterLevel() {
			vec3 point;
			uint tile;
			TheNomad::GameSystem::BBox bounds;
			
			//
			// get waterlevel, accounting for ducking
			//
			m_nWaterLevel = 0;
			m_nWaterType = WaterType::None;
			
			point = m_EntityData.GetOrigin();
			bounds = m_EntityData.GetBounds();
			tile = TheNomad::SGame::LevelManager.GetMapData().GetTile( point, bounds );
			
			if ( ( tile & SURFACEPARM_WATER ) != 0 || ( tile & SURFACEPARM_LAVA ) != 0 ) {
				m_nWaterType = WaterType( tile );
				m_nWaterLevel = 1; // just walking in a puddle
				
				// check the level below us
				bounds.m_Mins.z--;
				bounds.m_Maxs.z--;
				tile = TheNomad::SGame::LevelManager.GetMapData().GetTile( point, bounds );
				if ( ( tile & SURFACEPARM_WATER ) != 0 || ( tile & SURFACEPARM_LAVA ) != 0 ) {
					m_nWaterLevel = 2; // swimming now
					
					// check the level above us
					bounds.m_Mins.z += 2;
					bounds.m_Maxs.z += 2;
					tile = TheNomad::SGame::LevelManager.GetMapData().GetTile( point, bounds );
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
			ProfileBlock block( "PhysicsObject::OnRunTic" );
			
			// clip it
			ClipBounds();
			SetWaterLevel();
			
			float friction = TheNomad::SGame::sgame_Friction.GetFloat();
			vec3 origin = m_EntityData.GetOrigin();
			
			const uint tileFlags = TheNomad::SGame::LevelManager.GetMapData().GetTile( origin, m_EntityData.GetBounds() );
			if ( ( tileFlags & SURFACEPARM_WATER ) != 0 || ( tileFlags & SURFACEPARM_LAVA ) != 0 ) {
				friction = TheNomad::SGame::sgame_WaterFriction.GetFloat() * m_nWaterLevel;
			} else if ( origin.z > 0.0f && m_nWaterLevel == 0 ) {
				friction = TheNomad::SGame::sgame_AirFriction.GetFloat();
			}
			
			// calculate velocity
			m_Velocity.x = m_Acceleration.x;
			m_Velocity.y = m_Acceleration.y;
			m_Velocity.z += m_Acceleration.z;
			
			ApplyFriction();

			TheNomad::GameSystem::BBox bounds;
			bounds.m_nWidth = m_EntityData.GetBounds().m_nWidth;
			bounds.m_nHeight = m_EntityData.GetBounds().m_nHeight;
			bounds.MakeBounds( origin + m_Velocity );
			
			TheNomad::SGame::EntityObject@ active = @TheNomad::SGame::EntityManager.GetActiveEnts();
			TheNomad::SGame::EntityObject@ ent = null;
			for ( @ent = @active.m_Next; @ent.m_Next !is @active; @ent = @ent.m_Next ) {
				if ( bounds.IntersectsBounds( ent.GetBounds() ) && @m_EntityData !is @ent ) {
					m_Velocity = 0.0f;
					m_Acceleration = 0.0f;
					if ( ent.GetType() == TheNomad::GameSystem::EntityType::Mob &&
						( m_Velocity.x > 2.25f && m_Velocity.y > 2.25f ) )
					{
						// damage
						TheNomad::SGame::EntityManager.DamageEntity( @m_EntityData, @ent );
					}
					return; // clip
				}
			}

			vec3 tmp = origin;
			const TheNomad::GameSystem::DirType dir = CalcMoveDir();
			switch ( dir ) {
			case TheNomad::GameSystem::DirType::North:
				tmp.y -= m_EntityData.GetBounds().m_nHeight;
				break;
			case TheNomad::GameSystem::DirType::NorthEast:
				tmp.y -= m_EntityData.GetBounds().m_nHeight;
				tmp.x += m_EntityData.GetBounds().m_nWidth;
				break;
			case TheNomad::GameSystem::DirType::East:
				tmp.x += m_EntityData.GetBounds().m_nWidth;
				break;
			case TheNomad::GameSystem::DirType::SouthEast:
				tmp.y += m_EntityData.GetBounds().m_nHeight;
				tmp.x += m_EntityData.GetBounds().m_nWidth;
				break;
			case TheNomad::GameSystem::DirType::South:
				tmp.y += m_EntityData.GetBounds().m_nHeight;
				break;
			case TheNomad::GameSystem::DirType::SouthWest:
				tmp.y += m_EntityData.GetBounds().m_nHeight;
				tmp.x -= m_EntityData.GetBounds().m_nWidth;
				break;
			case TheNomad::GameSystem::DirType::West:
				tmp.x -= m_EntityData.GetBounds().m_nWidth;
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
				tmp.y -= m_EntityData.GetBounds().m_nHeight;
				tmp.x -= m_EntityData.GetBounds().m_nWidth;
				break;
			};

			if ( TheNomad::GameSystem::CheckWallHit( tmp, dir ) ) {
				m_Acceleration = 0.0f;

				const float z = m_Velocity.z;
				m_Velocity = 0.0f;
				if ( origin.z > 0.0f ) {
					m_Velocity.z = z;
				}
				return;
			}
			
			origin.x += m_Velocity.x;
			origin.y += m_Velocity.y;
			origin.z += m_Velocity.z;
			// apply gravity
			if ( origin.z < 0.0f ) {
				origin.z = 0.0f;
			}
			
			m_EntityData.SetOrigin( origin );
			
			// clip it
			ClipBounds();
			
			m_Acceleration = 0.0f;
        }
        
        private TheNomad::SGame::EntityObject@ m_EntityData = null;
        private vec3 m_Velocity = vec3( 0.0f );
        private vec3 m_Acceleration = vec3( 0.0f );
        private float m_nAngle = 0.0f;
        private int m_nWaterLevel = 0;
		private WaterType m_nWaterType = WaterType::None;
    };
};