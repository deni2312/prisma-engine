#include "Helpers/FPSCounter.h"

// Call at the beginning of each frame
void Prisma::FPSCounter::begin() { startTime = std::chrono::high_resolution_clock::now(); }

// Call at the end of each frame
void Prisma::FPSCounter::end() {
    ++frameCount;
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - lastTime;

    if (elapsed.count() >= 1.0) {
        fps = frameCount / elapsed.count();
        frameCount = 0;
        lastTime = endTime;
    }
}

// Get the current FPS
double Prisma::FPSCounter::getFPS() const { return fps; }
