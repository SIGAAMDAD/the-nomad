#include "g_game.h"
#include "g_sound.h"
#include "../system/sys_thread.h"

#include <ALsoft/al.h>
#include <ALsoft/alc.h>
#include <ALsoft/alext.h>
#include <sndfile.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#define MAX_SOUND_SOURCES 1024

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
    bool LoadFile(const char *npath);

    void Update( void ) const;
    void SetVolume( int tag );

    bool IsPlaying( void ) const;
    bool IsPaused( void ) const;
    bool IsLooping( void ) const;

    const char *GetName( void ) const;

    void Play( bool loop = false );
    void Stop( void );
    void Pause( void );

    CSoundSource *m_pNext;
private:
    int FileFormat( const char *ext ) const;
    void Alloc( void );
    ALenum Format( void ) const;

    char m_pName[MAX_GDR_PATH];

    void *m_pData;
    uint32_t m_nSize;
    uint32_t m_iType;
    uint32_t m_iTag;

    // AL data
    uint32_t m_iSource;
    uint32_t m_iBuffer;

    bool m_bPlaying;
    bool m_bLooping;
    bool m_bPaused;

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

    void AddSourceToHash( CSoundSource *src );
    CSoundSource *InitSource( const char *filename, int tag );

    GDR_INLINE uint64_t NumSources( void ) const { return m_nSources; }
    GDR_INLINE CSoundSource *GetSource( sfxHandle_t handle ) { return m_pSources[handle]; }
    GDR_INLINE const CSoundSource *GetSource( sfxHandle_t handle ) const { return m_pSources[handle]; }

    CSoundSource *m_pCurrentTrack;
private:
    CSoundSource *m_pSources[MAX_SOUND_SOURCES];
    uint64_t m_nSources;

    ALCdevice *m_pDevice;
    ALCcontext *m_pContext;

    bool m_bClearedQueue;
    bool m_bRegistered;

    CThread m_hThread;
    CThreadMutex m_hAllocLock;
    CThreadSpinRWLock m_hQueueLock;
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
    return m_bPaused;
}

bool CSoundSource::IsPlaying( void ) const {
    return m_bPlaying;
}

bool CSoundSource::IsLooping( void ) const {
    return m_bLooping;
}

void CSoundSource::Init( void )
{
    memset( m_pName, 0, sizeof(m_pName) );
    memset( &m_hFData, 0, sizeof(m_hFData) );
    if (!m_iSource) {
        ALCall(alGenSources( 1, (ALuint *)&m_iSource ));
    }
    if (!m_iBuffer) {
        ALCall(alGenBuffers( 1, (ALuint *)&m_iBuffer ));
    }
    m_pData = NULL;
    m_nSize = 0;
    m_iType = 0;
    m_bPlaying = false;
    m_bLooping = false;
    m_bPaused = false;
}

void CSoundSource::Shutdown( void )
{
    if (m_iSource) {
        {
            // stop the source if we're playing anything
            if (m_bPlaying) {
                ALCall(alSourceStop( m_iSource ));
            }
        }

        ALCall(alSourcei( m_iSource, AL_BUFFER, AL_NONE ));
        ALCall(alDeleteSources( 1, (const ALuint *)&m_iSource ));
    }
    if (m_iBuffer) {
        ALCall(alDeleteBuffers( 1, (const ALuint *)&m_iBuffer ));
    }

    m_pData = NULL;
    m_nSize = 0;
    m_iBuffer = 0;
    m_iSource = 0;
    m_iType = 0;
}

void CSoundSource::Update( void ) const {
    switch (m_iTag) {
    case TAG_MUSIC:
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_musicvol->f ));
        break;
    case TAG_SFX:
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_sfxvol->f ));
        break;
    };
}

void CSoundSource::SetVolume( int tag )
{
    m_iTag = tag;
    if (tag == TAG_MUSIC) {
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_musicvol->f ));
    } else if (tag == TAG_SFX) {
        ALCall(alSourcef( m_iSource, AL_GAIN, snd_sfxvol->f ));
    }
}

ALenum CSoundSource::Format( void ) const
{
    switch (m_hFData.samplerate) {
    case 8:
        return m_hFData.channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
    case 16:
        return m_hFData.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    };
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
        m_nSize = sizeof(float) * m_hFData.frames * m_hFData.channels;
        break;
    case SF_FORMAT_DOUBLE:
        m_iType = SNDBUF_DOUBLE;
        m_nSize = sizeof(double) * m_hFData.frames * m_hFData.channels;
        break;
    case SF_FORMAT_PCM_S8:
    case SF_FORMAT_PCM_U8:
    case SF_FORMAT_DPCM_8:
        m_iType = SNDBUF_8BIT;
        m_nSize = sizeof(char) * m_hFData.frames * m_hFData.channels;
        break;
    case SF_FORMAT_ALAC_16:
    case SF_FORMAT_PCM_16:
    case SF_FORMAT_DPCM_16:
    case SF_FORMAT_DWVW_16:
    case SF_FORMAT_NMS_ADPCM_16:
        m_iType = SNDBUF_16BIT;
        m_nSize = sizeof(int16_t) * m_hFData.frames * m_hFData.channels;
        break;
    };

    m_pData = Hunk_AllocateTempMemory( m_nSize );
}

void CSoundSource::Play( bool loop )
{
    if (IsPlaying() || (IsLooping() && loop)) {
        return;
    }
    m_bPaused = false;
    m_bLooping = loop;
    m_bPlaying = true;
    if (loop) {
        ALCall(alSourcei( m_iSource, AL_LOOPING, AL_TRUE ));
    }
    ALCall(alSourcePlay( m_iSource ));
}

void CSoundSource::Pause( void ) {
    if (!IsPlaying() && !IsLooping()) {
        return;
    }

    m_bPaused = true;
    m_bPlaying = false;
    m_bLooping = false;
    ALCall(alSourcePause( m_iSource ));
}

void CSoundSource::Stop( void ) {
    if (!IsPlaying() && !IsLooping()) {
        return; // nothing's playing
    }

    m_bPaused = false;
    m_bLooping = false;
    m_bPlaying = false;
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

bool CSoundSource::LoadFile( const char *npath )
{
    SNDFILE *sf;
    sf_count_t readCount;
    SF_VIRTUAL_IO vio;
    ALenum format;
    file_t f;

    // clear audio file data before anything
    memset( &m_hFData, 0, sizeof(m_hFData) );

    memset( &vio, 0, sizeof(vio) );

    N_strncpyz( m_pName, npath, sizeof(m_pName) );

    // hash it so that if we try loading it
    // even if it's failed, we won't try loading
    // it again
    sndManager->AddSourceToHash( this );

    f = FS_FOpenRead( npath );
    if (f == FS_INVALID_HANDLE) {
        return false;
    }

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

    // allocate the buffer
    Alloc();

    sf_read_raw( sf, m_pData, m_nSize );
    
    sf_close( sf );
    FS_FClose( f );

    format = Format();
    if (format == 0) {
        Con_Printf("Bad soundfile format for '%s', refusing to load\n", npath);
        return false;
    }

    ALCall(alBufferData( m_iBuffer, format, m_pData, m_nSize, m_hFData.samplerate ));
    ALCall(alSourcei( m_iSource, AL_BUFFER, m_iBuffer ));

    Hunk_FreeTempMemory( m_pData );

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
    }
    m_pCurrentTrack = snd;
    m_pCurrentTrack->SetVolume( TAG_MUSIC );
    m_pCurrentTrack->Play( true );
}

void CSoundManager::Shutdown( void )
{
    for (uint64_t i = 0; i < m_nSources; i++) {
        m_pSources[i]->Shutdown();
    }

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
        m_pSources[i]->LoadFile( m_pSources[i]->GetName() );
    }
}

CSoundSource *CSoundManager::InitSource( const char *filename, int tag )
{
    CSoundSource *src;

    if (strlen(filename) >= MAX_GDR_PATH) {
        Con_Printf("CSoundManager::InitSource: name '%s' too long\n", filename);
        return NULL;
    }
    if (m_nSources == MAX_SOUND_SOURCES) {
        N_Error(ERR_DROP, "CSoundManager::InitSource: MAX_SOUND_SOURCES hit");
    }

    m_hAllocLock.Lock();

    if ((!snd_sfxon->i && tag == TAG_SFX) || (!snd_musicon->i && tag == TAG_MUSIC)) {
        return NULL;
    }

    src = (CSoundSource *)Hunk_Alloc( sizeof(*src), h_low );
    memset(src, 0, sizeof(*src));
    src->Init();

    if (!src->LoadFile( filename )) {
        Con_Printf(COLOR_YELLOW "WARNING: failed to load sound file '%s'\n", filename);
        m_hAllocLock.Unlock();
        return NULL;
    }

    src->SetVolume( tag );

    m_hAllocLock.Unlock();

    return src;
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
    if (sfx == FS_INVALID_HANDLE) {
        return;
    }

    sndManager->PlaySound( sndManager->GetSource( sfx ) );
}

void Snd_StopSfx(sfxHandle_t sfx) {
    if (sfx == FS_INVALID_HANDLE) {
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

void Snd_SetLoopingTrack(sfxHandle_t handle) {
    sndManager->LoopTrack( sndManager->GetSource( handle ) );
}

void Snd_ClearLoopingTrack(sfxHandle_t handle) {
    if (sndManager->m_pCurrentTrack) {
        sndManager->m_pCurrentTrack->Stop();
    }
    sndManager->m_pCurrentTrack = NULL;
}

static void Snd_AudioInfo_f( void )
{
    Con_Printf("\n----- Audio Info -----\n");
    Con_Printf("Audio Driver: %s\n", SDL_GetCurrentAudioDriver());
    Con_Printf("Current Track: %s\n", sndManager->m_pCurrentTrack ? sndManager->m_pCurrentTrack->GetName() : "None");
    Con_Printf("Number of Sound Sources: %lu\n", sndManager->NumSources());
}

void Snd_Init(void)
{
    snd_sfxon = Cvar_Get("snd_sfxon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicon = Cvar_Get("snd_musicon", "1", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_sfxvol = Cvar_Get("snd_sfxvol", "1.1f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);
    snd_musicvol = Cvar_Get("snd_musicvol", "0.8f", CVAR_SAVE | CVAR_PRIVATE | CVAR_LATCH);

    if (!snd_sfxon->i || !snd_musicon->i)
        return;

    alListenerf( AL_GAIN,  50.0f );

    Con_Printf("---------- Snd_Init ----------\n");

    // init sound manager
    sndManager = (CSoundManager *)Hunk_Alloc( sizeof(*sndManager), h_low );
    memset( sndManager, 0, sizeof(*sndManager) );
    sndManager->Init();

    Cmd_AddCommand( "snd_restart", Snd_Restart );
}
