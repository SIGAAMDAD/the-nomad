#include "n_shared.h"
#include "m_renderer.h"
#include "n_scf.h"

#define MAX_CMD_LINE 1024

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool imgui_window_open = false;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(const char *msg, int length)
{
    if (con_buffer.size() > MAX_BUFFER_SIZE) {
        eastl::vector<char>::iterator ptr = con_buffer.end() - MAX_BUFFER_SIZE;
        while (ptr != con_buffer.end()) {
            if (*ptr == '\n') {
                ptr--;
                break;
            }
            ptr++;
        }
        con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
    con_buffer.emplace_back('\n');
}

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    if (level == DEV) {
        fprintf(stdout, "DEV: ");
    }
    else if (level == DEBUG) {
#ifdef _NOMAD_DEBUG
        fprintf(stdout, "DEBUG: ");
#else
        return;
#endif
    }

    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
    Con_ClearBuffer(msg, length);
}

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
    Con_ClearBuffer(msg, length);
}

#define MAX_CMD_BUFFER  65536

typedef struct
{
	byte *data;
	uint64_t maxsize;
	uint64_t cursize;
} cbuf_t;

static uint32_t cmd_wait;
static cbuf_t cmd_text;
static byte cmd_text_buf[MAX_CMD_BUFFER];

/*
============
Cbuf_Init
============
*/
void Cbuf_Init(void)
{
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void Cbuf_AddText(const char *text)
{

	const uint64_t l = (uint64_t)strlen(text);

	if (cmd_text.cursize + l >= cmd_text.maxsize) {
		Con_Printf ("Cbuf_AddText: overflow");
		return;
	}

	memcpy(&cmd_text.data[cmd_text.cursize], text, l);
	cmd_text.cursize += l;
}


/*
============
Cbuf_Add

// Adds command text at the specified position of the buffer, adds \n when needed
============
*/
uint64_t Cbuf_Add(const char *text, int64_t pos)
{

	uint64_t len = (uint64_t)strlen(text);
	qboolean separate = qfalse;
	uint32_t i;

	if (len == 0)
		return cmd_text.cursize;

	if (pos > cmd_text.cursize || pos < 0) {
		// insert at the text end
		pos = cmd_text.cursize;
	}

	if (text[len - 1] == '\n' || text[len - 1] == ';') {
		// command already has separator
	}
    else {
		separate = qtrue;
		len += 1;
	}

	if (len + cmd_text.cursize > cmd_text.maxsize) {
		Con_Printf("%s(%i) overflowed", __func__, pos);
		return cmd_text.cursize;
	}

	// move the existing command text
	for (i = cmd_text.cursize - 1; i >= pos; i--) {
		cmd_text.data[i + len] = cmd_text.data[i];
	}

	if (separate) {
		// copy the new text in + add a \n
		memcpy(cmd_text.data + pos, text, len - 1);
		cmd_text.data[pos + len - 1] = '\n';
	}
    else {
		// copy the new text in
		memcpy(cmd_text.data + pos, text, len);
	}

	cmd_text.cursize += len;

	return pos + len;
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void Cbuf_InsertText(const char *text)
{
	uint32_t len;
	int32_t i;

	len = strlen(text) + 1;

	if (len + cmd_text.cursize > cmd_text.maxsize) {
		Con_Printf("Cbuf_InsertText overflowed");
		return;
	}

	// move the existing command text
	for (i = cmd_text.cursize - 1; i >= 0; i--) {
		cmd_text.data[i + len] = cmd_text.data[i];
	}

	// copy the new text in
    memcpy(cmd_text.data, text, len - 1);

	// add a \n
	cmd_text.data[len - 1] = '\n';

	cmd_text.cursize += len;
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText(const char *text)
{
    Cmd_ExecuteString(text);
//	switch (exec_when) {
//	case EXEC_NOW: {
//		if (text && text[0] != '\0') {
//			Con_Printf(DEBUG, "EXEC_NOW %s", text);
//			Cmd_ExecuteString(text);
//		}
//        else {
//			Cbuf_Execute();
//			Con_Printf(DEBUG, "EXEC_NOW %s", cmd_text.data);
//		}
//		break; }
//	case EXEC_INSERT:
//		Cbuf_InsertText(text);
//		break;
//	case EXEC_APPEND:
//		Cbuf_AddText(text);
//		break;
//	default:
//	    N_Error("Cbuf_ExecuteText: bad exec_when");
//	};
}


/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute(void)
{
	uint32_t i;
	char *text;
	char line[MAX_CMD_LINE];
    uint32_t quotes;

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	qboolean in_star_comment = qfalse;
	qboolean in_slash_comment = qfalse;
	while (cmd_text.cursize > 0) {
		if (cmd_wait > 0) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}

		// find a \n or ; line break or comment: // or /* */
		text = (char *)cmd_text.data;

		quotes = 0;
		for (i = 0; i < cmd_text.cursize; i++) {
			if (text[i] == '"')
				quotes++;

			if (!(quotes & 1)) {
				if (i < cmd_text.cursize - 1) {
					if (!in_star_comment && text[i] == '/' && text[i+1] == '/')
						in_slash_comment = qtrue;
					else if (!in_slash_comment && text[i] == '/' && text[i+1] == '*')
						in_star_comment = qtrue;
					else if (in_star_comment && text[i] == '*' && text[i+1] == '/') {
						in_star_comment = qfalse;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if (!in_slash_comment && !in_star_comment && text[i] == ';')
					break;
			}
			if (!in_star_comment && (text[i] == '\n' || text[i] == '\r')) {
				in_slash_comment = qfalse;
				break;
			}
		}

		if (i >= (MAX_CMD_LINE - 1))
			i = MAX_CMD_LINE - 1;

		memcpy(line, text, i);
		line[i] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if (i == cmd_text.cursize)
			cmd_text.cursize = 0;
		else {
			i++;
			cmd_text.cursize -= i;
			// skip all repeating newlines/semicolons
			while ((text[i] == '\n' || text[i] == '\r' || text[i] == ';') && cmd_text.cursize > 0) {
				cmd_text.cursize--;
				i++;
			}
			memmove(text, text+i, cmd_text.cursize);
		}

		// execute the command line
		Cmd_ExecuteString(line);
	}
}

static void Con_ProcessInput(void)
{
    if (!console_open)
        return; // nothing to process
    
    char buffer[MAX_CMD_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    
    ImGui::Text("> ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", buffer, MAX_CMD_BUFFER, ImGuiInputTextFlags_EnterReturnsTrue)) {
        Con_Printf("> %s", buffer);
        Cmd_ExecuteText(buffer);
    }

}

void Con_EndFrame(void)
{
    int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
    ImGui::SetWindowSize(ImVec2(N_atoi(r_screenwidth.value), (float)(N_atoi(r_screenheight.value) >> 1)));
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::Begin("Command Console", NULL, flags);
    con_buffer.emplace_back('\0');
    ImGui::Text("%s", con_buffer.data());
    con_buffer.pop_back();

    Con_ProcessInput();

    ImGui::End();
    if (console_open) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void Con_Error(const char *fmt, ...)
{
    va_list argptr;

    fprintf(stderr,"ERROR: ");
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define MAX_CVAR_HASH 2048
static cvar_t cvar_list{0};

cvar_t* Cvar_Find(const char *name)
{
    cvar_t* cvar;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (N_strncasecmp(name, cvar->name, 79)) {
            return cvar;
        }
    }
    return NULL;
}

#if 0
cvar_t* Cvar_Set2(const char *name, const char *value, qboolean force)
{
    cvar_t *cvar;

    if (!Cvar_ValidateName(name)) {
        Con_Printf("invalid cvar name string: %s", name);
        name = "BADNAME";
    }

    cvar = Cvar_Find(name);
    if (!cvar) {
        if (!value)
            return NULL;
        // create it
//        if (!force)
//            return Cvar_Get(name, value);
//        else
        return Cvar_Get(name, value);
    }
}
#endif

static void Cvar_Print(const cvar_t *var)
{
    Con_Printf("\"%s\" is: \"%s\"", var->name, var->value);
}

static void Cvar_Print_f(void)
{
	const char *name;
	cvar_t *cv;
	
	if (Cmd_Argc() != 2) {
		Con_Printf("usage: print <variable>");
		return;
	}

	name = Cmd_Argv(1);

	cv = Cvar_Find(name);
	
	if (cv)
		Cvar_Print(cv);
	else
		Con_Printf("Cvar %s does not exist", name);
}

qboolean Cvar_Command(void)
{
    cvar_t *cvar;

    // check variables
    cvar = Cvar_Find(Cmd_Argv(0));
    if (!cvar)
        return qfalse;

    // perform a variable print or set
    if (Cmd_Argc() == 1) {
        Cvar_Print(cvar);
        return qtrue;
    }

    // set the value if forcing isn't required
//    Cvar_Set2(cvar->name, Cmd_ArgsFrom(1), false);
}

void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, qboolean save)
{
    cvar_t* cvar;

    if (Cvar_Find(name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", name);
        return;
    }

    for (cvar = &cvar_list; cvar->next; cvar = cvar->next);
    cvar->next = (cvar_t *)Z_Malloc(sizeof(*cvar), TAG_STATIC, &cvar->next, "cvar");
    cvar = cvar->next;

    cvar->name = (char *)Z_Malloc(80, TAG_STATIC, &cvar->name, "cvar");
    N_strncpy(cvar->name, name, 79);
    cvar->name[79] = '\0';

    cvar->value = (char *)Z_Malloc(80, TAG_STATIC, &cvar->value, "cvar");
    N_strncpy(cvar->value, value, 79);
    cvar->value[79] = '\0';

    cvar->type = type;
    cvar->save = save;
}

void Cvar_ChangeValue(const char *name, const char *value)
{
    cvar_t* cvar;

    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempting to change the value of non-existent cvar %s", name);
        return;
    }
    N_strncpy(cvar->value, value, 80);
    cvar->value[79] = '\0';
}

void Cvar_Register(cvar_t* cvar)
{
    cvar_t* other;

    if (Cvar_Find(cvar->name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", cvar->name);
        return;
    }

    for (other = &cvar_list; other->next; other = other->next);

    other->next = cvar;
}

#define DEFAULT_CFG_NAME "nomadconfig.scf"

static void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    json data;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }
    for (cvar = G_GetCvars()[0]; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }

    char cfgpath[256];
    stbsp_snprintf(cfgpath, sizeof(cfgpath), "%s%c%s", fs_gamedir.value, PATH_SEP, DEFAULT_CFG_NAME);
    std::ofstream file(cfgpath, std::ios::out);
    if (!file.is_open()) {
        Con_Printf("WARNING: failed to open file %s to write cvar config", cfgpath);
        return;
    }
    file << data;
    file.close();
}

static void Cvar_List_f(void)
{
    cvar_t *cvar;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        Con_Printf("%8s = %8s", cvar->name, cvar->value);
    }
}

void Con_Init()
{
    con_buffer.reserve(MAX_BUFFER_SIZE);
    Cmd_AddCommand("writecfg", Cvar_WriteCfg_f);
    Cmd_AddCommand("listcvars", Cvar_List_f);
}