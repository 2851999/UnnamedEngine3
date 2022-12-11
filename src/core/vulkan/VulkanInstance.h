#pragma once

#include "../../utils/Logging.h"
#include "../Settings.h"
#include "VulkanDevice.h"
#include "VulkanExtensions.h"
#include "VulkanValidationLayers.h"

// Forward declaration
class Window;

/*****************************************************************************
 * VulkanInstance class - For handling a Vulkan instance
 *****************************************************************************/

class VulkanInstance {
private:
    /* The actual instance*/
    VkInstance instance = nullptr;

    /* VulkanInstanceExtensions instance */
    VulkanInstanceExtensions* extensions = nullptr;

    /* VulkanValidationLayers instance (If requested) */
    VulkanValidationLayers* validationLayers = nullptr;

    /* Destroys the VkInstance */
    void destroy();

public:
    /* Constructor and destructor */
    VulkanInstance() {}
    virtual ~VulkanInstance() { destroy(); }

    /* Attempts to create the VkInstance and returns whether successful */
    bool create(const Settings& settings);

    /* Obtains an external function pointer using the instance */
    template <typename T>
    T loadExternal(const char* funcName) const {
        auto loadedFunc = reinterpret_cast<T>(vkGetInstanceProcAddr(instance, funcName));
        if (! loadedFunc)
            Logger::log("Failed to load the external function '" + utils_string::str(funcName) + "'", "VulkanInstance", LogType::Error);
        return loadedFunc;
    };

    /* Picks a suitable device and returns it - will ensure present queue
       support if window parameter is not nullptr */
    VulkanDevice* pickPhysicalDevice(const Settings& settings, const Window* window);

    /* Returns the VkInstance */
    inline VkInstance getVkInstance() const { return instance; }
};