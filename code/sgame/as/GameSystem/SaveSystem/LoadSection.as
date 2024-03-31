namespace TheNomad::GameSystem::SaveSystem {
    class LoadSection {
		LoadSection( const string& in name ) {
			handle = FindSaveSection( name );
		}

		bool Found() const {
			return handle != FS_INVALID_HANDLE;
		}

		vec2 LoadVec2( const string& in name ) const {
			vec2 value;

			LoadSection section( name );
			if ( !section.Found() ) {
				return vec2( 0.0f );
			}
			value.x = section.LoadFloat( name + ".x" );
			value.y = section.LoadFloat( name + ".y" );

			return value;
		}
		vec3 LoadVec3( const string& in name ) const {
			vec3 value;

			LoadSection section( name );
			if ( !section.Found() ) {
				return vec3( 0.0f );
			}
			value.x = section.LoadFloat( name + ".x" );
			value.y = section.LoadFloat( name + ".y" );
			value.z = section.LoadFloat( name + ".z" );

			return value;
		}
        vec4 LoadVec4( const string& in name ) const {
            vec4 value;

			LoadSection section( name );
			if ( !section.Found() ) {
				return vec4( 0.0f );
			}
			value.r = section.LoadFloat( name + ".r" );
			value.g = section.LoadFloat( name + ".g" );
			value.b = section.LoadFloat( name + ".b" );
            value.a = section.LoadFloat( name + ".a" );

			return value;
        }

		float LoadFloat( const string& in name ) const {
			return TheNomad::GameSystem::LoadFloat( name, handle );
		}

		uint8 LoadByte( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt8( name, handle );
		}
		uint16 LoadUShort( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt16( name, handle );
		}
		uint32 LoadUInt( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt32( name, handle );
		}
		uint64 LoadULong( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt64( name, handle );
		}

		uint8 LoadUInt8( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt8( name, handle );
		}
		uint16 LoadUInt16( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt16( name, handle );
		}
		uint32 LoadUInt32( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt32( name, handle );
		}
		uint64 LoadUInt64( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt64( name, handle );
		}

		int8 LoadChar( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt8( name, handle );
		}
		int16 LoadShort( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt16( name, handle );
		}
		int32 LoadInt( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt32( name, handle );
		}
		int64 LoadLong( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt64( name, handle );
		}

		int8 LoadInt8( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt8( name, handle );
		}
		int16 LoadInt16( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt16( name, handle );
		}
		int32 LoadInt32( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt32( name, handle );
		}
		int64 LoadInt64( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt64( name, handle );
		}

		private int handle;
	};
};