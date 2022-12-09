
#include "Input.h"

/*****************************************************************************
 * InputManager class
 *****************************************************************************/

std::vector<InputManager*> InputManager::activeManagers;

InputManager::InputManager(Window* window) : window(window) {
    activeManagers.push_back(this);

    // Add the input callbacks
    GLFWwindow* instance = window->getInstance();

    glfwSetKeyCallback(instance, keyCallback);
    glfwSetCharCallback(instance, charCallback);
    glfwSetCursorPosCallback(instance, cursorPosCallback);
    glfwSetCursorEnterCallback(instance, cursorEnterCallback);
    glfwSetMouseButtonCallback(instance, mouseButtonCallback);
    glfwSetScrollCallback(instance, scrollCallback);
}

InputManager::~InputManager() {
    activeManagers.erase(std::remove(activeManagers.begin(), activeManagers.end(), this));
}

void InputManager::keyCallback(GLFWwindow* windowInstance, int key, int scancode, int action, int mods) {
    InputManager* manager = findInputManager(windowInstance);

    // Check the action and call appropriately
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        manager->callOnKeyPressed(key, action == GLFW_REPEAT);
    else if (action == GLFW_RELEASE)
        manager->callOnKeyReleased(key);
}

void InputManager::charCallback(GLFWwindow* windowInstance, unsigned int codepoint) {
    InputManager* manager = findInputManager(windowInstance);

    manager->callOnChar(codepoint, (char) codepoint);
}

void InputManager::cursorPosCallback(GLFWwindow* windowInstance, double xpos, double ypos) {
    InputManager* manager = findInputManager(windowInstance);

    // Compute the difference in position
    double dx = xpos - manager->lastKnownCursorPos.getX();
    double dy = ypos - manager->lastKnownCursorPos.getY();

    // Reset if this is the first time this callback has been called
    if (manager->lastKnownCursorPos.getX() < 0) {
        dx = 0;
        dy = 0;
    }
    manager->lastKnownCursorPos.setX(xpos);
    manager->lastKnownCursorPos.setY(ypos);

    manager->callOnMouseMoved(xpos, ypos, dx, dy);

    // Call dragged as well if the mouse is pressed
    if (manager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        manager->callOnMouseDragged(xpos, ypos, dx, dy);
}

void InputManager::cursorEnterCallback(GLFWwindow* windowInstance, int entered) {
    InputManager* manager = findInputManager(windowInstance);

    entered ? manager->callOnMouseEnter() : manager->callOnMouseLeave();
}

void InputManager::mouseButtonCallback(GLFWwindow* windowInstance, int button, int action, int mods) {
    InputManager* manager = findInputManager(windowInstance);

    // Check the action and call appropriately
    if (action == GLFW_PRESS)
        manager->callOnMousePressed(button);
    else if (action == GLFW_RELEASE)
        manager->callOnMouseReleased(button);
}

void InputManager::scrollCallback(GLFWwindow* windowInstance, double xoffset, double yoffset) {
    InputManager* manager = findInputManager(windowInstance);

    manager->callOnScroll(xoffset, yoffset);
}