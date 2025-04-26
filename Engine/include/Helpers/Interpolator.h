#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Prisma {
class Interpolator {
public:
        struct Timeframe {
                glm::mat4 position;
                float duration;
        };

        Interpolator();

        glm::mat4 next(float dt);

        void timeframe(const std::vector<Timeframe>& timeframe);

private:
        std::vector<Timeframe> m_timeframe;
        size_t m_currentIndex;
        float m_elapsedTime;
};
}