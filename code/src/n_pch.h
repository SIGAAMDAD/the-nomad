#ifndef _N_PCH_
#define _N_PCH_

#pragma once

#if defined(__MINGW32__) || defined(__MINGW64__)
    #define GDR_THREADS_NO_SUPPORT 1
#endif

/*
OS-specific stuff
*/
#include <sys/types.h>
#ifdef __unix__
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <sys/dir.h>
    #include <sys/errno.h>
    #include <unistd.h>
    #include <dlfcn.h>
#elif defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <io.h>
    #include <libloaderapi.h>
    #pragma comment(lib, "Kernel32.lib")
    #pragma comment(lib, "libloader.lib")
#else
    #error Unsupported OS!
#endif

/*
Multithreading
*/
#ifdef GDR_THREADS_NO_SUPPORT // we're using mingw
    #include "mingw-std-threads/mingw.condition_variable.h"
    #include "mingw-std-threads/mingw.invoke.h"
    #include "mingw-std-threads/mingw.mutex.h"
    #include "mingw-std-threads/mingw.future.h"
    #include "mingw-std-threads/mingw.shared_mutex.h"
    #include "mingw-std-threads/mingw.thread.h"
#else
    #include <boost/thread/thread.hpp>
    #include <boost/thread/mutex.hpp>
    #include <boost/thread/condition_variable.hpp>
    #include <boost/thread/future.hpp>
    #include <boost/thread/shared_mutex.hpp>
#endif

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

/*
Dependencies
*/
#include <EA/EASTL/array.h>
#include <EA/EASTL/vector.h>
#include <EA/EASTL/string.h>
#include <EA/EASTL/map.h>
#include <EA/EASTL/unordered_map.h
#include <EA/EASTL/utility.h>
#include <EA/EASTL/algorithm.h>
#include <EA/EASTL/memory.h>
#include <EA/EASTL/numeric.h>
#include <EA/EASTL/unique_ptr.h>
#include <EA/EASTL/shared_ptr.h>
#include <EA/EASTL/weak_ptr.h>
#include <EA/EASTL/shared_array.h>
#include <EA/EASTL/vector_map.h>
#include <EA/EASTL/slist.h>
#include <EA/EASTL/dequeue.h>
#include <EA/EASTL/queue.h>
#include <EA/EASTL/list.h>
#include <EA/EASTL/iterator.h>
#include <EA/EASTL/internal/atomic/atomic.h>

#include <GDRLib/lib.hpp>

// eastl stuff
#include <EABase/eabase.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/iterator.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/algorithm.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>
#include <EASTL/utility.h>
#include <EASTL/tuple.h>
#include <EASTL/allocator_malloc.h>
#include <EASTL/core_allocator.h>
#include <EASTL/initializer_list.h>
#include <EASTL/array.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/slist.h>
#include <EASTL/queue.h>
#include <EASTL/unordered_map.h>
#include <EASTL/allocator.h>
#include <EASTL/list.h>

#include <EASTL/internal/atomic/atomic.h>
#include <EASTL/internal/atomic/atomic_standalone.h>
#include <EASTL/internal/function.h>

/*** deps ***/
// random
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "glad.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_opengl.h>
#include <SOIL/SOIL.h>
#include <tinyxml2.h>
#include <tmx/tmx>

// speed is key
#define USING_EASY_PROFILER
#include <easy/profiler.h>
#include "stb_sprintf.h"
#include <xalloc/Allocator.h>
#include <xalloc/xallocator.h>
#include <google/sparse_hash_map>
#include <google/dense_hash_map>
//#include "smmalloc.h"
#include <freetype2/ft2build.h>
#include <freetype2/freetype/ftrender.h>
#include <freetype2/freetype/freetype.h>
#include <foonathan_memory/foonathan/memory/container.hpp> // vector, list, list_node_size
#include <foonathan_memory/foonathan/memory/memory_pool.hpp> // memory_pool
#include <foonathan_memory/foonathan/memory/smart_ptr.hpp> // allocate_unique
#include <foonathan_memory/foonathan/memory/static_allocator.hpp> // static_allocator_storage, static_block_allocator
#include <foonathan_memory/foonathan/memory/temporary_allocator.hpp> // temporary_allocator
#include <foonathan_memory/foonathan/memory/namespace_alias.hpp>

// bff stuff
//#include <zlib.h>
//#include <zstd/zstd.h>
//#include <zip.h>
#ifdef __unix__
#include <dirent.h>
#endif

// audio i/o
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
//#include <sndfile.h>
#include <ALsoft/al.h>
#include <ALsoft/alc.h>

#endif