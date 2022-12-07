#pragma once

#include <GLFW/glfw3.h>

#include "Settings.h"

/*****************************************************************************
 * Window - Handles a window using GLFW
 *****************************************************************************/

class Window {
private:
    /* Window instance this handles */
    GLFWwindow* instance = nullptr;

    /* For assigning various window hints */
    inline void setResizable(bool resizable) { glfwWindowHint(GLFW_RESIZABLE, resizable); }
    inline void setDecorated(bool decorated) { glfwWindowHint(GLFW_DECORATED, decorated); }
    inline void setFloating(bool floating) { glfwWindowHint(GLFW_FLOATING, floating); }
    inline void setRefreshRate(int refreshRate) { glfwWindowHint(GLFW_REFRESH_RATE, refreshRate); }

    /* For centering the monitor shortly after creation - assumes its created on the primary */
    void center(int windowWidth, int windowHeight);

public:
    /* Constructor and destructors */
    Window() {}
    virtual ~Window() { destroy(); }

    /* Create the window - returns whether completed successfully */
    bool create(WindowSettings& windowSettings, VideoSettings& videoSettings);

    /* Returns whether the user has requested the window to close */
    inline bool shouldClose() { return glfwWindowShouldClose(instance); }

    /* Requests this window to close */
    inline void close() { return glfwSetWindowShouldClose(instance, true); }

    /* Assigns the position of this window */
    inline void setPosition(int x, int y) { glfwSetWindowPos(instance, x, y); }

    /* Destroy's the window (if created) */
    void destroy();
};