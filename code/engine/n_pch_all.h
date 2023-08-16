#ifndef _N_PCH_ALL_
#define _N_PCH_ALL_

#pragma once

/*
OS-specific stuff
*/
#include <sys/types.h>
#include <sys/errno.h>

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
#include <strings.h>

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

/*
Dependencies
*/

// boost, nuff said
#define BOOST_THREAD_PROVIDES_EXECUTORS
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_USES_MOVE
#define BOOST_DISABLE_ASSERTS
#include <boost/asio/io_service.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

// EASTL, a salute to EA where they suck everywhere else but do good with c++ libraries
#include <EASTL/array.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/memory.h>
#include <EASTL/numeric.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/shared_array.h>
#include <EASTL/vector_map.h>
#include <EASTL/slist.h>
#include <EASTL/deque.h>
#include <EASTL/queue.h>
#include <EASTL/list.h>
#include <EASTL/iterator.h>
#include <EASTL/internal/atomic/atomic.h>
#include <EASTL/internal/atomic/atomic_standalone.h>
#include <EASTL/internal/function.h>


// GDRLib, my custom stuff, specific to game development, some of the meat of the engine
#include <GDRLib/allocator.hpp>

// glm, funmathgames
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/type_trait.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <xalloc/Allocator.h>
#include <xalloc/xallocator.h>

#include <nlohmann/json.hpp>

#define USING_EASY_PROFILER
#include <easy/profiler.h>
#endif

// SDL2, i don't trust SDL3
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

// speed is key
#include "../src/stb_sprintf.h"

#include <GDRLib/lib.hpp>

#endif
