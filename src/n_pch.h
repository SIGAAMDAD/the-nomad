#ifndef _N_PCH_
#define _N_PCH_

#pragma once

//#include <boost/atomic/atomic.hpp>
//#include <boost/thread/futures/launch.hpp>
//#include <boost/thread.hpp>
//#include <boost/thread/mutex.hpp>
#ifdef __unix__
#   include <sys/mman.h>
#elif defined(_WIN32)
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <io.h>
#   include <sys/types.h>
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
#include <array>
#include <algorithm>
#include <utility>

// deps
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <ALsoft/al.h>
#include <ALsoft/alc.h>

#endif