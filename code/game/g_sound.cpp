#include "g_game.h"
#include "g_sound.h"
#include "../system/sys_thread.h"
#include <EASTL/stack.h>

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <ALsoft/alext.h>
#include <sndfile.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#define MAX_SOUND_SOURCES 1024
#define MAX_MUSIC_QUEUE 8

cvar_t *snd_musicvol;
cvar_t *snd_sfxvol;
cvar_t *snd_musicon;
cvar_t *snd_sfxon;
static cvar_t *snd_frequency;

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
    switch (error) {
    case AL_OUT_OF_MEMORY:
        N_Error(ERR_FATAL, "alGetError() -- 0x%04x, AL_OUT_OF_MEMORY after '%s'", AL_OUT_OF_MEMORY, op);
        break;
    case AL_INVALID_ENUM:
        N_Error(ERR_FATAL, "alGetError() -- 0x%04x, AL_ILLEGAL_ENUM after '%s'", AL_INVALID_ENUM, op);
        break;
    case AL_INVALID_OPERATION:
        N_Error(ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_OPERATION after '%s'", AL_INVALID_OPERATION, op);
        break;
    case AL_INVALID_VALUE:
        N_Error(ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_VALUE after '%s'", AL_INVALID_VALUE, op);
        break;
    case AL_INVALID_NAME:
        N_Error(ERR_FATAL, "alGetError() -- 0x%04x, AL_INVALID_NAME after '%s'", AL_INVALID_NAME, op);
        break;
    case AL_NO_ERROR:
    default:
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
    bool LoadFile( const char *npath, int tag );

    void SetVolume( void );

    bool IsPlaying( void ) const;
    bool IsPaused( void ) const;
    bool IsLooping( void ) const;

    const char *GetName( void ) const;
    uint32_t GetTag( void ) const { return m_iTag; }

    GDR_INLINE uint32_t GetSource( void ) const { return m_iSource; }
    GDR_INLINE uint32_t GetBuffer( void ) const { return m_iBuffer; }
    GDR_INLINE void SetSource( uint32_t source ) { m_iSource = source; }

    void Play( bool loop = false );
    void Stop( void );
    void Pause( void );

    CSoundSource *m_pNext;
private:
    int FileFormat( const char *ext ) const;
    void Alloc( void );
    ALenum Format( void ) const;

    char m_pName[MAX_GDR_PATH];

#ifndef USE_ZONE_SOUND_ALLOC
    eastl::vector<short> m_pData;
#else
    short *m_pData;
    uint32_t m_nSize;
#endif
    uint32_t m_iType;
    uint32_t m_iTag;

    // AL data
    uint32_t m_iSource;
    uint32_t m_iBuffer;

    bool m_bLoop;

    SF_INFO m_hFData;
};

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
    void Update( int msec );

    void UpdateParm( int tag );

    void AddSourceToHash( CSoundSource *src );
    CSoundSource *InitSource( const char *filename, int tag );

    GDR_INLINE uint64_t NumSources( void ) const { return m_nSources; }
    GDR_INLINE CSoundSource *GetSource( sfxHandle_t handle ) { return m_pSources[handle]; }
    GDR_INLINE const CSoundSource *GetSource( sfxHandle_t handle ) const { return m_pSources[handle]; }

    CSoundSource *m_pCurrentTrack;
    eastl::vector<CSoundSource *> m_TrackQueue;

//    CThreadMutex m_hAllocLock;
//    CThreadMutex m_hQueueLock;
private:
    CSoundSource *m_pSources[MAX_SOUND_SOURCES];
    uint64_t m_nSources;

    ALCdevice *m_pDevice;
    ALCcontext *m_pContext;

    uint32_t m_iMusicSource;

    bool m_bClearedQueue;
    bool m_bRegistered;
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
    ALCall(alGetSourcei( m_iSource, AL_SOURCE_STATE, &state ));
    return state == AL_PAUSED;
}

bool CSoundSource::IsPlaying( void ) const {
    ALint state;
    ALCall(alGetSourcei( m_iSource, AL_SOURCE_STATE, &state ));
    return state == AL_PLAYING;
}

bool CSoundSource::IsLooping( void ) const {
    ALint state;
    ALCall(alGetSourcei( m_iSource, AL_SOURCE_STATE, &state ));
    return state == AL_LOOPING;
}

void CSoundSource::Init( void )
{
    memset( m_pName, 0, sizeof(m_pName) );
    memset( &m_hFData, 0, sizeof(m_hFData) );

    // this could be the music source, so don't allocate a new redundant source just yet
    m_iSource = 0;

    if (m_iBuffer == 0) {
        ALCall(alGenBuffers( 1, &m_iBuffer));
    }
#ifdef USE_ZONE_SOUND_ALLOC
    m_nSize = 0;
    m_pData = NULL;
#endif
    m_iType = 0;
    m_bLoop = false;
}

void CSoundSource::Shutdown( void )
{
    if (m_iSource) {
        // stop the source if we're playing anything
        if (IsPlaying()) {
            ALCall(alSourceStop( m_iSource ));
        }

        ALCall(alSourcei( m_iSource, AL_BUFFER, AL_NONE ));
        ALCall(alDeleteSources( 1, (const ALuint *)&m_iSource ));

        m_iSource = 0;
    }
    if (m_iBuffer) {
        ALCall(alDeleteBuffers( 1, (const ALuint *)&m_iBuffer ));

        m_iBuffer = 0;
    }
    m_iBuffer = 0;
    m_iSource = 0;
    m_iType = 0;

#ifdef USE_ZONE_SOUND_ALLOC
    if (m_pData) {
        Z_Free( m_pData );
    }
#endif
}


void CSoundSource::SetVolume( void )
{
    if (m_iTag == TAG_MUSIC) {
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_musicvol->f ));
    } else if (m_iTag == TAG_SFX) {
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_sfxvol->f ));
    }
}

ALenum CSoundSource::Format( void ) const
{
    return m_hFData.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
}

int CSoundSource::FileFormat( const char *ext ) const
{
    if (!N_stricmp( ext, "wav" )) {
        return SF_FORMAT_WAV;
    } else if (!N_stricmp( ext, "aiff" )) {
        return SF_FORMAT_AIFF;
    } else if (!N_stricmp( ext, "mp3")) {
        // mp3 copyright laws and all that stuff
        Con_Printf( COLOR_YELLOW "WARNING: loading an mp3 audio file, this could lead to some legal shit...\n" );
        return SF_FORMAT_MPEG;
    } else if (!N_stricmp( ext, "ogg" )) {
        Con_Printf( "OGG\n" );
        return SF_FORMAT_OGG;
    } else if (!N_stricmp( ext, "opus" )) {
        return SF_FORMAT_OPUS;
    } else if (!N_stricmp( ext, "flac" )) {
        return SF_FORMAT_FLAC;
    } else if (!N_stricmp( ext, "sd2" )) {
        return SF_FORMAT_SD2;
    } else {
        Con_Printf( COLOR_YELLOW "WARNING: unknown audio file format extension '%s', refusing to load\n", ext );
        return 0;
    }
}

void CSoundSource::Alloc( void )
{
    switch (m_hFData.format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_ALAW:
        break;
    case SF_FORMAT_ULAW:
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
    case SF_FORMAT_NMS_ADPCM_16:
        m_iType = SNDBUF_16BIT;
        break;
    };

#ifdef USE_ZONE_SOUND_ALLOC
    m_nSize = m_hFData.channels * m_hFData.frames;
    m_pData = (short *)Z_Malloc( sizeof(short) * m_nSize, m_iTag );
#else
    m_pData.resize( m_hFData.channels * m_hFData.frames );
#endif
}

void CSoundSource::Play( bool loop )
{
    if (IsPlaying() || (IsLooping() && loop)) {
        return;
    }
    if (loop) {
        ALCall(alSourcei( m_iSource, AL_LOOPING, AL_TRUE ));
    }
    ALCall(alSourcePlay( m_iSource ));
}

void CSoundSource::Pause( void ) {
    if (!IsPlaying() && !IsLooping()) {
        return;
    }
    ALCall(alSourcePause( m_iSource ));
}

void CSoundSource::Stop( void ) {
    if (!IsPlaying() && !IsLooping()) {
        return; // nothing's playing
    }
    ALCall(alSourceStop( m_iSource ));
}

static sf_count_t SndFile_Read(void *data, sf_count_t size, void *file) {
    return FS_Read(data, size, *(file_t *)file);
}

static sf_count_t SndFile_Tell(void *file) {
    return FS_FileTell(*(file_t *)file);
}

static sf_count_t SndFile_GetFileLen(void *file) {
    return FS_FileLength(*(file_t *)file);
}

static sf_count_t SndFile_Seek(sf_count_t offset, int whence, void *file) {
    file_t f = *(file_t *)file;
    switch (whence) {
    case SEEK_SET:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_SET);
    case SEEK_CUR:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_CUR);
    case SEEK_END:
        return FS_FileSeek(f, (fileOffset_t)offset, FS_SEEK_END);
    default:
        break;
    };
    N_Error(ERR_FATAL, "SndFile_Seek: bad whence");
    return 0; // quiet compiler warning
}

bool CSoundSource::LoadFile( const char *npath, int tag )
{
    SNDFILE *sf;
    SF_VIRTUAL_IO vio;
    ALenum format;
    file_t f;
    FILE *fp;
    void *buffer;
    uint64_t length;
    const char *ospath;

    m_iTag = tag;

    // clear audio file data before anything
    memset( &m_hFData, 0, sizeof(m_hFData) );

    memset( &vio, 0, sizeof(vio) );

    N_strncpyz( m_pName, npath, sizeof(m_pName) );

    length = FS_LoadFile( npath, &buffer );

    // hash it so that if we try loading it
    // even if it's failed, we won't try loading
    // it again
    sndManager->AddSourceToHash( this );

    fp = tmpfile();
    AssertMsg( fp, "Failed to open temprorary file!" );

#if 0
    vio.get_filelen = SndFile_GetFileLen;
    vio.write = NULL; // no need for this
    vio.read = SndFile_Read;
    vio.tell = SndFile_Tell;
    vio.seek = SndFile_Seek;

    sf = sf_open_virtual( &vio, SFM_READ, &m_hFData, &f );
    if (!sf) {
        Con_Printf(COLOR_YELLOW "WARNING: libsndfile sf_open_virtual failed on '%s', sf_sterror(): %s\n", npath, sf_strerror( sf ));
        return false;
    }
#else
    fwrite( buffer, length, 1, fp );
    fseek( fp, 0L, SEEK_SET );

    sf = sf_open_fd( fileno(fp), SFM_READ, &m_hFData, SF_FALSE );
    if (!sf) {
        Con_Printf( COLOR_YELLOW "WARNING: libsndfile sf_open_fd failed on '%s', sf_strerror(): %s\n", npath, sf_strerror( sf ) );
        return false;
    }
#endif

    // allocate the buffer
    Alloc();

#ifdef USE_ZONE_SOUND_ALLOC
    if (!sf_read_short( sf, m_pData, m_nSize ))
#else
    if (!sf_read_short( sf, m_pData.data(), m_pData.size() * sizeof(short) ))
#endif
    {
#ifdef USE_ZONE_SOUND_ALLOC
        N_Error( ERR_FATAL, "CSoundSource::LoadFile(%s): failed to read %u bytes from audio stream, sf_strerror(): %s\n",
            npath, m_nSize * sizeof(short), sf_strerror( sf ) );
#else
        N_Error( ERR_FATAL, "CSoundSource::LoadFile(%s): failed to read %lu bytes from audio stream, sf_strerror(): %s\n",
            npath, m_pData.size() * sizeof(short), sf_strerror( sf ) );
#endif
    }
    
    sf_close( sf );
    fclose(fp);

    format = Format();
    if (format == 0) {
        Con_Printf("Bad soundfile format for '%s', refusing to load\n", npath);
        return false;
    }

    // generate a brand new source for each individual sfx
    if (tag == TAG_SFX && m_iSource == 0) {
        ALCall(alGenSources( 1, &m_iSource ));
    }

#ifdef USE_ZONE_SOUND_ALLOC
    ALCall(alBufferData( m_iBuffer, format, m_p, m_nSize * sizeof(short), m_hFData.samplerate ));
#else
    ALCall(alBufferData( m_iBuffer, format, m_pData.data(), m_pData.size() * sizeof(short), m_hFData.samplerate ));
#endif
    ALCall(alSourcei( m_iSource, AL_BUFFER, m_iBuffer ));

    Hunk_FreeTempMemory( buffer );

    return true;
}

void CSoundManager::Init( void )
{
    m_pDevice = alcOpenDevice( NULL );
    if (!m_pDevice) {
        N_Error(ERR_FATAL, "Snd_Init: failed to open OpenAL device");
    }

    m_pContext = alcCreateContext( m_pDevice, NULL );
    if (!m_pContext) {
        N_Error(ERR_FATAL, "Snd_Init: failed to create OpenAL context, reason: %s", alcGetString( m_pDevice, alcGetError( m_pDevice ) ) );
    }

    alcMakeContextCurrent( m_pContext );
    m_bRegistered = true;

    // generate the recyclable music source
    ALCall(alGenSources( 1, &m_iMusicSource ));

    m_TrackQueue.reserve( MAX_MUSIC_QUEUE );
}

void CSoundManager::PlaySound( CSoundSource *snd ) {
    snd->Play();
}

void CSoundManager::StopSound( CSoundSource *snd ) {
    snd->Stop();
}

void CSoundManager::LoopTrack( CSoundSource *snd ) {
    if (m_pCurrentTrack == snd) {
        return; // we're already playing it
    }
    if (m_pCurrentTrack) {
        // we're playing something rn
        m_pCurrentTrack->Stop();
        ALCall(alSourcei( m_iMusicSource, AL_BUFFER, AL_NONE )); // clear the source buffer
    }
    m_pCurrentTrack = snd;
    m_pCurrentTrack->SetVolume();

    // set the buffer
    ALCall(alSourcei( m_iMusicSource, AL_BUFFER, m_pCurrentTrack->GetBuffer() ));
    m_pCurrentTrack->Play( true );
}

void CSoundManager::Shutdown( void )
{
    for (uint64_t i = 0; i < m_nSources; i++) {
        m_pSources[i]->Shutdown();
    }

    ALCall(alDeleteSources( 1, &m_iMusicSource ));

    alcMakeContextCurrent( NULL );
    alcDestroyContext( m_pContext );
    alcCloseDevice( m_pDevice );
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
    // shutdown everything
    Shutdown();
    
    // re-init
    Init();

    // reload all sources
    for (uint64_t i = 0; i < m_nSources; i++) {
        m_pSources[i]->LoadFile( m_pSources[i]->GetName(), m_pSources[i]->GetTag() );
    }
}

//
// CSoundManager::InitSource: allocates a new sound source with openal
// NOTE: even if sound and music is disabled, we'll still allocate the memory,
// we just won't play any of the sources
//
CSoundSource *CSoundManager::InitSource( const char *filename, int tag )
{
    CSoundSource *src;
    nhandle_t hash;

    hash = Snd_HashFileName( filename );

    //
    // check if we already have it loaded
    //
    for (src = m_pSources[hash]; src; src = src->m_pNext) {
        if (!N_stricmp( filename, src->GetName() )) {
            return src;
        }
    }

    if (strlen(filename) >= MAX_GDR_PATH) {
        Con_Printf("CSoundManager::InitSource: name '%s' too long\n", filename);
        return NULL;
    }
    if (m_nSources == MAX_SOUND_SOURCES) {
        N_Error(ERR_DROP, "CSoundManager::InitSource: MAX_SOUND_SOURCES hit");
    }

//    m_hAllocLock.Lock();

    src = (CSoundSource *)Hunk_Alloc( sizeof(*src), h_low );
    memset(src, 0, sizeof(*src));
    src->Init();

    if (tag == TAG_MUSIC) {
        if (!alIsSource( m_iMusicSource )) { // make absolutely sure its a valid source
            ALCall( alGenSources( 1, &m_iMusicSource ));
        }
        src->SetSource( m_iMusicSource );
    }

    if (!src->LoadFile( filename, tag )) {
        Con_Printf(COLOR_YELLOW "WARNING: failed to load sound file '%s'\n", filename);
//        m_hAllocLock.Unlock();
        return NULL;
    }

    src->SetVolume();

//    m_hAllocLock.Unlock();

    return src;
}

void CSoundManager::UpdateParm( int tag )
{
    for (uint64_t i = 0; i < m_nSources; i++) {
        if (m_pSources[i]->GetTag() == tag) {
            m_pSources[i]->SetVolume();
        }
    }
}

void CSoundManager::DisableSounds( void ) {
    for (uint64_t i = 0; i < m_nSources; i++) {
        m_pSources[i]->Stop();
    }
}


void Snd_DisableSounds(void) {
    sndManager->DisableSounds();
//    ALCall(alListenerf( AL_GAIN, 0.0f ));
}

void Snd_StopAll(void) {
    sndManager->DisableSounds();
}

void Snd_PlaySfx(sfxHandle_t sfx) {
    if (sfx == FS_INVALID_HANDLE || !snd_sfxon->i) {
        return;
    }

    sndManager->PlaySound( sndManager->GetSource( sfx ) );
}

void Snd_StopSfx(sfxHandle_t sfx) {
    if (sfx == FS_INVALID_HANDLE || !snd_sfxon->i) {
        return;
    }

    sndManager->StopSound( sndManager->GetSource( sfx ) );
}

void Snd_Restart(void) {
    if (!sndManager) {
        return;
    }
    sndManager->Restart();
}

static void Snd_QueueTrack_r( sfxHandle_t handle, bool loop )
{
    CSoundSource *track;
//    CThreadAutoLock<CThreadMutex> lock( sndManager->m_hQueueLock );

    if (!snd_musicon->i) {
        return;
    }
    else if (handle == FS_INVALID_HANDLE) {
        Con_DPrintf( COLOR_RED "Snd_QueueTrack: invalid handle\n" );
        return;
    }

    track = sndManager->GetSource( handle );
    if (!track) {
        Con_DPrintf( COLOR_RED "Snd_QueueTrack: invalid handle" );
        return;
    }
    else if (sndManager->m_pCurrentTrack == track) {
        return; // already playing
    }

    if (!((ssize_t)sndManager->m_TrackQueue.capacity() - 1)) {
        sndManager->m_TrackQueue.reserve( MAX_MUSIC_QUEUE ); // lets try to be efficient with our memory
    }

    sndManager->m_TrackQueue.emplace_back( track );
    track->Play( loop );
}

//
// Snd_QueueTrack: simple wrapper for non-looping sounds
//
void Snd_QueueTrack( sfxHandle_t handle ) {
    Snd_QueueTrack_r( handle, false );
}

void Snd_Shutdown( void ) {
    if (!sndManager) {
        return;
    }
    sndManager->Shutdown();
}

sfxHandle_t Snd_RegisterTrack(const char *npath) {
    CSoundSource *track;

    track = sndManager->InitSource( npath, TAG_MUSIC );
    if (!track) {
        return FS_INVALID_HANDLE;
    }

    return Snd_HashFileName( track->GetName() );
}

sfxHandle_t Snd_RegisterSfx(const char *npath) {
    CSoundSource *sfx;

    sfx = sndManager->InitSource( npath, TAG_SFX );
    if (!sfx) {
        return FS_INVALID_HANDLE;
    }

    return Snd_HashFileName( sfx->GetName() );
}

void Snd_SetLoopingTrack( sfxHandle_t handle ) {
    if (!snd_musicon->i) {
        return;
    }

    Snd_QueueTrack_r( handle, true );
}

void Snd_ClearLoopingTrack( void ) {
    if (!snd_musicon->i || !sndManager->m_pCurrentTrack || !sndManager->m_pCurrentTrack->IsLooping()) {
        return;
    }

    // stop the track and pop it
    sndManager->m_pCurrentTrack->Stop();
    sndManager->m_TrackQueue.pop_back();

    // play the next sound
    if (sndManager->m_TrackQueue.size()) {
        sndManager->m_pCurrentTrack = sndManager->m_TrackQueue.back();
        sndManager->m_pCurrentTrack->Play();
    }
}

static void Snd_AudioInfo_f( void )
{
    Con_Printf("\n----- Audio Info -----\n");
    Con_Printf("Audio Driver: %s\n", SDL_GetCurrentAudioDriver());
    Con_Printf("Current Track: %s\n", sndManager->m_pCurrentTrack ? sndManager->m_pCurrentTrack->GetName() : "None");
    Con_Printf("Number of Sound Sources: %lu\n", sndManager->NumSources());
}

static void Snd_Toggle_f( void )
{
    const char *var;
    const char *toggle;
    bool option;

    if (Cmd_Argc() < 3) {
        Con_Printf( "usage: sndtoggle <sfx|music> <on|off, 1|0>\n" );
        return;
    }

    var = Cmd_Argv(1);
    toggle = Cmd_Argv(2);

    if ((toggle[0] == '1' && toggle[1] == '\0') || !N_stricmp( toggle, "on" )) {
        option = true;
    }
    else if ((toggle[0] == '0' && toggle[1] == '\0') || !N_stricmp( toggle, "off" )) {
        option = false;
    }

    if (!N_stricmp( var, "sfx" )) {
        Cvar_Set( "snd_sfxon", va("%i", option) );
    }
    else if (!N_stricmp( var, "music" )) {
        Cvar_Set( "snd_musicon", va("%i", option) );
    }
    else {
        Con_Printf( "sndtoggle: unknown parameter '%s', use either 'sfx' or 'music'\n", var );
        return;
    }
}

static void Snd_SetVolume_f( void )
{
    float vol;
    const char *change;

    if (Cmd_Argc() < 3) {
        Con_Printf( "usage: setvolume <sfx|music> <volume>\n" );
        return;
    }

    vol = N_atof(Cmd_Argv(1));
    vol = CLAMP(vol, 0.0f, 100.0f);

    change = Cmd_Argv(2);
    if (!N_stricmp( change, "sfx" )) {
        if (snd_sfxvol->f != vol) {
            Cvar_Set( "snd_sfxvol", va("%f", vol) );
            sndManager->UpdateParm( TAG_SFX );
        }
    }
    else if (!N_stricmp( change, "music" )) {
        if (snd_musicvol->f != vol) {
            Cvar_Set( "snd_musicvol", va("%f", vol) );
            sndManager->UpdateParm( TAG_MUSIC );
        }
    }
    else {
        Con_Printf( "setvolume: unknown parameter '%s', use either 'sfx' or 'music'\n", change );
        return;
    }
}

void Snd_Update( int msec )
{
    if (!sndManager->m_TrackQueue.size()) {
        // nothings queued and nothing's playing
        return;
    }
    if (!sndManager->m_TrackQueue.back()->IsPlaying()) {
        // pop the finished track, then begin the next one in the queue  
        sndManager->m_TrackQueue.pop_back();
        sndManager->m_pCurrentTrack = sndManager->m_TrackQueue.back();

        if (sndManager->m_pCurrentTrack) {
            sndManager->m_pCurrentTrack->Play();
        }
    }
}

void Snd_Init(void)
{
    snd_sfxon = Cvar_Get("snd_sfxon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicon = Cvar_Get("snd_musicon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_sfxvol = Cvar_Get("snd_sfxvol", "1.1f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicvol = Cvar_Get("snd_musicvol", "0.8f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);

    alListenerf( AL_GAIN,  50.0f );

    Con_Printf("---------- Snd_Init ----------\n");

    // init sound manager
    sndManager = (CSoundManager *)Hunk_Alloc( sizeof(*sndManager), h_low );
    memset( sndManager, 0, sizeof(*sndManager) );
    sndManager->Init();

    Cmd_AddCommand( "snd_restart", Snd_Restart );
    Cmd_AddCommand( "setvolume", Snd_SetVolume_f );
    Cmd_AddCommand( "sndtoggle", Snd_Toggle_f );
}
