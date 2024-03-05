namespace TheNomad::SGame {
    class EntityState {
        EntityState() {
        }

        uint GetId() const {
            return m_nId;
        }

        private uint m_nId;
        private uint m_nDuration;
    };
};