namespace TheNomad::SGame {
	//
	// WeaponSlot:
	// just a fancy wrapper around a player specific weapon
	//
	class WeaponSlot {
		WeaponSlot() {
		}

		WeaponSlot& opAssign( WeaponObject@ obj ) {
			@m_Base = @obj;
		}
		WeaponObject@ opConv() {
			return @m_Base;
		}
		const WeaponObject@ opConv() const {
			return @m_Base;
		}
		WeaponObject@ opImplConv() {
			return @m_Base;
		}
		const WeaponObject@ opImplConv() const {
			return @m_Base;
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
			m_nMode = InfoSystem::WeaponProperty( uint( m_nMode ) | uint( nMode ) );
		}
		InfoSystem::WeaponProperty GetMode() const {
			return m_nMode;
		}
		void ClearMode() {
			m_nMode = InfoSystem::WeaponProperty::None;
		}
		
		private WeaponObject@ m_Base = null;
		private uint m_nIndex = 0;
		private InfoSystem::WeaponProperty m_nMode = InfoSystem::WeaponProperty::None;
	};
};