#include "g_game.h"
#include "g_sound.h"
#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <ALsoft/alext.h>
#include <sndfile.h>
#include "../module_lib/module_memory.h"
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API // we're using the pulldata API
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include "stb_vorbis.c"

// AL_EXT_STATIC_BUFFER
static PFNALBUFFERDATASTATICPROC alBufferDataStatic;

// ALC_SOFT_HRTF
static LPALCRESETDEVICESOFT alcResetDeviceSOFT;

#define MAX_SOUND_SOURCES 2048
#define MAX_MUSIC_QUEUE 12

#define CLAMP_VOLUME 10.0f

cvar_t *snd_noSound;
cvar_t *snd_force22kHz;
cvar_t *snd_musicVolume;
cvar_t *snd_effectsVolume;
cvar_t *snd_musicOn;
cvar_t *snd_effectsOn;
cvar_t *snd_masterVolume;
cvar_t *snd_debugPrint;
cvar_t *snd_muteUnfocused;
cvar_t *snd_device;

//static idDynamicBlockAlloc<byte, 1<<20, 1<<10> soundCacheAllocator;

#define Snd_HashFileName(x) Com_GenerateHashValue((x),MAX_SOUND_SOURCES)

#define SNDBUF_FLOAT 0
#define SNDBUF_DOUBLE 1
#define SNDBUF_16BIT 2
#define SNDBUF_8BIT 3

#define ALCall(x) x; AL_CheckError(#x)

static void AL_CheckError( const char *op )
{
    ALenum error;

    error = alGetError();
    switch ( error ) {
    case AL_OUT_OF_MEMORY:
    #ifdef _NOMAD_DEBUG
        N_Error( ERR_FATAL, "alGetError() -- 0x%04x, AL_OUT_OF_MEMORY after '%s'", AL_OUT_OF_MEMORY, op );
    #else
        Con_Printf( COLOR_RED "WARNING: alGetError() -- 0x%04x, AL_OUT_OF_MEMORY after '%s'\n", AL_OUT_OF_MEMORY, op );
        Con_Printf( "Dumping stacktrace of 64 frames...\n" );
        Sys_DebugStacktrace( 64 );
    #endif
        break;
    case AL_INVALID_ENUM:
    #ifdef _NOMAD_DEBUG
        N_Error( ERR_FATAL, "alGetError() -- 0x%04x, AL_ILLEGAL_ENUM after '%s'", AL_INVALID_ENUM, op );
    #else
        Con_Printf( COLOR_RED "WARNING: alGetError() -- 0x%04x, AL_INVALID_ENUM after '%s'\n", AL_INVALID_ENUM, op );
        Con_Printf( "Dumping stacktrace of 64 frames...\n" );
        Sys_DebugStacktrace( 64 );
    #endif
        break;
    case AL_INVALID_OPERATION:
    #ifdef _NOMAD_DEBUG
        N_Error( ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_OPERATION after '%s'", AL_INVALID_OPERATION, op );
    #else
        Con_Printf( COLOR_RED "WARNING: alGetError() -- 0x%04x, AL_INVALID_OPERATION after '%s'\n", AL_INVALID_OPERATION, op );
        Con_Printf( "Dumping stacktrace of 64 frames...\n" );
        Sys_DebugStacktrace( 64 );
    #endif
        break;
    case AL_INVALID_VALUE:
    #ifdef _NOMAD_DEBUG
        N_Error( ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_VALUE after '%s'", AL_INVALID_VALUE, op );
    #else
        Con_Printf( COLOR_RED "WARNING: alGetError() -- 0x%04x, AL_INVALID_VALUE after '%s'\n", AL_INVALID_VALUE, op );
        Con_Printf( "Dumping stacktrace of 64 frames...\n" );
        Sys_DebugStacktrace( 64 );
    #endif
        break;
    case AL_INVALID_NAME:
    #ifdef _NOMAD_DEBUG
        N_Error( ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_NAME after '%s'", AL_INVALID_NAME, op );
    #else
        Con_Printf( COLOR_RED "WARNING: alGetError() -- 0x%04x, AL_INVALID_NAME after '%s'\n", AL_INVALID_NAME, op );
        Con_Printf( "Dumping stacktrace of 64 frames...\n" );
        Sys_DebugStacktrace( 64 );
    #endif
        break;
    case AL_NO_ERROR:
    default:
        if ( snd_debugPrint->i ) {
            Con_DPrintf( "OpenAL operation '%s' all good.\n", op );
        }
        break;
    };
}

class CSoundSource
{
public:
    CSoundSource( void );
    ~CSoundSource();

    void Init( void );
    void Shutdown( void );
    bool LoadFile( const char *npath, int64_t tag );

    void SetVolume( void ) const;

    bool IsPlaying( void ) const;
    bool IsPaused( void ) const;
    bool IsLooping( void ) const;

    const char *GetName( void ) const;
    uint32_t GetTag( void ) const { return m_iTag; }

    inline uint32_t GetSource( void ) const { return m_iSource; }
    inline uint32_t GetBuffer( void ) const { return m_iBuffer; }
    inline void SetSource( uint32_t source ) { m_iSource = source; }
    inline const SF_INFO& GetInfo( void ) const { return m_hFData; }

    void Play( bool loop = false );
    void Stop( void );
    void Pause( void );

    CSoundSource *m_pNext;
private:
    int64_t FileFormat( const char *ext ) const;
    void Alloc( void );
    ALenum Format( void ) const;
    void CheckForDownSample( void );

    char m_pName[MAX_NPATH];

    byte *m_pNonCacheData;

    uint32_t m_iType;
    uint32_t m_iTag;
    uint32_t m_nObjectSize;
    uint32_t m_nObjectMemSize;

    // AL data
    ALuint m_iSource;
    ALuint m_iBuffer;

    qboolean m_bLoop;

    SF_INFO m_hFData;
};

typedef struct trackQueue_s {
    struct trackQueue_s *next;
    struct trackQueue_s *prev;
    CSoundSource *track;
    qboolean used;
} trackQueue_t;

class CSoundManager
{
public:
    CSoundManager( void );
    ~CSoundManager();

    void Init( void );
    void Shutdown( void );
    void Restart( void );

    void PlaySound( CSoundSource *snd );
    void StopSound( CSoundSource *snd );
    void LoopTrack( CSoundSource *snd );
    void DisableSounds( void );
    void Update( int64_t msec );
    bool CheckDeviceAndRecoverIfNeeded( void );

    inline CSoundSource **GetSources( void ) { return m_pSources; }
    inline void Mute( bool bMute ) {
        uint64_t i;

        if ( !snd_musicOn->i && !snd_effectsOn->i ) {
            return;
        }
        if ( bMute && !m_bMuted ) {
            for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
                if ( m_pSources[i] ) {
                    alSourcef( m_pSources[i]->GetSource(), AL_GAIN, 0.0f );
                }
            }
            Con_Printf( "Muted sounds\n" );
        } else if ( !bMute && m_bMuted ) {
            for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
                if ( m_pSources[i] ) {
                    switch ( m_pSources[i]->GetTag() ) {
                    case TAG_MUSIC:
                        alSourcef( m_pSources[i]->GetSource(), AL_GAIN, snd_musicVolume->f / 100.0f );
                    case TAG_SFX:
                        alSourcef( m_pSources[i]->GetSource(), AL_GAIN, snd_effectsVolume->f / 100.0f );
                        break;
                    };
                }
            }
        }
        m_bMuted = bMute;
    }

    void UpdateParm( int64_t tag );
    uint32_t GetMusicSource( void ) const { return m_iMusicSource; }

    void AddSourceToHash( CSoundSource *src );
    CSoundSource *InitSource( const char *filename, int64_t tag );

    inline uint64_t NumSources( void ) const { return m_nSources; }
    inline CSoundSource *&GetSource( sfxHandle_t handle ) { return m_pSources[handle]; }
    inline const CSoundSource *GetSource( sfxHandle_t handle ) const { return m_pSources[handle]; }
    inline void SetListenerPos( const vec3_t origin ) { VectorCopy( m_ListenerPosition, origin ); }

    CSoundSource *m_pCurrentTrack, *m_pQueuedTrack;

    CThreadMutex m_hAllocLock;
    CThreadMutex m_hQueueLock;

    uint32_t m_nFirstLevelSource;
    uint32_t m_nLevelSources;

    eastl::fixed_vector<CSoundSource *, 10> m_LoopingTracks;
private:
    CSoundSource *m_pSources[MAX_SOUND_SOURCES];
    uint64_t m_nSources;

    uint64_t m_nLastCheckTime;
    uint32_t m_nResetRetryCount;

    vec3_t m_ListenerPosition;

    ALCdevice *m_pDevice;
    ALCcontext *m_pContext;

    uint32_t m_iMusicSource;

    qboolean m_bClearedQueue;
    qboolean m_bRegistered;

    qboolean m_bMuted;
};

static CSoundManager *sndManager;

CSoundSource::CSoundSource( void ) {
    Init();
}

CSoundSource::~CSoundSource() {
    Shutdown();
}

const char *CSoundSource::GetName( void ) const {
    return m_pName;
}

bool CSoundSource::IsPaused( void ) const {
    ALint state;
    alGetSourcei( m_iSource, AL_SOURCE_STATE, &state );
    return state == AL_PAUSED;
}

bool CSoundSource::IsPlaying( void ) const {
    ALint state;
    alGetSourcei( m_iSource, AL_SOURCE_STATE, &state );
    return state == AL_PLAYING;
}

bool CSoundSource::IsLooping( void ) const {
    ALint state;
    alGetSourcei( m_iSource, AL_SOURCE_STATE, &state );
    return state == AL_LOOPING;
}

void CSoundSource::Init( void )
{
    memset( m_pName, 0, sizeof( m_pName ) );
    memset( &m_hFData, 0, sizeof( m_hFData ) );

    // this could be the music source, so don't allocate a new redundant source just yet
    m_iSource = 0;

    if ( m_iBuffer == 0 ) {
        ALCall( alGenBuffers( 1, &m_iBuffer ) );
    }
    m_iType = 0;
    m_bLoop = false;
}

void CSoundSource::Shutdown( void )
{
    ALint state;
    ALenum err;

    if ( alIsBuffer( m_iBuffer ) ) {
        if ( alIsSource( m_iSource ) ) {
            alGetSourcei( m_iSource, AL_SOURCE_STATE, &state );
            if ( state == AL_PLAYING ) {
                alSourceStop( m_iSource );
            }
            alSourcei( m_iSource, AL_BUFFER, AL_NONE );
        }
        alDeleteBuffers( 1, &m_iBuffer );
        if ( ( err = alGetError() ) != AL_NO_ERROR ) {
            Con_Printf( COLOR_YELLOW "WARNING: Error deallocating OpenAL hardware buffer: 0x%04x\n", err );
        }
    }
    if ( alIsSource( m_iSource ) && m_iTag != TAG_MUSIC ) {
        alDeleteSources( 1, &m_iSource );
        if ( ( err = alGetError() ) != AL_NO_ERROR ) {
            Con_Printf( COLOR_YELLOW "WARNING: Error deallocating OpenAL hardware source: 0x%04x\n", err );
        }
    }

    m_iBuffer = 0;
    m_iSource = 0;
    m_iType = 0;
}


void CSoundSource::SetVolume( void ) const {
    if ( m_iTag == TAG_MUSIC || m_iSource == 0 ) {
        return;
    }
    alSourcef( m_iSource, AL_GAIN, snd_effectsVolume->f / CLAMP_VOLUME );
}

ALenum CSoundSource::Format( void ) const {
    return m_hFData.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
}

int64_t CSoundSource::FileFormat( const char *ext ) const
{
    if ( !N_stricmp( ext, "wav" ) ) {
        return SF_FORMAT_WAV;
    } else if ( !N_stricmp( ext, "aiff" ) ) {
        return SF_FORMAT_AIFF;
    } else if ( !N_stricmp( ext, "ogg" ) ) {
        return SF_FORMAT_OGG;
    } else if ( !N_stricmp( ext, "opus" ) ) {
        return SF_FORMAT_OPUS;
    } else if ( !N_stricmp( ext, "flac" ) ) {
        return SF_FORMAT_FLAC;
    } else if ( !N_stricmp( ext, "sd2" ) ) {
        return SF_FORMAT_SD2;
    } else {
        Con_Printf( COLOR_YELLOW "WARNING: unknown audio file format extension '%s', refusing to load\n", ext );
        return 0;
    }
}

static inline qboolean IsPCMFormat( int format )
{
    switch ( format & SF_FORMAT_SUBMASK ) {
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_PCM_16:
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
        return qtrue;
    default:
        break;
    };
    return qfalse;
}

void CSoundSource::CheckForDownSample( void )
{
    if ( !snd_force22kHz->i ) {
		return;
	}
	if ( !IsPCMFormat( m_hFData.format ) || m_hFData.samplerate != 44100 ) {
		return;
	}

	const int shortSamples = m_nObjectSize >> 1;
	short *converted = (short *)Z_Malloc( shortSamples * sizeof( short ), (memtag_t)m_iTag );

	if ( m_hFData.channels == 1 ) {
		for ( int i = 0; i < shortSamples; i++ ) {
			converted[i] = ((short *)m_pNonCacheData)[i*2];
		}
	} else {
		for ( int i = 0; i < shortSamples; i += 2 ) {
			converted[i+0] = ((short *)m_pNonCacheData)[i*2+0];
			converted[i+1] = ((short *)m_pNonCacheData)[i*2+1];
		}
	}
    Z_Free( m_pNonCacheData );
	m_pNonCacheData = (byte *)converted;
	m_nObjectSize >>= 1;
	m_nObjectMemSize >>= 1;
	m_hFData.samplerate >>= 1;
}

void CSoundSource::Alloc( void )
{
    uint64_t nLength;

    nLength = 0;
    switch ( m_hFData.format & SF_FORMAT_SUBMASK ) {
    case SF_FORMAT_ALAW:
        Con_Printf( COLOR_YELLOW "WARNING: alaw sound format not supported.\n" );
        break;
    case SF_FORMAT_ULAW:
        Con_Printf( COLOR_YELLOW "WARNING: ulaw sound format not supported.\n" );
        break;
    case SF_FORMAT_FLOAT:
        m_iType = SNDBUF_FLOAT;
        break;
    case SF_FORMAT_DOUBLE:
        m_iType = SNDBUF_DOUBLE;
        break;
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_DPCM_8:
        m_iType = SNDBUF_8BIT;
        break;
    case SF_FORMAT_ALAC_16:
    case SF_FORMAT_PCM_16:
    case SF_FORMAT_DPCM_16:
    case SF_FORMAT_DWVW_16:
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_NMS_ADPCM_16:
        m_iType = SNDBUF_16BIT;
        break;
    };
}

void CSoundSource::Play( bool loop )
{
    if ( ( IsLooping() && loop ) ) {
        return;
    }
    if ( loop ) {
        alSourcei( m_iSource, AL_LOOPING, AL_TRUE );
    }
    if ( m_iTag == TAG_SFX ) {
//        alSourcef( m_iSource, AL_GAIN, snd_effectsVolume->f / 100.0f );
    }
    alSourcePlay( m_iSource );
}

void CSoundSource::Pause( void ) {
    if ( !IsPlaying() && !IsLooping() ) {
        return;
    }
    alSourcePause( m_iSource );
}

void CSoundSource::Stop( void ) {
    if ( !IsPlaying() && !IsLooping() ) {
        return; // nothing's playing
    }
    if ( m_iTag == TAG_MUSIC ) {
        alSourcei( m_iSource, AL_BUFFER, AL_NONE );
    }
    alSourceStop( m_iSource );
}

static sf_count_t SndFile_Read( void *data, sf_count_t size, void *file ) {
    return FS_Read( data, size, (fileHandle_t)(uintptr_t)file );
}

static sf_count_t SndFile_Tell( void *file ) {
    return FS_FileTell( (fileHandle_t)(uintptr_t)file );
}

static sf_count_t SndFile_GetFileLen( void *file ) {
    return FS_FileLength( (fileHandle_t)(uintptr_t)file );
}

static sf_count_t SndFile_Seek( sf_count_t offset, int whence, void *file ) {
    fileHandle_t f = (fileHandle_t)(uintptr_t)file;
    switch ( whence ) {
    case SEEK_SET:
        return FS_FileSeek( f, (fileOffset_t)offset, FS_SEEK_SET );
    case SEEK_CUR:
        return FS_FileSeek( f, (fileOffset_t)offset, FS_SEEK_CUR );
    case SEEK_END:
        return FS_FileSeek( f, (fileOffset_t)offset, FS_SEEK_END );
    default:
        break;
    };
    N_Error( ERR_FATAL, "SndFile_Seek: bad whence" );
    return 0; // quiet compiler warning
}


static const char* my_stbv_strerror( int stbVorbisError )
{
	switch ( stbVorbisError ) {
	case VORBIS__no_error: return "No Error";
#define ERRCASE(X) \
	case VORBIS_ ## X : return #X;

	ERRCASE( need_more_data )    // not a real error

	ERRCASE( invalid_api_mixing )           // can't mix API modes
	ERRCASE( outofmem )                     // not enough memory
	ERRCASE( feature_not_supported )        // uses floor 0
	ERRCASE( too_many_channels )            // STB_VORBIS_MAX_CHANNELS is too small
	ERRCASE( file_open_failure )            // fopen() failed
	ERRCASE( seek_without_length )          // can't seek in unknown-length file

	ERRCASE( unexpected_eof )               // file is truncated?
	ERRCASE( seek_invalid )                 // seek past EOF

	// decoding errors (corrupt/invalid stream) -- you probably
	// don't care about the exact details of these

	// vorbis errors:
	ERRCASE( invalid_setup )
	ERRCASE( invalid_stream )

	// ogg errors:
	ERRCASE( missing_capture_pattern )
	ERRCASE( invalid_stream_structure_version )
	ERRCASE( continued_packet_flag_invalid )
	ERRCASE( incorrect_stream_serial_number )
	ERRCASE( invalid_first_page )
	ERRCASE( bad_packet_type )
	ERRCASE( cant_find_last_page )
	ERRCASE( seek_failed )
	ERRCASE( ogg_skeleton_not_supported )

#undef ERRCASE
	};
	Assert( 0 && "unknown stb_vorbis errorcode!" );
	return "Unknown Error!";
}

bool CSoundSource::LoadFile( const char *npath, int64_t tag )
{
    PROFILE_FUNCTION();

    SNDFILE *sf;
    SF_VIRTUAL_IO vio;
    ALenum format;
    fileHandle_t f;
    FILE *fp;
    void *buffer;
    uint64_t length;
    const char *ospath;
    short *data;

    m_iTag = tag;

    // clear audio file data before anything
    memset( &m_hFData, 0, sizeof( m_hFData ) );
    memset( &vio, 0, sizeof( vio ) );

    N_strncpyz( m_pName, npath, sizeof( m_pName ) );

    // hash it so that if we try loading it
    // even if it's failed, we won't try loading
    // it again
    sndManager->AddSourceToHash( this );

    length = FS_LoadFile( npath, &buffer );
    if ( !length || !buffer ) {
        Con_Printf( COLOR_RED "CSoundSource::LoadFile: failed to load file '%s'.\n", npath );
        return false;
    }

    fp = tmpfile();
    Assert( fp );

    /*
    vio.get_filelen = SndFile_GetFileLen;
    vio.write = NULL; // no need for this
    vio.read = SndFile_Read;
    vio.tell = SndFile_Tell;
    vio.seek = SndFile_Seek;

    sf = sf_open_virtual( &vio, SFM_READ, &m_hFData, (void *)(uintptr_t)f );
    if ( !sf ) {
        Con_Printf( COLOR_YELLOW "WARNING: libsndfile sf_open_virtual failed on '%s', sf_sterror(): %s\n", npath, sf_strerror( sf ) );
        return false;
    }
    */
    fwrite( buffer, length, 1, fp );
    fseek( fp, 0L, SEEK_SET );
    FS_FreeFile( buffer );
    
    sf = sf_open_fd( fileno( fp ), SFM_READ, &m_hFData, SF_FALSE );
    if ( !sf ) {
        Con_Printf( COLOR_YELLOW "WARNING: libsndfile sf_open_fd failed on '%s', sf_strerror(): %s\n", npath, sf_strerror( sf ) );
        return false;
    }

    m_nObjectSize = ( sizeof( short ) * 8 ) * m_hFData.channels;
    m_nObjectMemSize = m_nObjectSize * sizeof( short );
    
    // allocate the buffer
    Alloc();

//    data = (short *)soundCacheAllocator.Alloc( sizeof( short ) * m_hFData.channels * m_hFData.frames );
    data = (short *)Hunk_AllocateTempMemory( sizeof( short ) * m_hFData.channels * m_hFData.frames );
    if ( !sf_read_short( sf, data, m_hFData.channels * m_hFData.frames ) ) {
        N_Error( ERR_FATAL, "CSoundSource::LoadFile(%s): failed to read %lu bytes from audio stream, sf_strerror(): %s\n",
            m_pName, sizeof( short ) * m_hFData.channels * m_hFData.frames, sf_strerror( sf ) );
    }

    sf_close( sf );
    fclose( fp );

    format = Format();
    if ( format == 0 ) {
        Con_Printf( COLOR_RED "Bad soundfile format for '%s', refusing to load\n", npath );
        return false;
    }

    ALCall( alGenBuffers( 1, &m_iBuffer ) );

    // generate a brand new source for each individual sfx
    if ( tag == TAG_SFX && m_iSource == 0 ) {
        ALCall( alGenSources( 1, &m_iSource ) );
    }

    if ( alBufferDataStatic ) {
        ALCall( alBufferDataStatic( m_iBuffer, format, data, sizeof( short ) * m_hFData.channels * m_hFData.frames, m_hFData.samplerate ) );
    } else {
        ALCall( alBufferData( m_iBuffer, format, data, sizeof( short ) * m_hFData.channels * m_hFData.frames, m_hFData.samplerate ) );
    }

    if ( tag == TAG_SFX ) {
        ALCall( alSourcef( m_iSource, AL_GAIN, snd_effectsVolume->f ) );
        ALCall( alSourcei( m_iSource, AL_BUFFER, m_iBuffer ) );
    } else if ( tag == TAG_MUSIC ) {
        ALCall( alSourcef( m_iSource, AL_GAIN, snd_musicVolume->f ) );
        ALCall( alSourcei( m_iSource, AL_BUFFER, 0 ) );
    }

    Hunk_FreeTempMemory( data );
    if ( gi.mapLoaded && gi.state == GS_LEVEL ) {
        sndManager->m_nLevelSources++;
    }

    return true;
}

void CSoundManager::Init( void )
{
    memset( this, 0, sizeof( *this ) );

    // no point in initializing OpenAL if sound is disabled with snd_noSound
	if ( snd_noSound->i ) {
		Con_Printf( "Sound disabled with snd_noSound 1!\n" );
		m_pDevice = NULL;
		m_pContext = NULL;
        return;
	} else {
		// set up openal device and context
		Con_Printf( "Setup OpenAL device and context\n" );

		const char *device = snd_device->s;
		if ( strlen( device ) < 1 ) {
			device = NULL;
		} else if ( !N_stricmp( device, "default" ) ) {
			device = NULL;
        }

		if ( alcIsExtensionPresent( NULL, "ALC_ENUMERATE_ALL_EXT" ) ) {
			const char *devs = alcGetString( NULL, ALC_ALL_DEVICES_SPECIFIER );
			bool found = false;

			while ( devs && *devs ) {
				Con_Printf( "OpenAL: found device '%s'", devs );

				if ( device && !N_stricmp( devs, device ) ) {
					Con_Printf( " (ACTIVE)\n" );
					found = true;
				} else {
					Con_Printf( "\n" );
				}

				devs += strlen( devs ) + 1;
			}

			if ( device && !found ) {
				Con_Printf( "OpenAL: device %s not found, using default\n", device );
				device = NULL;
			}
		}

		m_pDevice = alcOpenDevice( device );
		if ( !m_pDevice && device ) {
			Con_Printf( "OpenAL: failed to open device '%s' (0x%x), trying default...\n", device, alGetError() );
			m_pDevice = alcOpenDevice( NULL );
		}

		// DG: handle the possibility that opening the default device or creating context failed
		if ( m_pDevice == NULL ) {
			Con_Printf( "OpenAL: failed to open default device (0x%x), disabling sound\n", alGetError() );
			m_pContext = NULL;
		} else {
			m_pContext = alcCreateContext( m_pDevice, NULL );
			if ( m_pContext == NULL ) {
				Con_Printf( "OpenAL: failed to create context (0x%x), disabling sound\n", alcGetError( m_pDevice ) );
				alcCloseDevice( m_pDevice );
				m_pDevice = NULL;
			}
		}
	}
    if ( !m_pDevice || !m_pContext ) {
        Cvar_Set( "snd_noSound", "1" );
        return;
    }

/*
    m_pDevice = alcOpenDevice( NULL );
    if ( !m_pDevice ) {
        N_Error( ERR_FATAL, "Snd_Init: failed to open OpenAL device" );
    }

    m_pContext = alcCreateContext( m_pDevice, NULL );
    if ( !m_pContext ) {
        N_Error( ERR_FATAL, "Snd_Init: failed to create OpenAL context, reason: %s", alcGetString( m_pDevice, alcGetError( m_pDevice ) ) );
    }
    */

    alcMakeContextCurrent( m_pContext );
    m_bRegistered = true;

    // generate the recyclable music source
    ALCall( alGenSources( 1, &m_iMusicSource ) );
    alSourcef( m_iMusicSource, AL_GAIN, snd_musicVolume->f / 100.0f );

    Con_Printf( "OpenAL vendor: %s\n", alGetString( AL_VENDOR ) );
    Con_Printf( "OpenAL renderer: %s\n", alGetString( AL_RENDERER ) );
    Con_Printf( "OpenAL version: %s\n", alGetString( AL_VERSION ) );

    if ( alcIsExtensionPresent( m_pDevice, "AL_EXT_STATIC_BUFFER" ) ) {
        Con_Printf( "AL_EXT_STATIC_BUFFER found\n" );
        alBufferDataStatic = (PFNALBUFFERDATASTATICPROC)alcGetProcAddress( m_pDevice, "alBufferDataStatic" );
    } else {
        Con_Printf( "AL_EXT_STATIC_BUFFER not found\n" );
        alBufferDataStatic = NULL;
    }

    if ( alcIsExtensionPresent( m_pDevice, "ALC_EXT_disconnect" ) && alcIsExtensionPresent( m_pDevice, "ALC_SOFT_HRTF" ) ) {
        Con_Printf( "ALC_EXT_disconnect and ALC_SOFT_HRTF found, resetting disconnected devices now possible\n" );
        alcResetDeviceSOFT = (LPALCRESETDEVICESOFT)alcGetProcAddress( m_pDevice, "alcResetDeviceSOFT" );
    } else {
        Con_Printf( "ALC_EXT_disconnect or ALC_SOFT_HRTF not found, resetting disconnected devices not possible\n" );
        alcResetDeviceSOFT = NULL;
    }

    gi.soundStarted = qtrue;
}

void CSoundManager::PlaySound( CSoundSource *snd ) {
    if ( gi.state == GS_LEVEL ) {
        vec3_t pos;
        alGetListenerfv( AL_POSITION, pos );
        alSource3f( snd->GetSource(), AL_POSITION, pos[0], pos[1], pos[2] );
    }
    snd->Play();
}

void CSoundManager::StopSound( CSoundSource *snd ) {
    snd->Stop();
}

void CSoundManager::Shutdown( void )
{
    uint64_t i;
    trackQueue_t *pTrack, *pNext;

    for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
        if ( !m_pSources[i] ) {
            continue;
        }
        m_pSources[i]->Shutdown();
        m_pSources[i] = NULL;
    }

    m_nSources = 0;
    m_bRegistered = false;
    memset( m_pSources, 0, sizeof( m_pSources ) );

    ALCall( alDeleteSources( 1, &m_iMusicSource ) );

    Z_FreeTags( TAG_SFX );
    Z_FreeTags( TAG_MUSIC );

    Cmd_RemoveCommand( "snd.setvolume" );
    Cmd_RemoveCommand( "snd.toggle" );
    Cmd_RemoveCommand( "snd.updatevolume" );
    Cmd_RemoveCommand( "snd.list_files" );
    Cmd_RemoveCommand( "snd.clear_tracks" );
    Cmd_RemoveCommand( "snd.play_sfx" );
    Cmd_RemoveCommand( "snd.queue_track" );
    Cmd_RemoveCommand( "snd.audio_info" );
    Cmd_RemoveCommand( "snd.startup_level" );
    Cmd_RemoveCommand( "snd.unload_level" );

    alcMakeContextCurrent( NULL );
    alcDestroyContext( m_pContext );
    alcCloseDevice( m_pDevice );

    gi.soundStarted = qfalse;

//    soundCacheAllocator.Shutdown();
}

void CSoundManager::AddSourceToHash( CSoundSource *src )
{
    nhandle_t hash;

    hash = Snd_HashFileName( src->GetName() );

    src->m_pNext = m_pSources[hash];
    m_pSources[hash] = src;
}

void CSoundManager::Restart( void )
{
    uint64_t i;

    // shutdown everything
    Shutdown();
    
    // re-init
    Init();
}

//
// CSoundManager::InitSource: allocates a new sound source with openal
// NOTE: even if sound and music is disabled, we'll still allocate the memory,
// we just won't play any of the sources
//
CSoundSource *CSoundManager::InitSource( const char *filename, int64_t tag )
{
    CSoundSource *src;
    nhandle_t hash;

    hash = Snd_HashFileName( filename );

    //
    // check if we already have it loaded
    //
    for ( src = m_pSources[hash]; src; src = src->m_pNext ) {
        if ( !N_stricmp( filename, src->GetName() ) ) {
            return src;
        }
    }

    if ( strlen( filename ) >= MAX_NPATH ) {
        Con_Printf( "CSoundManager::InitSource: name '%s' too long\n", filename );
        return NULL;
    }
    if ( m_nSources == MAX_SOUND_SOURCES ) {
        N_Error( ERR_DROP, "CSoundManager::InitSource: MAX_SOUND_SOURCES hit" );
    }

    m_hAllocLock.Lock();

    src = (CSoundSource *)Hunk_Alloc( sizeof( *src ), h_low );
    memset( src, 0, sizeof( *src ) );

    if ( tag == TAG_MUSIC ) {
        if ( !alIsSource( m_iMusicSource ) ) { // make absolutely sure its a valid source
            ALCall( alGenSources( 1, &m_iMusicSource ));
        }
        src->SetSource( m_iMusicSource );
    }

    if ( !src->LoadFile( filename, tag ) ) {
        Con_Printf( COLOR_YELLOW "WARNING: failed to load sound file '%s'\n", filename );
        m_hAllocLock.Unlock();
        return NULL;
    }

    src->SetVolume();

    m_hAllocLock.Unlock();

    return src;
}

void CSoundManager::UpdateParm( int64_t tag )
{
    for ( uint64_t i = 0; i < m_nSources; i++ ) {
        if ( m_pSources[i]->GetTag() == tag ) {
            m_pSources[i]->SetVolume();
        }
    }
}

void CSoundManager::DisableSounds( void ) {
    for ( uint64_t i = 0; i < MAX_SOUND_SOURCES; i++ ) {
        if ( !m_pSources[i] ) {
            continue;
        }
        m_pSources[i]->Shutdown();
    }
    memset( m_pSources, 0, sizeof( m_pSources ) );
}


/*
===============
CSoundManager::CheckDeviceAndRecoverIfNeeded

 DG: returns true if m_pDevice is still available,
     otherwise it will try to recover the device and return false while it's gone
     (display audio sound devices sometimes disappear for a few seconds when switching resolution)
===============
*/
bool CSoundManager::CheckDeviceAndRecoverIfNeeded( void )
{
	static const int maxRetries = 20;

	if ( alcResetDeviceSOFT == NULL ) {
		return true; // we can't check or reset, just pretend everything is fine..
	}

	unsigned int curTime = Sys_Milliseconds();
	if ( curTime - m_nLastCheckTime >= 1000 ) { // check once per second
		m_nLastCheckTime = curTime;

		ALCint connected; // ALC_CONNECTED needs ALC_EXT_disconnect (we check for that in Init())
		alcGetIntegerv( m_pDevice, ALC_CONNECTED, 1, &connected );
		if ( connected ) {
			m_nResetRetryCount = 0;
			return true;
		}

		if ( m_nResetRetryCount == 0 ) {
			Con_Printf( COLOR_YELLOW "WARNING: OpenAL device disconnected! Will try to reconnect.." );
			m_nResetRetryCount = 1;
		} else if ( m_nResetRetryCount > maxRetries ) { // give up after 20 seconds
			if ( m_nResetRetryCount == maxRetries+1 ) {
				Con_Printf( COLOR_YELLOW "WARNING: OpenAL device still disconnected! Giving up!" );
				++m_nResetRetryCount; // this makes sure the warning is only shown once

				// TODO: can we shut down sound without things blowing up?
				//       if we can, we could do that if we don't have alcResetDeviceSOFT but ALC_EXT_disconnect
			}
			return false;
		}

		if ( alcResetDeviceSOFT( m_pDevice, NULL ) ) {
			Con_Printf( "OpenAL: resetting device succeeded!\n" );
			m_nResetRetryCount = 0;
			return true;
		}

		++m_nResetRetryCount;
		return false;
	}

	return m_nResetRetryCount == 0; // if it's 0, state on last check was ok
}


void Snd_DisableSounds( void ) {
    sndManager->DisableSounds();
    ALCall( alListenerf( AL_GAIN, 0.0f ) );
//    soundCacheAllocator.FreeEmptyBaseBlocks();
}

void Snd_StopAll( void ) {
    sndManager->DisableSounds();
}

void Snd_PlaySfx( sfxHandle_t sfx ) {
    if ( sfx == FS_INVALID_HANDLE || !snd_effectsOn->i ) {
        return;
    }

    sndManager->PlaySound( sndManager->GetSource( sfx ) );
}

void Snd_PlayWorldSfx( const vec3_t origin, sfxHandle_t hSfx )
{
    CSoundSource *source;

    if ( hSfx == FS_INVALID_HANDLE || !snd_effectsOn->i ) {
        return;
    }

    source = sndManager->GetSource( hSfx );
    if ( !source ) {
        return;
    }
    Assert( source );

    alSourcei( source->GetSource(), AL_SOURCE_RELATIVE, AL_TRUE );
    alSource3f( source->GetSource(), AL_POSITION, origin[0], origin[1], origin[2] );
    alSource3f( source->GetSource(), AL_DIRECTION, 0.0f, 0.0f, 0.0f );
//    alSourcei( source->GetSource(), AL_ROLLOFF_FACTOR, 1.0f );
    source->Play();
}

void Snd_StopSfx( sfxHandle_t sfx ) {
    if ( sfx == FS_INVALID_HANDLE || !snd_effectsOn->i ) {
        return;
    }

    sndManager->StopSound( sndManager->GetSource( sfx ) );
}

void Snd_SetWorldListener( const vec3_t origin ) {
    sndManager->SetListenerPos( origin );
    alListener3f( AL_POSITION, origin[0], origin[2], origin[1] );
}

void Snd_Restart( void ) {
    if ( !sndManager ) {
        return;
    }
    sndManager->Restart();
}

void Snd_Shutdown( void ) {
    if ( !sndManager ) {
        return;
    }
    sndManager->Shutdown();
}

sfxHandle_t Snd_RegisterTrack( const char *npath ) {
    CSoundSource *track;

    track = sndManager->InitSource( npath, TAG_MUSIC );
    if ( !track ) {
        return -1;
    }

    return Snd_HashFileName( track->GetName() );
}

sfxHandle_t Snd_RegisterSfx( const char *npath ) {
    CSoundSource *sfx;

    sfx = sndManager->InitSource( npath, TAG_SFX );
    if ( !sfx ) {
        return -1;
    }

    return Snd_HashFileName( sfx->GetName() );
}

void Snd_SetLoopingTrack( sfxHandle_t handle ) {
    CSoundSource *track;
    trackQueue_t *pTrack;

    if ( !snd_musicOn->i ) {
        return;
    }

    if ( handle == -1 ) {
        Con_Printf( COLOR_RED "Snd_SetLoopingTrack: invalid handle, ignoring call.\n" );
        return;
    }

    CThreadAutoLock<CThreadMutex> lock( sndManager->m_hQueueLock );
    track = sndManager->GetSource( handle );
    if ( !track ) {
        Con_Printf( COLOR_RED "Snd_SetLoopingTrack: invalid handle, ignoring call.\n" );
        return;
    } else if ( sndManager->m_pCurrentTrack == track ) {
        return; // already playing
    }

    Snd_ClearLoopingTrack();

    alSourcei( sndManager->GetMusicSource(), AL_BUFFER, AL_NONE );
    alSourcei( sndManager->GetMusicSource(), AL_BUFFER, track->GetBuffer() );
    alSourcef( sndManager->GetMusicSource(), AL_GAIN, snd_musicVolume->f / 100.0f );
    sndManager->m_pCurrentTrack = track;
    sndManager->m_pCurrentTrack->Play( true );
}

void Snd_AddLoopingTrack( sfxHandle_t handle ) {
    CSoundSource *track;

    if ( !snd_musicOn->i ) {
        return;
    }

    if ( handle == -1 ) {
        Con_Printf( COLOR_RED "Snd_AddLoopingTrack: invalid handle, ignoring call.\n" );
        return;
    }

    CThreadAutoLock<CThreadMutex> lock( sndManager->m_hQueueLock );
    track = sndManager->GetSource( handle );
    if ( !track ) {
        Con_Printf( COLOR_RED "Snd_AddLoopingTrack: invalid handle, ignoring call.\n" );
        return;
    }
    Snd_ClearLoopingTrack();

    alSourcei( track->GetSource(), AL_LOOPING, AL_TRUE );
    alSourcef( track->GetSource(), AL_GAIN, snd_musicVolume->f / 100.0f );
    alSourcePlay( track->GetSource() );

    sndManager->m_LoopingTracks.emplace_back( track );
}

void Snd_ClearLoopingTracks( void ) {
    for ( auto& it : sndManager->m_LoopingTracks ) {
        if ( it->IsPlaying() ) {
            it->Stop();
        }
    }
    sndManager->m_LoopingTracks.clear();
}

void Snd_ClearLoopingTrack( void ) {
    if ( !snd_musicOn->i || !sndManager->m_pCurrentTrack ) {
        return;
    }

    // stop the track and pop it
    alSourcei( sndManager->GetMusicSource(), AL_LOOPING, AL_FALSE );
    alSourceStop( sndManager->GetMusicSource() );
    alSourcei( sndManager->GetMusicSource(), AL_BUFFER, AL_NONE );

    sndManager->m_pCurrentTrack = NULL;
}

static void Snd_AudioInfo_f( void )
{
    Con_Printf( "\n----- Audio Info -----\n" );
    Con_Printf( "Audio Driver: %s\n", SDL_GetCurrentAudioDriver() );
    Con_Printf( "Current Track: %s\n", sndManager->m_pCurrentTrack ? sndManager->m_pCurrentTrack->GetName() : "None" );
    Con_Printf( "Number of Sound Sources: %lu\n", sndManager->NumSources() );
    Con_Printf( "OpenAL vendor: %s\n", alGetString( AL_VENDOR ) );
    Con_Printf( "OpenAL renderer: %s\n", alGetString( AL_RENDERER ) );
    Con_Printf( "OpenAL version: %s\n", alGetString( AL_VERSION ) );
}

static void Snd_Toggle_f( void )
{
    const char *var;
    const char *toggle;
    bool option;

    if ( Cmd_Argc() < 3 ) {
        Con_Printf( "usage: snd.toggle <sfx|music> <on|off, 1|0>\n" );
        return;
    }

    var = Cmd_Argv( 1 );
    toggle = Cmd_Argv( 2 );

    if ( ( toggle[0] == '1' && toggle[1] == '\0' ) || !N_stricmp( toggle, "on" ) ) {
        option = true;
    } else if ( ( toggle[0] == '0' && toggle[1] == '\0' ) || !N_stricmp( toggle, "off" ) ) {
        option = false;
    }

    if ( !N_stricmp( var, "sfx" ) ) {
        Cvar_Set( "snd_effectsOn", va( "%i", option ) );
    } else if ( !N_stricmp( var, "music" ) ) {
        Cvar_Set( "snd_musicOn", va( "%i", option ) );
    } else {
        Con_Printf( "snd.toggle: unknown parameter '%s', use either 'sfx' or 'music'\n", var );
        return;
    }
}

static void Snd_SetVolume_f( void )
{
    float vol;
    const char *change;

    if ( Cmd_Argc() < 3 ) {
        Con_Printf( "usage: snd.setvolume <sfx|music> <volume>\n" );
        return;
    }

    vol = N_atof( Cmd_Argv( 1 ) );
    vol = CLAMP( vol, 0.0f, 100.0f );

    change = Cmd_Argv( 2 );
    if ( !N_stricmp( change, "sfx" ) ) {
        if ( snd_effectsVolume->f != vol ) {
            Cvar_Set( "snd_effectsVolume", va("%f", vol) );
            sndManager->UpdateParm( TAG_SFX );
        }
    }
    else if ( !N_stricmp( change, "music" ) ) {
        if ( snd_musicVolume->f != vol ) {
            Cvar_Set( "snd_musicVolume", va( "%f", vol ) );
            sndManager->UpdateParm( TAG_MUSIC );
        }
    }
    else {
        Con_Printf( "snd.setvolume: unknown parameter '%s', use either 'sfx' or 'music'\n", change );
        return;
    }
}

static void Snd_UpdateVolume_f( void ) {
    sndManager->UpdateParm( TAG_SFX );
    sndManager->UpdateParm( TAG_MUSIC );
}

/*
* Snd_Update: checks all sound system cvars for updates
*/
void Snd_Update( int32_t msec )
{
    uint64_t i;
    CSoundSource *source;
    ALfloat v;

    if ( !sndManager->CheckDeviceAndRecoverIfNeeded() ) {
        return; // don't play anything
    }

//    if ( snd_muteUnfocused->i ) {
//        sndManager->Mute( !gw_active );
//    }
    if ( snd_masterVolume->modified ) {
        snd_effectsVolume->modified = qtrue;
        snd_musicVolume->modified = qtrue;
    }
    if ( snd_effectsVolume->modified ) {
        for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
            source = sndManager->GetSource( i );
            if ( !source ) {
                continue;
            }
            if ( source->GetTag() == TAG_SFX ) {
                source->SetVolume();
            }
        }
        snd_effectsVolume->modified = qfalse;
    }
    alGetSourcef( sndManager->GetMusicSource(), AL_GAIN, &v );
    if ( v != snd_musicVolume->f ) {
        alSourcePause( sndManager->GetMusicSource() );
        alSourcef( sndManager->GetMusicSource(), AL_GAIN, snd_musicVolume->f / 100.0f );
        alSourcePlay( sndManager->GetMusicSource() );
        snd_musicVolume->modified = qfalse;

        for ( i = 0; i < sndManager->m_LoopingTracks.size(); i++ ) {
            source = sndManager->m_LoopingTracks[i];
            if ( source->IsPlaying() ) {
                alSourcef( source->GetSource(), AL_GAIN, snd_musicVolume->f / 100.0f );
            }
        }
    }
    if ( sndManager->m_pCurrentTrack && !sndManager->m_pCurrentTrack->IsPlaying() || sndManager->m_pQueuedTrack ) {
        Snd_ClearLoopingTrack();
    }
}

static void Snd_ListFiles_f( void )
{
    uint64_t i, numFiles;
    const CSoundSource *source;

    Con_Printf( "\n---------- Snd_ListFiles_f ----------\n" );
    Con_Printf( "                      --channels-- ---samplerate--- ----cache size----\n" );

    numFiles = 0;
    for ( i = 0; i < MAX_SOUND_SOURCES; i++ ) {
        source = sndManager->GetSource( i );

        if ( !source ) {
            continue;
        }
        numFiles++;
        Con_Printf( "%10s: %4i %4i %8lu\n", source->GetName(), source->GetInfo().channels, source->GetInfo().samplerate,
            source->GetInfo().channels * source->GetInfo().frames );
    }
    Con_Printf( "Total sound files loaded: %lu\n", numFiles );
}

static void Snd_QueueTrack_f( void ) {
    sfxHandle_t hSfx;
    const char *music;

    if ( Cmd_Argc() != 2 ) {
        Con_Printf( "usage: snd.queuetrack <music>\n" );
        return;
    }

    music = Cmd_Argv( 1 );
    hSfx = Com_GenerateHashValue( music, MAX_SOUND_SOURCES );
    
    if ( sndManager->GetSource( hSfx ) == NULL ) {
        Con_Printf( "invalid track '%s'\n", music );
        return;
    }

    Snd_AddLoopingTrack( hSfx );
}

static void Snd_ClearTracks_f( void ) {
    Snd_ClearLoopingTracks();
}

static void Snd_PlaySfx_f( void ) {
    sfxHandle_t hSfx;
    const char *sound;

    if ( Cmd_Argc() != 2 ) {
        Con_Printf( "usage: snd.play_sfx <music>\n" );
        return;
    }

    sound = Cmd_Argv( 1 );
    hSfx = Com_GenerateHashValue( sound, MAX_SOUND_SOURCES );
    
    if ( sndManager->GetSource( hSfx ) == NULL ) {
        Con_Printf( "invalid sfx '%s'\n", sound );
        return;
    }

    Snd_PlaySfx( hSfx );
}

void Snd_UnloadLevel_f( void ) {
    int i;
    
    for ( i = 0; i < sndManager->m_nLevelSources; i++ ) {
        sndManager->GetSource( sndManager->m_nFirstLevelSource + i )->Shutdown();
        sndManager->GetSource( sndManager->m_nFirstLevelSource + i ) = NULL;
    }
}

void Snd_StartupLevel_f( void ) {
    sndManager->m_nFirstLevelSource = sndManager->NumSources();
    sndManager->m_nLevelSources = 0;
}

void Snd_Init( void )
{
    Con_Printf( "---------- Snd_Init ----------\n" );

    snd_effectsOn = Cvar_Get( "snd_effectsOn", "1", CVAR_SAVE );
    Cvar_CheckRange( snd_effectsOn, "0", "1", CVT_INT );
    Cvar_SetDescription( snd_effectsOn, "Toggles sound effects." );

    snd_musicOn = Cvar_Get( "snd_musicOn", "1", CVAR_SAVE );
    Cvar_CheckRange( snd_musicOn, "0", "1", CVT_INT );
    Cvar_SetDescription( snd_musicOn, "Toggles music." );

    snd_effectsVolume = Cvar_Get( "snd_effectsVolume", "50", CVAR_SAVE );
    Cvar_CheckRange( snd_effectsVolume, "0", "100", CVT_INT );
    Cvar_SetDescription( snd_effectsVolume, "Sets global sound effects volume." );

    snd_musicVolume = Cvar_Get( "snd_musicVolume", "80", CVAR_SAVE );
    Cvar_CheckRange( snd_musicVolume, "0", "100", CVT_INT );
    Cvar_SetDescription( snd_musicVolume, "Sets volume for music." );

    snd_masterVolume = Cvar_Get( "snd_masterVolume", "80", CVAR_SAVE );
    Cvar_CheckRange( snd_masterVolume, "0", "100", CVT_INT );
    Cvar_SetDescription( snd_masterVolume, "Sets the cap for sfx and music volume." );

    snd_debugPrint = Cvar_Get( "snd_debugPrint", "0", CVAR_CHEAT | CVAR_TEMP );
    Cvar_CheckRange( snd_debugPrint, "0", "1", CVT_INT );
    Cvar_SetDescription( snd_debugPrint, "Toggles OpenAL-soft debug messages." );

    snd_noSound = Cvar_Get( "s_noSound", "0", CVAR_LATCH );

    snd_device = Cvar_Get( "snd_device", "default", CVAR_LATCH | CVAR_SAVE );
    Cvar_SetDescription( snd_device, "the audio device to use ('default' for the default audio device)" );

//    snd_muteUnfocused = Cvar_Get( "snd_muteUnfocused", "1", CVAR_SAVE );
//    Cvar_SetDescription( snd_muteUnfocused, "Toggles muting sounds when the game's window isn't focused." );

    // init sound manager
    sndManager = (CSoundManager *)Hunk_Alloc( sizeof( *sndManager ), h_low );
    sndManager->Init();

    Cmd_AddCommand( "snd.setvolume", Snd_SetVolume_f );
    Cmd_AddCommand( "snd.toggle", Snd_Toggle_f );
    Cmd_AddCommand( "snd.updatevolume", Snd_UpdateVolume_f );
    Cmd_AddCommand( "snd.list_files", Snd_ListFiles_f );
    Cmd_AddCommand( "snd.clear_tracks", Snd_ClearTracks_f );
    Cmd_AddCommand( "snd.play_sfx", Snd_PlaySfx_f );
    Cmd_AddCommand( "snd.queue_track", Snd_QueueTrack_f );
    Cmd_AddCommand( "snd.audio_info", Snd_AudioInfo_f );
    Cmd_AddCommand( "snd.startup_level", Snd_StartupLevel_f );
    Cmd_AddCommand( "snd.unload_level", Snd_UnloadLevel_f );

    alDistanceModel( AL_EXPONENT_DISTANCE_CLAMPED );

    gi.soundStarted = qtrue;
    gi.soundRegistered = qtrue;

    Con_Printf( "----------------------------------\n" );

//    soundCacheAllocator.Init();
}