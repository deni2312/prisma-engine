#include "Helpers/Interpolator.h"
#include <glm/gtc/quaternion.hpp>

Prisma::Interpolator::Interpolator() : m_currentIndex(0), m_elapsedTime(0.0f) {
}

glm::mat4 Prisma::Interpolator::next(float dt) {
        if (m_timeframe.empty()) return glm::mat4(1.0f);

        m_elapsedTime += dt;

        if (m_elapsedTime >= m_timeframe[m_currentIndex].duration) {
                m_elapsedTime -= m_timeframe[m_currentIndex].duration;
                m_currentIndex++;
                if (m_currentIndex >= m_timeframe.size()) {
                        m_currentIndex = 0; // Looping behavior
                }
        }

        size_t nextIndex = (m_currentIndex + 1) % m_timeframe.size();
        float alpha = m_elapsedTime / m_timeframe[m_currentIndex].duration;

        auto posA = glm::vec3(m_timeframe[m_currentIndex].position[3]);
        auto posB = glm::vec3(m_timeframe[nextIndex].position[3]);
        glm::vec3 interpPos = mix(posA, posB, alpha);

        glm::quat rotA = quat_cast(m_timeframe[m_currentIndex].position);
        glm::quat rotB = quat_cast(m_timeframe[nextIndex].position);
        glm::quat interpRot = slerp(rotA, rotB, alpha);

        auto scaleA = glm::vec3(length(m_timeframe[m_currentIndex].position[0]),
                                length(m_timeframe[m_currentIndex].position[1]),
                                length(m_timeframe[m_currentIndex].position[2]));
        auto scaleB = glm::vec3(length(m_timeframe[nextIndex].position[0]),
                                length(m_timeframe[nextIndex].position[1]),
                                length(m_timeframe[nextIndex].position[2]));
        glm::vec3 interpScale = mix(scaleA, scaleB, alpha);

        glm::mat4 result = mat4_cast(interpRot);
        result = scale(result, interpScale);
        result[3] = glm::vec4(interpPos, 1.0f);
        return result;
}

void Prisma::Interpolator::timeframe(const std::vector<Timeframe>& timeframe) {
        m_timeframe = timeframe;
        m_currentIndex = 0;
        m_elapsedTime = 0.0f;
}