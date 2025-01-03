namespace moblib {
	class AISensorSound {
		AISensorSound() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) {
			const TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();
			
			return mob.GetBounds().IntersectsSphere( TheNomad::SGame::EntityManager.GetActivePlayer().GetOrigin(), info.soundRange );
		}
	};
};