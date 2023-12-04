#if 0
#ifndef _N_PCH_ALL_
#define _N_PCH_ALL_

#pragma once

#ifndef Q3_VM

/*
Standard Library
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <math.h>

#ifdef __cplusplus
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>

#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <future>


// glm, funmathgames
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/type_trait.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <nlohmann/json.hpp>

#define USING_EASY_PROFILER
#include <easy/profiler.h>
#endif

// speed is key
#include "../game/stb_sprintf.h"

#endif

#ifndef Q3_VM
// SDL2, I don't trust SDL3
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_types.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_cpuinfo.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#endif

#endif

#endif