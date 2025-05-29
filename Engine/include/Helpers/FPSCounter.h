#pragma once

#include <chrono>

namespace Prisma {

    class FPSCounter {
       public:
        FPSCounter();

        // Call at the end of each frame
        void calculate();

        // Get the current FPS
        double getFPS() const;

       private:
        double m_fps;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
    };
}  // namespace Prisma