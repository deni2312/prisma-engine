#include <chrono>

namespace Prisma {

    class TimeCounter {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        bool running;

    public:
        TimeCounter();

        void start();

        void stop();

        double duration_seconds() const;
    };

}
