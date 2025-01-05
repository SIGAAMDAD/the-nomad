#include "nomadmain/SGame/MobObject.as"
#include "moblib/Script/MercShotty.as"

namespace moblib::System {
	const uint MAX_SQUAD_SIZE = 20;
	
	class AISquad {
		AISquad() {
		}

		void InitSquad( uint nSquadID ) {
			m_nSquadID = nSquadID;

			m_SquadAABB.Clear();
			m_nSquadMembers = 0;

			m_bSquadMemberDied = false;
		}
		void SquadBark( int hBark ) {
			m_nSquadBark = hBark;
		}
		void AddSquadMember( moblib::Script::MercShotty@ member ) {
			if ( m_nSquadMembers < MAX_SQUAD_SIZE ) {
				@m_SquadMembers[ m_nSquadMembers ] = @member;
				++m_nSquadMembers;
			}
		}
		void Update() {
			if ( m_nSquadBark != -1 ) {
				int rand = TheNomad::Util::PRandom() & ( m_nSquadMembers - 1 );
				for ( int iMember = 0; iMember < m_nSquadMembers; ++iMember ) {
					if ( iMember == rand ) {
						m_SquadMembers[ iMember ].GetData().EmitSound( m_nSquadBark, 10.0f, 0xff );
						break;
					}
				}
				m_nSquadBark = -1;
			}
		}
		/*
		void Update() {
			HandleSquadDeaths();
		}
		void HandleSquadDeaths() {
			bool bDead = false;
			int iMember;
			for ( iMember = m_nSquadMembers; ++iMember ) {
				if ( m_SquadMembers[i].m_EntityData.CheckFlags( EntityFlags::Dead ) ) {
					@m_SquadMembers[i] = null;
					bDead = true;
					break;
				}
			}

			if ( !bDead ) {
				return;
			}
			m_bSquadMemberDied = true;

			int iDead = iMember;
			for ( iMember = iDead; iMember < m_nSquadMembers - 1; ++iMember ) {
				@m_SquadMembers[i] = @m_SquadMembers[ i + 1 ];
			}
		}
		*/
		void SetTarget( TheNomad::SGame::EntityObject@ ent ) {
			m_Leader.GetData().SetTarget( @ent );
		}

		array<moblib::Script::MercShotty@>@ GetMembers() {
			return @m_SquadMembers;
		}
		moblib::Script::MercShotty@ GetLeader() {
			return @m_Leader;
		}
		uint GetID() const {
			return m_nSquadID;
		}
		
		private TheNomad::Engine::Physics::Bounds m_SquadAABB;
		private moblib::Script::MercShotty@[] m_SquadMembers( MAX_SQUAD_SIZE );
		private moblib::Script::MercShotty@ m_Leader = null;
		private float m_nMorale = 0.0f;
		private int m_nSquadID = 0;
		private uint m_nSquadMembers = 0;
		private int m_nSquadBark = -1;
		private bool m_bSquadMemberDied = false;
	};
};