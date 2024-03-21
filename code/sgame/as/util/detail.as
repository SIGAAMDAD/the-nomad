namespace TheNomad::Util {
	class Convert {
		Convert() {
		}
	};

	int Clamp( int value, int min, int max ) {
		if ( value > max ) {
			return max;
		} else if ( value < min ) {
			return min;
		}
		return value;
	}
	
	uint Clamp( uint value, uint min, uint max ) {
		if ( value > max ) {
			return max;
		} else if ( value < min ) {
			return min;
		}
		return value;
	}

	float Clamp( float value, float min, float max ) {
		if ( value > max ) {
			return max;
		} else if ( value < min ) {
			return min;
		}
		return value;
	}

	int Hex( char c ) {
		if ( c >= '0' && c <= '9' ) {
			return c - '0';
		} else if ( c >= 'A' && c <= 'F' ) {
			return 10 + c - 'A';
		} else if ( c >= 'a' && c <= 'f' ) {
			return 10 + c - 'a';
		}

		return -1;
	}

	int HexStringToInt( const string& in str )
	{
		if ( str.size() < 1 ) {
			return -1;
		}

		// check for hex code
		if ( str[ 0 ] == '0' && str[ 1 ] == 'x' && str[ 2 ] != '\0' ) {
		    int i, digit, n = 0, len = str.size();

			for ( i = 2; i < len; i++ ) {
				n *= 16;

				digit = Hex( str[ i ] );

				if ( digit < 0 ) {
					return -1;
				}

				n += digit;
			}

			return n;
		}

		return -1;
	}

	bool GetHashColor( const string& in str, vec3& out color )
	{
		uint i, len;
		int[] hex( 6 );

		color[0] = color[1] = color[2] = 0;

		if ( str[1] != '#' ) {
			return false;
		}

		len = str.size();
		if ( len <= 0 || len > 6 ) {
			return false;
		}

		for ( i = 1; i < len; i++ ) {
			hex[i] = Hex( str[i] );
			if ( hex[i] < 0 ) {
				return false;
			}
		}

		switch ( len ) {
		case 3: // #rgb
			color[0] = hex[0] << 4 | hex[0];
			color[1] = hex[1] << 4 | hex[1];
			color[2] = hex[2] << 4 | hex[2];
			break;
		case 6: // #rrggbb
			color[0] = hex[0] << 4 | hex[1];
			color[1] = hex[2] << 4 | hex[3];
			color[2] = hex[4] << 4 | hex[5];
			break;
		default: // unsupported format
			return false;
		};

		return true;
	}
	
	bool IntToBool( int64 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint64 i ) {
		return i == 1 ? true : false;
	}
	
	bool IntToBool( int32 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint32 i ) {
		return i == 1 ? true : false;
	}

	bool IntToBool( int16 i ) {
		return i == 1 ? true : false;
	}

	bool UIntToBool( uint16 i ) {
		return i == 1 ? true : false;
	}

	bool StringToBool( const string& in str ) {
		return StrICmp( str, "true" ) == 0 ? true : false;
	}

	float DEG2RAD( float x ) {
		return ( ( x * M_PI ) / 180.0F );
	}

	float RAD2DEG( float x ) {
		return ( ( x * 180.0f ) / M_PI );
	}

	//
	// Dir2Angle: returns absolute degrees
	//
	float Dir2Angle( TheNomad::GameSystem::DirType dir ) {
		switch ( dir ) {
		case TheNomad::GameSystem::DirType::North:
			return 0.0f;
		case TheNomad::GameSystem::DirType::NorthEast:
			return 45.0f;
		case TheNomad::GameSystem::DirType::East:
			return 90.0f;
		case TheNomad::GameSystem::DirType::SouthEast:
			return 135.0f;
		case TheNomad::GameSystem::DirType::South:
			return 180.0f;
		case TheNomad::GameSystem::DirType::SouthWest:
			return 225.0f;
		case TheNomad::GameSystem::DirType::West:
			return 270.0f;
		case TheNomad::GameSystem::DirType::NorthWest:
			return 315.0f;
		default:
			GameError( "Dir2Angle: invalid dir " + formatUInt( dir ) );
		};
		return -1.0f;
	}

	//
	// Angle2Dir:
	//
	TheNomad::GameSystem::DirType Angle2Dir( float angle ) {
		if ( angle >= 337.5f && angle <= 22.5f ) {
			return TheNomad::GameSystem::DirType::North;
		} else if ( angle >= 22.5f && angle <= 67.5f ) {
			return TheNomad::GameSystem::DirType::NorthEast;
		} else if ( angle >= 67.5f && angle <= 112.5f ) {
			return TheNomad::GameSystem::DirType::East;
		} else if ( angle >= 112.5f && angle <= 157.5f ) {
			return TheNomad::GameSystem::DirType::SouthEast;
		} else if ( angle >= 157.5f && angle <= 202.5f ) {
			return TheNomad::GameSystem::DirType::South;
		} else if ( angle >= 202.5f && angle <= 247.5f ) {
			return TheNomad::GameSystem::DirType::SouthWest;
		} else if ( angle >= 247.5f && angle <= 292.5f ) {
			return TheNomad::GameSystem::DirType::West;
		} else if ( angle >= 292.5f && angle <= 337.5f ) {
			return TheNomad::GameSystem::DirType::NorthWest;
		} else {
			DebugPrint( "Angle2Dir: funny angle " + formatFloat( angle ) + "\n" );
		}
		return TheNomad::GameSystem::DirType::North;
	}
};