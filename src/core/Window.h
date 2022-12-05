#pragma once

#include <GLFW/glfw3.h>

/*****************************************************************************
 * Window - Handles a window using GLFW
 *****************************************************************************/

class Window {
private:
    /* Window instance this handles */
    GLFWwindow* instance = nullptr;

public:
    /* Constructor and destructors */
    Window() {}
    virtual ~Window() { destroy(); }

    /* Create then window */
    void create();

    /* Updates the window */
    void update();

    /* Destroy's the window (if created) */
    void destroy();
};