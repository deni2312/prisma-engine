#include "../include/NodeViewer.h"

void Prisma::NodeViewer::varsDispatcher(Prisma::Component::ComponentType types, int index) {
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    auto type = std::get<0>(types);
    auto name = std::get<1>(types);
    auto variable = std::get<2>(types);
    std::string label = "##dispatcherlabel" + std::to_string(index);
    switch (type) {
    case Component::TYPES::BUTTON:
        if (ImGui::Button(name.c_str())) {
            (*((std::function<void()>*)variable))();
        }
        break;
    case Component::TYPES::VEC3:
        ImGui::Text(name.c_str());
        ImGui::InputFloat3(label.c_str(), glm::value_ptr(*static_cast<glm::vec3*>(variable)));
        break;
    case Component::TYPES::VEC2:
        ImGui::Text(name.c_str());
        ImGui::InputFloat2(label.c_str(), glm::value_ptr(*static_cast<glm::vec2*>(variable)));
        break;
    case Component::TYPES::INT:
        ImGui::Text(name.c_str());
        ImGui::InputInt(name.c_str(), static_cast<int*>(variable));
        break;
    case Component::TYPES::FLOAT:
        ImGui::Text(name.c_str());
        ImGui::InputFloat(label.c_str(), static_cast<float*>(variable));
        break;
    case Component::TYPES::STRING:
        ImGui::Text("%s", ((std::string*)variable)->c_str());
        break;
    case Component::TYPES::BOOL:
        ImGui::Text(name.c_str());
        ImGui::Checkbox(label.c_str(), static_cast<bool*>(variable));
        break;
    case Component::TYPES::STRINGLIST:
        ImGui::Text(name.c_str());
        auto comboData = static_cast<Prisma::Component::ComponentList*>(variable);
        ImGui::Combo(label.c_str(), &comboData->currentitem, comboData->items.data(), comboData->items.size());
        break;

    }
    ImGui::Dummy(ImVec2(0.0f, 2.0f));
}

Prisma::NodeViewer::NodeViewer() {
    m_rotateTexture = std::make_shared<Prisma::Texture>();
    m_rotateTexture->loadTexture("../../../GUI/icons/rotate.png", false, false, false);

    m_translateTexture = std::make_shared<Prisma::Texture>();
    m_translateTexture->loadTexture("../../../GUI/icons/move.png", false, false, false);

    m_scaleTexture = std::make_shared<Prisma::Texture>();
    m_scaleTexture->loadTexture("../../../GUI/icons/scale.png", false, false, false);

    m_eyeOpen = std::make_shared<Prisma::Texture>();
    m_eyeOpen->loadTexture("../../../GUI/icons/eyeopen.png", false, false, false);

    m_eyeClose = std::make_shared<Prisma::Texture>();
    m_eyeClose->loadTexture("../../../GUI/icons/eyeclose.png", false, false, false);
}

void Prisma::NodeViewer::showComponents(Prisma::Node* nodeData) {
    auto components = nodeData->components();
    int i = 0;
    for (const auto& component : components) {
        ImGui::Separator();
        if (component.second->isUi()) {
            auto fields = component.second->globalVars();
            ImGui::Text(component.second->name().c_str());
            for (auto field : fields) {
                Prisma::NodeViewer::getInstance().varsDispatcher(field,i);
            }
        }
        i++;
    }
}

// Getters for textures

void Prisma::NodeViewer::showSelected(const NodeData& nodeData, bool end) {
    if (nodeData.node) {

        float windowWidth = nodeData.translate * nodeData.width / 2.0f;
        auto nextRight = [&](float pos) {
            ImGui::SetNextWindowPos(ImVec2(windowWidth + nodeData.scale * nodeData.width, pos));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, nodeData.height * nodeData.scale - pos));
        };
        nextRight(nodeData.initOffset);
        ImGui::Begin(nodeData.node->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::ImageButton((void*)m_rotateTexture->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton((void*)m_translateTexture->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton((void*)m_scaleTexture->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        }
        ImGui::SameLine();

        auto textureId = 0;

        if (nodeData.node->visible()) {
            textureId = m_eyeOpen->id();
        }
        else {
            textureId = m_eyeClose->id();
        }

        if (ImGui::ImageButton((void*)textureId, ImVec2(24, 24))) {
            nodeData.node->visible(!nodeData.node->visible());
            hideChilds(nodeData.node, nodeData.node->visible());
        }

        ImGui::InputFloat3("Translation", glm::value_ptr(m_translation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Rotation", glm::value_ptr(m_rotation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Scale", glm::value_ptr(m_scale), "%.3f", ImGuiInputTextFlags_ReadOnly);

        drawGizmo(nodeData);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        showComponents(nodeData.node);
        if (end) {
            ImGui::End();
        }
    }
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::rotateTexture() const {
    return m_rotateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::translateTexture() const {
    return m_translateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::scaleTexture() const {
    return m_scaleTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::eyeOpenTexture() const {
    return m_eyeOpen;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::eyeCloseTexture() const {
    return m_eyeClose;
}

void Prisma::NodeViewer::drawGizmo(const NodeData& nodeData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 model = nodeData.node->finalMatrix();
    glm::mat4 inverseParent = glm::mat4(1.0f);
    if (nodeData.node->parent()) {
        inverseParent = glm::inverse(nodeData.node->parent()->finalMatrix());
    }

    ImGuizmo::Manipulate(glm::value_ptr(nodeData.camera->matrix()), glm::value_ptr(nodeData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));

    nodeData.node->matrix(inverseParent * model);
}

void Prisma::NodeViewer::hideChilds(Prisma::Node* root, bool hide) {
    for (auto child : root->children()) {
        child->visible(hide);
        hideChilds(child.get(), hide);
    }
}