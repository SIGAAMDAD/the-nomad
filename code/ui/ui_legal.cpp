#include "ui_lib.h"

typedef struct {
    char *GPL_v2;
    char *Apache_v2;
    char *OFL_v1;
    char *MIT;
} legal_t;

static legal_t menu;

void LegalMenu_Draw( void )
{
    ui->EscapeMenuToggle( STATE_MAIN );
    if ( ui->GetState() != STATE_LEGAL ) {
        return;
    } else if ( ui->Menu_Title( "LEGAL STUFF" ) ) {
        ui->SetState( STATE_MAIN );
        return;
    }

    if ( ImGui::CollapsingHeader( "Apache v2.0 License" ) ) {
        ImGui::TextUnformatted( "Usages: RobotoMono Font" );
        ImGui::Separator();
        ImGui::TextWrapped( "%s", menu.Apache_v2 );
    }
    if ( ImGui::CollapsingHeader( "GNU General Public License (GPL) v2.0" ) ) {
        ImGui::TextUnformatted( "Usages: glnomad (This Game), Quake3e, gzdoom, crispy-doom" );
        ImGui::Separator();
        ImGui::TextWrapped( "%s", menu.GPL_v2 );
    }
    if ( ImGui::CollapsingHeader( "Open Font License (OFL) v1.1" ) ) {
        ImGui::TextUnformatted( "Usages: PressStart2P Font, Alegreya Font, Lato Font" );
        ImGui::Separator();
        ImGui::TextWrapped( "%s", menu.OFL_v1 );
    }
    if ( ImGui::CollapsingHeader( "MIT License" ) ) {
        ImGui::TextUnformatted( "Usages: ImGui" );
        ImGui::Separator();
        ImGui::TextWrapped( "%s", menu.MIT );
    }
}

void LegalMenu_Cache( void )
{
    memset( &menu, 0, sizeof(menu) );

    //
    // load the license files
    //

    FS_LoadFile( "licenses/Apache_v2.0.txt", (void **)&menu.Apache_v2 );
    FS_LoadFile( "licenses/GPL_v2.txt", (void **)&menu.GPL_v2 );
    FS_LoadFile( "licenses/OFL_v1.1.txt", (void **)&menu.OFL_v1 );
    FS_LoadFile( "licenses/MIT.txt", (void **)&menu.MIT );

    if ( !menu.Apache_v2 ) {
        N_Error( ERR_FATAL, "LegalMenu_Cache: couldn't load licenses/Apache_v2.0.txt, DO NOT REMOVE LICENSES, THEY ARE REQUIRED." );
    }
    if ( !menu.GPL_v2 ) {
        N_Error( ERR_FATAL, "LegalMenu_Cache: couldn't load licenses/GPL_v2.txt, DO NOT REMOVE LICENSES, THEY ARE REQUIRED." );
    }
    if ( !menu.OFL_v1 ) {
        N_Error( ERR_FATAL, "LegalMenu_Cache: couldn't load licenses/OFL_v1.1.txt, DO NOT REMOVE LICENSES, THEY ARE REQUIRED." );
    }
    if ( !menu.MIT ) {
        N_Error( ERR_FATAL, "LegalMenu_Cache: couldn't load licenses/MIT.txt, DO NOT REMOVE LICENSES, THEY ARE REQUIRED." );
    }
}
