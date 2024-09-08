#include "../include/NodeViewer.h"

void Prisma::NodeViewer::varsDispatcher(Prisma::Component::ComponentType types) {
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    auto type = std::get<0>(types);
    auto name = std::get<1>(types);
    auto variable = std::get<2>(types);

    switch (type) {
    case Component::TYPES::BUTTON:
        if (ImGui::Button(name.c_str())) {
            (*((std::function<void()>*)variable))();
        }
        break;
    case Component::TYPES::VEC3:
        ImGui::InputFloat3(name.c_str(), glm::value_ptr(*static_cast<glm::vec3*>(variable)));
        break;
    case Component::TYPES::VEC2:
        ImGui::InputFloat2(name.c_str(), glm::value_ptr(*static_cast<glm::vec2*>(variable)));
        break;
    case Component::TYPES::INT:
        ImGui::InputInt(name.c_str(), static_cast<int*>(variable));
        break;
    case Component::TYPES::FLOAT:
        ImGui::InputFloat(name.c_str(), static_cast<float*>(variable));
        break;
    case Component::TYPES::STRING:
        ImGui::Text("%s", ((std::string*)variable)->c_str());
        break;
    case Component::TYPES::BOOL:
        ImGui::Checkbox(name.c_str(), static_cast<bool*>(variable));
        break;
    case Component::TYPES::STRINGLIST:
        auto comboData = static_cast<Prisma::Component::ComponentList*>(variable);
        ImGui::Combo(name.c_str(), &comboData->currentitem, comboData->items.data(), comboData->items.size());
        break;

    }
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
}
