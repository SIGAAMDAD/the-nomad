namespace TheNomad::SGame {
    class GrappleHook {
        GrappleHook() {
        }

        bool IsLatched() const {
            return m_bLatched;
        }
        void Use( const vec3& in origin, float angle, float strength ) {
            m_bLatched = false;

            m_LineData.m_Start = origin;
            m_LineData.m_nLength = strength;
            m_LineData.m_nEntityNumber = ENTITYNUM_INVALID;
            m_LineData.m_nEntityId = 0;
            m_LineData.m_nAngle = angle;
        }

        void OnRunTic() {
            ray.Cast();
        }

        private GameSystem::RayCast m_LineData;
        private bool m_bLatched = false;
    };
};