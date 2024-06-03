#include "g_game.h"
#include "snd_public.h"
#include "snd_local.h"

sfx_t *Snd_LoadFile( const char *npath )
{
    sfx_t *source;
    SNDFILE *sf;
    FILE *fp;

    source = (sfx_t *)Hunk_Alloc( sizeof( *source ), h_low );
}

void Snd_ReleaseSource( sfx_t *source )
{

}
