#ifndef _N_PCH_
#define _N_PCH_

#pragma once

// n_pch.h -- precompiled header for the SIR Engine

#ifdef __cplusplus
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_allocator.h>
#include <EASTL/fixed_list.h>
#include <EASTL/fixed_string.h>
#include <EASTL/chrono.h>
#include <EASTL/utility.h>
#include <EASTL/unordered_map.h>
#include <EASTL/map.h>
#include <EASTL/hash_map.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/vector_map.h>
#include <EASTL/string_hash_map.h>
#include <EASTL/string.h>
#include <EASTL/random.h>
#include <EASTL/bonus/lru_cache.h>
#include <EASTL/bonus/fixed_ring_buffer.h>
#include <EASTL/bonus/ring_buffer.h>

#include "code/rendercommon/imgui.h"
#endif

#if !defined(Q3_VM) || defined(GDR_DLLCOMPILE)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#endif
