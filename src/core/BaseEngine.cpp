#include "BaseEngine.h"

#include "../utils/Logging.h"
#include "../utils/TimeUtils.h"
#include "render/Framebuffer.h"
#include "render/GraphicsPipeline.h"
#include "render/RenderPass.h"
#include "render/Shader.h"
#include "vulkan/VulkanBuffer.h"

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

        // clang-format off
        std::vector<float> vertexData = {
             0.0f, -0.5f,   1.0f, 0.0f, 0.0f,
             0.5f,  0.5f,   0.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,   0.0f, 0.0f, 1.0f
        };
        // clang-format on

        vertexBuffer = new VulkanBuffer(vulkanDevice, sizeof(float) * vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
        vertexBuffer->copy(vertexData.data(), sizeof(float) * vertexData.size());

        // Vertex input binding description
        // TODO: Move to a VBO class
        VkVertexInputBindingDescription vertexInputBindingDescription{};
        vertexInputBindingDescription.binding   = 0;
        vertexInputBindingDescription.stride    = 5 * sizeof(float);
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset   = 0;

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = sizeof(float) * 2;

        shaderGroup           = ShaderGroup::load(vulkanDevice, "./resources/shaders/simple");
        pipelineLayout        = new GraphicsPipelineLayout(vulkanDevice);
        renderPass            = new RenderPass(vulkanDevice, swapChain);
        pipeline              = new GraphicsPipeline(pipelineLayout, renderPass, shaderGroup, settings.video.resolution.getX(), settings.video.resolution.getY(), vertexInputBindingDescription, attributeDescriptions, swapChain);
        swapChainFramebuffers = swapChain->createFramebuffers(renderPass);

        VulkanDevice::QueueFamilyIndices queueFamilyIndices = vulkanDevice->getQueueFamilyIndices();

        // Create a command pool
        VkCommandPool commandPool;
        vulkanDevice->createCommandPool(
            queueFamilyIndices.graphicsFamily.value(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,  // Optional VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - if buffers will be updated many times
            &commandPool);

        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool                 = commandPool;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount          = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(vulkanDevice->getVkLogical(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to allocate command buffer", "BaseEngine");

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

        vulkanDevice->destroyCommandPool(commandPool);

        // Destroy the Vulkan swap chain and device
        for (unsigned int i = 0; i < swapChainFramebuffers.size(); ++i)
            delete swapChainFramebuffers[i];
        delete vertexBuffer;
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

    VkBuffer vertexBuffers[] = {vertexBuffer->getVkInstance()};
    VkDeviceSize offsets[]   = {0};
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

    vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

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