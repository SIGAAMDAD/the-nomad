#ifndef __RVK_LOCAL__
#define __RVK_LOCAL__

#pragma once

#include "../engine/n_shared.h"

#include <vulkan/vulkan.h>

#ifdef PLATFORM_ANDROID
    #include <vulkan/vulkan_android.h>
#endif

typedef struct
{
    VkDevice m_pDevice;
    VkAllocationCallbacks *m_pAllocator;
    VkPhysicalDevice m_pPhysicalDevice;
} renderGlobals_t;

extern renderGlobals_t rg;

#endif