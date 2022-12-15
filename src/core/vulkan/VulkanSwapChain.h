#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Settings.h"
#include "../WindowResizeListener.h"

// Forward declarations
class Window;
class VulkanDevice;
class Framebuffer;
class RenderPass;

/*****************************************************************************
 * VulkanSwapChainListener class - Interface that can be used to obtain swap
 *                                 chain recreation events
 *****************************************************************************/

class VulkanSwapChainListener {
public:
    /* Constructor and destructor */
    VulkanSwapChainListener() {}
    virtual ~VulkanSwapChainListener() {}

    /* Called when the swap chain has just been recreated */
    virtual void onSwapChainRecreation() {}
};

/*****************************************************************************
 * VulkanSwapChain class - For handling a swap chain
 *****************************************************************************/

class VulkanSwapChain : WindowResizeListener {
private:
    /* Device used to create this swap chain */
    VulkanDevice* device;

    /* Window this swap chain is for */
    Window* window;

    /* Reference to the settings used to create this swap chain (will update
       video settings on swap chain recreation) */
    Settings& settings;

    /* Swap chain instance */
    VkSwapchainKHR instance;

    /* Surface image format */
    VkFormat imageFormat;

    /* Extent */
    VkExtent2D extent;

    /* Images in the swap chain */
    std::vector<VkImage> images;

    /* Image views for the images in the swap chain */
    std::vector<VkImageView> imageViews;

    /* Current image index (required when displaying images)*/
    uint32_t imageIndex = 0;

    /* Keeps track if the window has been resized (not all drivers will return
       VK_ERROR_OUT_OF_DATE_KHR when the window is resized) */
    bool framebufferResized = false;

    /* Listeners for swap chain recreation events */
    std::vector<VulkanSwapChainListener*> listeners;

    /* Creates this swap chain - the settings will be modified to reflect
       the actual VSync/video resolution & aspect radio chosen */
    void create();

    /* Destroys resources ready for swap chain recreation */
    void destroy();

    /* Trigger swap chain recreation when the window is resized */
    void onWindowResized(unsigned int oldWidth, unsigned int oldHeight, unsigned int newWidth, unsigned int newHeight) override { framebufferResized = true; }

    /* Calls the onSwapChainRecreation function for all added listeners */
    inline void callOnSwapChainRecreation() {
        for (const auto& listener : listeners)
            listener->onSwapChainRecreation();
    }

    /* Helper function - returns the corresponding present mode for a given
       VSync setting */
    static inline VkPresentModeKHR vSyncToPresentMode(int vSync) {
        switch (vSync) {
            case 0:
                // ~No buffering
                return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case 1:
                // ~VSync
                return VK_PRESENT_MODE_FIFO_KHR;
            case 2:
                // ~Triple buffering
                return VK_PRESENT_MODE_MAILBOX_KHR;
            case 3:
                return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            default:
                // Guaranteed to be supported
                return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    /* Helper function - returns the VSync setting corresponding to  a given
       present mode */
    static inline int presentModeToVSync(VkPresentModeKHR presentMode) {
        switch (presentMode) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                // ~No buffering
                return 0;
            case VK_PRESENT_MODE_FIFO_KHR:
                // ~VSync
                return 1;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                // ~Triple buffering
                return 2;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return 3;
            default:
                return 0;
        }
    }

    /* Helper function that gives a surface format a score based on its value */
    static int scoreSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat);

    /* Helper function for choosing a swap chain surface format from a list of
       supported formats */
    static VkSurfaceFormatKHR pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /* Helper function that returns whether a given present mode is present
       in a given list of modes */
    static bool isPresentModeAvailable(VkPresentModeKHR presentMode, const std::vector<VkPresentModeKHR>& availablePresentModes);

    /* Helper function for choosing a swap chain present mode from a list of
       supported modes - Takes settings to allow preference for current VSync
       setting but may also modify it to what is actually chosen */
    static VkPresentModeKHR pickPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const VideoSettings& videoSettings);

    /* Helper function for choosing a swap extent from the surface capabilities */
    static VkExtent2D pickSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const WindowSettings& windowSettings);

public:
    /* Structure used to return support information about the supported
       swap chain capabilities */
    struct Support {
        // Surface the capabilities were queried for (avoids needing more
        // parameters in the constructor)
        VkSurfaceKHR surface;

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /* Constructor and destructor - the settings will be modified to reflect
       the actual VSync/video resolution & aspect radio chosen */
    VulkanSwapChain(VulkanDevice* device, Window* window, Settings& settings);
    virtual ~VulkanSwapChain();

    /* Should be called to acquire a swap chain image at the start of a frame
       - will return a boolean value representing whether rendering should go
       ahead (will be false if swap chain will be recreated) */
    bool acquireNextImage(VkSemaphore semaphore, VkFence fence);

    /* Should be called to present a swap chain image at the end of a frame
       - will return a boolean value representing whether rendering should go
       ahead (will be false if swap chain will be recreated) */
    bool presentImage(uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores);

    /* Recreates this swap chain (for when it becomes out of date) */
    void recreate();

    /* Creates and returns framebuffers for rendering to this swap chain using
       a given render pass*/
    std::vector<Framebuffer*> createFramebuffers(RenderPass* renderPass);

    /* Adds/removes a swap chain listener by value */
    inline void addListener(VulkanSwapChainListener* listener) { listeners.push_back(listener); }
    inline void removeListener(VulkanSwapChainListener* listener) { listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end()); }

    /* Returns various swap chain properties */
    inline VkSwapchainKHR getVkInstance() const { return instance; }
    inline VkFormat getImageFormat() const { return imageFormat; }
    inline VkExtent2D getExtent() const { return extent; }
    inline size_t getImageCount() const { return images.size(); }
    inline uint32_t getCurrentImageIndex() { return imageIndex; }

    /* Returns the given image view */
    inline VkImageView getImageView(unsigned int index) { return imageViews[index]; }

    /* Obtains information about the swap chain support of the given device */
    static VulkanSwapChain::Support querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);
};