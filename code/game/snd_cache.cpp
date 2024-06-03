#include "g_game.h"
#include "snd_local.h"
#include "module_lib/module_memory.h"

static qboolean IsPCMFormat( int format )
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

void SndCache_CheckForDownSample( sfx_t *source )
{
    if ( !snd_force22kHz->i ) {
		return;
	}
	if ( !IsPCMFormat( source->objectInfo.format ) || source->objectInfo.samplerate != 44100 ) {
		return;
	}

	const int shortSamples = source->objectSize >> 1;
	short *converted = (short *)soundCacheAllocator.Alloc( shortSamples * sizeof( short ) );

	if ( source->objectInfo.channels == 1 ) {
		for ( int i = 0; i < shortSamples; i++ ) {
			converted[i] = ((short *)source->nonCacheData)[i*2];
		}
	} else {
		for ( int i = 0; i < shortSamples; i += 2 ) {
			converted[i+0] = ((short *)source->nonCacheData)[i*2+0];
			converted[i+1] = ((short *)source->nonCacheData)[i*2+1];
		}
	}
	soundCacheAllocator.Free( source->nonCacheData );
	source->nonCacheData = (byte *)converted;
	source->objectSize >>= 1;
	source->objectMemSize >>= 1;
	source->objectInfo.avgBytesPerSec >>= 1;
	source->objectInfo.samplerate >>= 1;
}

void SndCache_LoadFile( sfx_t *source )
{
    SNDFILE *sf;
    SF_VIRTUAL_IO vio;
    ALenum format;
    fileHandle_t f;
    FILE *fp;
    void *buffer;
    uint64_t length;
    const char *ospath;
    short *data;
    SF_INFO fdata;

    source->purged = qfalse;
    source->hardwareBuffer = qfalse;

    // clear audio file data before anything
    memset( &fdata, 0, sizeof( fdata ) );

    length = FS_LoadFile( source->name, &buffer );
    if ( !length || !buffer ) {
        Con_Printf( COLOR_RED "SndCache_LoadFile: failed to load file '%s'.\n", source->name );
        return;
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
    
    sf = sf_open_fd( fileno( fp ), SFM_READ, &fdata, SF_FALSE );
    if ( !sf ) {
        Con_Printf( COLOR_YELLOW "WARNING: libsndfile sf_open_fd failed on '%s', sf_strerror(): %s\n", source->name, sf_strerror( sf ) );
        return;
    }

    source->objectInfo.format = fdata.format;
    source->objectInfo.channels = fdata.channels;
    source->objectInfo.samplerate = fdata.samplerate;
    

    Alloc();

    data = (short *)Hunk_AllocateTempMemory( sizeof( *data ) * fdata.channels * fdata.frames );
    if ( !sf_read_short( sf, data, fdata.channels * fdata.frames ) ) {
        N_Error( ERR_FATAL, "CSoundSource::LoadFile(%s): failed to read %lu bytes from audio stream, sf_strerror(): %s\n",
            source->name, sizeof( *data ) * fdata.channels * fdata.frames, sf_strerror( sf ) );
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

    ALCall( alBufferData( m_iBuffer, format, data, sizeof( *data ) * m_hFData.channels * m_hFData.frames, m_hFData.samplerate ) );

    if ( tag == TAG_SFX ) {
        ALCall( alSourcef( m_iSource, AL_GAIN, snd_sfxvol->f ) );
        ALCall( alSourcei( m_iSource, AL_BUFFER, m_iBuffer ) );
    } else if ( tag == TAG_MUSIC ) {
        ALCall( alSourcef( m_iSource, AL_GAIN, snd_musicvol->f ) );
        ALCall( alSourcei( m_iSource, AL_BUFFER, 0 ) );
    }

    Hunk_FreeTempMemory( data );
}

sfx_t *SndCache_FindSound( const char *npath )
{
    uint64_t i;
    sfx_t *source;
    sfxHandle_t hash;

    source = NULL;

    // check if we already have it
    hash = Com_GenerateHashValue( npath, MAX_SOUNDS );
    for ( source = soundSystem->soundList[ hash ]; source; source = source->next ) {
        if ( !N_stricmp( npath, source->name ) ) {
            return source;
        }
    }

    return source;
}
