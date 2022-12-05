#include "BaseEngine.h"

#include <stdexcept>

#include "../utils/Logging.h"
#include "GLFW/glfw3.h"

/*****************************************************************************
 * BaseEngine class
 *****************************************************************************/

void BaseEngine::create() {
    // Initialise
    this->initialise();

    // Initialise GLFW
    if (! glfwInit()) {
        Logger::log("Failed to initialise GLFW", "GLFW", LogType::Error);
        throw std::runtime_error("Failed to initialise GLFW");
    }

    /* Terminate GLFW */
    glfwTerminate();
}