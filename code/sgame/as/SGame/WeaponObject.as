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

		void Think() {
		}
		void Spawn( uint id, const vec3& in origin ) override {
			m_nID = id;
		}

		private InfoSystem::WeaponType m_nType;
		private uint m_nID;
	};
};