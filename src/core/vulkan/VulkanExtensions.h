#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Settings.h"

// Forward declaration
class VulkanInstance;

/*****************************************************************************
 * VulkanExtensions class - For handling Vulkan instacne extensions
 *****************************************************************************/

class VulkanExtensions {
private:
    /* The instance these extensions are loaded for */
    VkInstance vkInstance = nullptr;

    /* List of required extensions */
    std::vector<const char*> requiredExtensions;

    /* Various Vulkan extension methods that need to be loaded */

    /* Instance */
    PFN_vkCreateDebugUtilsMessengerEXT loaded_vkCreateDebugUtilsMessengerEXT   = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT loaded_vkDestroyDebugUtilsMessengerEXT = nullptr;

public:
    /* Constructor and destructor */
    VulkanExtensions() {}
    virtual ~VulkanExtensions() {}

    /* Adds the required extensions to the list given the engine settings
       to use */
    void addRequired(const Settings& settings);

    /* Checks the required extensions are supported by the instance - logs
       any that aren't in debug */
    bool checkInstanceSupport();

    /* Loads instance extension methods ready for use */
    void loadInstanceExtensions(const VulkanInstance* instance);

    /* Returns a reference to the required extension names */
    inline std::vector<const char*>& getRequiredExtensions() { return requiredExtensions; }

    /* Various methods to call loaded external functions (VkInstance is taken as the one they were loaded for) */
    inline VkResult vkCreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) const { return loaded_vkCreateDebugUtilsMessengerEXT(vkInstance, pCreateInfo, pAllocator, pDebugMessenger); }
    inline void vkDestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const { loaded_vkDestroyDebugUtilsMessengerEXT(vkInstance, messenger, pAllocator); }
};