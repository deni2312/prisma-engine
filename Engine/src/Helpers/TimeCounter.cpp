#include "Helpers/TimeCounter.h"

Prisma::TimeCounter::TimeCounter() : running(false) {
}

void Prisma::TimeCounter::start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
}

void Prisma::TimeCounter::stop() {
        end_time = std::chrono::high_resolution_clock::now();
        running = false;
}

double Prisma::TimeCounter::duration_seconds() const {
        std::chrono::duration<double> duration;
        if (running) {
                duration = std::chrono::high_resolution_clock::now() - start_time;
        } else {
                duration = end_time - start_time;
        }
        return duration.count();
}