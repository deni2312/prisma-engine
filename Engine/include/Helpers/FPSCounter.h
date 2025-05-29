#pragma once

#include <chrono>

namespace Prisma {

    class FPSCounter {
       public:
        FPSCounter() : frameCount(0), fps(0.0) { lastTime = std::chrono::high_resolution_clock::now(); }

        // Call at the beginning of each frame
        void begin();

        // Call at the end of each frame
        void end();

        // Get the current FPS
        double getFPS() const;

       private:
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point lastTime;
        int frameCount;
        double fps;
    };
}  // namespace Prisma