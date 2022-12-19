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

        // Create swap chain
        swapChain = new SwapChain(vulkanDevice, window, settings);

        // Listen for recreation events
        swapChain->addListener(this);

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

        meshRenderData = new MeshRenderData(vulkanDevice, meshData);
        delete meshData;

        ShaderInterface shaderInterface;
        shaderInterface.addAttributeLocation(MeshData::POSITION, 0);
        shaderInterface.addAttributeLocation(MeshData::COLOUR, 1);

        descriptorSetLayout = new DescriptorSetLayout(vulkanDevice);
        descriptorSetLayout->addUBO(0, VK_SHADER_STAGE_VERTEX_BIT);

        shaderGroup           = ShaderGroup::load(vulkanDevice, "./resources/shaders/simple");
        pipelineLayout        = new GraphicsPipelineLayout(vulkanDevice);
        renderPass            = new RenderPass(vulkanDevice, swapChain);
        pipeline              = new GraphicsPipeline(pipelineLayout, renderPass, shaderGroup, settings.video.resolution.getX(), settings.video.resolution.getY(), MeshData::computeVertexInputDescription(2, {MeshData::POSITION, MeshData::COLOUR}, MeshData::SEPARATE_NONE, shaderInterface), swapChain);
        swapChainFramebuffers = swapChain->createFramebuffers(renderPass);

        VulkanDevice::QueueFamilyIndices queueFamilyIndices = vulkanDevice->getQueueFamilyIndices();

        // Create command buffers
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanDevice->createGraphicsCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

        currentFrame = 0;

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signalled as default is not and would cause 'vkWaitForFences called for fence 0x11 which has not been submitted on a Queue or during acquire next image'

        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            if (vkCreateSemaphore(vulkanDevice->getVkLogical(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkanDevice->getVkLogical(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkanDevice->getVkLogical(), &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                Logger::logAndThrowError("Failed to create synchronisation objects for a frame", "BaseEngine");
            }
        }

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

        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(vulkanDevice->getVkLogical(), imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanDevice->getVkLogical(), renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vulkanDevice->getVkLogical(), inFlightFences[i], nullptr);
        }

        // Destroy the Vulkan swap chain and device
        for (unsigned int i = 0; i < swapChainFramebuffers.size(); ++i)
            delete swapChainFramebuffers[i];
        delete meshRenderData;
        delete pipeline;
        delete pipelineLayout;
        delete descriptorSetLayout;
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

void BaseEngine::drawFrame() {
    // Wait for all fences (VK_TRUE)
    vkWaitForFences(vulkanDevice->getVkLogical(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire the next swap chain image (don't render if recreating swap chain)
    if (! swapChain->acquireNextImage(imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE))
        return;

    // Only reset if actually submitting work
    vkResetFences(vulkanDevice->getVkLogical(), 1, &inFlightFences[currentFrame]);  // Unlike semaphores have to reset after use

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    // Begin recording to command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to start recording to command buffer", "BaseEngine");

    renderPass->begin(commandBuffers[currentFrame], swapChainFramebuffers[swapChain->getCurrentImageIndex()], swapChain->getExtent());

    pipeline->bind(commandBuffers[currentFrame]);

    // Perform any rendering
    this->render();

    meshRenderData->render(commandBuffers[currentFrame]);

    renderPass->end(commandBuffers[currentFrame]);

    // Stop recording to command buffer
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to stop recording to command buffer", "BaseEngine");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[]      = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;
    submitInfo.commandBufferCount     = 1;
    submitInfo.pCommandBuffers        = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    // Fence added here to be signalled when command buffer finishes executing
    if (vkQueueSubmit(vulkanDevice->getVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to submit draw command buffer", "BaseEngine");

    // Present the next image in the swap chain
    if (! swapChain->presentImage(1, signalSemaphores))
        return;

    // vkAcquireNextImageKHR semaphore signalled will be the one with this index (so must increase before it is called again)
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void BaseEngine::onSwapChainRecreation(float scaleX, float scaleY) {
    for (unsigned int i = 0; i < swapChainFramebuffers.size(); ++i)
        delete swapChainFramebuffers[i];

    // Render pass only needs destroying due to format changing e.g. going from non-hdr to hdr monitor
    renderPass->recreate(swapChain);
    swapChainFramebuffers = swapChain->createFramebuffers(renderPass);
}