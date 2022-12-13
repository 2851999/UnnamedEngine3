#pragma once

#include "maths/Vector.h"

/*****************************************************************************
 * VideoResolution namespace - Contains various predetermined video
 * resolutions
 *****************************************************************************/

namespace VideoResolution {
    const Vector2i RES_640x480   = Vector2i(640, 480);
    const Vector2i RES_1280x720  = Vector2i(1280, 720);
    const Vector2i RES_1366x768  = Vector2i(1366, 768);
    const Vector2i RES_1920x1080 = Vector2i(1920, 1080);
    const Vector2i RES_2560x1440 = Vector2i(2560, 1440);
    const Vector2i RES_3840x2160 = Vector2i(3840, 2160);

    /* Named video resolutions */
    const Vector2i RES_DEFAULT = RES_1280x720;
    const Vector2i RES_720p    = RES_1280x720;
    const Vector2i RES_1080p   = RES_1920x1080;
    const Vector2i RES_1440p   = RES_2560x1440;
    const Vector2i RES_4K      = RES_3840x2160;

    /* Used to convert a Vector2i into a string representing a resolution */
    std::string toString(const Vector2i& resolution);

    /* Used to convert a string resolution in the format widthxheight into a
       Vector2i */
    Vector2i fromString(const std::string& resolution);
}  // namespace VideoResolution

/*****************************************************************************
 * WindowSettings struct - Contains various settings for a Window
 *****************************************************************************/

struct WindowSettings {
    std::string title        = "Unnamed Engine 3";
    unsigned int width       = 0;
    unsigned int height      = 0;
    bool resizable           = false;
    bool decorated           = true;
    bool borderless          = false;
    bool floating            = false;
    unsigned int refreshRate = 0;
};

/*****************************************************************************
 * VideoSettings struct - Contains various video settings
 *****************************************************************************/
struct VideoSettings {
    // These are settings that are involved in the window creation
    bool fullscreen          = 0;
    unsigned int refreshRate = 0;
    Vector2i resolution      = VideoResolution::RES_DEFAULT;
    float aspectRatio        = 0.0f;
    int vSync                = 0;  // 1 for VSync, 2 for triple buffering, 3 for VK_PRESENT_MODE_MAILBOX_KHR
    // Other settings
    unsigned int maxFPS = 0;
    // This may be reassigned after a suitable physical device is found based
    // on its capabilities
    bool rayTracing = false;
};

/*****************************************************************************
 * Debugging settings struct - Contains various debugging settings
 *****************************************************************************/
struct DebugSettings {
    bool validationLayers = false;
};

/*****************************************************************************
 * Settings struct - Contains various settings for the engine
 *****************************************************************************/

struct Settings {
    /* Window settings */
    WindowSettings window{};

    /* Video settings */
    VideoSettings video{};

    /* Debug settings */
    DebugSettings debug{};
};