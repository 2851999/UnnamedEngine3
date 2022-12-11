#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>

#include "../Settings.h"

// Forward declaration
class VulkanInstance;

/*****************************************************************************
 * VulkanExtensions class - For handling Vulkan instance extensions
 *****************************************************************************/

class VulkanExtensions {
private:
    /* The instance these extensions are loaded for */
    VkInstance vkInstance = nullptr;

    /* List of required extensions */
    std::vector<const char*> requiredExtensions;

    /* List of optional extensions with a queryable name*/
    std::map<std::string, std::vector<const char*>> optionalExtensions;

    /* Various Vulkan extension methods that need to be loaded */

    /* Instance */
    PFN_vkCreateDebugUtilsMessengerEXT loaded_vkCreateDebugUtilsMessengerEXT   = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT loaded_vkDestroyDebugUtilsMessengerEXT = nullptr;

    /* Utility function for comparing and removing found device extensions
       from a list */
    void removeSupportedExtensions(const std::vector<VkExtensionProperties>& supportedDeviceExtensions, std::vector<const char*>& extensionsList) const;

public:
    /* Names for optional extensions */
    static const std::string RAY_TRACING;

    /* Structure used to return support information about a physical device's
       supported extensions */
    struct PhysicalDeviceSupport {
        /* States whether the required extensions are supported */
        bool required = false;

        /* Map of optional extensions and whether they are supported by the
           device */
        std::map<std::string, bool> optionals = {};
    };

    /* Constructor and destructor */
    VulkanExtensions() {}
    virtual ~VulkanExtensions() {}

    /* Adds the required/optional extensions to the lists given the engine
       settings to use */
    void addExtensions(const Settings& settings);

    /* Checks the required extensions are supported by the instance - logs
       any that aren't in debug */
    bool checkInstanceSupport() const;

    /* Checks whether the required/optional extensions are supported by a
       physical device */
    PhysicalDeviceSupport checkPhysicalDeviceSupport(VkPhysicalDevice physicalDevice) const;

    /* Loads instance extension methods ready for use */
    void loadInstanceExtensions(const VulkanInstance* instance);

    /* Returns a reference to the required extension names */
    inline std::vector<const char*>& getRequiredExtensions() { return requiredExtensions; }

    /* Various methods to call loaded external functions (VkInstance is taken as the one they were loaded for) */
    inline VkResult vkCreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) const { return loaded_vkCreateDebugUtilsMessengerEXT(vkInstance, pCreateInfo, pAllocator, pDebugMessenger); }
    inline void vkDestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const { loaded_vkDestroyDebugUtilsMessengerEXT(vkInstance, messenger, pAllocator); }
};