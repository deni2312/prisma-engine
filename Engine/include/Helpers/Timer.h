#include <chrono>

namespace Prisma {

    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        bool running;

    public:
        Timer() : running(false) {}

        void start() {
            start_time = std::chrono::high_resolution_clock::now();
            running = true;
        }

        void stop() {
            end_time = std::chrono::high_resolution_clock::now();
            running = false;
        }

        double duration_seconds() const {
            std::chrono::duration<double> duration;
            if (running) {
                duration = std::chrono::high_resolution_clock::now() - start_time;
            }
            else {
                duration = end_time - start_time;
            }
            return duration.count();
        }
    };

}
