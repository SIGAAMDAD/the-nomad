#include "../module_public.h"
#include "module_funcdefs.h"

void ScriptLib_Register_ImGui( void )
{
	SET_NAMESPACE( "ImGui" );

	REGISTER_ENUM_TYPE( "ImGuiCol" );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Text", ImGuiCol_Text );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TextDisabled", ImGuiCol_TextDisabled );
	REGISTER_ENUM_VALUE( "ImGuiCol", "WindowBg", ImGuiCol_WindowBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ChildBg", ImGuiCol_ChildBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "PopupBg", ImGuiCol_PopupBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Border", ImGuiCol_Border );
	REGISTER_ENUM_VALUE( "ImGuiCol", "BorderShadow", ImGuiCol_BorderShadow );
	REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBg", ImGuiCol_FrameBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBgHovered", ImGuiCol_FrameBgHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "FrameBgActive", ImGuiCol_FrameBgActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TitleBg", ImGuiCol_TitleBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TitleBgActive", ImGuiCol_TitleBgActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TitleBgCollapsed", ImGuiCol_TitleBgCollapsed );
	REGISTER_ENUM_VALUE( "ImGuiCol", "MenuBarBg", ImGuiCol_MenuBarBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarBg", ImGuiCol_ScrollbarBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrab", ImGuiCol_ScrollbarGrab );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "CheckMark", ImGuiCol_CheckMark );
	REGISTER_ENUM_VALUE( "ImGuiCol", "SliderGrab", ImGuiCol_SliderGrab );
	REGISTER_ENUM_VALUE( "ImGuiCol", "SliderGrabActive", ImGuiCol_ScrollbarGrabActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Button", ImGuiCol_Button );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ButtonHovered", ImGuiCol_ButtonHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ButtonActive", ImGuiCol_ButtonActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Header", ImGuiCol_Header );
	REGISTER_ENUM_VALUE( "ImGuiCol", "HeaderHovered", ImGuiCol_HeaderHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "HeaderActive", ImGuiCol_HeaderActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Separator", ImGuiCol_Separator );
	REGISTER_ENUM_VALUE( "ImGuiCol", "SeparatorHovered", ImGuiCol_SeparatorHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "SeparatorActive", ImGuiCol_SeparatorActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ResizeGrip", ImGuiCol_ResizeGrip );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ResizeGripHovered", ImGuiCol_ResizeGripHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ResizeGripActive", ImGuiCol_ResizeGripActive );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabHovered", ImGuiCol_TabHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "Tab", ImGuiCol_Tab );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabSelected", ImGuiCol_TabSelected );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabSelectedOverline", ImGuiCol_TabSelectedOverline );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabDimmed", ImGuiCol_TabDimmed );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabDimmedSelected", ImGuiCol_TabDimmedSelected );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TabDimmedSelectedOverline", ImGuiCol_TabDimmedSelectedOverline );
	REGISTER_ENUM_VALUE( "ImGuiCol", "PlotLines", ImGuiCol_PlotLines );
	REGISTER_ENUM_VALUE( "ImGuiCol", "PlotLinesHovered", ImGuiCol_PlotLinesHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "PlotHistogram", ImGuiCol_PlotHistogram );
	REGISTER_ENUM_VALUE( "ImGuiCol", "PlotHistogramHovered", ImGuiCol_PlotHistogramHovered );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TableHeaderBg", ImGuiCol_TableHeaderBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TableBorderStrong", ImGuiCol_TableBorderStrong );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TableBorderLight", ImGuiCol_TableBorderLight );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TableRowBg", ImGuiCol_TableRowBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TableRowBgAlt", ImGuiCol_TableRowBgAlt );
	REGISTER_ENUM_VALUE( "ImGuiCol", "TextSelectedBg", ImGuiCol_TextSelectedBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "DragDropTarget", ImGuiCol_DragDropTarget );
	REGISTER_ENUM_VALUE( "ImGuiCol", "NavHighlight", ImGuiCol_NavHighlight );
	REGISTER_ENUM_VALUE( "ImGuiCol", "NavWindowingHighlight", ImGuiCol_NavWindowingHighlight );
	REGISTER_ENUM_VALUE( "ImGuiCol", "NavWindowingDimBg", ImGuiCol_NavWindowingDimBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "ModalWindowDimBg", ImGuiCol_ModalWindowDimBg );
	REGISTER_ENUM_VALUE( "ImGuiCol", "COUNT", ImGuiCol_COUNT );

	REGISTER_ENUM_TYPE( "ImGuiWindowFlags" );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "None", ImGuiWindowFlags_None );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoTitleBar", ImGuiWindowFlags_NoTitleBar );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoResize", ImGuiWindowFlags_NoResize );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoMouseInputs", ImGuiWindowFlags_NoMouseInputs );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoMove", ImGuiWindowFlags_NoMove );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoCollapse", ImGuiWindowFlags_NoCollapse );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoDecoration", ImGuiWindowFlags_NoDecoration );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoBackground", ImGuiWindowFlags_NoBackground );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoSavedSettings", ImGuiWindowFlags_NoSavedSettings );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "NoScrollbar", ImGuiWindowFlags_NoScrollbar );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar );
	REGISTER_ENUM_VALUE( "ImGuiWindowFlags", "AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize );

	REGISTER_ENUM_TYPE( "ImGuiTableFlags" );
	REGISTER_ENUM_VALUE( "ImGuiTableFlags", "None", ImGuiTableFlags_None );
	REGISTER_ENUM_VALUE( "ImGuiTableFlags", "Resizable", ImGuiTableFlags_Resizable );
	REGISTER_ENUM_VALUE( "ImGuiTableFlags", "Reorderable", ImGuiTableFlags_Reorderable );

	REGISTER_ENUM_TYPE( "ImGuiInputTextFlags" );
	REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "None", ImGuiInputTextFlags_None );
	REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue );
	REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "AllowTabInput", ImGuiInputTextFlags_AllowTabInput );
	REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine );
	REGISTER_ENUM_VALUE( "ImGuiInputTextFlags", "ReadOnly", ImGuiInputTextFlags_ReadOnly );

	REGISTER_ENUM_TYPE( "ImGuiDir" );
	REGISTER_ENUM_VALUE( "ImGuiDir", "Left", ImGuiDir_Left );
	REGISTER_ENUM_VALUE( "ImGuiDir", "Right", ImGuiDir_Right );
	REGISTER_ENUM_VALUE( "ImGuiDir", "Down", ImGuiDir_Down );
	REGISTER_ENUM_VALUE( "ImGuiDir", "Up", ImGuiDir_Up );
	REGISTER_ENUM_VALUE( "ImGuiDir", "None", ImGuiDir_None );

	REGISTER_GLOBAL_FUNCTION( "bool ImGui::Begin( const string& in, bool& in, ImGuiWindowFlags = ImGuiWindowFlags::None )", asFUNCTION( ImGui::Begin ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "void ImGui::End()", asFUNCTION( ImGui::End ), asCALL_CDECL );
	REGISTER_GLOBAL_FUNCTION( "bool ImGui::SliderInt( const string& in, int& in )", asFUNCTION( ImGui::SliderInt ), asCALL_CDECL );

	RESET_NAMESPACE();
}