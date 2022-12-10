#include "VulkanValidationLayers.h"

#include "../../utils/Logging.h"
#include "../../utils/StringUtils.h"
#include "VulkanInstance.h"

/*****************************************************************************
 * VulkanValidationLayers class
 *****************************************************************************/

bool VulkanValidationLayers::checkInstanceSupport() {
    // Obtain a list of supported instance layers
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> supportedLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

    // Go though and remove present layers
    std::vector<const char*> missingLayerNames = validationLayers;

    for (const auto& layer : supportedLayers) {
        for (unsigned int i = 0; i < missingLayerNames.size(); ++i) {
            if (strcmp(missingLayerNames[i], layer.layerName)) {
                missingLayerNames.erase(missingLayerNames.begin() + i);
                break;
            }
        }
    }

    // Log any that are not supported
    for (unsigned int i = 0; i < missingLayerNames.size(); ++i)
        Logger::log("The '" + utils_string::str(missingLayerNames[i]) + "' validation layer is not present", "VulkanValidationLayers", LogType::Debug);

    return missingLayerNames.size() == 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanValidationLayers::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    LogType logType = LogType::Debug;
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        logType = LogType::Debug;
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        logType = LogType::Information;
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        logType = LogType::Warning;
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        logType = LogType::Error;

    // Output the message
    Logger::log(pCallbackData->pMessage, "VulkanValidationLayers", logType);

    return VK_FALSE;
}

void VulkanValidationLayers::assignDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData       = nullptr;
}

void VulkanValidationLayers::createDebugMessenger() {
    // Create info
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    assignDebugMessengerCreateInfo(createInfo);

    // Create
    if (extensions.vkCreateDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        Logger::log("Failed to create a debug messenger", "VulkanValidationLayers", LogType::Error);
}

void VulkanValidationLayers::destroyDebugMessenger() {
    if (debugMessenger) {
        // Destroy
        extensions.vkDestroyDebugUtilsMessengerEXT(debugMessenger, nullptr);
        debugMessenger = nullptr;
    }
}