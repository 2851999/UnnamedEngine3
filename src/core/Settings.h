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
    std::string title   = "Unnamed Engine 3";
    unsigned int width  = 0;
    unsigned int height = 0;
    float aspectRatio   = 0.0f;
    bool resizable      = false;
    bool decorated      = true;
    bool borderless     = false;
    bool floating       = false;
};

/*****************************************************************************
 * Settings struct - Contains various settings for the engine
 *****************************************************************************/

struct Settings {
    /* Window settings */
    WindowSettings window = {}
};