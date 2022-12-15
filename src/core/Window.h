#pragma once

#include "Settings.h"
#include "vulkan/VulkanInstance.h"
#include "WindowResizeListener.h"

/*****************************************************************************
 * Window - Handles a window using GLFW
 *****************************************************************************/

class Window {
private:
    /* Window instance this handles */
    GLFWwindow* instance = nullptr;

    /* Vulkan instance and surface */
    VkInstance vulkanInstance = VK_NULL_HANDLE;
    VkSurfaceKHR surface      = VK_NULL_HANDLE;

    /* Listeners for window resizing events */
    std::vector<WindowResizeListener*> resizeListeners;

    /* Reference to the window settings - used for updating the window size */
    WindowSettings& windowSettings;

    /* For assigning various window hints */
    inline void setResizable(bool resizable) { glfwWindowHint(GLFW_RESIZABLE, resizable); }
    inline void setDecorated(bool decorated) { glfwWindowHint(GLFW_DECORATED, decorated); }
    inline void setFloating(bool floating) { glfwWindowHint(GLFW_FLOATING, floating); }
    inline void setRefreshRate(int refreshRate) { glfwWindowHint(GLFW_REFRESH_RATE, refreshRate); }

    /* For centering the monitor shortly after creation - assumes its created on the primary */
    void center(int windowWidth, int windowHeight);

    /* Called when framebuffer size changes */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    /* Calls a window resize event (works out the old and new window sizes itself) */
    void callOnWindowResized();

public:
    /* Constructor and destructors */
    Window(WindowSettings& windowSettings) : windowSettings(windowSettings) {}
    virtual ~Window() { destroy(); }

    /* Create the window - returns whether completed successfully */
    bool create(VideoSettings& videoSettings, const VulkanInstance* vulkanInstance);

    /* Returns whether the user has requested the window to close */
    inline bool shouldClose() const { return glfwWindowShouldClose(instance); }

    /* Requests this window to close */
    inline void close() { glfwSetWindowShouldClose(instance, true); }

    /* Assigns the position of this window */
    inline void setPosition(int x, int y) { glfwSetWindowPos(instance, x, y); }

    /* Destroy's the window (if created) */
    void destroy();

    /* Adds/removes a window resize listener by value */
    inline void addResizeListener(WindowResizeListener* listener) { resizeListeners.push_back(listener); }
    inline void removeResizeListener(WindowResizeListener* listener) { resizeListeners.erase(std::remove(resizeListeners.begin(), resizeListeners.end(), listener), resizeListeners.end()); }

    /* Returns the GLFW instance */
    inline GLFWwindow* getInstance() const { return instance; }

    /* Returns the VkSurface */
    inline VkSurfaceKHR getVkSurface() const { return surface; }
};