namespace TheNomad::Engine::Physics {
    class PhysicsObject {
        PhysicsObject() {
        }

        void Init( TheNomad::SGame::EntityObject@ ent, const vec3& in speed, const vec3& in maxSpeed ) {
            @m_EntityData = @ent;
            m_Speed = speed;
            m_MaxSpeed = maxSpeed;
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

        const vec3& GetVelocity() const {
            return m_Velocity;
        }
        vec3& GetVelocity() {
            return m_Velocity;
        }
        int GetWaterLevel() const {
            return m_nWaterLevel;
        }

        private void ClipBounds() {
            TheNomad::GameSystem::BBox mapBounds;
            mapBounds.m_Mins = vec3( 0.0f, 0.0f, 0.0f );
            mapBounds.m_Maxs = vec3( float( TheNomad::SGame::LevelManager.GetMapData().GetWidth() ),
                float( TheNomad::SGame::LevelManager.GetMapData().GetHeight() ), 0 );

            if ( !Util::BoundsIntersect( m_EntityData.GetBounds(), mapBounds ) ) {
                const float width = m_EntityData.GetBounds().m_Maxs.x - m_EntityData.GetBounds().m_Mins.x;
                const float height = m_EntityData.GetBounds().m_Maxs.y - m_EntityData.GetBounds().m_Mins.y;
                m_EntityData.SetOrigin( vec3( width, height, m_EntityData.GetOrigin().z ) );
            }
            if ( m_EntityData.GetOrigin().x < 0.0f ) {
                m_EntityData.SetOrigin( vec3( 0.0f, m_EntityData.GetOrigin().y, m_EntityData.GetOrigin().z ) );
            }
            if ( m_EntityData.GetOrigin().y < 0.0f ) {
                m_EntityData.SetOrigin( vec3( m_EntityData.GetOrigin().x, 0.0f, m_EntityData.GetOrigin().z ) );
            }
        }

        void OnRunTic() {
            const uint gameTic = TheNomad::GameSystem::GameManager.GetDeltaTics();
            vec3 accel;
            float friction = TheNomad::SGame::sgame_Friction.GetFloat();
            vec3 origin;

            // clip it
            ClipBounds();

            origin = m_EntityData.GetOrigin();

            const uint tileFlags = TheNomad::SGame::LevelManager.GetMapData().GetTile( origin, m_EntityData.GetBounds() );
            if ( ( tileFlags & SURFACEPARM_WATER ) != 0 || ( tileFlags & SURFACEPARM_LAVA ) != 0 ) {
                friction = TheNomad::SGame::sgame_WaterFriction.GetFloat() * m_nWaterLevel;
            } else if ( origin.z > 0.0f && m_nWaterLevel == 0 ) {
                friction = TheNomad::SGame::sgame_AirFriction.GetFloat();
            }

            accel.x = m_Speed.x + m_Acceleration.x;
            accel.y = m_Speed.y + m_Acceleration.y;
            accel.z = m_Speed.z + m_Acceleration.z;

            // calculate velocity
            m_Velocity.x = gameTic * accel.x;
            m_Velocity.y = gameTic * accel.y;
            m_Velocity.z = gameTic * accel.z;

            // apply gravity
            if ( m_Velocity.z > 0.0f ) {
                m_Velocity.z -= gameTic * TheNomad::SGame::sgame_Gravity.GetFloat();
            }

            if ( m_Velocity.x > 0 ) {
                m_Velocity.x -= friction * gameTic;
                if ( m_Velocity.x < 0 ) {
                    m_Velocity.x = 0.0f;
                }
            } else if ( m_Velocity.x < 0 ) {
                m_Velocity.x += friction * gameTic;
                if ( m_Velocity.x > 0 ) {
                    m_Velocity.x = 0.0f;
                }
            }
            if ( m_Velocity.y > 0 ) {
                m_Velocity.y -= friction * gameTic;
                if ( m_Velocity.y < 0 ) {
                    m_Velocity.y = 0.0f;
                }
            } else if ( m_Velocity.y < 0 ) {
                m_Velocity.y += friction * gameTic;
                if ( m_Velocity.y > 0 ) {
                    m_Velocity.y = 0.0f;
                }
            }

            m_Velocity.x = Util::Clamp( m_Velocity.x, -m_MaxSpeed.x, m_MaxSpeed.x );
            m_Velocity.y = Util::Clamp( m_Velocity.y, -m_MaxSpeed.y, m_MaxSpeed.y );
            m_Velocity.z = Util::Clamp( m_Velocity.z, -9999.0f, m_MaxSpeed.z );

            origin.x += m_Velocity.x;
            origin.y += m_Velocity.y;
            origin.z += m_Velocity.z;

            m_EntityData.SetOrigin( origin );

            // clip it
            ClipBounds();

            m_Acceleration = 0.0f;
        }

        private TheNomad::SGame::EntityObject@ m_EntityData = null;
        private vec3 m_Velocity = vec3( 0.0f );
        private vec3 m_Speed = vec3( 0.0f );
        private vec3 m_Acceleration = vec3( 0.0f );
        private vec3 m_MaxSpeed = vec3( 0.0f );
        private float m_nAngle = 0.0f;
        private int m_nWaterLevel = 0;
    };
};