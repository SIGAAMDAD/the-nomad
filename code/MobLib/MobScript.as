#include "nomadmain/SGame/MobObject.as"

namespace TheNomad::MobLib {
	class MobScript {
		MobScript() {
		}

		void Link( TheNomad::SGame::MobObject@ mob ) {
			const uint id = cast<TheNomad::InfoSystem::MobInfo@>( @mob.GetInfo() ).type;

			@m_IdleState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_IDLE ) + id );
			@m_SearchState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_SEARCH ) + id );
			@m_ChaseState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_CHASE ) + id );
			@m_FightState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT ) + id );
			@m_FightMeleeState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT_MELEE ) + id );
			@m_FightMissileState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_FIGHT_MISSILE ) + id );
			@m_DeathState = @TheNomad::SGame::StateManager.GetStateForNum( uint( TheNomad::SGame::StateNum::ST_MOB_DEAD ) + id );
		}

		private TheNomad::SGame::EntityState@ m_IdleState = null;
		private TheNomad::SGame::EntityState@ m_SearchState = null;
		private TheNomad::SGame::EntityState@ m_ChaseState = null;
		private TheNomad::SGame::EntityState@ m_FightState = null;
		private TheNomad::SGame::EntityState@ m_FightMeleeState = null;
		private TheNomad::SGame::EntityState@ m_FightMissileState = null;
		private TheNomad::SGame::EntityState@ m_DeathState = null;
	};
};