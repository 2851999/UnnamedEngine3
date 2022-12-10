#include "BaseEngine.h"

#include "../utils/Logging.h"
#include "../utils/TimeUtils.h"

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

    // Create the window
    this->window       = new Window();
    bool windowCreated = this->window->create(this->settings.window, this->settings.video);

    if (! windowCreated)
        Logger::log("Failed to create a window", "BaseEngine", LogType::Error);
    else {
        // Create the input manager
        inputManager = new InputManager(this->window);
        inputManager->addListener(this);

        // Vulkan initialisation
        vulkanInstance = new VulkanInstance();
        vulkanInstance->create(settings);

        // Pick a physical device
        vulkanInstance->pickPhysicalDevice();

        // Now we are ready to create things for Vulkan
        this->created();

        // Setup the FPS limiter
        this->fpsLimiter.setTarget(settings.video.maxFPS);

        // Start tracking FPS
        this->fpsCalculator.start();

        // Main engine loop (Continue unless requested to stop)
        while (! this->window->shouldClose()) {
            // Start of frame
            this->fpsLimiter.startFrame();

            // Update the fps calculator
            this->fpsCalculator.update();

            // Poll any glfw events
            glfwPollEvents();

            // Update any game logic
            this->update();

            // Perform any rendering
            this->render();

            // End of frame
            this->fpsLimiter.endFrame();
        }

        // Now to destroy everything
        this->destroy();

        // Vulkan
        delete vulkanInstance;
    }

    // Destroy the input manager and window
    delete this->inputManager;
    delete this->window;

    // Terminate GLFW
    glfwTerminate();
}