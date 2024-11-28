#include "nomadmain/SGame/MobObject.as"

namespace moblib {
	const uint MAX_SQUAD_SIZE = 20;
	
	class AISquad {
		AISquad() {
		}
		
		//
		// AISquad::CheckLeaderShip: if the squad's morale is less than given
		// threshold, then disband it
		//
		bool CheckLeaderShip() {
			if ( @m_Leader is null ) {
				Disband();
				return false;
			}
			else if ( m_nMorale < m_Leader.GetMoraleRequirement() ) {
				return false;
			}
			return true; // all good
		}
		void MergeSquad( AISquad@ other ) {
			for ( uint i = 0; i < other.m_nSquadMembers; ++i ) {
				// bail if squad is full
				if ( m_nSquadMembers < m_Leader.GetInfo().maxSquadSize ) {
					@m_SquadMembers[ m_nSquadMembers ] = @other.m_SquadMembers[ i ];
					@m_SquadMembers[ m_nSquadMembers ].SetGroup( @this );
					++m_nSquadMembers;
				}
			}
			m_SquadAABB.Merge( other.m_SquadAAB );
			
			other.m_SquadMembers.Clear();
			other.m_nSquadMembers = 0;
			@other.m_Leader = null;
			other.m_nSquadID = -1;
		}
		void Disband() {
			DebugPrint( "Disbanding Squad #" + m_nSquadID + "\n" );
			for ( uint i = 0; i < m_nSquadMembers; i++ ) {
				m_Members[ i ].SetGroup( null );
			}
		}
		void AssignLeader( TheNomad::SGame::MobObject@ leader ) {
			// add or detract from the group's morale
			if ( @m_Leader !is null && @leader is null ) {
				m_nMorale -= m_Leader.GetLeaderMoraleBuff();
			} else if ( @leader !is null ) {
				m_nMorale += m_Leader.GetLeaderMoraleBuff();
			}
			
			for ( uint i = 0; i < m_nSquadMembers; i++ ) {
				m_Members[ i ].SetGroupLeader( @leader );
			}
			@m_Leader = @leader;
		}
		void RemoveMember( TheNomad::SGame::MobObject@ member ) {
			if ( floor( m_nMorale ) < m_nSquadMembers ) {
				Disband();
			}
		}
		
		void AddMember( TheNomad::SGame::MobObject@ member, const TheNomad::GameSystem::BBox& in bounds ) {
			// bail if squad is full
			
			if ( m_nSquadMembers < m_Leader.GetInfo().maxSquadSize ) {
				DebugPrint( "Joining Squad " + m_nSquadID + "\n" );
				
				// expand the squad's bounds to encompass the new member
				if ( m_nSquadMembers == 0 ) {
					m_SquadAABB = bounds;
					AssignLeader( @member );
				} else {
					m_SquadAABB.Merge( bounds );
				}
				
				@m_SquadMembers[ m_nSquadMembers ] = @member;
				++m_nSquadMembers;
			}
			member.SetGroup( @this );
			
			// depending on how big this member is, add to the
			// mental strength
			m_nMorale += member.GetMoraleBuff();
		}
		array<TheNomad::SGame::MobObject@>@ GetMembers() {
			return @m_SquadMembers;
		}
		TheNomad::SGame::MobObject@ GetLeader() {
			return @m_Leader;
		}
		uint GetID() const {
			return m_nSquadID;
		}
		
		private TheNomad::GameSystem::BBox m_SquadAABB;
		private array<TheNomad::SGame::MobObject@> m_SquadMembers;
		private TheNomad::SGame::MobObject@ m_Leader = null;
		private float m_nMorale = 0.0f;
		private int m_nSquadID = 0;
		private uint m_nSquadMembers = 0;
	};
};