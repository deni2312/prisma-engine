#pragma once

#include <memory>
#include "../../Engine/include/SceneObjects/Node.h"
#include "ImGuiCamera.h"

namespace Prisma {
    class ImGuiTabs {
    public:
        static ImGuiTabs &getInstance();

        ImGuiTabs(const ImGuiTabs &) = delete;

        ImGuiTabs &operator=(const ImGuiTabs &) = delete;

        void showNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera);

        ImGuiTabs();
    private:
        static std::shared_ptr<ImGuiTabs> instance;

        void showCurrentNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera);

        unsigned long int index = 0;
    };

}
