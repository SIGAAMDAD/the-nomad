#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"

vec4_t colorGold = { 0.71f, 0.65f, 0.26f, 1.0f };

//sfxHandle_t menu_in_sound;
//sfxHandle_t menu_move_sound;
//sfxHandle_t menu_out_sound;
//sfxHandle_t menu_buzz_sound;
//sfxHandle_t menu_null_sound;
//sfxHandle_t weaponChangeSound;

//static nhandle_t	sliderBar;
//static nhandle_t	sliderButton_0;
//static nhandle_t	sliderButton_1;

//vec4_t menu_text_color	    = {1.0f, 1.0f, 1.0f, 1.0f};
//vec4_t menu_dim_color       = {0.0f, 0.0f, 0.0f, 0.75f};
vec4_t color_black	    = {0.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_white	    = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_yellow	    = {1.00f, 1.00f, 0.00f, 1.00f};
vec4_t color_blue	    = {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_lightOrange    = {1.00f, 0.68f, 0.00f, 1.00f };
vec4_t color_orange	    = {1.00f, 0.43f, 0.00f, 1.00f};
vec4_t color_red	    = {1.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_dim	    = {0.00f, 0.00f, 0.00f, 0.25f};

// current color scheme
//vec4_t pulse_color          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t text_color_disabled  = { 0.50f, 0.50f, 0.50f, 1.00f };	// light gray
vec4_t text_color_normal    = { 1.00f, 0.43f, 0.00f, 1.00f};	// white
//vec4_t text_color_highlight = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow
//vec4_t listbar_color        = {1.00f, 0.43f, 0.00f, 0.30f};	// transluscent orange
//vec4_t text_color_status    = {1.00f, 1.00f, 1.00f, 1.00f};	// bright white	


static void Slider_Draw( menuslider_t *slider );
static void TabList_Draw( menutab_t *tab );
static void Table_Draw( menutable_t *table );
static void Text_Draw( menutext_t *text );
static void RadioButton_Draw( menuswitch_t *button );
static void List_Draw( menulist_t *list );
static void Button_Draw( menubutton_t *button );

static void Field_CharEvent( ImGuiInputTextCallbackData *data, int ch );
static void Field_Paste( ImGuiInputTextCallbackData *data );
static void Field_Clear( menufield_t *edit );

static void Menu_DrawItemList( void **items, int numitems );
static void Menu_DrawItemGeneric( menucommon_t *generic );

static void Field_Clear( menufield_t *edit )
{
	edit->buffer[0] = 0;
	edit->cursor = 0;
	edit->scroll = 0;
}

/*
==================
Field_CharEvent
==================
*/
void Field_CharEvent( ImGuiInputTextCallbackData *data, int ch )
{
	int len;
	menufield_t *edit;

	edit = (menufield_t *)data->UserData;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Field_Paste( data );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Field_Clear( edit );
		data->DeleteChars( 0, data->BufTextLen );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1, edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			data->DeleteChars( data->CursorPos, 1 );
			if ( edit->cursor < edit->scroll ) {
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
		edit->scroll = edit->cursor - edit->widthInChars + 1;
		if ( edit->scroll < 0 ) {
			edit->scroll = 0;
		}
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( !Key_GetOverstrikeMode() ) {	
		if ( ( edit->cursor == MAX_EDIT_LINE - 1 ) || ( edit->maxchars && edit->cursor >= edit->maxchars ) ) {
			return;
        }
	} else {
		// insert mode
		if ( ( len == MAX_EDIT_LINE - 1 ) || ( edit->maxchars && len >= edit->maxchars ) ) {
			return;
        }
		memmove( edit->buffer + edit->cursor + 1, edit->buffer + edit->cursor, len + 1 - edit->cursor );
	}

	edit->buffer[edit->cursor] = ch;
	if ( !edit->maxchars || edit->cursor < edit->maxchars - 1 ) {
		data->InsertChars( data->CursorPos, &edit->buffer[ edit->cursor ] );
		edit->cursor++;
    }

	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1 ) {
		edit->buffer[edit->cursor] = 0;
	}

	data->CursorPos = edit->cursor;
	data->BufTextLen = strlen( edit->buffer );
}

/*
================
MField_Paste
================
*/
static void Field_Paste( ImGuiInputTextCallbackData *data ) {
	char	pasteBuffer[MAX_EDIT_LINE];
	int		pasteLen, i;

    memcpy( pasteBuffer, Sys_GetClipboardData(), sizeof( pasteBuffer ) );

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( pasteBuffer );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		Field_CharEvent( data, pasteBuffer[i] );
	}
}

static int Field_Callback( ImGuiInputTextCallbackData *data )
{
	Field_CharEvent( data, (int)data->EventChar );
}

static void Field_Draw( menufield_t *edit )
{
	Assert( edit->generic.type == MTYPE_FIELD );

	const int flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackEdit;
	if ( ImGui::InputText( edit->generic.name, edit->buffer, edit->maxchars, flags, Field_Callback, edit ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
}

static void Button_Draw( menubutton_t *button )
{
	Assert( button->generic.type == MTYPE_BUTTON );

	if ( ImGui::Button( button->generic.name ) ) {
		Snd_PlaySfx( ui->sfx_select );
		button->generic.eventcallback( button, EVENT_ACTIVATED );
	}
}

static void Arrow_Draw( menuarrow_t *arrow )
{
	Assert( arrow->generic.type == MTYPE_ARROW );

	if ( ImGui::ArrowButton( arrow->generic.name, arrow->direction ) ) {
		Snd_PlaySfx( ui->sfx_select );
		arrow->generic.eventcallback( arrow, EVENT_ACTIVATED );
	}
}

static void Slider_Draw( menuslider_t *slider )
{
	int flags;

	Assert( slider->generic.type == MTYPE_SLIDER );

	flags = ImGuiSliderFlags_None;
	if ( slider->generic.flags & QMF_INACTIVE ) {
		flags |= ImGuiSliderFlags_NoInput;
	}
	if ( slider->generic.flags & QMF_MOUSEONLY ) {
//		flags |= ImGuiSliderFlags_;
	}

	if ( slider->isIntegral ) {
		if ( ImGui::SliderInt( slider->generic.name, (int *)&slider->curvalue, (int)slider->minvalue, (int)slider->maxvalue, "%d", flags ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		slider->curvalue = (int)Com_Clamp( slider->minvalue, slider->maxvalue, slider->curvalue );
	} else {
		if ( ImGui::SliderFloat( slider->generic.name, &slider->curvalue, (int)slider->minvalue, (int)slider->maxvalue, "%.3f", flags ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		slider->curvalue = Com_Clamp( slider->minvalue, slider->maxvalue, slider->curvalue );
	}
}

static void RadioButton_Draw( menuswitch_t *button )
{
	Assert( button->generic.type == MTYPE_RADIOBUTTON );
	
	if ( ImGui::RadioButton( button->curvalue ? va( "ON##%sON", button->generic.name ) : va( "OFF##%sOFF", button->generic.name ),
		button->curvalue ) )
	{
		if ( !( button->generic.flags & QMF_INACTIVE ) ) {
			if ( !( button->generic.flags & QMF_SILENT ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			button->curvalue = !button->curvalue;
			if ( button->generic.eventcallback ) {
				button->generic.eventcallback( button, EVENT_ACTIVATED );
			}
		}
	}
}

static void ListEx_Draw( menulistex_t *list )
{
	int i;

	if ( ImGui::BeginMenu( list->generic.name, ( list->generic.flags & QMF_INACTIVE ) ) ) {
		if ( ImGui::IsItemActive() && ImGui::IsItemClicked() && !( list->generic.flags & QMF_SILENT ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		for ( i = 0; i < list->numitems; i++ ) {
			if ( list->items[i]->flags & QMF_OWNERDRAW ) {
				list->items[i]->ownerdraw( list->items[i] );
				continue;
			}
			switch ( list->items[i]->type ) {
			case MTYPE_TEXT: {
				if ( ImGui::MenuItem( ( (menutext_t *)list->items[i] )->text ) ) {
					if ( list->items[i]->eventcallback ) {
						list->items[i]->eventcallback( list->items[i], EVENT_ACTIVATED );
					}
				}
				break; }
			case MTYPE_SLIDER:
			case MTYPE_BUTTON:
			case MTYPE_ARROW:
			case MTYPE_LIST:
			case MTYPE_CUSTOM:
				Menu_DrawItemGeneric( list->items[i] );
				break;
			case MTYPE_NULL:
			default:
				break;
			};
		}
		ImGui::EndMenu();
	}
	if ( !ImGui::IsItemActive() && ImGui::IsItemClicked() && !( list->generic.flags & QMF_SILENT ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
}

static void Tree_Draw( menutree_t *tree )
{
	int flags;
	
	flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader;
	
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)tree->generic.name, flags, tree->generic.name ) ) {
		if ( ImGui::IsItemClicked() && !( tree->generic.flags & QMF_SILENT ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		Menu_DrawItemList( (void **)tree->items, tree->numitems );
		ImGui::TreePop();
	}
	else if ( !ImGui::IsItemActive() && ImGui::IsItemClicked() && !( tree->generic.flags & QMF_SILENT ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
}

static void TabList_Draw( menutab_t *tab )
{
	int i;
	
	Assert( tab->generic.type == MTYPE_TAB );
	
	if ( ImGui::BeginTabBar( tab->generic.name ) ) {
		if ( ImGui::IsItemActive() && ImGui::IsItemClicked() && !( tab->generic.flags & QMF_SILENT ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		ImGui::PushStyleColor( ImGuiCol_Tab, tab->tabColor );
		ImGui::PushStyleColor( ImGuiCol_TabHovered, tab->tabColorFocused );
		ImGui::PushStyleColor( ImGuiCol_TabActive, tab->tabColorActive );
		for ( i = 0; i < tab->numitems; i++ ) {
			if ( ImGui::BeginTabItem( tab->items[i]->name ) ) {
				if ( tab->curitem != i ) {
					if ( !( tab->generic.flags & QMF_SILENT ) && !( tab->items[i]->flags & QMF_SILENT ) ) {
						Snd_PlaySfx( ui->sfx_select );
					}
					tab->curitem = i;
					if ( tab->generic.eventcallback ) {
						tab->generic.eventcallback( tab, EVENT_ACTIVATED );
					}
				}
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}
	if ( !ImGui::IsItemActive() && ImGui::IsItemClicked() && !( tab->generic.flags & QMF_SILENT ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
}

static void List_Draw( menulist_t *list )
{
	int i;
	int flags;

	Assert( list->generic.type == MTYPE_LIST );

	if ( ImGui::BeginCombo( va( "%s##%sDropDown", list->generic.name, list->generic.name ), list->itemnames[list->curitem] ) ) {
		if ( ImGui::IsItemActive() && ImGui::IsItemClicked() && !( list->generic.flags & QMF_SILENT ) ) {
			Snd_PlaySfx( ui->sfx_select );
		}
		for ( i = 0; i < list->numitems; i++ ) {
			if ( ImGui::Selectable( va( "%s##%sSelectable_%i", list->itemnames[i], list->itemnames[i], i ), list->curitem == i ) ) {
				list->curitem = i;
				if ( list->generic.eventcallback ) {
					list->generic.eventcallback( list, EVENT_ACTIVATED );
				}
			}
		}
		ImGui::EndCombo();
	}
	if ( !ImGui::IsItemActive() && ImGui::IsItemClicked() && !( list->generic.flags & QMF_SILENT ) ) {
		Snd_PlaySfx( ui->sfx_select );
	}
}

static void Table_Draw( menutable_t *table )
{
	int i, c;
	menucommon_t *generic;

	Assert( table->generic.type == MTYPE_TABLE );

	ImGui::BeginTable( table->generic.name, table->columns );

	if ( table->generic.flags & QMF_OWNERDRAW && table->generic.ownerdraw ) {
		table->generic.ownerdraw( table );
	}
	else {
		for ( i = 0; i < table->rows; i++ ) {
			for ( c = 0; c < table->columns; c++ ) {
				ImGui::TableNextColumn();

				generic = table->items[i * table->columns + c];

				Menu_DrawItemGeneric( generic );
			}
			if ( i != table->rows - 1 ) {
				ImGui::TableNextRow();
			}
		}
	}
	ImGui::EndTable();
}

static void Text_Draw( menutext_t *text )
{
	qboolean colorChanged;

	Assert( text->generic.type == MTYPE_TEXT );

	colorChanged = qfalse;
	if ( text->generic.flags & QMF_GRAYED ) {
		ImGui::PushStyleColor( ImGuiCol_Text, colorMdGrey );
		colorChanged = qtrue;
	}
	else if ( !( ( text->generic.flags & QMF_HIGHLIGHT_IF_FOCUS && text->generic.focused )
		|| text->generic.flags & QMF_HIGHLIGHT ) )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, text->color );
		colorChanged = qtrue;
	}
	if ( text->generic.flags & QMF_OWNERDRAW && text->generic.ownerdraw ) {
		text->generic.ownerdraw( text );
	} else {
		ImGui::TextUnformatted( text->text );
	}

	if ( colorChanged ) {
		ImGui::PopStyleColor();
	}
}

static void Menu_DrawItemGeneric( menucommon_t *generic )
{
	int colorDepth;
	float fontScale;

	// set font
	if ( generic->font != ui->currentFont ) {
		FontCache()->SetActiveFont( generic->font );
	}

	// scale appropriately
	fontScale = ImGui::GetFont()->Scale;
	ImGui::SetWindowFontScale( ( generic->parent->textFontScale * fontScale ) * ui->scale );

	// set colors
	colorDepth = 0;
	if ( generic->flags & QMF_GRAYED ) {
		switch ( generic->type ) {
		case MTYPE_BUTTON:
			ImGui::PushStyleColor( ImGuiCol_Button, colorMdGrey );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, colorMdGrey );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colorMdGrey );
			colorDepth = 3;
			break;
		case MTYPE_TEXT:
			ImGui::PushStyleColor( ImGuiCol_Text, colorMdGrey );
			colorDepth = 1;
			break;
		case MTYPE_FIELD:
		case MTYPE_ARROW:
		case MTYPE_RADIOBUTTON:
		case MTYPE_SLIDER:
		case MTYPE_TREE:
			ImGui::PushStyleColor( ImGuiCol_FrameBg, colorMdGrey );
			ImGui::PushStyleColor( ImGuiCol_FrameBgActive, colorMdGrey );
			ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, colorMdGrey );
			colorDepth = 3;
			break;
		case MTYPE_NULL:
		default:
			break;
		};
	}
	else if ( generic->flags & QMF_HIDDEN ) {
		return;
	}
	else if ( generic->flags & QMF_HIGHLIGHT ) {
		switch ( generic->type ) {
		case MTYPE_BUTTON:
			ImGui::PushStyleColor( ImGuiCol_Button, colorGold );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, colorGold );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colorGold );
			colorDepth = 3;
			break;
		case MTYPE_TEXT:
			ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
			colorDepth = 1;
			break;
		case MTYPE_FIELD:
		case MTYPE_ARROW:
		case MTYPE_RADIOBUTTON:
		case MTYPE_SLIDER:
			ImGui::PushStyleColor( ImGuiCol_FrameBg, colorGold );
			ImGui::PushStyleColor( ImGuiCol_FrameBgActive, colorGold );
			ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, colorGold );
			colorDepth = 3;
			break;
		case MTYPE_NULL:
		default:
			break;
		};
	}

	if ( generic->focused && generic->flags & QMF_HIGHLIGHT_IF_FOCUS && !( generic->flags & QMF_HIGHLIGHT ) ) {
		switch ( generic->type ) {
		case MTYPE_BUTTON:
			ImGui::PushStyleColor( ImGuiCol_Button, colorGold );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, colorGold );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, colorGold );
			colorDepth += 3;
			break;
		case MTYPE_TEXT:
			ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
			colorDepth += 1;
			break;
		case MTYPE_FIELD:
		case MTYPE_ARROW:
		case MTYPE_RADIOBUTTON:
		case MTYPE_SLIDER:
			ImGui::PushStyleColor( ImGuiCol_FrameBg, colorGold );
			ImGui::PushStyleColor( ImGuiCol_FrameBgActive, colorGold );
			ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, colorGold );
			colorDepth += 3;
			break;
		case MTYPE_NULL:
		default:
			break;
		};
	}

	if ( generic->flags & QMF_SAMELINE_PREV ) {
		ImGui::SameLine();
	}

	switch ( generic->type ) {
	case MTYPE_NULL:
	default:
		break;
	case MTYPE_TABLE:
		Table_Draw( (menutable_t *)generic );
		break;
	case MTYPE_FIELD:
		Field_Draw( (menufield_t *)generic );
		break;
	case MTYPE_TEXT:
		Text_Draw( (menutext_t *)generic );
		break;
	case MTYPE_ARROW:
		Arrow_Draw( (menuarrow_t *)generic );
		break;
	case MTYPE_BUTTON:
		Button_Draw( (menubutton_t *)generic );
		break;
	case MTYPE_RADIOBUTTON:
		RadioButton_Draw( (menuswitch_t *)generic );
		break;
	case MTYPE_SLIDER:
		Slider_Draw( (menuslider_t *)generic );
		break;
	case MTYPE_LISTEXT:
		ListEx_Draw( (menulistex_t *)generic );
		break;
	case MTYPE_LIST:
		List_Draw( (menulist_t *)generic );
		break;
	case MTYPE_TAB:
		TabList_Draw( (menutab_t *)generic );
		break;
	case MTYPE_TREE:
		Tree_Draw( (menutree_t *)generic );
		break;
	};

	if ( generic->flags & QMF_SAMELINE_NEXT ) {
		ImGui::SameLine();
	}

	generic->focused = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );

	if ( ImGui::IsItemClicked() && !( generic->flags & QMF_GRAYED ) ) {
		if ( generic->eventcallback ) {
			generic->eventcallback( generic, EVENT_ACTIVATED );
		}
	}
	ImGui::PopStyleColor( colorDepth );

	ImGui::SetWindowFontScale( fontScale * ui->scale );
}

static void Menu_DrawItemList( void **items, int numitems )
{
	int i;
	menucommon_t *generic;

	for ( i = 0; i < numitems; i++ ) {
		generic = ( (menucommon_t **)items )[i];

		Menu_DrawItemGeneric( generic );
	}
}

void Menu_Draw( menuframework_t *menu ) {
	ImGui::Begin( menu->name, NULL, menu->flags );
	ImGui::SetWindowPos( ImVec2( menu->x, menu->y ) );
	ImGui::SetWindowSize( ImVec2( menu->width, menu->height ) );

	UI_EscapeMenuToggle();
	if ( UI_MenuTitle( menu->name, menu->titleFontScale ) ) {
		UI_PopMenu();
		Snd_PlaySfx( ui->sfx_back );

		ImGui::End();
		return;
	}

	Menu_DrawItemList( menu->items, menu->nitems );

	ImGui::End();
}

void MenuEvent_ArrowLeft( void *ptr, int event )
{
	int item;

	if ( event != EVENT_ACTIVATED ) {
		return;
	}

	item = ( (menulist_t *)ptr )->curitem;
	if ( item == 0 ) {
		item = ( (menulist_t *)ptr )->numitems - 1;
	} else {
		item--;
	}
	( (menulist_t *)ptr )->curitem = item;
}

void MenuEvent_ArrowRight( void *ptr, int event )
{
	int item;

	if ( event != EVENT_ACTIVATED ) {
		return;
	}

	item = ( (menulist_t *)ptr )->curitem;
	if ( item >= ( (menulist_t *)ptr )->numitems - 1 ) {
		item = 0;
	} else {
		item++;
	}
	( (menulist_t *)ptr )->curitem = item;
}

void Table_AddRow( menutable_t *table ) {
	table->rows++;
}

void Table_AddItem( menutable_t *table, void *item )
{
	menucommon_t *generic;

	if ( table->numitems >= MAX_TABLE_ITEMS ) {
		N_Error( ERR_DROP, "Table_AddItem: too many items" );
	}

	generic = (menucommon_t *)item;
	table->items[table->numitems] = generic;
	table->numitems++;

	generic->parent = table->generic.parent;
	generic->flags &= ~QMF_HASMOUSEFOCUS;
}

void Menu_AddItem( menuframework_t *menu, void *item )
{
	menucommon_t *itemptr;

	if ( menu->nitems >= MAX_MENU_ITEMS ) {
		N_Error( ERR_DROP, "Menu_AddItem: too many items" );
	}

	itemptr = (menucommon_t *)item;
	menu->items[ menu->nitems ] = item;
	itemptr->parent = menu;
	itemptr->flags &= ~QMF_HASMOUSEFOCUS;

	menu->nitems++;
}

const char *StringDup( const stringHash_t *str, const char *id ) {
    char *out;
    int length;

    length = PAD( strlen( str->value ) + strlen( id ) + 8, sizeof( uintptr_t ) );
    out = (char *)Hunk_Alloc( length, h_high );
    Com_snprintf( out, length, "%s##%s", str->value, id );

    return (const char *)out;
}