#include "Helpers/FPSCounter.h"
#include <iostream>

Prisma::FPSCounter::FPSCounter() : m_fps(60) { 
    m_lastTime = std::chrono::high_resolution_clock::now();
}

void Prisma::FPSCounter::calculate() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - m_lastTime;
        m_lastTime = currentTime;
        m_fps = 1.0f / deltaTime.count();
}

// Get the current FPS
double Prisma::FPSCounter::getFPS() const { return m_fps; }
