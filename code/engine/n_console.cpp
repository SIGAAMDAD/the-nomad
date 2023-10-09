#include "n_shared.h"
#include "n_scf.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../rendercommon/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../rendercommon/imstb_truetype.h"
#include "../rendercommon/imgui.h"
#include "../rendergl/rgl_public.h"

#define  CON_TEXTSIZE   65536

#define IMGUI_CONSOLE_WINDOW_FLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)

static char conBuffer[CON_TEXTSIZE];
static uint64_t usedBytes;

file_t logfile;
field_t con_field;
cvar_t *con_noprint;
cvar_t *con_color;
vec4_t conColor;
static boost::shared_mutex conLock;

static void Con_TextInput(void)
{
    // its a command
    if (con_field.buffer[0] == '/' || con_field.buffer[0] == '\\') {
        Cbuf_AddText(con_field.buffer + 1);
        Cbuf_AddText("\n");
        Cbuf_Execute();
    }
    else {
        if (!con_field.buffer[0]) {
            return; // empty lines just scroll th econsole without adding to history
        }
        else {
    //            Cbuf_AddText("cmd say ");
            Cbuf_AddText(con_field.buffer);
            Cbuf_AddText("\n");
        }
    }

    // copy to history buffer
    Con_SaveField(&con_field);
    con_field.widthInChars = 0;
}

#define IMGUI_CONSOLE_INPUT_FLAGS (ImGuiInputTextFlags_EnterReturnsTrue)

void Con_DrawConsole(void)
{
    uint32_t colorIndex, currentColorIndex;
    qboolean colorSet = qtrue;
    ImVec4 color;
    char msg[MAXPRINTMSG];
    uint64_t used;
    int c;
    const char *txt;

    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2(Cvar_VariableInteger("r_screenwidth"), Cvar_VariableInteger("r_screenheight") / 2));
    ImGui::Begin("Command Console", NULL, ImGuiWindowFlags_NoResize);

    if (con_noprint && con_noprint->i) {
        return; // not printing anything
    }

    if (!Key_GetCatcher() & KEYCATCH_CONSOLE)
        return; // nothing to process

    // ctrl-L clears the screen
    if (Key_IsDown(KEY_L) && keys[KEY_LCTRL].down) {
        Cbuf_AddText("clear\n");
        return;
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, { conColor[0], conColor[1], conColor[2], conColor[3] });

    {
        // quake3 style console
        for (used = 0; used < ImGui::GetWindowSize().x; used++)
            msg[used] = '^';
        
        msg[used] = 0;
        ImGui::Text("%s", msg);
        ImGui::NewLine();
        ImGui::NewLine();

        memset(msg, 0, used);
        used = 0;
    }

    currentColorIndex = ColorIndex(S_COLOR_WHITE);
    memcpy(&color, g_color_table[currentColorIndex], sizeof(vec4_t));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    used = 0;
    txt = conBuffer;
    while ((c = *txt) != 0) {
        if (used + 1 >= sizeof(msg) - 1) {
            msg[used + 1] = '\0';
            ImGui::Text("%s", msg);
            ImGui::SameLine();
        }
        
        if (Q_IsColorString(txt) && *(txt+1) != '\n') {
            colorIndex = ColorIndexFromChar(*(txt+1));
            if (currentColorIndex != colorIndex) {
                // flush before changing the color
                msg[used + 1] = 0;
                used = 0;
                ImGui::Text("%s", msg);
                ImGui::SameLine();

                currentColorIndex = colorIndex;
                if (colorSet) {
                    ImGui::PopStyleColor();
                }
                memcpy(&color, g_color_table[colorIndex], sizeof(vec4_t));
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                colorSet = qtrue;
            }
            txt += 2;
            continue;
        }

        switch (c) {
        case '\n':
            if (colorSet && currentColorIndex != colorIndex) { // pop the current color
                colorIndex = ColorIndex(S_COLOR_WHITE);
                currentColorIndex = colorIndex;
                memcpy(&color, g_color_table[colorIndex], sizeof(vec4_t));

                msg[used + 1] = 0;
                ImGui::Text("%s", msg);
                ImGui::SameLine();

                ImGui::PopStyleColor();
                ImGui::PushStyleColor(ImGuiCol_Text, color);
            }
            ImGui::NewLine();
            break;
        case '\r':
            ImGui::SameLine();
            break;
        default:
            msg[used] = c;
            used++;
            break;
        };
    }

    ImGui::Text("] ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", con_field.buffer, sizeof(con_field.buffer), IMGUI_CONSOLE_INPUT_FLAGS, NULL)) {
        Con_Printf("]%s\n", con_field.buffer); // echo it into the console
        Con_TextInput();
    }
}


//
// Con_AddText: appends the given string into the console buffer
//
void Con_AddText(const char *msg)
{
    boost::lock_guard<boost::shared_mutex> lock{conLock};

    const uint64_t length = strlen(msg);
    if (usedBytes + length >= CON_TEXTSIZE) {
        char *p = strrchr(conBuffer, '\n');
        if (p) { // linux console it
            memmove(conBuffer, p, (uintptr_t)(p - conBuffer));
        }
    }

    N_strcat(conBuffer, CON_TEXTSIZE, msg);
    usedBytes += length;
}

//
// Con_Clear_f: clears the console text buffer
//
static void Con_Clear_f(void)
{
    *conBuffer = 0;
    usedBytes = 0;
}

/*
================
Con_Init
================
*/
void Con_Init( void ) 
{
	Field_Clear( &con_field );
	con_field.widthInChars = MAX_EDIT_LINE;

    con_color = Cvar_Get("con_color", "1.00 1.00 1.00 1.00", CVAR_SAVE | CVAR_PRIVATE | CVAR_INIT);
    con_noprint = Cvar_Get("con_noprint", "0", CVAR_LATCH | CVAR_PRIVATE);

    conColor[0] = 1.0f;
    conColor[1] = 0.0f;
    conColor[2] = 0.0f;
    conColor[3] = 1.0f;
    sscanf(con_color->s, "%f %f %f %f", &conColor[0], &conColor[1], &conColor[2], &conColor[3]);

    Con_Clear_f();

	Cmd_AddCommand( "clear", Con_Clear_f );
    Cmd_AddCommand( "cls", Con_Clear_f );
//	Cmd_AddCommand( "condump", Con_Dump_f );
//	Cmd_SetCommandCompletionFunc( "condump", Cmd_CompleteTxtName );
//	Cmd_AddCommand( "toggleconsole", Con_ToggleConsole_f );
}


/*
================
Con_Shutdown
================
*/
void Con_Shutdown( void )
{
	Cmd_RemoveCommand( "clear" );
    Cmd_RemoveCommand( "cls" );
//	Cmd_RemoveCommand( "condump" );
//	Cmd_RemoveCommand( "toggleconsole" );
}
