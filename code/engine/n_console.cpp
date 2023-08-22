#include "n_shared.h"
#include "n_scf.h"
#include "../rendergl/imgui.h"
#include "../rendergl/rgl_public.h"

#if 0
typedef struct
{
    uint16_t text[MAX_CMD_BUFFER];
    uint32_t bytesUsed;
    uint32_t curlines;
    uint32_t maxlines;
    uint32_t current;
    uint32_t totallines;

    uint32_t x;
    uint32_t linewidth;

    qboolean newline;
    qboolean initialized;
} console_t;
static console_t con;
#endif

#define IMGUI_CONSOLE_WINDOW_FLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)

extern file_t logfile;

static cvar_t *con_noprint;

bool imgui_window_open = false;
static boost::shared_mutex conLock;
static eastl::vector<char> con_buffer;

#if 0
static void Con_Clear_f(void)
{
    for (uint32_t i = 0; i < con.linewidth; i++) {
        con.text[i] = (ColorIndex(C_WHITE) << 8) | ' ';
    }

    con.x = 0;
    con.current = 0;
    con.newline = qtrue;

    Con_Bottom();
}

/*
Con_Dump_f: save the current console buffer out to a file
*/
static void Con_Dump_f(void)
{
    uint16_t *line;
    char *buffer;
    uint64_t bufLen;
    char filename[MAX_GDR_PATH];
    const char *ext;
    file_t f;

    if (Cmd_Argc() != 2) {
        Con_Printf("usage: condump <filename>");
        return;
    }

    N_strncpyz(filename, Cmd_Argv(1), sizeof(filename));
    COM_DefaultExtension(filename, sizeof(filename), ".txt");

    if (!FS_AllowedExtension(filename, qfalse, &ext)) {
        Con_Printf("%s: Invalid filename extension '%s'.", __func__, ext);
        return;
    }

    f = FS_FOpenWrite(filename);
    if (f == FS_INVALID_HANDLE) {
        Con_Printf("ERROR: couldn't open %s.", filename);
        return;
    }

    Con_Printf("Dumped console buffer to %s.", filename);

    bufLen = con.bytesUsed;
    buffer = (char *)Hunk_AllocateTempMemory(bufLen);

    buffer[bufLen - 1] = '\0';

    for (uint64_t i = 0; i < n; i++) {
        line = con.text + ();
    }
}
#endif

static const char *Con_TextToColor(const char *s, ImVec4& v, qboolean *reset)
{
    static const char *colorStr;
    int colorIndex;

    if (*s != '\\' && *(s + 1) != '^')
        return NULL;

    s += 2;
    *reset = qfalse;
    if (*s == '8') {
        *reset = qtrue;
        return NULL;
    }

    colorIndex = ColorIndexFromChar(*s);
    memcpy(eastl::addressof(v), g_color_table[colorIndex], sizeof(vec4_t));
    return colorStr;
}

void Con_DrawConsole(void)
{
    ImVec4 colorVec;
    const char *colorStr;
    int c;
    qboolean reset;
    char *buf_p, *p;

    buf_p = con_buffer.begin();

    if ((p = strstr(buf_p, "\\^")) == NULL) {
        ImGui::Text("%s", con_buffer.data());
    }

    while (buf_p != con_buffer.end()) {
        p = strstr(buf_p, "\\^");

        if (p == NULL) {
            break;
        }
        else if (p == buf_p) {
            colorStr = Con_TextToColor(p, colorVec, &reset);

            if (!reset && colorStr) {
                ImGui::PushStyleColor(ImGuiCol_Text, colorVec);
                buf_p += 3;
            }
            else if (reset) {
                ImGui::PopStyleColor();
            }

            p = strstr(buf_p, "\\^");
            if (p == NULL) {
                ImGui::Text("%s", buf_p);
                break;
            }
        }
        else if (p > buf_p) {
            char buf[MAXPRINTMSG];
            strncpy(buf, p - 1, (size_t)(p - buf_p));
            buf[(size_t)(p - buf_p) - 1] = '\0';
            ImGui::Text("%s", buf);

            colorStr = Con_TextToColor(p, colorVec, &reset);

            if (!reset && colorStr) {
                ImGui::PushStyleColor(ImGuiCol_Text, colorVec);
            }
            else if (reset) {
                ImGui::PopStyleColor();
            }

            buf_p = p + 3;
            p = strstr(buf_p, "\\^");
            if (p == NULL) {
                ImGui::Text("%s", buf_p);
                break;
            }
        }
    }
}

/*
Con_AddText: appends the given string to the end of the console buffer
*/
void Con_AddText(const char *msg)
{
    boost::lock_guard<boost::shared_mutex> lock{conLock};

    const uint64_t length = strlen(msg);
    if (con_buffer.size() + length >= MAX_CMD_BUFFER) {
        char *ptr = strrchr(con_buffer.begin(), '\n');
        if (ptr)
            con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
}

/*
Con_InsertText: appends the given string into the console buffer, adds a '\n' to the end
*/
void Con_InsertText(const char *msg)
{
    boost::lock_guard<boost::shared_mutex> lock{conLock};

    const uint64_t length = strlen(msg);
    if (con_buffer.size() + length >= MAX_CMD_BUFFER) {
        char *ptr = strrchr(con_buffer.begin(), '\n');
        if (ptr)
            con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
    con_buffer.emplace_back('\n');
}

void GDR_DECL Con_Printf(loglevel_t level, const char *fmt, ...)
{
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured of %i bytes, buffer size is %lu bytes", length, sizeof(msg));
    }

    switch (level) {
#ifndef _NOMAD_DEBUG
    case DEBUG:
        return;
#else
    case DEBUG:
        Sys_Print(C_BLUE "DEBUG: ");
        break;
#endif
    case DEV:
        if (!Cvar_VariableBoolean("c_devmode"))
            return;
        
        Sys_Print(C_GREEN);
        break;
    case WARNING:
        Sys_Print(C_YELLOW "WARNING: ");
        break;
    case ERROR:
        Sys_Print(C_RED "ERROR: ");
        break;
    };

    Con_InsertText(msg);
    Sys_Printf("%s\n" C_RESET, msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n" C_RESET, 7, logfile);
    }
}

void GDR_DECL Con_Printf(const char *fmt, ...)
{
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }

    Con_InsertText(msg);
    Sys_Printf("%s\n", msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }
}

/*
================
Field_Paste
================
*/
static void Field_Paste( field_t *edit )
{
	char	*cbd;
	int		pasteLen, i;

//	cbd = Sys_GetClipboardData();

//	if ( !cbd ) {
//		return;
//	}
//
//	// send as if typed, so insert / overstrike works properly
//	pasteLen = strlen( cbd );
//	for ( i = 0 ; i < pasteLen ; i++ ) {
//		Field_CharEvent( edit, cbd[i] );
//	}
//
//	Z_Free( cbd );
}


/*
=================
Field_NextWord
=================
*/
static void Field_SeekWord( field_t *edit, int direction )
{
	if ( direction > 0 ) {
		while ( edit->buffer[ edit->cursor ] == ' ' )
			edit->cursor++;
		while ( edit->buffer[ edit->cursor ] != '\0' && edit->buffer[ edit->cursor ] != ' ' )
			edit->cursor++;
		while ( edit->buffer[ edit->cursor ] == ' ' )
			edit->cursor++;
	} else {
		while ( edit->cursor > 0 && edit->buffer[ edit->cursor-1 ] == ' ' )
			edit->cursor--;
		while ( edit->cursor > 0 && edit->buffer[ edit->cursor-1 ] != ' ' )
			edit->cursor--;
		if ( edit->cursor == 0 && ( edit->buffer[ 0 ] == '/' || edit->buffer[ 0 ] == '\\' ) )
			edit->cursor++;
	}
}


/*
=================
Field_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
static void Field_KeyDownEvent( field_t *edit )
{
	int		len;

	// shift-insert is paste
	if ( ImGui::IsKeyDown(ImGuiKey_Insert) && ImGui::IsKeyDown(ImGuiKey_LeftShift) ) {
		Field_Paste( edit );
		return;
	}

	len = strlen( edit->buffer );

    if (ImGui::IsKeyDown(ImGuiKey_Delete)) {
        if ( edit->cursor < len ) {
			memmove( edit->buffer + edit->cursor,
				edit->buffer + edit->cursor + 1, len - edit->cursor );
		}
    }
    else if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
        if ( edit->cursor < len ) {
			if ( ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ) {
				Field_SeekWord( edit, 1 );
			} else {
				edit->cursor++;
			}
		}
    }
    else if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
        if ( edit->cursor > 0 ) {
			if ( ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ) {
				Field_SeekWord( edit, -1 );
			} else {
				edit->cursor--;
			}
		}
    }
    else if (ImGui::IsKeyDown(ImGuiKey_Home)) {
        edit->cursor = 0;
    }
    else if (ImGui::IsKeyDown(ImGuiKey_End)) {
        edit->cursor = len;
    }

	// Change scroll if cursor is no longer visible
	if ( edit->cursor < edit->scroll ) {
		edit->scroll = edit->cursor;
	} else if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len ) {
		edit->scroll = edit->cursor - edit->widthInChars + 1;
	}
}


/*
==================
Field_CharEvent
==================
*/
static void Field_CharEvent( field_t *edit, int ch )
{
	int		len;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Field_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Field_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1,
				edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll )
			{
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < ' ' ) {
		return;
	}

#if 0
	if ( key_overstrikeMode ) {
		// - 2 to leave room for the leading slash and trailing \0
		if ( edit->cursor == MAX_EDIT_LINE - 2 )
			return;
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	} else {	// insert mode
#endif
		// - 2 to leave room for the leading slash and trailing \0
		if ( len == MAX_EDIT_LINE - 2 ) {
			return; // all full
		}
		memmove( edit->buffer + edit->cursor + 1,
			edit->buffer + edit->cursor, len + 1 - edit->cursor );
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
#if 0
	}
#endif

	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1) {
		edit->buffer[edit->cursor] = '\0';
	}
}

static field_t con_field;
static int Con_TextInputCallback(ImGuiInputTextCallbackData *data)
{
    N_strncpyz(con_field.buffer, data->Buf, sizeof(con_field.buffer));
    con_field.scroll = 0;
    con_field.cursor = data->CursorPos;
    con_field.widthInChars = strlen(con_field.buffer);

    if (ImGui::IsKeyDown(ImGuiKey_A) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        data->SelectAll();
    }

#if 0
    // command completetion
    if (ImGui::IsKeyDown(ImGuiKey_Tab)) {
        Field_AutoComplete(&con_field);
        data->BufTextLen = strlen(con_field.buffer);
        data->CursorPos = con_field.cursor;
        N_strncpyz(data->Buf, con_field.buffer, MAX_EDIT_LINE);
    }

    // command history, ctrl-p/ctrl-n for *nix style
    if ((ImGui::GetIO().MouseWheel >= 1.0 && ImGui::IsKeyDown(ImGuiKey_LeftShift)) || ImGui::IsKeyDown(ImGuiKey_UpArrow)
        || (ImGui::IsKeyDown(ImGuiKey_P) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))) {
        Con_HistoryGetPrev(&con_field);
        con_field.widthInChars = strlen(con_field.buffer);

        data->BufTextLen = strlen(con_field.buffer);
        data->CursorPos = con_field.cursor;
        N_strncpyz(data->Buf, con_field.buffer, MAX_EDIT_LINE);
        return 1;
    }
    if ((ImGui::GetIO().MouseWheel <= 0.0 && ImGui::IsKeyDown(ImGuiKey_LeftShift)) || ImGui::IsKeyDown(ImGuiKey_DownArrow)
        || (ImGui::IsKeyDown(ImGuiKey_N) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))) {
        Con_HistoryGetNext(&con_field);
        con_field.widthInChars = strlen(con_field.buffer);

        data->BufTextLen = strlen(con_field.buffer);
        data->CursorPos = con_field.cursor;
        N_strncpyz(data->Buf, con_field.buffer, MAX_EDIT_LINE);
        return 1;
    }
#endif

    // console scrolling
//    if (ImGui::IsKeyDown(ImGuiKey_PageUp) || ImGui::GetIO().MouseWheel >= 1.0) {
//        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
//        }
//    }
    // pass to the normal editline routine
	Field_KeyDownEvent( &con_field );
    return 1;
}

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
void Con_GetInput(void)
{
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2(Cvar_VariableInteger("r_screenwidth"), Cvar_VariableInteger("r_screenheight") / 2));
    ImGui::Begin("Command Console", NULL, IMGUI_CONSOLE_WINDOW_FLAGS);

    con_buffer.emplace_back('\0');
    ImGui::Text("%s", con_buffer.data());
    con_buffer.pop_back();

    if (!RE_ConsoleIsOpen())
        return; // nothing to process

    // ctrl-L clears the screen
    if (Key_IsDown(KEY_L) && keys[KEY_LCTRL].down) {
        Cbuf_AddText("clear\n");
        return;
    }

    ImGui::Text("] ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", con_field.buffer, sizeof(con_field.buffer), IMGUI_CONSOLE_INPUT_FLAGS, Con_TextInputCallback)) {
        Con_Printf("]%s", con_field.buffer); // echo it into the console
        Con_TextInput();
    }
    ImGui::End();
}

static void Con_Clear_f(void)
{
    con_buffer.clear();
}

void GDR_DECL Con_Error(bool exit, const char *fmt, ...)
{
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    Sys_Printf(C_RED "ERROR: %s\n" C_RESET, msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }
    if (exit)
        Sys_Exit(-1);
}

void Con_Shutdown(void)
{
    con_buffer.clear();
}

void Con_Init(void)
{
    con_buffer.reserve(MAX_CMD_BUFFER);
    Cmd_AddCommand("clear", Con_Clear_f);
    Cmd_AddCommand("cls", Con_Clear_f);
}