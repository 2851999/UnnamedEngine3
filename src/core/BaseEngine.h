#pragma once

#include "Settings.h"
#include "Window.h"

/*****************************************************************************
 * BaseEngine class - Handles setup and execution of the main engine loop
 *****************************************************************************/

class BaseEngine {
private:
    /* Engine settings*/
    Settings settings = {};

    /* Window instance for the engine */
    Window* window = nullptr;

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
};