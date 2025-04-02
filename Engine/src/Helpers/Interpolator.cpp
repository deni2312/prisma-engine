#include "Helpers/Interpolator.h"
#include <glm/gtc/quaternion.hpp>

Prisma::Interpolator::Interpolator() : m_currentIndex(0), m_elapsedTime(0.0f) {}

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

    glm::vec3 posA = glm::vec3(m_timeframe[m_currentIndex].position[3]);
    glm::vec3 posB = glm::vec3(m_timeframe[nextIndex].position[3]);
    glm::vec3 interpPos = glm::mix(posA, posB, alpha);

    glm::quat rotA = glm::quat_cast(m_timeframe[m_currentIndex].position);
    glm::quat rotB = glm::quat_cast(m_timeframe[nextIndex].position);
    glm::quat interpRot = glm::slerp(rotA, rotB, alpha);

    glm::vec3 scaleA = glm::vec3(glm::length(m_timeframe[m_currentIndex].position[0]),
        glm::length(m_timeframe[m_currentIndex].position[1]),
        glm::length(m_timeframe[m_currentIndex].position[2]));
    glm::vec3 scaleB = glm::vec3(glm::length(m_timeframe[nextIndex].position[0]),
        glm::length(m_timeframe[nextIndex].position[1]),
        glm::length(m_timeframe[nextIndex].position[2]));
    glm::vec3 interpScale = glm::mix(scaleA, scaleB, alpha);

    glm::mat4 result = glm::mat4_cast(interpRot);
    result = glm::scale(result, interpScale);
    result[3] = glm::vec4(interpPos, 1.0f);
    return result;
}

void Prisma::Interpolator::timeframe(const std::vector<Timeframe>& timeframe) {
    m_timeframe = timeframe;
    m_currentIndex = 0;
    m_elapsedTime = 0.0f;
}
