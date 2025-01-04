#include "nomadmain/SGame/MobObject.as"
#include "moblib/System/AISensor.as"
#include "moblib/System/AISensorSight.as"
#include "moblib/System/AISensorSound.as"

namespace moblib {
	class MobScript {
		MobScript() {
		}
		
		void Link( TheNomad::SGame::MobObject@ mob ) {
			TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();
			const uint id = info.type;
			
			@m_IdleState = @info.idleState;
			@m_SearchState = @info.searchState;
			@m_ChaseState = @info.chaseState;
			@m_FightMeleeState = @info.meleeState;
			@m_FightMissileState = @info.missileState;
//			@m_FleeState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FLEE ) + id );
			@m_DeathState = @info.dieLowState;
			
			@m_EntityData = @mob;
			m_Sensor.Init( @mob );
		}
		
		void FleeThink() {
			GameError( "MobScript::FleeThink: pure virtual function called" );
		}
		void IdleThink() {
			GameError( "MobScript::IdleThink: pure virtual function called" );
		}
		void DeadThink() {
			GameError( "MobScript::DeadThink: pure virtual function called" );
		}
		void FightMissile() {
			GameError( "MobScript::FightMissile: pure virtual function called" );
		}
		void FightMelee() {
			GameError( "MobScript::FightMelee: pure virtual function called" );
		}
		void SearchThink() {
			GameError( "MobScript::SearchThink: pure virtual function called" );
		}
		void ChaseThink() {
			GameError( "MobScript::ChaseThink: pure virtual function called" );
		}
		void OnSpawn() {
			GameError( "MobScript::OnSpawn: pure virtual function called" );
		}
		void OnDeath() {
			GameError( "MobScript::OnDeath: pure virtual function called" );
		}
		
		void SetState( TheNomad::SGame::EntityState@ state ) {
			m_EntityData.SetState( @state );
		}
		
		TheNomad::SGame::EntityState@ GetIdleState() {
			return @m_IdleState;
		}
		TheNomad::SGame::EntityState@ GeSearchState() {
			return @m_SearchState;
		}
		TheNomad::SGame::EntityState@ GetChaseState() {
			return @m_ChaseState;
		}
		TheNomad::SGame::EntityState@ GetFightMissileState() {
			return @m_FightMissileState;
		}
		TheNomad::SGame::EntityState@ GetFightMeleeStateState() {
			return @m_FightMeleeState;
		}
		TheNomad::SGame::EntityState@ GetFleeState() {
			return @m_FleeState;
		}
		TheNomad::SGame::EntityState@ GetDeathState() {
			return @m_DeathState;
		}
		
		protected TheNomad::SGame::MobObject@ m_EntityData = null;
		protected TheNomad::SGame::EntityState@ m_IdleState = null;
		protected TheNomad::SGame::EntityState@ m_SearchState = null;
		protected TheNomad::SGame::EntityState@ m_ChaseState = null;
		protected TheNomad::SGame::EntityState@ m_FightMeleeState = null;
		protected TheNomad::SGame::EntityState@ m_FightMissileState = null;
		protected TheNomad::SGame::EntityState@ m_FleeState = null;
		protected TheNomad::SGame::EntityState@ m_DeathState = null;
		
		protected moblib::System::AISensor m_Sensor;
	};
};