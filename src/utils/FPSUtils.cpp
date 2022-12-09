#include "FPSUtils.h"

#include "TimeUtils.h"

/*****************************************************************************
 * FPSCalculator class
 *****************************************************************************/

void FPSCalculator::start() {
    // Ensure haven't already started
    if (! started) {
        // Start measuring from now
        lastFrameTime = utils_time::getSeconds();
        started       = true;
    }
}

void FPSCalculator::update() {
    // Don't calculate if not necessary
    if (started && mode != OFF) {
        // Obtain current time
        double currentTime = utils_time::getSeconds();

        // With a high FPS the delta will become zero, but to avoid this
        // causing issues the current delta will only be assigned when
        // this is not the case and at least some time has passed
        if (currentTime != lastFrameTime) {
            // Compute the current delta
            currentDelta = currentTime - lastFrameTime;

            // Assign the last frame to now
            lastFrameTime = currentTime;
        }

        // Check the FPS calculation mode
        if (mode == PER_FRAME) {
            // Prevent divide by 0
            if (currentDelta != 0)
                // Compute the FPS
                currentFPS = static_cast<unsigned int>(1.0 / currentDelta);
        } else {
            // Calculating per second

            // Increment counter and check whether its been 1 second since the last update
            ++frameCount;

            if (currentTime - lastFPSCountUpdate >= 1.0) {
                // Time to update
                lastFPSCountUpdate = currentTime;
                currentFPS         = frameCount;
                frameCount         = 0;
            }
        }
    }
}

void FPSCalculator::reset() {
    lastFrameTime      = 0;
    currentDelta       = 0;
    currentFPS         = 0;
    frameCount         = 0;
    lastFPSCountUpdate = 0;
    started            = false;
}

/*****************************************************************************
 * FPSLimiter class
 *****************************************************************************/

void FPSLimiter::setTarget(unsigned int targetFPS) {
    this->targetFPS   = targetFPS;
    this->targetDelta = 1.0 / static_cast<double>(targetFPS);
}

void FPSLimiter::startFrame() {
    // Obtain current time
    this->frameStart = utils_time::getSeconds();
}

void FPSLimiter::endFrame() {
    // Ensure there is a cap
    if (this->targetFPS > 0) {
        // Calculate difference in target delta and current and wait the
        // difference
        double difference = this->targetDelta - (utils_time::getSeconds() - this->frameStart);

        if (difference > 0)
            utils_time::wait(difference);
    }
}