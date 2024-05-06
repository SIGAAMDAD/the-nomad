#ifndef __MODULE_PROC_DEF__
#define __MODULE_PROC_DEF__

#pragma once

#include "module_public.h"

//==============================================================
// ImGui
//

void Script_ImGuiBegin( asIScriptGeneric *pGeneric );
void Script_ImGuiEnd( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderInt( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderFloat( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderVec2( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderVec3( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderVec4( asIScriptGeneric *pGeneric );
void Script_ImGuiSliderAngle( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::Engine
//

void Script_CvarRegister( asIScriptGeneric *pGeneric );
void Script_CvarUpdate( asIScriptGeneric *pGeneric );
void Script_CvarVariableInteger( asIScriptGeneric *pGeneric );
void Script_CvarVariableFloat( asIScriptGeneric *pGeneric );
void Script_CvarVariableString( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::Engine::SoundSystem
//

void Script_RegisterSfx( asIScriptGeneric *pGeneric );
void Script_RegisterTrack( asIScriptGeneric *pGeneric );
void Script_PlaySfx( asIScriptGeneric *pGeneric );
void Script_SetLoopingTrack( asIScriptGeneric *pGeneric );
void Script_ClearLoopingTrack( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::Engine::FileSystem
//

void Script_OpenFileRead( asIScriptGeneric *pGeneric );
void Script_OpenFileWrite( asIScriptGeneric *pGeneric );
void Script_OpenFileRW( asIScriptGeneric *pGeneric );
void Script_OpenFileAppend( asIScriptGeneric *pGeneric );
void Script_OpenFileMode( asIScriptGeneric *pGeneric );
void Script_CloseFile( asIScriptGeneric *pGeneric );
void Script_LoadFile( asIScriptGeneric *pGeneric );
void Script_GetFileLength( asIScriptGeneric *pGeneric );
void Script_GetFilePosition( asIScriptGeneric *pGeneric );
void Script_SetFilePosition( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::Engine::Renderer
//

void Script_DrawImage( asIScriptGeneric *pGeneric );
void Script_AddSpriteToScene( asIScriptGeneric *pGeneric );
void Script_AddPolyToScene( asIScriptGeneric *pGeneric );
void Script_AddDLightToScene( asIScriptGeneric *pGeneric );
void Script_ClearScene( asIScriptGeneric *pGeneric );
void Script_RenderScene( asIScriptGeneric *pGeneric );
void Script_RegisterShader( asIScriptGeneric *pGeneric );
void Script_RegisterSpriteSheet( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::GameSystem
//

void Script_BBoxAssign( asIScriptGeneric *pGeneric );

void Script_SetCameraPos( asIScriptGeneric *pGeneric );
void Script_GetString( asIScriptGeneric *pGeneric );
void Script_GetGPUConfig( asIScriptGeneric *pGeneric );

void Script_CastRay( asIScriptGeneric *pGeneric );
void Script_CheckWallHit( asIScriptGeneric *pGeneric );

void Script_LoadMap( asIScriptGeneric *pGeneric );
void Script_SetActiveMap( asIScriptGeneric *pGeneric );
void Script_GetSpawnData( asIScriptGeneric *pGeneric );
void Script_GetCheckpointData( asIScriptGeneric *pGeneric );
void Script_GetTileData( asIScriptGeneric *pGeneric );

//==============================================================
// TheNomad::Util
//

void Script_GetModuleList( asIScriptGeneric *pGeneric );
void Script_IsModuleActive( asIScriptGeneric *pGeneric );
void Script_StrICmp( asIScriptGeneric *pGeneric );

void Script_ConsolePrint( asIScriptGeneric *pGeneric );
void Script_GameError( asIScriptGeneric *pGeneric );

#endif