#ifndef __SND_LOCAL__
#define __SND_LOCAL__

#pragma once

#include "../engine/n_shared.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <sndfile.h>
#include <ALsoft/alext.h>
#include <ALsoft/al.h>

#define MIXBUFFER_SAMPLES 4096
#define SOUND_MAX_LIST_SAMPLES 4096

// sound shader flags
static const unsigned SSF_PRIVATE_SOUND =       BIT( 0 );	// only plays for the current listenerId
static const unsigned SSF_ANTI_PRIVATE_SOUND =  BIT( 1 );	// plays for everyone but the current listenerId
static const unsigned SSF_NO_OCCLUSION =        BIT( 2 );	// don't flow through portals, only use straight line
static const unsigned SSF_GLOBAL =              BIT( 3 );	// play full volume to all speakers and all listeners
static const unsigned SSF_OMNIDIRECTIONAL =     BIT( 4 );	// fall off with distance, but play same volume in all speakers
static const unsigned SSF_LOOPING =             BIT( 5 );	// repeat the sound continuously
static const unsigned SSF_PLAY_ONCE =           BIT( 6 );	// never restart if already playing on any channel of a given emitter
static const unsigned SSF_UNCLAMPED =           BIT( 7 );	// don't clamp calculated volumes at 1.0
static const unsigned SSF_NO_FLICKER =          BIT( 8 );	// always return 1.0 for volume queries
static const unsigned SSF_NO_DUPS =             BIT( 9 );	// try not to play the same sound twice in a row

// these options can be overriden from sound shader defaults on a per-emitter and per-channel basis
typedef struct {
	float					minDistance;
	float					maxDistance;
	float					volume;					// in dB, unfortunately.  Negative values get quieter
	float					shakes;
	unsigned				soundShaderFlags;		// SSF_* bit flags
	int						soundClass;				// for global fading of sounds
} soundShaderParms_t;


/*
===================================================================================

  This class holds the actual wavefile bitmap, size, and info.

===================================================================================
*/

const int SCACHE_SIZE = MIXBUFFER_SAMPLES * 20; // 1/2 of a second (aroundabout)

class CSoundSample
{
public:
	CSoundSample( void );
	~CSoundSample();

    // name of the sample file
    char name[MAX_NPATH];

    // the most recent of all images used in creation, for reloadImages command
	time_t timestamp;

	SF_INFO objectInfo;				// what are we caching
	size_t objectSize;				// size of waveform in samples, excludes the header
	size_t objectMemSize;			// object size in memory
	byte *nonCacheData;				// if it's not cached
	byte *amplitudeData;			// precomputed min,max amplitude pairs
	ALuint openalBuffer;			// openal buffer
	qboolean hardwareBuffer;
	qboolean defaultSound;
	qboolean onDemand;
	qboolean purged;
	qboolean levelLoadReferenced;	// so we can tell which samples aren't needed any more

	size_t LengthIn44kHzSamples( void ) const;
	time_t GetNewTimeStamp( void ) const;
	void MakeDefault( void );		// turns it into a beep
	void Load( void );				// loads the current sound based on name
	void Reload( bool force );		// reloads if timestamp has changed, or always if force
	void PurgeSoundSample( void );	// frees all data
	void CheckForDownSample( void );// down sample if required
	bool FetchFromCache( int offset, const byte **output, int *position, int *size, const bool allowIO );
};

/*
===================================================================================

  Sound sample decoder.

===================================================================================
*/

class CSampleDecoder {
public:
    ~CSampleDecoder() = default;

	static void Init( void );
	static void Shutdown( void );
	static CSampleDecoder *Alloc( void );
	static void Free( CSampleDecoder *decoder );
	static uint64_t GetNumUsedBlocks( void );
	static uint64_t GetUsedBlockMemory( void );

	virtual void Decode( CSampleDecoder *sample, int sampleOffset44k, int sampleCount44k, float *dest ) = 0;
	virtual void ClearDecoder( void ) = 0;
	virtual CSampleDecoder *GetSample( void ) const = 0;
	virtual uint64_t GetLastDecodeTime( void ) const = 0;
};

class CSoundShader
{
public:
    CSoundShader( void );
    ~CSoundShader();

    size_t Size( void ) const;
    bool SetDefaultText( void );
    const char *DefaultDefinition( void ) const;
    bool Parse( const char *text, size_t textLength );
    void FreeData( void );
    void List( void ) const;

    const char *GetDescription( void ) const;

    // so the editor can draw correct default sound spheres
	// this is currently defined as meters, which sucks, IMHO.
	float GetMinDistance( void ) const;		// FIXME: replace this with a GetSoundShaderParms()
	float GetMaxDistance( void ) const;

	// returns NULL if an AltSound isn't defined in the shader.
	// we use this for pairing a specific broken light sound with a normal light sound
	const CSoundShader *GetAltSound( void ) const;

	bool HasDefaultSound( void ) const;

	const soundShaderParms_t *GetParms( void ) const;
	uint64_t GetNumSounds( void ) const;
	const char *GetSound( uint64_t index ) const;

	bool CheckShakesAndOgg( void ) const;

    const char *GetName( void ) const;

    void SetText( const char *text );
    const char *GetText( void ) const;
private:
    friend class CSoundWorld;
    friend class CSoundCache;
    friend class CSoundChannel;

    // options from sound shader text
    // can be overriden on a per-channel basis
	soundShaderParms_t parms;

    char name[MAX_NPATH];

    // only load when played, and free when finished
	qboolean onDemand;
	int speakerMask;
    const CSoundShader *altSound;

    // description
	eastl::string desc;
	qboolean errorDuringParse;

    // allows light breaking leadin sounds to be much louder than the broken loop
	float leadinVolume;

	CSoundSample *leadins[SOUND_MAX_LIST_SAMPLES];
	uint64_t numLeadins;
	CSoundSample *entries[SOUND_MAX_LIST_SAMPLES];
	uint64_t numEntries;

    char *text;
    uint64_t nLength;
private:
	void					Init( void );
	bool					ParseShader( const char *text );
};

class CSoundManager
{
public:
    CSoundManager( void );
    ~CSoundManager();


private:
};

/*
===================================================================================

  The actual sound cache.

===================================================================================
*/

class CSoundCache {
public:
    CSoundCache( void );
    ~CSoundCache();

	CSoundCache *FindSound( const eastl::string& fname, qboolean loadOnDemandOnly );

	const uint64_t GetNumObjects( void ) { return listCache.size(); }
	const CSoundSample *GetObject( const uint64_t index ) const;

	void					ReloadSounds( bool force );

	void					BeginLevelLoad( void );
	void					EndLevelLoad( void );

//	void					PrintMemInfo( MemInfo_t *mi );
private:
	qboolean insideLevelLoad;
	eastl::vector<CSoundSample *> listCache;
};

#endif