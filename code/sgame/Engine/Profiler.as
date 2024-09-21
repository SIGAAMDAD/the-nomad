namespace TheNomad::Engine {
	class ProfileBlock {
		ProfileBlock( const string& in name ) {
			ProfileBlockBegin( name );
		}
		~ProfileBlock() {
			ProfileBlockEnd();
		}
	};
};