/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../engine/n_shared.h"
#include "../rendercommon/r_public.h"
#ifdef USE_OPENGL_API
#include "../rendergl/rgl_local.h"
#include "../rendergl/stb_image.h"
#endif
#ifdef USE_VULKAN_API
#include "../rendervk/rvk_local.h"
#include "../rendervk/stb_image.h"
#endif

void R_LoadJPG( const char *filename, unsigned char **pic, int *width, int *height, int *channels )
{
	ri.G_LoadJPG( filename, pic, width, height );
}
