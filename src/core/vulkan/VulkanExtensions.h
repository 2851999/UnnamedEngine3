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
protected:
    /* List of required extensions */
    std::vector<const char*> requiredExtensions;

    /* List of optional extensions with a queryable name*/
    std::map<std::string, std::vector<const char*>> optionalExtensions;

    /* Utility function for comparing and removing found extensions from a
       list */
    void removeSupportedExtensions(const std::vector<VkExtensionProperties>& supportedDeviceExtensions, std::vector<const char*>& extensionsList) const;

public:
    /* Structure used to return support information about supported
       extensions */
    struct Support {
        /* States whether the required extensions are supported */
        bool required = false;

        /* Map of optional extensions and whether they are supported by the
           device */
        std::map<std::string, bool> optionals = {};
    };

    /* Constructor and destructor */
    VulkanExtensions() {}
    virtual ~VulkanExtensions() {}

    /* Returns a reference to the required extension names */
    inline std::vector<const char*>& getRequiredExtensions() { return requiredExtensions; }
};

/*****************************************************************************
 * VulkanInstanceExtensions class - For handling Vulkan instance extensions
 *****************************************************************************/

class VulkanInstanceExtensions : public VulkanExtensions {
private:
    /* The instance these extensions are loaded for */
    VkInstance vkInstance = nullptr;

    /* Various Vulkan extension methods that need to be loaded */
    PFN_vkCreateDebugUtilsMessengerEXT loaded_vkCreateDebugUtilsMessengerEXT   = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT loaded_vkDestroyDebugUtilsMessengerEXT = nullptr;

public:
    /* Names for optional extensions */
    static const std::string RAY_TRACING;

    /* Constructor and destructor */
    VulkanInstanceExtensions() {}
    virtual ~VulkanInstanceExtensions() {}

    /* Adds the required/optional extensions to the lists given the engine
       settings to use */
    void addExtensions(const Settings& settings);

    /* Checks the required extensions are supported by the instance - logs
       any that aren't in debug */
    bool checkSupport() const;

    /* Loads extension methods ready for use */
    void loadExtensions(const VulkanInstance* instance);

    /* Various methods to call loaded external functions (VkInstance is taken as the one they were loaded for) */
    inline VkResult vkCreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) const { return loaded_vkCreateDebugUtilsMessengerEXT(vkInstance, pCreateInfo, pAllocator, pDebugMessenger); }
    inline void vkDestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) const { loaded_vkDestroyDebugUtilsMessengerEXT(vkInstance, messenger, pAllocator); }
};

/*****************************************************************************
 * VulkanDeviceExtensions class - For handling Vulkan instance extensions
 *****************************************************************************/

class VulkanDeviceExtensions : public VulkanExtensions {
private:
    /* The instance these extensions are loaded for */
    VkInstance vkInstance = nullptr;

    /* Various Vulkan extension methods that need to be loaded */

public:
    /* Names for optional extensions */
    static const std::string RAY_TRACING;

    /* Constructor and destructor */
    VulkanDeviceExtensions() {}
    virtual ~VulkanDeviceExtensions() {}

    /* Adds the required/optional extensions to the lists given the engine
       settings to use */
    void addExtensions(const Settings& settings);

    /* Checks whether the required/optional extensions are supported by a
       physical device */
    VulkanExtensions::Support checkSupport(VkPhysicalDevice physicalDevice) const;
};