namespace TheNomad::SGame {
	const uint NUM_WEAPON_SLOTS = 9;

	class InventoryManager {
		InventoryManager( PlayrObject@ ent, ArmData@ left, ArmData@ right ) {
			@m_WeaponSlots[0] = @m_HeavyPrimary;
			@m_WeaponSlots[1] = @m_HeavySidearm;
			@m_WeaponSlots[2] = @m_LightPrimary;
			@m_WeaponSlots[3] = @m_LightSidearm;
			@m_WeaponSlots[4] = @m_Melee1;
			@m_WeaponSlots[5] = @m_Melee2;
			@m_WeaponSlots[6] = @m_RightHand;
			@m_WeaponSlots[7] = @m_LeftHand;
			@m_WeaponSlots[8] = @m_Ordnance;

			@m_LeftArm = @left;
			@m_RightArm = @right;
			@m_EntityData = @ent;

			for ( uint i = 0; i < m_WeaponSlots.Count(); ++i ) {
				m_WeaponSlots[i] = WeaponSlot( i );
			}
		}

		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			section.SaveUInt( "currentWeapon", m_nCurrentWeapon );

			const uint leftSlot = m_LeftArm.GetEquippedWeapon();
			section.SaveUInt( "leftHandSlot", leftSlot );
			if ( leftSlot != uint( -1 ) ) {
				section.SaveUInt( "leftHandMode", uint( m_WeaponSlots[ leftSlot ].GetMode() ) );
			}

			const uint rightSlot = m_RightArm.GetEquippedWeapon();
			section.SaveUInt( "rightHandSlot", rightSlot );
			if ( rightSlot != uint( -1 ) ) {
				section.SaveUInt( "rightHandMode", uint( m_WeaponSlots[ rightSlot ].GetMode() ) );
			}
		}
		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			m_nCurrentWeapon = section.LoadUInt( "currentWeapon" );

			const uint leftSlot = section.LoadUInt( "leftHandSlot" );
			m_LeftArm.SetEquippedSlot( leftSlot );
			if ( leftSlot != uint( -1 ) ) {
				const InfoSystem::WeaponProperty mode = InfoSystem::WeaponProperty( section.LoadUInt( "leftHandMode" ) );
				m_WeaponSlots[ leftSlot ].SetMode( mode );
			}

			const uint rightSlot = section.LoadUInt( "rightHandSlot" );
			m_RightArm.SetEquippedSlot( rightSlot );
			if ( rightSlot != uint( -1 ) ) {
				const InfoSystem::WeaponProperty mode = InfoSystem::WeaponProperty( section.LoadUInt( "rightHandMode" ) );
				m_WeaponSlots[ rightSlot ].SetMode( mode );
			}

			return true;
		}

		const WeaponSlot@ GetEquippedWeapon() const {
			return @m_WeaponSlots[ m_nCurrentWeapon ];
		}
		WeaponSlot@ GetEquippedWeapon() {
			return @m_WeaponSlots[ m_nCurrentWeapon ];
		}

		const WeaponSlot@ GetSlot( uint nIndex ) const {
			return @m_WeaponSlots[ nIndex ];
		}
		WeaponSlot@ GetSlot( uint nIndex ) {
			return @m_WeaponSlots[ nIndex ];
		}

		void SetRightHandWeapon( WeaponObject@ weapon ) {
			m_WeaponSlots[ m_EntityData.GetRightArm().GetEquippedWeapon() ] = @weapon;
		}
		void SetLeftHandWeapon( WeaponObject@ weapon ) {
			m_WeaponSlots[ m_EntityData.GetLeftArm().GetEquippedWeapon() ] = @weapon;
		}
		const WeaponObject@ GetRightHandWeapon() const {
			return @m_WeaponSlots[ m_EntityData.GetRightArm().GetEquippedWeapon() ].GetData();
		}
		const WeaponObject@ GetLeftHandWeapon() const {
			return @m_WeaponSlots[ m_EntityData.GetLeftArm().GetEquippedWeapon() ].GetData();
		}
		WeaponObject@ GetRightHandWeapon() {
			return @m_WeaponSlots[ m_EntityData.GetRightArm().GetEquippedWeapon() ].GetData();
		}
		WeaponObject@ GetLeftHandWeapon() {
			return @m_WeaponSlots[ m_EntityData.GetLeftArm().GetEquippedWeapon() ].GetData();
		}

		void EquipSlot( uint nIndex ) {
			m_nCurrentWeapon = nIndex;
		}
		uint GetSlotIndex() const {
			return m_nCurrentWeapon;
		}

		private WeaponSlot@[] m_WeaponSlots( NUM_WEAPON_SLOTS );
		private WeaponSlot m_HeavyPrimary( 0 );
		private WeaponSlot m_HeavySidearm( 1 );
		private WeaponSlot m_LightPrimary( 2 );
		private WeaponSlot m_LightSidearm( 3 );
		private WeaponSlot m_Melee1( 4 );
		private WeaponSlot m_Melee2( 5 );
		private WeaponSlot m_RightHand( 6 );
		private WeaponSlot m_LeftHand( 7 );
		private WeaponSlot m_Ordnance( 8 );

		private PlayrObject@ m_EntityData = null;
		private ArmData@ m_LeftArm = null;
		private ArmData@ m_RightArm = null;

		private uint m_nCurrentWeapon = 0;
	};
};