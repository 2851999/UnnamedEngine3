#pragma once

/*****************************************************************************
 * FPSCalculator class - Tool for calculating FPS
 *****************************************************************************/

class FPSCalculator {
public:
    /* Various calculation modes */
    enum Mode {
        /* Don't calculate the FPS */
        OFF,
        /* Calculate the FPS based on the current delta between frames */
        PER_FRAME,
        /* Calculate FPS by counting how many frames occur in a second */
        PER_SECOND,
    };

private:
    /* Last frame time in milliseconds */
    double lastFrameTime = 0;
    /* Current time between frames in seconds */
    double currentDelta = 0;
    /* Current measured FPS */
    unsigned int currentFPS = 0;
    /* Current frame count used in PER_SECOND mode */
    unsigned int frameCount = 0;
    /* Time of last FPS count update (For PER_SECOND)*/
    double lastFPSCountUpdate = 0;
    /* Mode this calculator should use */
    Mode mode;
    /* Has this calculator started monitoring? */
    bool started = false;

public:
    /* Constructor and destructor */
    FPSCalculator(Mode mode = PER_SECOND) : mode(mode) {}
    virtual ~FPSCalculator() {}

    /* Start the monitoring */
    void start();

    /* Update and recalculate the frame delta/FPS if necessary
       Should be called once per frame */
    void update();

    /* Reset */
    void reset();

    /* Returns the current frame delta (in seconds) (For convenience convert to float here) */
    inline float getDelta() { return static_cast<float>(currentDelta); }

    /* Assigns the mode - best to set this before starting */
    inline void setMode(Mode mode) { this->mode = mode; }

    /* Returns the current measured FPS */
    unsigned int getFPS() { return currentFPS; }
};

/*****************************************************************************
 * FPSLimiter class - Tool for limiting FPS
 *****************************************************************************/

class FPSLimiter {
private:
    /* Target FPS - value of 0 means no target */
    unsigned int targetFPS;

    /* Time at the start of the current frame */
    double frameStart = 0;

    /* Target delta for the above FPS */
    double targetDelta = 0;

public:
    /* Constructor and destructor */
    FPSLimiter(unsigned int targetFPS = 0) : targetFPS(targetFPS) {}
    virtual ~FPSLimiter() {}

    /* Assigns the target FPS */
    void setTarget(unsigned int targetFPS);

    /* Should be called at the start of a frame */
    void startFrame();

    /* Should be called at the end of a frame - pauses the thread to match the
       target FPS */
    void endFrame();
};