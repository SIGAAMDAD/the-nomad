#ifndef _N_PCH_LINUX_
#define _N_PCH_LINUX_

#pragma once

#ifdef __unix__

#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_opengles2_gl2ext.h>

#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XLIB_XRANDR_EXT
#endif

#endif