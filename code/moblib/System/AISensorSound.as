namespace moblib {
	class AISensorSound {
		AISensorSound() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) {
			if ( @mob.GetTarget() is null ) {
				return false;
			}

			const TheNomad::SGame::InfoSystem::MobInfo@ info = cast<TheNomad::SGame::InfoSystem::MobInfo@>( @mob.GetInfo() );
			
			const float tolerance = info.soundTolerance;
			
			return mob.GetTarget().GetBounds().IntersectsSphere( mob.GetOrigin(), info.soundRadius );
		}
	};
};