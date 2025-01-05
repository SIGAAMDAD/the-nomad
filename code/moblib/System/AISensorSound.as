namespace moblib {
	class AISensorSound {
		AISensorSound() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) {
			const TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();
			TheNomad::SGame::PlayrObject@ player = @TheNomad::SGame::EntityManager.GetActivePlayer();

			TheNomad::Engine::Physics::Sphere soundDetection( mob.GetOrigin(), info.soundRange );
			if ( soundDetection.ContainsPoint( player.GetOrigin() ) ) {
				if ( player.GetSoundLevel() >= info.soundTolerance
					|| TheNomad::Util::Distance( player.GetOrigin(), mob.GetOrigin() ) < 2.0f )
				{
					mob.SetTarget( @player );
					return true;
				}
				return false;
			}
			return false;
		}
	};
};