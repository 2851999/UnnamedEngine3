#include "VulkanExtensions.h"

#include "../../utils/Logging.h"
#include "VulkanInstance.h"

/*****************************************************************************
 * VulkanExtensions class
 *****************************************************************************/

void VulkanExtensions::removeSupportedExtensions(const std::vector<VkExtensionProperties>& supportedDeviceExtensions, std::vector<const char*>& extensionsList) const {
    for (const auto& extension : supportedDeviceExtensions) {
        for (unsigned int i = 0; i < extensionsList.size(); ++i) {
            if (strcmp(extensionsList[i], extension.extensionName)) {
                extensionsList.erase(extensionsList.begin() + i);
                break;
            }
        }
    }
}

/*****************************************************************************
 * VulkanInstanceExtensions class
 *****************************************************************************/

void VulkanInstanceExtensions::addExtensions(const Settings& settings) {
    // Obtain those required by GLFW
    uint32_t glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Assign the extensions starting with the above
    requiredExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Validation layer extension
    if (settings.debug.validationLayers)
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

bool VulkanInstanceExtensions::checkSupport() const {
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

    return missingExtensionNames.empty();
}

void VulkanInstanceExtensions::loadExtensions(const VulkanInstance* instance) {
    this->vkInstance                       = instance->getVkInstance();
    loaded_vkCreateDebugUtilsMessengerEXT  = instance->loadExternal<PFN_vkCreateDebugUtilsMessengerEXT>("vkCreateDebugUtilsMessengerEXT");
    loaded_vkDestroyDebugUtilsMessengerEXT = instance->loadExternal<PFN_vkDestroyDebugUtilsMessengerEXT>("vkDestroyDebugUtilsMessengerEXT");
}

/*****************************************************************************
 * VulkanDeviceExtensions class
 *****************************************************************************/

const std::string VulkanDeviceExtensions::RAY_TRACING = "ray_tracing";

void VulkanDeviceExtensions::addExtensions(const Settings& settings) {
    // Add required extensions

    // Required for swap chain support
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Extensions required for ray tracing
    if (settings.video.rayTracing) {
        // Add optional extensions
        std::vector<const char*> rayTracingExtensions = {
            // Ray tracing extensions
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            // Required by VK_KHR_acceleration_structure
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            // Required by VK_KHR_ray_tracing_pipeline
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            // Required by VK_KHR_spirv_1_4,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
            // Required for random number generation in shaders
            VK_KHR_SHADER_CLOCK_EXTENSION_NAME,
        };
        optionalExtensions.insert(std::pair<std::string, std::vector<const char*>>(RAY_TRACING, rayTracingExtensions));
    }
}

VulkanExtensions::Support VulkanDeviceExtensions::querySupport(VkPhysicalDevice physicalDevice) const {
    // Obtain the supported extensions
    uint32_t supportedExtensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedDeviceExtensions(supportedExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, supportedDeviceExtensions.data());

    // Go though and remove present extensions
    std::vector<const char*> missingExtensionNames = requiredExtensions;
    removeSupportedExtensions(supportedDeviceExtensions, missingExtensionNames);

    // Support structure to return
    VulkanExtensions::Support physicalDeviceSupport{};
    physicalDeviceSupport.required = missingExtensionNames.empty();

    // Go through each set of options and determine their support too
    for (auto const& pair : optionalExtensions) {
        // Get list of missing
        missingExtensionNames = pair.second;
        removeSupportedExtensions(supportedDeviceExtensions, missingExtensionNames);

        physicalDeviceSupport.optionals.insert(std::pair<std::string, bool>(pair.first, missingExtensionNames.empty()));
    }

    return physicalDeviceSupport;
}

std::vector<const char*> VulkanDeviceExtensions::getExtensions(VulkanExtensions::Support& supportedExtensions) {
    std::vector<const char*> extensions = requiredExtensions;
    for (auto const& pair : supportedExtensions.optionals) {
        if (pair.second)
            // Want to enable
            extensions.insert(extensions.end(), optionalExtensions[pair.first].begin(), optionalExtensions[pair.first].end());
    }
    return extensions;
}