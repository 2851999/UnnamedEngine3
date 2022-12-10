#include "VulkanExtensions.h"

#include "../../utils/Logging.h"
#include "VulkanInstance.h"

/*****************************************************************************
 * VulkanExtensions class
 *****************************************************************************/

void VulkanExtensions::addRequired(const Settings& settings) {
    // Obtain those required by GLFW
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Assign the extensions starting with the above
    requiredExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Validation layer extension
    if (settings.debug.validationLayers)
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool VulkanExtensions::checkInstanceSupport() {
    // Obtain a list of supported instance extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

    // Go though and remove present extensions
    std::vector<const char*> missingExtensionNames = requiredExtensions;

    for (const auto& extension : supportedExtensions) {
        for (unsigned int i = 0; i < missingExtensionNames.size(); ++i) {
            if (strcmp(missingExtensionNames[i], extension.extensionName)) {
                missingExtensionNames.erase(missingExtensionNames.begin() + i);
                break;
            }
        }
    }

    // Log any that are not supported
    for (unsigned int i = 0; i < missingExtensionNames.size(); ++i)
        Logger::log("The '" + utils_string::str(missingExtensionNames[i]) + "' extension is not present", "VulkanExtensions", LogType::Debug);

    return missingExtensionNames.size() == 0;
}

void VulkanExtensions::loadInstanceExtensions(const VulkanInstance* instance) {
    this->vkInstance                       = instance->getVkInstance();
    loaded_vkCreateDebugUtilsMessengerEXT  = instance->loadExternal<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
    loaded_vkDestroyDebugUtilsMessengerEXT = instance->loadExternal<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
}