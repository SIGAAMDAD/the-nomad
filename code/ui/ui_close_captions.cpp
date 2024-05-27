#include "ui_lib.h"

#define CLOSE_CAPTIONS_STACK 64
#define CLOSE_CAPTIONS_TIME 5000

static const char *closeCaptions[CLOSE_CAPTIONS_STACK];
static int closeCaptionsDepth;
static int closeCaptionsTime;

void UI_PushClosedCaption( const char *pString )
{
    if ( closeCaptionsDepth == CLOSE_CAPTIONS_STACK ) {
        closeCaptionsDepth = 0;
    }

    closeCaptionsDepth++;
}

void UI_DrawClosedCaptions( void )
{
    closeCaptionsTime += Sys_Milliseconds();
    if ( closeCaptionsTime > CLOSE_CAPTIONS_TIME ) {
    }
}

void UI_InitClosedCaptions( void )
{
    memset( closeCaptions, 0, sizeof( closeCaptions ) );
    closeCaptionsDepth = 0;
}
