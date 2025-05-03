#pragma once
#include "ImGuiCamera.h"
#include "NodeCreator.h"

namespace Prisma::GUI {
class ImGuiAddingMenu {
public:
    void addMenu(ImGuiCamera& camera) const;

private:
    const int m_subDivisions = 16;
};
}