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

        VulkanDevice::QueueFamilyIndices queueFamilyIndices = vulkanDevice->getQueueFamilyIndices();

        VkCommandPool commandPool;

        // Command pool create info
        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;  // Optional VK_COMMAND_POOL_CREATE_TRANSIENT_BIT - if buffers will be updated many times

        // Attempt to create the command pool
        if (vkCreateCommandPool(vulkanDevice->getVkLogical(), &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create command pool", "BaseEngine");

        const unsigned int MAX_FRAMES_IN_FLIGHT = 2;

        std::vector<VkCommandBuffer> commandBuffers(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool                 = commandPool;
        allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount          = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(vulkanDevice->getVkLogical(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to allocate command buffer", "BaseEngine");

        unsigned int imageIndex = 0;
        unsigned int currentFrame = 0;

        std::vector<VkSemaphore> imageAvailableSemaphores(MAX_FRAMES_IN_FLIGHT);
        std::vector<VkSemaphore> renderFinishedSemaphores(MAX_FRAMES_IN_FLIGHT);
        std::vector<VkFence> inFlightFences(MAX_FRAMES_IN_FLIGHT);

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

            // Aquire the next swap chain image
            vkAcquireNextImageKHR(vulkanDevice->getVkLogical(), swapChain->getVkInstance(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);  // Image index is next in chain

            // Wait for all fences (VK_TRUE)
            vkWaitForFences(vulkanDevice->getVkLogical(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
            vkResetFences(vulkanDevice->getVkLogical(), 1, &inFlightFences[currentFrame]);  // Unlike semaphores have to reset after use

            vkResetCommandBuffer(commandBuffers[currentFrame], 0);

            // Begin recording to command buffer
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo         = nullptr;  // Optional

            if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
                Logger::logAndThrowError("Failed to start recording to command buffer", "BaseEngine");

            renderPass->begin(commandBuffers[currentFrame], swapChainFramebuffers[imageIndex], swapChain->getExtent());

            pipeline->bind(commandBuffers[currentFrame]);

            // Perform any rendering
            this->render();

            vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);

            renderPass->end(commandBuffers[currentFrame]);

            // Stop recording to command buffer
            if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
                Logger::logAndThrowError("Failed to stop recording to command buffer", "BaseEngine");

            VkSubmitInfo submitInfo = {};
            submitInfo.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

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
            VkResult result = vkQueueSubmit(vulkanDevice->getVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
            if (result != VK_SUCCESS)  // vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS
                Logger::logAndThrowError("Failed to submit draw command buffer, result " + utils_string::str(result), "BaseEngine");

            VkPresentInfoKHR presentInfo   = {};
            presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores    = signalSemaphores;

            VkSwapchainKHR swapChains[] = {swapChain->getVkInstance()};
            presentInfo.swapchainCount  = 1;
            presentInfo.pSwapchains     = swapChains;
            presentInfo.pImageIndices   = &imageIndex;
            presentInfo.pResults        = nullptr;

            result = vkQueuePresentKHR(vulkanDevice->getVkPresentQueue(), &presentInfo);
            if (result != VK_SUCCESS)
                Logger::logAndThrowError("Failed to present image from queue " + utils_string::str(result), "BaseEngine");

            // vkAcquireNextImageKHR semaphore signalled will be the one with this index (so must increase before it is called again)
            imageIndex = (imageIndex + 1) % swapChain->getImageCount();

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

            // End of frame
            this->fpsLimiter.endFrame();
        }

        // Ensure everything has finished renderingh
        vkDeviceWaitIdle(vulkanDevice->getVkLogical());

        // Now to destroy everything
        this->destroy();

        // Destroy input manager
        delete this->inputManager;

        for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(vulkanDevice->getVkLogical(), imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanDevice->getVkLogical(), renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vulkanDevice->getVkLogical(), inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(vulkanDevice->getVkLogical(), commandPool, nullptr);

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