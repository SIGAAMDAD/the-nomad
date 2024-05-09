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

        void OnRunTic() {
            const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
            vec3 accel;
            float friction = TheNomad::SGame::sgame_Friction.GetFloat();
            vec3 origin = m_EntityData.GetOrigin();

            const uint tileFlags = TheNomad::SGame::LevelManager.GetMapData().GetTile( origin, m_EntityData.GetBounds() );

            if ( ( tileFlags & SURFACEPARM_WATER ) != 0 || ( tileFlags & SURFACEPARM_LAVA ) != 0 ) {
                friction = TheNomad::SGame::sgame_WaterFriction.GetFloat() * m_nWaterLevel;
            } else if ( origin.z > 0.0f && m_nWaterLevel == 0 ) {
                friction = TheNomad::SGame::sgame_AirFriction.GetFloat();
            }

            // apply friction
            accel.x = ( m_Speed.x - friction ) * m_Acceleration.x;
            accel.y = ( m_Speed.y - friction ) * m_Acceleration.y;
            accel.z = ( m_Speed.z - friction ) * m_Acceleration.z;

            // calculate velocity
            m_Velocity.x = gameTic * accel.x;
            m_Velocity.y = gameTic * accel.y;
            m_Velocity.z = gameTic * accel.z;

            // apply gravity
            if ( m_Velocity.z > 0.0f ) {
                m_Velocity.z -= gameTic * TheNomad::SGame::sgame_Gravity.GetFloat();
            }

            m_Velocity.x = Util::Clamp( m_Velocity.x, 0.0f, m_MaxSpeed.x );
            m_Velocity.y = Util::Clamp( m_Velocity.y, 0.0f, m_MaxSpeed.y );
            m_Velocity.z = Util::Clamp( m_Velocity.z, -9999.0f, m_MaxSpeed.z );

            origin.x += cos( m_nAngle ) * m_Velocity.x;
            origin.y += sin( m_nAngle ) * m_Velocity.y;
            origin.z += m_Velocity.z;
            m_EntityData.SetOrigin( origin );
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