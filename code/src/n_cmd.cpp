#include "n_shared.h"

#define MAX_CMD_BUFFER  65536

typedef struct
{
    byte *data;
    uint32_t maxsize;
    uint32_t cursize;
} cmd_t;

static cmd_t cmd_text;
static byte cmd_text_buf[MAX_CMD_BUF];

void Cbuf_Init(void)
{
    cmd_text.data = cmd_text_buf;
    cmd_text.maxsize = MAX_CMD_BUFFER;
    cmd_text.cursize = 0;
}

void Cbud_AddText(const char *text)
{
    const int l = (int)strlen(text);
    
    if (cmd_text.cursize + l >= cmd_text.maxsize) {
        Con_Printf("Cbuf_AddText: overflow");
        return;
    }

    memcpy(&cmd_text.data[cmd_text.cursize], text, l);
    cmd_text.cursize += l;
}

int Cbuf_Add( const char *text, int pos )
{
	int len = (int)strlen( text );
	qboolean separate = qfalse;
	int i;

	if ( len == 0 ) {
		return cmd_text.cursize;
	}

	if ( pos > cmd_text.cursize || pos < 0 ) {
		// insert at the text end
		pos = cmd_text.cursize;
	}

	if ( text[len - 1] == '\n' || text[len - 1] == ';' ) {
		// command already has separator
	} else {
		separate = qtrue;
		len += 1;
	}

	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Con_Printf("%s(%i) overflowed\n", FUNC_SIG, pos );
		return cmd_text.cursize;
	}

	// move the existing command text
	for ( i = cmd_text.cursize - 1; i >= pos; i-- ) {
		cmd_text.data[i + len] = cmd_text.data[i];
	}

	if ( separate ) {
		// copy the new text in + add a \n
		memcpy( cmd_text.data + pos, text, len - 1 );
		cmd_text.data[pos + len - 1] = '\n';
	} else {
		// copy the new text in
		memcpy( cmd_text.data + pos, text, len );
	}

	cmd_text.cursize += len;

	return pos + len;
}