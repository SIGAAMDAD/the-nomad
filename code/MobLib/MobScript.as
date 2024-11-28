#include "nomadmain/SGame/MobObject.as"
#include "System/AISensorSight.as"
#include "System/AISensorSound.as"
#include "System/AISquad.as"

namespace MobLib {
	class MobScript {
		MobScript() {
		}
		
		void Link( TheNomad::SGame::MobObject@ mob ) {
			const uint id = mob.GetInfo().type;
			
			@m_IdleState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_IDLE ) + id );
			@m_SearchState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_SEARCH ) + id );
			@m_ChaseState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_CHASE ) + id );
			@m_FightState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT ) + id );
			@m_FightMeleeState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT_MELEE ) + id );
			@m_FightMissileState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT_MISSILE ) + id );
			@m_FleeState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FLEE ) + id );
			@m_DeathState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_DEAD ) + id );
			
			@m_EntityData = @mob;
			m_Sensor.Init( @mob.GetInfo(), @mob );
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
		void FightThink() {
			GameError( "MobScript::FightThink: pure virtual function called" );
		}
		void FightMissile() {
			GameError( "MobScript::FightMissile: pure virtual function called" );
		}
		void FightMelee() {
			GameError( "MobScript::FightMelee: pure virtual function called" );
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
			return @m_ChaseTarget;
		}
		TheNomad::SGame::EntityState@ GetFightState() {
			return @m_FightState;
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
		protected TheNomad::SGame::EntityState@ m_FightState = null;
		protected TheNomad::SGame::EntityState@ m_FightMeleeState = null;
		protected TheNomad::SGame::EntityState@ m_FightMissileState = null;
		protected TheNomad::SGame::EntityState@ m_FleeState = null;
		protected TheNomad::SGame::EntityState@ m_DeathState = null;
		
		protected System::AISensor m_Sensor;
	};
};