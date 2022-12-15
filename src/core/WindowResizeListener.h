#pragma once

/*****************************************************************************
 * WindowResizeListener class - Interface that can be used to obtain window
 *                              resizing events (needs different header due
 *                              to forward declarations in SwapChain)
 *****************************************************************************/

class WindowResizeListener {
public:
    /* Constructor and destructor */
    WindowResizeListener() {}
    virtual ~WindowResizeListener() {}

    /* Window resize event */
    virtual void onWindowResized(unsigned int oldWidth, unsigned int oldHeight, unsigned int newWidth, unsigned int newHeight) {}
};