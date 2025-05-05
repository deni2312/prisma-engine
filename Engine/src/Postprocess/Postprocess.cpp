#include "Postprocess/Postprocess.h"

Prisma::Postprocess::Postprocess() {
}

void Prisma::Postprocess::render() {
    for (auto effect : m_effects) {
        effect->render();
    }
}

void Prisma::Postprocess::addPostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect) {
    m_effects.push_back(postprocessEffect);
}

void Prisma::Postprocess::removePostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect) {
    // Find the effect in the m_effects list
    auto it = std::find(m_effects.begin(), m_effects.end(), postprocessEffect);
    if (it != m_effects.end()) {
        m_effects.erase(it);
    }
}