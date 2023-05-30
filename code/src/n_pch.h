#ifndef _N_PCH_
#define _N_PCH_

#pragma once

//#include <boost/atomic/atomic.hpp>
//#include <boost/thread/futures/launch.hpp>
//#include <boost/thread.hpp>
//#include <boost/thread/mutex.hpp>
#ifdef __unix__
#   include <sys/mman.h>
#   include <sys/fcntl.h>
#   include <unistd.h>
#   include <dlfcn.h>
#elif defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <io.h>
#   include <sys/types.h>
#   include <libloaderapi.h>
#   pragma comment(lib, "Kernel32.lib")
#else
#   error Unsupported OS!
#endif

#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include <future>
#include <stdlib.h>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <vector>
#include <algorithm>
#include <utility>

/*** deps ***/
// random
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <glad/glad.h>
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_opengl.h>
#include <SOIL/SOIL.h>

// speed is key
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