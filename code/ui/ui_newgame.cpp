#include "ui_lib.h"

typedef struct {
    menuframework_t menu;

    char saveName[MAX_NPATH];

    int diff;

    const char **difficultyList;
    char **hardestStrings;
    int64_t numHardestStrings;
    int64_t hardestIndex;

    const stringHash_t *title;
    const stringHash_t *newGameSaveNamePrompt;
    const stringHash_t *newGameBegin;

    nhandle_t accept_0;
    nhandle_t accept_1;

    qboolean acceptHovered;
    qboolean virtualKeyboard;
} newGameMenu_t;

namespace ImGui {
    static eastl::map<const char *, int> g_KeypadApplyMap; // map of labels to return values
    static const char *g_KeypadCurrentLabel; // only one instance of Keypad is open at any one time
    static char g_KeypadEditStrPtr[MAX_NPATH];
    static char g_KeypadEditStrRestore[MAX_NPATH];
//    static eastl::string  *g_KeypadEditStrPtr;  // pointer to string to edit
//    static eastl::string  g_KeypadEditStrRestore; // stored value for undo
    
    // Draw a 4x5 button matrix entry keypad edits a *value std::string,
    // scaled to the current content region height with square buttons    
    int InputKeypad(const char *label, bool* p_visible, char *buffer, size_t bufferSize) {
        int ret = 0; 

        if (p_visible && !*p_visible)
            return ret;  
        if (!value || !label)
            return ret;

        ImVec2 csize = GetContentRegionAvail();
        int n = (csize.y / 5); // height / 5 button rows
      
        ImGuiStyle &style = GetStyle();
      
        if (BeginChild(label, ImVec2((n * 4) + style.WindowPadding.x, n * 5), true)) {
            
            csize = GetContentRegionAvail(); // now inside this child
            n = (csize.y / 5) - style.ItemSpacing.y; // button size
            ImVec2 bsize(n, n); // buttons are square

            PushStyleVar(ImGuiStyleVar_FrameRounding,6);      
            static std::string k = "";
            if (Button("ESC", bsize)) { k = "X"; } SameLine(); 
            if (Button("/", bsize)) { k = "/"; } SameLine(); 
            if (Button("*", bsize)) { k = "*"; } SameLine(); 
            if (Button("-", bsize)) { k = "-"; }
            if (Button("7", bsize)) { k = "7"; } SameLine();
            if (Button("8", bsize)) { k = "8"; } SameLine(); 
            if (Button("9", bsize)) { k = "9"; } SameLine(); 
            if (Button("+", bsize)) { k = "+"; }
            if (Button("4", bsize)) { k = "4"; } SameLine(); 
            if (Button("5", bsize)) { k = "5"; } SameLine(); 
            if (Button("6", bsize)) { k = "6"; } SameLine(); 
            if (Button("<-", bsize)) { k = "B"; }  
            if (Button("1", bsize)) { k = "1"; } SameLine(); 
            if (Button("2", bsize)) { k = "2"; } SameLine(); 
            if (Button("3", bsize)) { k = "3"; } SameLine(); 
            if (Button("CLR", bsize)) { k = "C";} 
            if (Button("0", bsize)) { k = "0"; } SameLine(); 
            if (Button("0", bsize)) { k = "0"; } SameLine(); 
            if (Button(".", bsize)) { k = "."; } SameLine(); 
            if (Button("=", bsize)) { k = "E"; }
            PopStyleVar();           

            // logic
            if (k != "") {
                if (k != "E" && k != "X" && k != "B" && k != "C"){
                    value->append(k); // add k to the string
                } 
                else {
                    if (k == "E") { // enter                  
                        ret = 1; // value has been accepted                                                
                    }
                    else if (k == "B") { // remove one char from the string
                        std::string tvalue = value->substr(0, value->length() - 1);
                        value->swap(tvalue);
                    } else if (k == "C"){
                        value->clear();
                    }
                    else if (k == "X") { // cancel                        
                        ret = -1; //  restore old value
                    }                        
                }
                if (ret) *p_visible = false;
            }            
            k = "";        
            EndChild();           
        }    
        g_KeypadApplyMap[label] = ret; // store results in map
        return ret;
    }

    // The widget takes a label, and a *std::string
    // returns 1 if the new value should accepted, enter pressed
    // returns 0 if nothing happened
    // return -1 if cancel pressed, previous value has been restored     
    int KeypadEditString(const char *label, char *buffer, size_t bufferSize){
        if (!label || !buffer) return 0;

        Text(label);
        SameLine();
        InputText(label, buffer, bufferSize); 
    
        if (IsItemHovered() && IsMouseClicked(ImGuiMouseButton_Left)) {        
            if ( N_stricmp( g_KeypadEditStrPtr, buffer ) ){
                N_strncpyz( g_KeypadEditStrRestore, buffer, sizeof( g_KeypadEditStrPtr ) );
                N_strncpyz( g_KeypadEditStrPtr, buffer, sizeof( g_KeypadEditStrPtr ) );
                g_KeypadApplyMap[label] = 0;      
                g_KeypadCurrentLabel = label;   
            }
            OpenPopup("KeypadX"); 
        }

        if (g_KeypadApplyMap[label] == 1) {      
            g_KeypadApplyMap[label] = 0;
            return 1;
        } else if (g_KeypadApplyMap[label] == -1) {
            g_KeypadApplyMap[label] = 0;
            return -1;
        }
        return 0;
    }

    // Show the popup keypad box as required.
    void PopupKeypad(void){
        // Always center this window when appearing
        ImVec2 center = GetMainViewport()->GetCenter();
        SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        SetNextWindowSize(ImVec2(300,400));

        if (BeginPopupModal("KeypadX", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (g_KeypadEditStrPtr != nullptr) {
                bool gShowKeypad = true;                       
                Text(g_KeypadEditStrPtr);
                int r = InputKeypad("Keypad Input", &gShowKeypad, g_KeypadEditStrPtr);
                if (r == -1) {
                    // undo - restore previous value
                    char tmp[MAX_STRING_TOKENS];
                    N_strncpyz( tmp, g_KeypadEditStrPtr, sizeof( tmp ) );
                    N_strncpyz( g_KeypadEditStrPtr, g_KeypadEditStrRestore, sizeof( g_KeypadEditStrPtr ) );
                    N_strncpyz( g_KeypadEditStrRestore, tmp, sizeof( g_KeypadEditStrPtr ) );
                    g_KeypadApplyMap[g_KeypadCurrentLabel] = -1;
                }
                else if (r == 1) {
                    // set - we should apply the new value 
                    g_KeypadApplyMap[g_KeypadCurrentLabel] = 1;               
                }
                if (!gShowKeypad) CloseCurrentPopup();            
            }
            EndPopup();            
        }
    }
};

#define ID_BEGINGAME        1
#define ID_SAVENAMEPROMPT   2
#define ID_DIFFICULTY       3

static newGameMenu_t *s_newGame;

static void BeginNewGame( void )
{
    UI_SetActiveMenu( UI_MENU_PAUSE );

    gi.state = GS_LEVEL;

    N_strncpyz( s_newGame->saveName, COM_SkipPath( s_newGame->saveName ), sizeof( s_newGame->saveName ) );

    Cvar_Set( "g_paused", "0" );
    Cvar_Set( "sgame_SaveName", s_newGame->saveName );
    Cvar_SetIntegerValue( "g_levelIndex", 0 );
    Cvar_Set( "mapname", *gi.mapCache.mapList );

    memset( s_newGame->saveName, 0, sizeof( s_newGame->saveName ) );

    // start a new game
    g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelStart, 0 );
}

static void NewGameMenu_Draw( void )
{
    int i;
    extern cvar_t *in_joystick;

    ImGui::Begin( s_newGame->menu.name, NULL, s_newGame->menu.flags );
    ImGui::SetWindowSize( ImVec2( s_newGame->menu.width, s_newGame->menu.height ) );
    ImGui::SetWindowPos( ImVec2( s_newGame->menu.x, s_newGame->menu.y ) );

    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( s_newGame->title->value, s_newGame->menu.titleFontScale ) ) {
        UI_PopMenu();
        return;
    }

    ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * s_newGame->menu.textFontScale ) * ui->scale );

    ImGui::BeginTable( "##SinglePlayerMenuNewGameConfigTable", 2 );
    {
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( s_newGame->newGameBegin->value );
        ImGui::TableNextColumn();
        if ( ImGui::InputText( "##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName, sizeof( s_newGame->saveName ) - 1,
            ImGuiInputTextFlags_EnterReturnsTrue ) )
        {
            // accessing it through the controller, toggle the keyboard
            if ( in_joystick->i && !ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && Key_IsDown( KEY_PAD0_A ) ) {
                s_newGame->virtualKeyboard = qtrue;
            }
            Snd_PlaySfx( ui->sfx_select );
        }
        if ( s_newGame->virtualKeyboard ) {
            if ( Key_IsDown( KEY_PAD0_B ) ) {
                s_newGame->virtualKeyboard = qfalse;
            }
        }

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( "Difficulty" );
        ImGui::TableNextColumn();
        if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigLeft", ImGuiDir_Left ) ) {
            Snd_PlaySfx( ui->sfx_select );
            s_newGame->diff--;
            if ( s_newGame->diff <= DIF_NOOB ) {
                s_newGame->diff = DIF_HARDEST;
            }
        }
        ImGui::SameLine();
        if ( ImGui::BeginCombo( "##SinglePlayerMenuDifficultyConfigList", s_newGame->difficultyList[ (int)s_newGame->diff ] ) ) {
            if ( ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
                Snd_PlaySfx( ui->sfx_select );
            }
            for ( i = 0; i < NUMDIFS; i++ ) {
                if ( ImGui::Selectable( va( "%s##%sSinglePlayerMenuDifficultySelectable_%i", s_newGame->difficultyList[ i ],
                    s_newGame->difficultyList[ i ], i ), ( s_newGame->diff == i ) ) )
                {
                    Snd_PlaySfx( ui->sfx_select );
                    s_newGame->diff = i;
                }
            }
            ImGui::EndCombo();
        }
        if ( !ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            Snd_PlaySfx( ui->sfx_select );
        }
        ImGui::SameLine();
        if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigRight", ImGuiDir_Right ) ) {
            Snd_PlaySfx( ui->sfx_select );
            s_newGame->diff++;
            if ( s_newGame->diff > DIF_HARDEST ) {
                s_newGame->diff = DIF_NOOB;
            }
        }
    }
    ImGui::EndTable();

    ImGui::SetCursorScreenPos( ImVec2( 970 * ui->scale, 680 * ui->scale ) );
    ImGui::Image( (ImTextureID)(uintptr_t)( s_newGame->acceptHovered ? s_newGame->accept_1 : s_newGame->accept_0 ),
		ImVec2( 256 * ui->scale, 72 * ui->scale ) );
	s_newGame->acceptHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
	if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
		Snd_PlaySfx( ui->sfx_select );
        BeginNewGame();
	}

    ImGui::SetCursorScreenPos( ImVec2( 16 * ui->scale, 300 * ui->scale ) );

/*
    FontCache()->SetActiveFont( AlegreyaSC );
    ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 3.75f ) * ui->scale );
    ImGui::TextUnformatted( "Difficulty Description" );
    ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.75f ) * ui->scale );
*/

    FontCache()->SetActiveFont( RobotoMono );
    ImGui::TextWrapped( "%s", difficultyTable[ (int)s_newGame->diff ].tooltip );

    ImGui::End();
}

int32_t count_fields( const char *line );
char **parse_csv( const char *line );

void NewGameMenu_Cache( void )
{
    const stringHash_t *hardest;

    hardest = strManager->ValueForKey( "SP_DIFF_THE_MEMES" );

    if ( !ui->uiAllocated ) {
        s_newGame = (newGameMenu_t *)Hunk_Alloc( sizeof( *s_newGame ), h_high );
        memset( s_newGame, 0, sizeof( *s_newGame ) );
        s_newGame->hardestStrings = parse_csv( hardest->value );
        s_newGame->numHardestStrings = count_fields( hardest->value );
    }

    srand( Sys_Milliseconds() );
    s_newGame->hardestIndex = rand() % s_newGame->numHardestStrings;
    static const char *difficulties[NUMDIFS] = {
        difficultyTable[ DIF_NOOB ].name,
        difficultyTable[ DIF_RECRUIT ].name,
        difficultyTable[ DIF_MERC ].name,
        difficultyTable[ DIF_NOMAD ].name,
        difficultyTable[ DIF_BLACKDEATH ].name,
        s_newGame->hardestStrings[ s_newGame->hardestIndex ]
    };

    s_newGame->title = strManager->ValueForKey( "SP_NEWGAME" );
    s_newGame->newGameSaveNamePrompt = strManager->ValueForKey( "SP_SAVE_NAME_PROMPT" );
    s_newGame->newGameBegin = strManager->ValueForKey( "SP_BEGIN_NEWGAME" );

    s_newGame->menu.name = "##SinglePlayerNewGameMenu";
    s_newGame->menu.fullscreen = qtrue;
    s_newGame->menu.draw = NewGameMenu_Draw;
    s_newGame->menu.flags = MENU_DEFAULT_FLAGS;
    s_newGame->menu.width = ui->gpuConfig.vidWidth;
    s_newGame->menu.height = ui->gpuConfig.vidHeight;
    s_newGame->menu.titleFontScale = 3.5f;
    s_newGame->menu.textFontScale = 1.5f;
    s_newGame->menu.x = 0;
    s_newGame->menu.y = 0;

    s_newGame->difficultyList = difficulties;

    s_newGame->accept_0 = re.RegisterShader( "menu/accept_0" );
    s_newGame->accept_1 = re.RegisterShader( "menu/accept_1" );
}

void UI_NewGameMenu( void )
{
    NewGameMenu_Cache();
    UI_PushMenu( &s_newGame->menu );
}



//
// a small csv parser for c, credits to semitrivial for this
// https://github.com/semitrivial/csv_parser.git
//

void free_csv_line( char **parsed ) {
    char **ptr;

    for ( ptr = parsed; *ptr; ptr++ ) {
        Z_Free( *ptr );
    }

    Z_Free( parsed );
}

int32_t count_fields( const char *line ) {
    const char *ptr;
    int32_t cnt, fQuote;

    for ( cnt = 1, fQuote = 0, ptr = line; *ptr; ptr++ ) {
        if ( fQuote ) {
            if ( *ptr == '\"' ) {
                fQuote = 0;
            }
            continue;
        }

        switch ( *ptr ) {
        case '\"':
            fQuote = 1;
            continue;
        case ',':
            cnt++;
            continue;
        default:
            continue;
        };
    }

    if ( fQuote ) {
        return -1;
    }

    return cnt;
}

static char *CopyUIString( const char *str ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Hunk_Alloc( len, h_high );
    N_strncpyz( out, str, len );

    return out;
}

/*
 *  Given a string containing no linebreaks, or containing line breaks
 *  which are escaped by "double quotes", extract a NULL-terminated
 *  array of strings, one for every cell in the row.
 */
char **parse_csv( const char *line ) {
    char **buf, **bptr, *tmp, *tptr;
    const char *ptr;
    int32_t fieldcnt, fQuote, fEnd;

    fieldcnt = count_fields( line );

    if ( fieldcnt == -1 ) {
        return NULL;
    }

    buf = (char **)Hunk_Alloc( sizeof( char * ) * ( fieldcnt + 1 ), h_high );
    tmp = (char *)Hunk_AllocateTempMemory( strlen( line ) + 1 );

    bptr = buf;

    for ( ptr = line, fQuote = 0, *tmp = '\0', tptr = tmp, fEnd = 0; ; ptr++ ) {
        if ( fQuote ) {
            if ( !*ptr ) {
                break;
            }

            if ( *ptr == '\"' ) {
                if ( ptr[1] == '\"' ) {
                    *tptr++ = '\"';
                    ptr++;
                    continue;
                }
                fQuote = 0;
            }
            else {
                *tptr++ = *ptr;
            }

            continue;
        }

        switch ( *ptr ) {
        case '\"':
            fQuote = 1;
            continue;
        case '\0':
            fEnd = 1;
        case ',':
            *tptr = '\0';
            *bptr = CopyUIString( tmp );

            if ( !*bptr ) {
                for ( bptr--; bptr >= buf; bptr-- ) {
//                    Z_Free( *bptr );
                }
//                Z_Free( buf );
                Hunk_FreeTempMemory( tmp );

                return NULL;
            }

            bptr++;
            tptr = tmp;

            if ( fEnd ) {
                break;
            }
            continue;
        default:
            *tptr++ = *ptr;
            continue;
        };

        if ( fEnd ) {
            break;
        }
    }

    *bptr = NULL;
    Hunk_FreeTempMemory( tmp );
    return buf;
}
