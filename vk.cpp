#include <vulkan/vulkan.hpp>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main()
{
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    std::vector<const char*> instanceExtensions = {
//        "VK_KHX_external_memory_capabilities",
//        "VK_KHR_get_physical_device_properties2"
    };
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledExtensionCount = (int32_t)instanceExtensions.size();

    VkInstance instance = NULL;
    if (vkCreateInstance(&instanceCreateInfo, NULL, &instance) != VK_SUCCESS) {
        exit(-1);
    }

    PFN_vkGetPhysicalDeviceProperties2KHR getPhysicalDeviceProperties2KHR = 
        reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
    
    if (getPhysicalDeviceProperties2KHR == NULL) {
        exit(-1);
    }
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);

    std::vector<VkPhysicalDevice> devices{ physicalDeviceCount };
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties2KHR properties{};
        properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;

        VkPhysicalDeviceIDPropertiesKHR deviceIdProperties{};
        deviceIdProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;

        properties.pNext = &deviceIdProperties;

        getPhysicalDeviceProperties2KHR(device, &properties);

        printf("    Vulkan Device: %s\n", properties.properties.deviceName);
    }
    return 0;
}