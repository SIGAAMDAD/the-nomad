#include "g_game.h"
#include "snd_public.h"
#include "snd_local.h"

cvar_t *snd_noSound;
cvar_t *snd_masterVolume;
cvar_t *snd_musicVolume;
cvar_t *snd_sfxVolume;
cvar_t *snd_device;

soundSystem_t *soundSystem;

LPALGENEFFECTS alGenEffects;
LPALDELETEEFFECTS alDeleteEffects;
LPALISEFFECT alIsEffect;
LPALEFFECTI alEffecti;
LPALEFFECTF alEffectf;
LPALEFFECTFV alEffectfv;
LPALGENFILTERS alGenFilters;
LPALDELETEFILTERS alDeleteFilters;
LPALISFILTER alIsFilter;
LPALFILTERI alFilteri;
LPALFILTERF alFilterf;
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
LPALCRESETDEVICESOFT alcResetDeviceSOFT; // needs ALC_SOFT_HRTF extension

static void Snd_ReloadSounds_f( void )
{
    soundSystem->muted = qtrue;
    SndCache_ReloadSounds( Cmd_Argc() == 2 );
    soundSystem->muted = qfalse;
    Con_Printf( "Sounds reloaded.\n" );
}

void Snd_Init( void )
{
    const char *device;

    soundSystem = (soundSystem_t *)Hunk_Alloc( sizeof( *soundSystem ), h_low );

    soundCacheAllocator.Init();

    snd_noSound = Cvar_Get( "snd_noSound", "0", CVAR_ROM );
    snd_device = Cvar_Get( "snd_device", "default", CVAR_SAVE | CVAR_CHEAT );
    Cvar_SetDescription( snd_device, "the audio device to use ('default' for the default audio device)" );

    soundSystem->muted = qfalse;

    if ( snd_noSound->i ) {
        Con_Printf( "Sound disabled with snd_noSound \"1\"!\n" );
        soundSystem->openalDevice = NULL;
        soundSystem->openalContext = NULL;
    } else {
        Con_Printf( "Initializing OpenAL device and context\n" );

        device = snd_device->s;
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
                Con_Printf( "OpenAL: device '%s' not found, using default\n", device );
                device = NULL;
            }
        }
        soundSystem->openalDevice = alcOpenDevice( device );
        if ( !soundSystem->openalDevice && device ) {
            Con_Printf( "OpenAL: failed to open device '%s' (0x%x), trying default...\n", device, alGetError() );
            soundSystem->openalDevice = alcOpenDevice( NULL );
        }

        if ( soundSystem->openalDevice == NULL ) {
            Con_Printf( "OpenAL: failed to open default device (0x%x), disabling sound\n", alGetError() );
            soundSystem->openalContext = NULL;
        } else {
            soundSystem->openalContext = alcCreateContext( soundSystem->openalDevice, NULL );
            if ( soundSystem->openalContext == NULL ) {
                Con_Printf( "OpenAL: failed to create context (0x%x), disabling sound\n", alcGetError( soundSystem->openalDevice ) );
            }
        }
    }

    if ( soundSystem->openalContext ) {
        alcMakeContextCurrent( soundSystem->openalContext );

        Con_Printf( "OpenAL vendor: %s\n", alGetString( AL_VENDOR ) );
        Con_Printf( "OpenAL renderer: %s\n", alGetString( AL_RENDERER ) );
        Con_Printf( "OpenAL version: %s\n", alGetString( AL_VERSION ) );
    }
}

void Snd_Shutdown( void )
{
    soundCacheAllocator.Shutdown();
}
