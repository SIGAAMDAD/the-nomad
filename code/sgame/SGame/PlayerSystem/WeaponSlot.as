namespace TheNomad::SGame {
	class WeaponSlot {
		WeaponSlot() {
		}

		bool IsUsed() const {
			return @m_Base !is null;
		}
		WeaponObject@ GetData() {
			return @m_Base;
		}
		const WeaponObject@ GetData() const {
			return @m_Base;
		}
		void SetData( WeaponObject@ obj ) {
			@m_Base = @obj;
		}

		void AddMode( InfoSystem::WeaponProperty nMode ) {
			m_nMode |= uint( nMode );
		}
		InfoSystem::WeaponProperty GetMode() const {
			return m_nMode;
		}
		void ClearMode() {
			m_nMode = InfoSystem::WeaponProperty::None;
		}
		
		WeaponObject@ m_Base = null;
		uint m_nIndex = 0;
		InfoSystem::WeaponProperty m_nMode = InfoSystem::WeaponProperty::None;
	};
};