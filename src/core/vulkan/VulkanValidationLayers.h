#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "VulkanExtensions.h"

/*****************************************************************************
 * VulkanValidationLayers class - For handling Vulkan validation layers
 *****************************************************************************/

class VulkanValidationLayers {
private:
    /* Stores the requested validation layers */
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    /* Extensions - required for access to the debug extension */
    const VulkanExtensions& extensions;

    /* Debug messenger */
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;

    /* Callback method for debug messeges */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

public:
    /* Constructor and destructor */
    VulkanValidationLayers(const VulkanExtensions& extensions) : extensions(extensions) {}
    virtual ~VulkanValidationLayers() {
        destroyDebugMessenger();
    }

    /* Checks for instance support of the requested validation layers */
    bool checkInstanceSupport();

    /* Assigns a VkDebugUtilsMessengerCreateInfoEXT instance - for use to debug instance creation and destruction */
    void assignDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /* Creates the debug messenger - logs an error if it fails */
    void createDebugMessenger();

    /* Destroys the debug messenger */
    void destroyDebugMessenger();

    /* Returns a reference the requested validation layer names */
    inline std::vector<const char*>& getRequested() { return validationLayers; }
};