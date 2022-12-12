#include "Window.h"

#include "../utils/Logging.h"

/*****************************************************************************
 * Window
 *****************************************************************************/

bool Window::create(WindowSettings& windowSettings, VideoSettings& videoSettings, const VulkanInstance* vulkanInstance) {
    this->vulkanInstance = vulkanInstance->getVkInstance();

    // Setup the default parameters
    glfwDefaultWindowHints();

    // Here we assume Vulkan will be used
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Assign various parameters
    setResizable(windowSettings.resizable);
    setDecorated(windowSettings.decorated);
    setFloating(windowSettings.floating);
    setRefreshRate(windowSettings.refreshRate);

    // Monitor to create the window on - only needed when going fullscreen
    GLFWmonitor* monitor = nullptr;

    if (videoSettings.fullscreen) {
        // Now assign the monitor
        monitor = glfwGetPrimaryMonitor();

        // Obtain the normal video mode of the monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Assign the relevant video settings for this monitor
        setRefreshRate(mode->refreshRate);

        if (windowSettings.borderless) {
            // Assign the video mode to match
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        }
    }

    // Attempt to create the window
    instance = glfwCreateWindow(videoSettings.resolution.getX(), videoSettings.resolution.getY(), windowSettings.title.c_str(), monitor, nullptr);

    // Check if unsuccessful
    if (! instance) {
        Logger::log("Failed to create a window", "Window", LogType::Error);
        return false;
    }

    // Now the window should have been created, assign the settings to match
    // what has actually been chosen (Get size in pixels)
    int width, height;
    glfwGetFramebufferSize(this->instance, &width, &height);
    windowSettings.width  = width;
    windowSettings.height = height;

    // Center the window by default (if not fullscreen)
    if (! monitor)
        this->center(width, height);

    // Create the window surface
    if (glfwCreateWindowSurface(this->vulkanInstance, instance, nullptr, &surface) != VK_SUCCESS) {
        Logger::log("Failed to create the window surface", "Window", LogType::Error);
        return false;
    }

    // If reached this point, assume creation was successful
    return true;
}

void Window::destroy() {
    if (surface)
        vkDestroySurfaceKHR(vulkanInstance, surface, nullptr);
    if (instance)
        glfwDestroyWindow(instance);
}

void Window::center(int windowWidth, int windowHeight) {
    // Obtain the video mode of the primary monitor (window should be created there by default)
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    setPosition(mode->width / 2 - windowWidth / 2, mode->height / 2 - windowHeight / 2);
}