#pragma once

#include "../utils/FPSUtils.h"
#include "Settings.h"
#include "Window.h"
#include "input/Input.h"
#include "render/Renderer.h"
#include "vulkan/VulkanInstance.h"

// TODO: Remove these
class ShaderGroup;
class GraphicsPipeline;
class GraphicsPipelineLayout;
class MeshRenderData;
class DescriptorSetLayout;
class DescriptorSet;
class UBO;

/*****************************************************************************
 * BaseEngine class - Handles setup and execution of the main engine loop
 *****************************************************************************/

class BaseEngine : public InputListener {
private:
    /* Engine settings*/
    Settings settings{};

    /* Window instance for the engine */
    Window* window = nullptr;

    /* InputManager instance for the engine */
    InputManager* inputManager = nullptr;

    /* Frame rate calculator and limiter */
    FPSCalculator fpsCalculator;
    FPSLimiter fpsLimiter;

    /* Vulkan instance */
    VulkanInstance* vulkanInstance = nullptr;

    /* TODO: Remove */
    VulkanDevice* vulkanDevice;
    Renderer* renderer;
    ShaderGroup* shaderGroup;
    GraphicsPipelineLayout* pipelineLayout;
    GraphicsPipeline* pipeline;
    MeshRenderData* meshRenderData;
    DescriptorSetLayout* descriptorSetLayout;
    DescriptorSet* descriptorSet;

    struct ShaderBlock_Test {
        float test;
    };
    ShaderBlock_Test shaderBlockTest;
    UBO* testUBO;

public:
    /* Constructor and destructors */
    BaseEngine() {}
    virtual ~BaseEngine() {}

    /* Should be called to initialise the engine */
    void create();

    /* Called to initialise anything that needs to be done before creating a
       window */
    virtual void initialise() {}

    /* Called after window creation - can use Vulkan from this point */
    virtual void created() {}

    /* Called to update the main game loop */
    virtual void update() {}

    /* Called to render the main game loop */
    virtual void render() {}

    /* Called to destroy any created resources just before the engine stops */
    virtual void destroy() {}

    /* TODO: Move??? */
    void drawFrame();

    /* Returns a reference to the settings for assigning */
    inline Settings& getSettings() { return settings; }

    /* Returns the window instance */
    inline Window* getWindow() { return window; }

    /* For obtaining FPS and current frame delta (in seconds) */
    inline unsigned int getFPS() { return fpsCalculator.getFPS(); }
    inline float getDelta() { return fpsCalculator.getDelta(); }
};