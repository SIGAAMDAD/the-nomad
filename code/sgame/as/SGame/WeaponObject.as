#include "SGame/InfoSystem/InfoDataManager.as"
#include "SGame/InfoSystem/WeaponInfo.as"

namespace TheNomad::SGame {
    class WeaponObject : EntityObject {
		WeaponObject() {
		}
		InfoSystem::WeaponType GetWeaponType() const {
			return m_nType;
		}
		uint GetWeaponID() const {
			return m_nID;
		}

		void SetWeaponType( InfoSystem::WeaponType type ) {
			m_nType = type;
		}
		void SetWeaponID( uint id ) {
			m_nID = id;
		}
		InfoSystem::WeaponProperty GetProperties() const {
			return m_Info.weaponProps;
		}
		
		bool IsOneHanded() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsOneHanded ) == 1;
		}
		bool IsTwoHanded() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsTwoHanded ) == 1;
		}
		bool IsBladed() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsBladed ) == 1;
		}
		bool IsPolearm() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsPolearm ) == 1;
		}
		bool IsFirearm() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsFirearm ) == 1;
		}
		bool IsBlunt() const {
			return ( m_Info.weaponProps & InfoSystem::WeaponProperty::IsBlunt ) == 1;
		}
		
		void Use( EntityObject@ ent ) {
			
		}
		void UseAlt( EntityObject@ ent ) {
			
		}

		void Think() {
			GameError( "WeaponObject::Think: called" );
		}
		void Spawn( uint id, const vec3& in origin ) override {
			m_nID = id;
			m_Link.m_Origin = origin;
			m_Link.m_Bounds.MakeBounds( origin );
		}

		uint GetSpriteIndex() const {
			return m_nSpriteOffset;
		}

		private InfoSystem::WeaponType m_nType;
		private uint m_nID;
		private uint m_nSpriteOffset;
		private InfoSystem::WeaponInfo@ m_Info;
	};
};