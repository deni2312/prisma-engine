#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Prisma::GUI {
class ImGuiLog {
public:
    void render();

private:
    void print(const glm::vec4& color, const std::string& text);
};
}