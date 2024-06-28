#include "snd_local.h"

/*
===============
CSoundShader::Init
===============
*/
void CSoundShader::Init( void ) {
	desc = "<no description>";
	errorDuringParse = false;
	onDemand = false;
	numEntries = 0;
	numLeadins = 0;
	leadinVolume = 0;
	altSound = NULL;
}

/*
===============
CSoundShader::CSoundShader
===============
*/
CSoundShader::CSoundShader( void ) {
	Init();
}

/*
===============
CSoundShader::~CSoundShader
===============
*/
CSoundShader::~CSoundShader() {
}

/*
=================
CSoundShader::Size
=================
*/
size_t CSoundShader::Size( void ) const {
	return sizeof( CSoundShader );
}

/*
===============
CSoundShader::FreeData
===============
*/
void CSoundShader::FreeData( void ) {
	numEntries = 0;
	numLeadins = 0;
}

const char *CSoundShader::GetName( void ) const {
    return name;
}

const char *CSoundShader::GetText( void ) const {
    return text;
}

void CSoundShader::SetText( const char *_text )
{
    if ( text ) {
        Z_Free( text );
    }

    nLength = strlen( _text ) + 1;
    text = (char *)Z_Malloc( nLength, TAG_SFX );
    memcpy( text, _text, nLength );
}

/*
===================
CSoundShader::SetDefaultText
===================
*/
bool CSoundShader::SetDefaultText( void ) {
    char wavname[MAX_NPATH];

    N_strncpyz( wavname, GetName(), sizeof( wavname ) - 1 );
	COM_DefaultExtension( wavname, sizeof( wavname ) - 1, ".wav" ); // if the name has .ogg in it, that will stay

	// if there exists a wav file with the same name
	if ( 1 ) { //fileSystem->ReadFile( wavname, NULL ) != -1 ) {
		char generated[2048];
		Com_snprintf( generated, sizeof( generated ),
						"sound %s // IMPLICITLY GENERATED\n"
						"{\n"
						"\t%s\n"
						"}\n", GetName(), wavname );
		SetText( generated );
		return true;
	} else {
		return false;
	}
}

/*
===================
DefaultDefinition
===================
*/
const char *CSoundShader::DefaultDefinition( void ) const {
	return
		"{\n"
	"\t"	"_default.wav\n"
		"}";
}

/*
===============
CSoundShader::Parse

  this is called by the declManager
===============
*/
bool CSoundShader::Parse( const char *text, const uint64_t textLength ) {
    const char **text_p, *buf;

    text_p = (const char **)&text;

	// deeper functions can set this, which will cause MakeDefault() to be called at the end
	errorDuringParse = false;

	if ( !ParseShader( src ) || errorDuringParse ) {
		MakeDefault();
		return false;
	}
	return true;
}

/*
===============
CSoundShader::ParseShader
===============
*/
bool CSoundShader::ParseShader( idLexer &src ) {
	int			i;
	idToken		token;

	parms.minDistance = 1;
	parms.maxDistance = 10;
	parms.volume = 1;
	parms.shakes = 0;
	parms.soundShaderFlags = 0;
	parms.soundClass = 0;

	speakerMask = 0;
	altSound = NULL;

	for( i = 0; i < SOUND_MAX_LIST_WAVS; i++ ) {
		leadins[i] = NULL;
		entries[i] = NULL;
	}
	numEntries = 0;
	numLeadins = 0;

	int	maxSamples = idSoundSystemLocal::s_maxSoundsPerShader.GetInteger();
	if ( com_makingBuild.GetBool() || maxSamples <= 0 || maxSamples > SOUND_MAX_LIST_WAVS ) {
		maxSamples = SOUND_MAX_LIST_WAVS;
	}

	while ( 1 ) {
		if ( !src.ExpectAnyToken( &token ) ) {
			return false;
		}
		// end of definition
		else if ( token == "}" ) {
			break;
		}
		// minimum number of sounds
		else if ( !token.Icmp( "minSamples" ) ) {
			maxSamples = idMath::ClampInt( src.ParseInt(), SOUND_MAX_LIST_WAVS, maxSamples );
		}
		// description
		else if ( !token.Icmp( "description" ) ) {
			src.ReadTokenOnLine( &token );
			desc = token.c_str();
		}
		// mindistance
		else if ( !token.Icmp( "mindistance" ) ) {
			parms.minDistance = src.ParseFloat();
		}
		// maxdistance
		else if ( !token.Icmp( "maxdistance" ) ) {
			parms.maxDistance = src.ParseFloat();
		}
		// shakes screen
		else if ( !token.Icmp( "shakes" ) ) {
			src.ExpectAnyToken( &token );
			if ( token.type == TT_NUMBER ) {
				parms.shakes = token.GetFloatValue();
			} else {
				src.UnreadToken( &token );
				parms.shakes = 1.0f;
			}
		}
		// reverb
		else if ( !token.Icmp( "reverb" ) ) {
			src.ParseFloat();
			if ( !src.ExpectTokenString( "," ) ) {
				src.FreeSource();
				return false;
			}
			src.ParseFloat();
			// no longer supported
		}
		// volume
		else if ( !token.Icmp( "volume" ) ) {
			parms.volume = src.ParseFloat();
		}
		// leadinVolume is used to allow light breaking leadin sounds to be much louder than the broken loop
		else if ( !token.Icmp( "leadinVolume" ) ) {
			leadinVolume = src.ParseFloat();
		}
		// speaker mask
		else if ( !token.Icmp( "mask_center" ) ) {
			speakerMask |= 1<<SPEAKER_CENTER;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_left" ) ) {
			speakerMask |= 1<<SPEAKER_LEFT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_right" ) ) {
			speakerMask |= 1<<SPEAKER_RIGHT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_backright" ) ) {
			speakerMask |= 1<<SPEAKER_BACKRIGHT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_backleft" ) ) {
			speakerMask |= 1<<SPEAKER_BACKLEFT;
		}
		// speaker mask
		else if ( !token.Icmp( "mask_lfe" ) ) {
			speakerMask |= 1<<SPEAKER_LFE;
		}
		// soundClass
		else if ( !token.Icmp( "soundClass" ) ) {
			parms.soundClass = src.ParseInt();
			if ( parms.soundClass < 0 || parms.soundClass >= SOUND_MAX_CLASSES ) {
				src.Warning( "SoundClass out of range" );
				return false;
			}
		}
		// altSound
		else if ( !token.Icmp( "altSound" ) ) {
			if ( !src.ExpectAnyToken( &token ) ) {
				return false;
			}
			altSound = declManager->FindSound( token.c_str() );
		}
		// ordered
		else if ( !token.Icmp( "ordered" ) ) {
			// no longer supported
		}
		// no_dups
		else if ( !token.Icmp( "no_dups" ) ) {
			parms.soundShaderFlags |= SSF_NO_DUPS;
		}
		// no_flicker
		else if ( !token.Icmp( "no_flicker" ) ) {
			parms.soundShaderFlags |= SSF_NO_FLICKER;
		}
		// plain
		else if ( !token.Icmp( "plain" ) ) {
			// no longer supported
		}
		// looping
		else if ( !token.Icmp( "looping" ) ) {
			parms.soundShaderFlags |= SSF_LOOPING;
		}
		// no occlusion
		else if ( !token.Icmp( "no_occlusion" ) ) {
			parms.soundShaderFlags |= SSF_NO_OCCLUSION;
		}
		// private
		else if ( !token.Icmp( "private" ) ) {
			parms.soundShaderFlags |= SSF_PRIVATE_SOUND;
		}
		// antiPrivate
		else if ( !token.Icmp( "antiPrivate" ) ) {
			parms.soundShaderFlags |= SSF_ANTI_PRIVATE_SOUND;
		}
		// once
		else if ( !token.Icmp( "playonce" ) ) {
			parms.soundShaderFlags |= SSF_PLAY_ONCE;
		}
		// global
		else if ( !token.Icmp( "global" ) ) {
			parms.soundShaderFlags |= SSF_GLOBAL;
		}
		// unclamped
		else if ( !token.Icmp( "unclamped" ) ) {
			parms.soundShaderFlags |= SSF_UNCLAMPED;
		}
		// omnidirectional
		else if ( !token.Icmp( "omnidirectional" ) ) {
			parms.soundShaderFlags |= SSF_OMNIDIRECTIONAL;
		}
		// onDemand can't be a parms, because we must track all references and overrides would confuse it
		else if ( !token.Icmp( "onDemand" ) ) {
			// no longer loading sounds on demand
			//onDemand = true;
		}

		// the wave files
		else if ( !token.Icmp( "leadin" ) ) {
			// add to the leadin list
			if ( !src.ReadToken( &token ) ) {
				src.Warning( "Expected sound after leadin" );
				return false;
			}
			if ( soundSystemLocal.soundCache && numLeadins < maxSamples ) {
				leadins[ numLeadins ] = soundSystemLocal.soundCache->FindSound( token.c_str(), onDemand );
				numLeadins++;
			}
		} else if ( token.Find( ".wav", false ) != -1 || token.Find( ".ogg", false ) != -1 ) {
			// add to the wav list
			if ( soundSystemLocal.soundCache && numEntries < maxSamples ) {
				token.BackSlashesToSlashes();
				idStr lang = cvarSystem->GetCVarString( "sys_lang" );
				if ( lang.Icmp( "english" ) != 0 && token.Find( "sound/vo/", false ) >= 0 ) {
					idStr work = token;
					work.ToLower();
					work.StripLeading( "sound/vo/" );
					work = va( "sound/vo/%s/%s", lang.c_str(), work.c_str() );
					if ( fileSystem->ReadFile( work, NULL, NULL ) > 0 ) {
						token = work;
					} else {
						// also try to find it with the .ogg extension
						work.SetFileExtension( ".ogg" );
						if ( fileSystem->ReadFile( work, NULL, NULL ) > 0 ) {
							token = work;
						}
					}
				}
				entries[ numEntries ] = soundSystemLocal.soundCache->FindSound( token.c_str(), onDemand );
				numEntries++;
			}
		} else {
			src.Warning( "unknown token '%s'", token.c_str() );
			return false;
		}
	}

	if ( parms.shakes > 0.0f ) {
		CheckShakesAndOgg();
	}

	return true;
}

/*
===============
CSoundShader::CheckShakesAndOgg
===============
*/
bool CSoundShader::CheckShakesAndOgg( void ) const {
	int i;
	bool ret = false;

	for ( i = 0; i < numLeadins; i++ ) {
		if ( leadins[ i ]->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) {
			common->Warning( "sound shader '%s' has shakes and uses OGG file '%s'",
								GetName(), leadins[ i ]->name.c_str() );
			ret = true;
		}
	}
	for ( i = 0; i < numEntries; i++ ) {
		if ( entries[ i ]->objectInfo.wFormatTag == WAVE_FORMAT_TAG_OGG ) {
			common->Warning( "sound shader '%s' has shakes and uses OGG file '%s'",
								GetName(), entries[ i ]->name.c_str() );
			ret = true;
		}
	}
	return ret;
}

/*
===============
CSoundShader::List
===============
*/
void CSoundShader::List() const {
	idStrList	shaders;

	common->Printf( "%4i: %s\n", Index(), GetName() );
	if ( idStr::Icmp( GetDescription(), "<no description>" ) != 0 ) {
		common->Printf( "      description: %s\n", GetDescription() );
	}
	for( int k = 0; k < numLeadins ; k++ ) {
		const idSoundSample *objectp = leadins[k];
		if ( objectp ) {
			common->Printf( "      %5dms %4dKb %s (LEADIN)\n", soundSystemLocal.SamplesToMilliseconds(objectp->LengthIn44kHzSamples()), (objectp->objectMemSize/1024)
				,objectp->name.c_str() );
		}
	}
	for( int k = 0; k < numEntries; k++ ) {
		const idSoundSample *objectp = entries[k];
		if ( objectp ) {
			common->Printf( "      %5dms %4dKb %s\n", soundSystemLocal.SamplesToMilliseconds(objectp->LengthIn44kHzSamples()), (objectp->objectMemSize/1024)
				,objectp->name.c_str() );
		}
	}
}

/*
===============
CSoundShader::GetAltSound
===============
*/
const CSoundShader *CSoundShader::GetAltSound( void ) const {
	return altSound;
}

/*
===============
CSoundShader::GetMinDistance
===============
*/
float CSoundShader::GetMinDistance() const {
	return parms.minDistance;
}

/*
===============
CSoundShader::GetMaxDistance
===============
*/
float CSoundShader::GetMaxDistance() const {
	return parms.maxDistance;
}

/*
===============
CSoundShader::GetDescription
===============
*/
const char *CSoundShader::GetDescription() const {
	return desc;
}

/*
===============
CSoundShader::HasDefaultSound
===============
*/
bool CSoundShader::HasDefaultSound() const {
	for ( int i = 0; i < numEntries; i++ ) {
		if ( entries[i] && entries[i]->defaultSound ) {
			return true;
		}
	}
	return false;
}

/*
===============
CSoundShader::GetParms
===============
*/
const soundShaderParms_t *CSoundShader::GetParms() const {
	return &parms;
}

/*
===============
CSoundShader::GetNumSounds
===============
*/
int CSoundShader::GetNumSounds() const {
	return numLeadins + numEntries;
}

/*
===============
CSoundShader::GetSound
===============
*/
const char *CSoundShader::GetSound( int index ) const {
	if ( index >= 0 ) {
		if ( index < numLeadins ) {
			return leadins[index]->name.c_str();
		}
		index -= numLeadins;
		if ( index < numEntries ) {
			return entries[index]->name.c_str();
		}
	}
	return "";
}
