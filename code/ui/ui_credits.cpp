#include "ui_lib.h"

typedef struct {
    menuframework_t menu;

    menutext_t title;
    menutext_t message;
} creditsMenu_t;

static creditsMenu_t credits;

static const char *creditsString =
"I would not have gotten to this point without the help of many.\n"
"I would like to take the time to thank the following people for contributing\n"
"to this massive project\n";

static const char *signingOffString =
"Sincerely, Your Resident Fiend,\nNoah Van Til";

typedef struct {
    const char *name;
    const char *reason;
} collaborator_t;

static const collaborator_t collaborators[] = {
    { "Ben Pavlovic", "Some weapon ideas, created the name of the hardest difficulty: \"Just A Minor Inconvience\"" },
    { "Tucker Kemnitz", "Art, ideas for some NPCs" },
    { "Alpeca Grenade", "A music piece" },
    { "Jack Rosenthal", "A couple of ideas" },
    { "My Family & Friends", "Helping me get through some tough times" },
    { "My Father", "Giving me feedback, tips and tricks for programming when I was struggling, and helped test the first working version" },
};

static void CreditsMenu_Draw( void )
{
    int i;

    ImGui::Begin( "MainMenu##CreditsMenu", NULL, MENU_DEFAULT_FLAGS );
    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( "CREDITS" ) ) {
        UI_PopMenu();
    } else {
        for ( i = 0; i < arraylen( collaborators ); i++ ) {
            ImGui::Bullet();
            ImGui::TextWrapped( "%-24s %s", collaborators[i].name, collaborators[i].reason );
        }
        ImGui::TextUnformatted( signingOffString );
    }
    ImGui::End();
}

void CreditsMenu_Cache( void )
{
    memset( &credits, 0, sizeof( credits ) );

    credits.message.text = creditsString;
}

void UI_CreditsMenu( void )
{

}
