#pragma once

#include "Renderer.h"

/*****************************************************************************
 * RendererResource class - For objects that need access to the Renderer
 *****************************************************************************/

class RendererResource {
protected:
    /* Renderer for use by the resource */
    Renderer* renderer;

public:
    /* Constructor and destructor */
    RendererResource(Renderer* renderer) : renderer(renderer) {}
    virtual ~RendererResource() {}
};