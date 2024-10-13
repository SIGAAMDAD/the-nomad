#include "g_game.h"
#include "g_font.h"
#include "../rendercommon/imgui_impl_opengl3.h"

CUIFontCache *g_pFontCache;

ImFont *AlegreyaSC;
ImFont *PressStart2P;
ImFont *RobotoMono;


CUIFontCache::CUIFontCache( void ) {
	union {
		void *v;
		char *b;
	} f;
	uint64_t nLength;
	const char **text;
	const char *tok, *text_p;
	float scale;
	char name[MAX_NPATH];

	Con_Printf( "Initializing font cache...\n" );

	memset( m_FontList, 0, sizeof( m_FontList ) );
	m_pCurrentFont = NULL;

	nLength = FS_LoadFile( "fonts/font_config.txt", &f.v );
	if ( !nLength || !f.v ) {
		N_Error( ERR_FATAL, "CUIFontCache::Init: failed to load fonts/font_config.txt" );
	}

	text_p = f.b;
	text = (const char **)&text_p;

	COM_BeginParseSession( "fonts/font_config.txt" );

	tok = COM_ParseExt( text, qtrue );
	if ( tok[0] != '{' ) {
		COM_ParseError( "expected '{' at beginning of file" );
		FS_FreeFile( f.v );
		return;
	}
	while ( 1 ) {
		tok = COM_ParseExt( text, qtrue );
		if ( !tok[0] ) {
			COM_ParseError( "unexpected end of file" );
			break;
		}
		if ( tok[0] == '}' ) {
			break;
		}

		scale = 1.0f;

		if ( tok[0] == '{' ) {
			while ( 1 ) {
				tok = COM_ParseExt( text, qtrue );
				if ( !tok[0] ) {
					COM_ParseError( "unexpected end of font defintion" );
					FS_FreeFile( f.v );
					return;
				}
				if ( tok[0] == '}' ) {
					break;
				}
				if ( !N_stricmp( tok, "name" ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter for 'name'" );
						FS_FreeFile( f.v );
						return;
					}
					N_strncpyz( name, tok, sizeof( name ) - 1 );
				}
				else if ( !N_stricmp( tok, "scale" ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter for 'scale'" );
						FS_FreeFile( f.v );
						return;
					}
					scale = atof( tok );
				}
				else {
					COM_ParseWarning( "unrecognized token '%s'", tok );
				}
			}
			if ( !AddFontToCache( name, "", scale ) ) {
				N_Error( ERR_FATAL, "CUIFontCache::Init: failed to load font data for 'fonts/%s/%s.ttf'", name, name );
			}
		}
	}

	FS_FreeFile( f.v );
}

void CUIFontCache::SetActiveFont( ImFont *font )
{
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( !ImGui::GetFont()->ContainerAtlas ) {
		return;
	}
	if ( m_pCurrentFont ) {
		ImGui::PopFont();
	}
	m_pCurrentFont = font;
	ImGui::PushFont( font );
}

void CUIFontCache::SetActiveFont( nhandle_t hFont )
{
	if ( !ImGui::GetFont()->ContainerAtlas ) {
		return;
	}
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( hFont == FS_INVALID_HANDLE || !m_FontList[ hFont ] ) {
		return;
	}

	if ( m_pCurrentFont ) {
		ImGui::PopFont();
	}

	m_pCurrentFont = m_FontList[ hFont ]->m_pFont;

	ImGui::PushFont( m_pCurrentFont );
}

uiFont_t *CUIFontCache::GetFont( const char *fileName ) {
	return m_FontList[ Com_GenerateHashValue( fileName, MAX_UI_FONTS ) ];
}

void CUIFontCache::ClearCache( void ) {
	if ( ImGui::GetCurrentContext() && ImGui::GetIO().Fonts ) {
		ImGui::GetIO().Fonts->Clear();
	}
	memset( m_FontList, 0, sizeof( m_FontList ) );
	m_pCurrentFont = NULL;

	RobotoMono = NULL;
	PressStart2P = NULL;
	AlegreyaSC = NULL;
}

void CUIFontCache::Finalize( void ) {
	ImGui::GetIO().Fonts->Build();
	ImGui_ImplOpenGL3_CreateFontsTexture();
}

nhandle_t CUIFontCache::RegisterFont( const char *filename, const char *variant, float scale ) {
	uint64_t hash;
	char rpath[MAX_NPATH];
	char hashpath[MAX_NPATH];

	COM_StripExtension( filename, rpath, sizeof( rpath ) );
	if ( rpath[ strlen( rpath ) - 1 ] == '.' ) {
		rpath[ strlen( rpath) - 1 ] = 0;
	}
	Com_snprintf( hashpath, sizeof( hashpath ) - 1, "%s", rpath );

	hash = Com_GenerateHashValue( hashpath, MAX_UI_FONTS );
	AddFontToCache( filename, variant, scale );

	return hash;
}

ImFont *CUIFontCache::AddFontToCache( const char *filename, const char *variant, float scale )
{
	uiFont_t *font;
	uint64_t size;
	uint64_t hash;
	ImFontConfig config;
	const char *path;
	union {
		void *v;
		char *b;
	} f;
	char rpath[MAX_NPATH];
	char hashpath[MAX_NPATH];

	COM_StripExtension( filename, rpath, sizeof( rpath ) );
	if ( rpath[ strlen( rpath ) - 1 ] == '.' ) {
		rpath[ strlen( rpath) - 1 ] = 0;
	}

	Com_snprintf( hashpath, sizeof( hashpath ) - 1, "%s", rpath );

	path = va( "fonts/%s.ttf", hashpath );
	hash = Com_GenerateHashValue( hashpath, MAX_UI_FONTS );

	//
	// see if we already have the font in the cache
	//
	for ( font = m_FontList[hash]; font; font = font->m_pNext ) {
		if ( !N_stricmp( font->m_szName, hashpath ) ) {
			return font->m_pFont; // its already been loaded
		}
	}

	Con_Printf( "CUIFontCache: loading font '%s'...\n", path );

	if ( strlen( hashpath ) >= MAX_NPATH ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: name '%s' is too long", hashpath );
	}

	size = FS_LoadFile( path, &f.v );
	if ( !size || !f.v ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: failed to load font file '%s'", path );
	}

	font = (uiFont_t *)Hunk_Alloc( sizeof( *font ), h_low );

	font->m_pNext = m_FontList[hash];
	m_FontList[hash] = font;

	config.FontDataOwnedByAtlas = false;
	config.GlyphExtraSpacing.x = 0.0f;

	N_strncpyz( font->m_szName, hashpath, sizeof( font->m_szName ) );
	font->m_nFileSize = size;
	font->m_pFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF( f.v, size, 16.0f * scale, &config );

	FS_FreeFile( f.v );

	return font->m_pFont;
}

void CUIFontCache::ListFonts_f( void ) {
	uint64_t memSize, i;
	uint64_t numFonts;
	const uiFont_t *font;

	Con_Printf( "---------- Font Cache Info ----------\n" );

	numFonts = 0;
	memSize = 0;
	for ( i = 0; i < MAX_UI_FONTS; i++ ) {
		font = g_pFontCache->m_FontList[i];

		if ( !font ) {
			continue;
		}

		Con_Printf( "[%s]\n", font->m_szName );
		Con_Printf( "File Size: %lu\n", font->m_nFileSize );

		memSize += font->m_nFileSize;
		numFonts++;
	}

	Con_Printf( "\n" );
	Con_Printf( "%-8lu total bytes in font cache\n", memSize );
	Con_Printf( "%-8lu total fonts in cache\n", numFonts );
}