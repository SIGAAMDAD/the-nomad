namespace TheNomad::SGame {
	class ParticleManager {
		ParticleManager() {
		}

		const Effect@ GetEffect( const string& in name ) const {
			return cast<const Effect@>( @m_EffectTable[ name ] );
		}

		private dictionary m_EffectTable;
	};
};