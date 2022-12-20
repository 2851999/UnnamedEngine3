#include "BaseEngine.h"

#include "../utils/Logging.h"
#include "../utils/TimeUtils.h"
#include "render/DescriptorSet.h"
#include "render/Framebuffer.h"
#include "render/GraphicsPipeline.h"
#include "render/Mesh.h"
#include "render/RenderData.h"
#include "render/RenderPass.h"
#include "render/Shader.h"
#include "render/ShaderInterface.h"

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
    this->window = new Window(this->settings.window);
    if (! this->window->create(this->settings.video, vulkanInstance)) {
        Logger::log("Failed to create a window", "BaseEngine", LogType::Error);
        initSuccess = false;
    }

    if (initSuccess) {
        // Create the input manager
        inputManager = new InputManager(this->window);
        inputManager->addListener(this);

        // Pick a physical device
        vulkanDevice = vulkanInstance->pickPhysicalDevice(settings, this->window);

        // Assign the value of ray tracing to whether it is actually supported
        settings.video.rayTracing = vulkanDevice->isSupported(VulkanDeviceExtensions::RAY_TRACING);

        // Create tne renderer
        renderer = new Renderer(vulkanDevice, window, settings);

        MeshData* meshData = new MeshData(MeshData::DIMENSIONS_2D);

        // clang-format off
        meshData->addPosition(Vector2f(-0.5f, -0.5f)); meshData->addColour(Colour(1.0f, 0.0f, 0.0f, 1.0f));
        meshData->addPosition(Vector2f( 0.5f, -0.5f)); meshData->addColour(Colour(0.0f, 1.0f, 0.0f, 1.0f));
        meshData->addPosition(Vector2f( 0.5f,  0.5f)); meshData->addColour(Colour(0.0f, 0.0f, 1.0f, 1.0f));
        meshData->addPosition(Vector2f(-0.5f,  0.5f)); meshData->addColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
        meshData->addIndex(0);
        meshData->addIndex(1);
        meshData->addIndex(2);
        meshData->addIndex(2);
        meshData->addIndex(3);
        meshData->addIndex(0);
        // clang-format on

        meshRenderData = new MeshRenderData(renderer, meshData);
        delete meshData;

        ShaderInterface shaderInterface;
        shaderInterface.addAttributeLocation(MeshData::POSITION, 0);
        shaderInterface.addAttributeLocation(MeshData::COLOUR, 1);

        descriptorSetLayout = new DescriptorSetLayout(vulkanDevice);
        descriptorSetLayout->addUBO(0, VK_SHADER_STAGE_VERTEX_BIT);

        shaderGroup    = ShaderGroup::load(vulkanDevice, "./resources/shaders/simple");
        pipelineLayout = new GraphicsPipelineLayout(vulkanDevice);
        pipeline       = new GraphicsPipeline(pipelineLayout, renderer->getDefaultRenderPass(), shaderGroup, settings.video.resolution.getX(), settings.video.resolution.getY(), MeshData::computeVertexInputDescription(2, {MeshData::POSITION, MeshData::COLOUR}, MeshData::SEPARATE_NONE, shaderInterface), renderer->getSwapChain());

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

            // Draw the frame
            drawFrame();

            // End of frame
            this->fpsLimiter.endFrame();
        }

        // Ensure everything has finished rendering
        vulkanDevice->waitIdle();

        // Now to destroy everything
        this->destroy();

        // Destroy input manager
        delete this->inputManager;

        delete meshRenderData;
        delete pipeline;
        delete pipelineLayout;
        delete descriptorSetLayout;
        delete shaderGroup;
        delete renderer;
        delete vulkanDevice;
    }

    // Destroy the window and Vulkan instance
    delete this->window;
    delete vulkanInstance;

    // Terminate GLFW
    glfwTerminate();
}

void BaseEngine::drawFrame() {
    if (! this->renderer->beginFrame())
        return;

    VkCommandBuffer currentCommandBuffer = renderer->getCurrentCommandBuffer();

    renderer->beginDefaultRenderPass();

    pipeline->bind(currentCommandBuffer);

    // Perform any rendering
    this->render();

    meshRenderData->render(currentCommandBuffer);

    renderer->endDefaultRenderPass();

    if (! this->renderer->endFrame())
        return;
}