#include "../include/ImGuiHelper.h"
#include "ThirdParty/imgui/imgui.h"

void Prisma::GUI::ImGuiHelper::clipVertical(int size, std::function<void(int)> data) {
    ImGuiListClipper clipper;
    clipper.Begin(size);
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
            data(i);
        }
    }
}