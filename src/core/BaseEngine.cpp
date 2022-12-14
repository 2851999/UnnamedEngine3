#include "BaseEngine.h"

#include "../utils/Logging.h"
#include "../utils/TimeUtils.h"
#include "render/Framebuffer.h"
#include "render/GraphicsPipeline.h"
#include "render/RenderPass.h"
#include "render/Shader.h"

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

    // States whether initialisation was successful and we can continue to
    // the main engine loop
    bool initSuccess = true;

    // Vulkan initialisation
    vulkanInstance = new VulkanInstance();
    if (! vulkanInstance->create(settings)) {
        Logger::log("Failed to create a Vulkan instance", "BaseEngine", LogType::Error);
        initSuccess = false;
    }

    // Create the window
    this->window = new Window();
    if (! this->window->create(this->settings.window, this->settings.video, vulkanInstance)) {
        Logger::log("Failed to create a window", "BaseEngine", LogType::Error);
        initSuccess = false;
    }

    if (initSuccess) {
        // Create the input manager
        inputManager = new InputManager(this->window);
        inputManager->addListener(this);

        // Pick a physical device
        VulkanDevice* vulkanDevice = vulkanInstance->pickPhysicalDevice(settings, this->window);

        // Assign the value of ray tracing to whether it is actually supported
        settings.video.rayTracing = vulkanDevice->isSupported(VulkanDeviceExtensions::RAY_TRACING);

        // Create swap chain
        VulkanSwapChain* swapChain = new VulkanSwapChain(vulkanDevice, settings);

        ShaderGroup* shaderGroup               = ShaderGroup::load(vulkanDevice, "./resources/shaders/test");
        RenderPass* renderPass                 = new RenderPass(vulkanDevice, swapChain);
        GraphicsPipelineLayout* pipelineLayout = new GraphicsPipelineLayout(vulkanDevice);
        GraphicsPipeline* pipeline             = new GraphicsPipeline(pipelineLayout, renderPass, shaderGroup, swapChain->getExtent());
        std::vector<Framebuffer*> swapChainFramebuffers(swapChain->getImageCount());
        for (unsigned int i = 0; i < swapChainFramebuffers.size(); ++i)
            swapChainFramebuffers[i] = new Framebuffer(renderPass, swapChain->getExtent(), swapChain->getImageView(i));

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

        // Destroy input manager
        delete this->inputManager;

        // Destroy the Vulkan swap chain and device
        for (unsigned int i = 0; i < swapChainFramebuffers.size(); ++i)
            delete swapChainFramebuffers[i];
        delete pipeline;
        delete pipelineLayout;
        delete renderPass;
        delete shaderGroup;
        delete swapChain;
        delete vulkanDevice;
    }

    // Destroy the window and Vulkan instance
    delete this->window;
    delete vulkanInstance;

    // Terminate GLFW
    glfwTerminate();
}