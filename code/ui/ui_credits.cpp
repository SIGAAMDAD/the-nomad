#include "ui_lib.h"

typedef struct {
    menuframework_t menu;

    menutext_t signingOff;
    menutext_t creditsString;
    menutext_t playTesters;
} creditsMenu_t;

static creditsMenu_t *s_credits;

// NOTE: this will be updated every time I actually get a new playtester,
// although the update won't be sent out until the patch is done
static const char *s_currentPlayTesters[] = {
    "Matthew", // TODO: figure out his last name
    "Rocko Clark",
    "Will Cripe", // TODO: ask if he goes by will or william
    "Xavy", // TODO: figure out his last name
    "Patrick Boyle",
    "Nicky"
};

static void CreditsMenu_DrawPlayTesters( void *self )
{
    menutext_t *text;
    int i;

    text = (menutext_t *)self;

    ImGui::TextUnformatted( text->text );
    for ( i = 0; i < arraylen( s_currentPlayTesters ); i++ ) {
        ImGui::BulletText( "%s", s_currentPlayTesters[i] );
    }
    ImGui::NewLine();
}

void CreditsMenu_Cache( void )
{
    static const char *creditsString =
    "I would not have gotten to this point without the help of many.\n"
    "I would like to take the time to thank the following people for contributing\n"
    "to this massive project:\n"
    "\n"
    "- Ben Pavlovic: Some weapon ideas, created the name of the hardest difficulty: \"Just A Minor Inconvience\"\n"
    "- Tucker Kemnitz: Art, ideas for some NPCs\n"
    "- Alpeca Grenade: A music piece\n"
    "- Jack Rosenthal: A couple of ideas\n"
    "- My Family & Friends: Helping me get through some tough times\n"
    "- My Father: Giving me feedback, tips and tricks for programming when I was struggling, and helped test the first working version\n";

    static const char *signingOffString =
    "Sincerely, Your Resident Fiend,\nNoah Van Til";

    if ( !ui->uiAllocated ) {
        s_credits = (creditsMenu_t *)Hunk_Alloc( sizeof( *s_credits ), h_high );
    }
    memset( s_credits, 0, sizeof( *s_credits ) );

    s_credits->menu.flags = MENU_DEFAULT_FLAGS;
    s_credits->menu.x = 0;
    s_credits->menu.y = 0;
    s_credits->menu.width = ui->gpuConfig.vidWidth;
    s_credits->menu.height = ui->gpuConfig.vidHeight;
    s_credits->menu.fullscreen = qtrue;
    s_credits->menu.textFontScale = 1.5f;
    s_credits->menu.titleFontScale = 3.5f;
    s_credits->menu.name = "Credits##MainMenuTheNomadCredits";

    s_credits->creditsString.generic.type = MTYPE_TEXT;
    s_credits->creditsString.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_credits->creditsString.text = creditsString;
    s_credits->creditsString.color = color_white;

    s_credits->signingOff.generic.type = MTYPE_TEXT;
    s_credits->signingOff.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_credits->signingOff.text = signingOffString;
    s_credits->signingOff.color = color_white;

    s_credits->playTesters.generic.type = MTYPE_TEXT;
    s_credits->playTesters.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_credits->playTesters.generic.flags = QMF_OWNERDRAW;
    s_credits->playTesters.generic.ownerdraw = CreditsMenu_DrawPlayTesters;
    s_credits->playTesters.text = "And I would also like to give thanks to the many people who are playtesting this game:\n";
    s_credits->playTesters.color = color_white;

    Menu_AddItem( &s_credits->menu, &s_credits->creditsString );
    Menu_AddItem( &s_credits->menu, &s_credits->playTesters );
    Menu_AddItem( &s_credits->menu, &s_credits->signingOff );
}

void UI_CreditsMenu( void )
{
    CreditsMenu_Cache();
    UI_PushMenu( &s_credits->menu );
}
