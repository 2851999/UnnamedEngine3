#pragma once

#include <vector>

#include "../Window.h"

/*****************************************************************************
 * InputListener class - Interface that can be used to obtain input events
 *****************************************************************************/

class InputListener {
public:
    /* Constructor and destructor */
    InputListener() {}
    virtual ~InputListener() {}

    /* Keyboard input events */
    virtual void onKeyPressed(int key, bool repeated) {}
    virtual void onKeyReleased(int key) {}
    virtual void onChar(int key, char character) {}

    /* Mouse input events */
    virtual void onMousePressed(int button) {}
    virtual void onMouseReleased(int button) {}
    virtual void onMouseMoved(double x, double y, double dx, double dy) {}
    virtual void onMouseDragged(double x, double y, double dx, double dy) {}
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}
    virtual void onScroll(double dx, double dy) {}
};

/*****************************************************************************
 * InputManager class - Handles InputListener instances for input from a
 *                      Window
 *****************************************************************************/

class InputManager {
private:
    /* As GLFW callbacks are static, want to store a list of input managers
       so we can look them up and choose the correct one for the window */
    static std::vector<InputManager*> activeManagers;

    /* Attached input listeners*/
    std::vector<InputListener*> listeners;

    /* Window this manager handles input for */
    Window* window;

    /* Stores the last known cursor position */
    Vector2d lastKnownCursorPos{-1.0, -1.0};

    /* Various methods for calling input events on the listeners */
    inline void callOnKeyPressed(int key, bool repeated) {
        for (const auto& listener : listeners)
            listener->onKeyPressed(key, repeated);
    }

    inline void callOnKeyReleased(int key) {
        for (const auto& listener : listeners)
            listener->onKeyReleased(key);
    }

    inline void callOnChar(int key, char character) {
        for (const auto& listener : listeners)
            listener->onChar(key, character);
    }

    inline void callOnMousePressed(int key) {
        for (const auto& listener : listeners)
            listener->onMousePressed(key);
    }

    inline void callOnMouseReleased(int key) {
        for (const auto& listener : listeners)
            listener->onMouseReleased(key);
    }

    inline void callOnMouseMoved(double x, double y, double dx, double dy) {
        for (const auto& listener : listeners)
            listener->onMouseMoved(x, y, dx, dy);
    }

    inline void callOnMouseDragged(double x, double y, double dx, double dy) {
        for (const auto& listener : listeners)
            listener->onMouseDragged(x, y, dx, dy);
    }

    inline void callOnMouseEnter() {
        for (const auto& listener : listeners)
            listener->onMouseEnter();
    }

    inline void callOnMouseLeave() {
        for (const auto& listener : listeners)
            listener->onMouseEnter();
    }

    inline void callOnScroll(double dx, double dy) {
        for (const auto& listener : listeners)
            listener->onScroll(dx, dy);
    }

    /* Method to select which input manager to send events to given the window
       instance - returns nullptr if not found */
    static inline InputManager* findInputManager(GLFWwindow* windowInstance) {
        for (const auto& manager : activeManagers) {
            if (manager->getWindow()->getInstance() == windowInstance)
                return manager;
        }
        return nullptr;
    }

    /* Callbacks to be called by the GLFW */
    static void keyCallback(GLFWwindow* windowInstance, int key, int scancode, int action, int mods);
    static void charCallback(GLFWwindow* windowInstance, unsigned int codepoint);
    static void cursorPosCallback(GLFWwindow* windowInstance, double xpos, double ypos);
    static void cursorEnterCallback(GLFWwindow* windowInstance, int entered);
    static void mouseButtonCallback(GLFWwindow* windowInstance, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* windowInstance, double xoffset, double yoffset);

public:
    /* Constructor and destructor */
    /* Note: Should only create one input manager per Window */
    InputManager(Window* window);
    virtual ~InputManager();

    /* Adds/removes an input listener by value */
    inline void addListener(InputListener* listener) { listeners.push_back(listener); }
    inline void removeListener(InputListener* listener) { listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end()); }

    /* Returns whether a given mouse button is currently pressed */
    inline bool isMouseButtonPressed(int button) { return glfwGetMouseButton(window->getInstance(), button); }

    /* Returns whether a given key is currently pressed */
    inline bool isKeyPressed(int key) {
        int state = glfwGetKey(window->getInstance(), key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    /* Returns the window associated with this manager */
    inline Window* getWindow() { return window; }
};