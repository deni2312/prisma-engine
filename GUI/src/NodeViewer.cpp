#include "../include/NodeViewer.h"
#include "Components/Component.h"
#include "Components/RegisterComponent.h"
#include "GlobalData/GlobalData.h"
#include "../include/ImGuiDebug.h"
#include "../include/ImGuiStyle.h"
#include "glm/gtx/string_cast.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "../imguizmo/imguizmo.h"


struct PrivateImguizmo {
    ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE currentGizmoMode = ImGuizmo::LOCAL;
};

static std::unique_ptr<PrivateImguizmo> privateImguizmo;


void Prisma::GUI::NodeViewer::varsDispatcher(Component::Options types, int index, unsigned int componentIndex) {
    auto type = std::get<0>(types.type);
    auto name = std::get<1>(types.type);
    auto variable = std::get<2>(types.type);
    if (variable) {
        std::string label = "##dispatcherlabel" + std::to_string(index) + name;
        switch (type) {
            case Component::TYPES::BUTTON: {
                if (ImGui::Button(name.c_str())) {
                    (*static_cast<std::function<void()>*>(variable))();
                }
            }
            break;
            case Component::TYPES::VEC3: {
                ImGui::Text(name.c_str());
                ImGui::InputFloat3(label.c_str(), value_ptr(*static_cast<glm::vec3*>(variable)));
            }
            break;
            case Component::TYPES::VEC2: {
                ImGui::Text(name.c_str());
                ImGui::InputFloat2(label.c_str(), value_ptr(*static_cast<glm::vec2*>(variable)));
            }
            break;
            case Component::TYPES::INT: {
                ImGui::Text(name.c_str());
                ImGui::InputInt(name.c_str(), static_cast<int*>(variable));
            }
            break;
            case Component::TYPES::FLOAT: {
                ImGui::Text(name.c_str());
                ImGui::InputFloat(label.c_str(), static_cast<float*>(variable));
            }
            break;
            case Component::TYPES::STRING: {
                ImGui::Text("%s", static_cast<std::string*>(variable)->c_str());
            }
            break;
            case Component::TYPES::BOOL: {
                ImGui::Checkbox(name.c_str(), static_cast<bool*>(variable));
            }
            break;
            case Component::TYPES::STRINGLIST: {
                ImGui::Text(name.c_str());
                auto comboData = static_cast<Component::ComponentList*>(variable);
                ImGui::Combo(label.c_str(), &comboData->currentitem, comboData->items.data(),
                             comboData->items.size());
            }
            break;
            case Component::TYPES::COLOR: {
                ImGui::Text(name.c_str());
                ImGui::ColorPicker3(label.c_str(), value_ptr(*static_cast<glm::vec3*>(variable)));
            }
            break;
            case Component::TYPES::TEXTURE: {
                auto texture = static_cast<Texture*>(variable);
                ImGui::Image(texture->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
                             ImVec2(types.size.x * ImguiDebug::getInstance().globalSize().x,
                                    types.size.y * ImguiDebug::getInstance().globalSize().y));
            }
            break;
            case Component::TYPES::TEXTURE_BUTTON: {
                ImGui::PushID(componentIndex);
                auto imageButton = static_cast<Component::ImageButton*>(variable);
                if (ImGui::ImageButton(
                    imageButton->texture.texture()->
                                 GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
                    ImVec2(types.size.x * ImguiDebug::getInstance().globalSize().x,
                           types.size.y * ImguiDebug::getInstance().globalSize().y))) {
                    (*static_cast<std::function<void()>*>(variable))();
                }
                ImGui::PopID();
            }
            break;
        }
    }
    if (types.sameline) {
        ImGui::SameLine();
    }
}

Prisma::GUI::NodeViewer::NodeViewer() {
    m_rotateTexture = std::make_shared<Texture>();
    m_rotateTexture->loadTexture({"../../../GUI/icons/rotate.png", false});

    m_translateTexture = std::make_shared<Texture>();
    m_translateTexture->loadTexture({"../../../GUI/icons/move.png", false});

    m_scaleTexture = std::make_shared<Texture>();
    m_scaleTexture->loadTexture({"../../../GUI/icons/scale.png", false});

    m_eyeOpen = std::make_shared<Texture>();
    m_eyeOpen->loadTexture({"../../../GUI/icons/eyeopen.png", false});

    m_eyeClose = std::make_shared<Texture>();
    m_eyeClose->loadTexture({"../../../GUI/icons/eyeclose.png", false});

    privateImguizmo = std::make_unique<PrivateImguizmo>();
}

void Prisma::GUI::NodeViewer::showComponents(std::shared_ptr<Node> nodeData) {
    auto components = nodeData->components();
    int i = 0;
    std::string indexRemove = "";
    bool visible = true;

    for (const auto& component : components) {
        ImGui::Separator();
        if (component.second->isUi()) {
            auto fields = component.second->globalVars();
            auto dispatch = [&]() {
                ImGuiStyles::getInstance().clearTreeStyle();
                for (int j = 0; j < fields.size(); j++) {
                    getInstance().varsDispatcher(fields[j], i, j);
                }
            };

            ImGuiStyles::getInstance().treeStyle();

            if (component.second->uiRemovable()) {
                if (ImGui::CollapsingHeader((component.second->name()).c_str(), &visible)) {
                    dispatch();
                } else {
                    ImGuiStyles::getInstance().clearTreeStyle();
                }
                if (!visible && indexRemove.empty()) {
                    indexRemove = component.first;
                }
            } else {
                if (ImGui::CollapsingHeader((component.second->name()).c_str())) {
                    dispatch();
                } else {
                    ImGuiStyles::getInstance().clearTreeStyle();
                }
            }
        }
        std::string nameRemove = "Remove Component##" + std::to_string(i);
        i++;
    }
    if (!indexRemove.empty()) {
        nodeData->removeComponent(indexRemove);
    }
}

// Getters for textures

void Prisma::GUI::NodeViewer::showSelected(const NodeData& nodeData, bool end, bool showData,
                                           std::shared_ptr<Node> componentAdding) {
    if (nodeData.node) {
        if (m_current != nodeData.node) {
            m_current = nodeData.node;
            glm::mat4 model = m_current->finalMatrix();
            decomposeTransform(model, m_translation, m_rotation, m_scale);
            ImGuizmo::DecomposeMatrixToComponents(value_ptr(model), value_ptr(m_translation),
                                                  value_ptr(m_rotation), value_ptr(m_scale));
        }

        float windowWidth = nodeData.translate * nodeData.width / 2.0f;
        auto nextRight = [&](float pos) {
            ImGui::SetNextWindowPos(ImVec2(windowWidth + nodeData.scale * nodeData.width, pos));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, nodeData.height * nodeData.scale + 44 - pos));
        };
        nextRight(nodeData.initOffset);
        ImGui::Begin(m_current->name().c_str(), nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::ImageButton(
            m_rotateTexture->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
            ImVec2(24, 24))) {
            privateImguizmo->currentGizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton(
            m_translateTexture->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
            ImVec2(24, 24))) {
            privateImguizmo->currentGizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton(
            m_scaleTexture->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
            ImVec2(24, 24))) {
            privateImguizmo->currentGizmoOperation = ImGuizmo::SCALE;
        }
        ImGui::SameLine();

        auto textureId = m_eyeOpen->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

        if (m_current->visible()) {
            textureId = m_eyeOpen->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        } else {
            textureId = m_eyeClose->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        }

        if (ImGui::ImageButton(textureId, ImVec2(24, 24))) {
            m_current->visible(!m_current->visible());
            hideChilds(m_current, m_current->visible());
        }

        if (ImGui::InputFloat3("Translation", value_ptr(m_translation), "%.3f")) {
            recompose(nodeData);
        }

        if (ImGui::InputFloat3("Rotation", value_ptr(m_rotation), "%.3f")) {
            recompose(nodeData);
        }

        if (ImGui::InputFloat3("Scale", value_ptr(m_scale), "%.3f")) {
            recompose(nodeData);
        }

        drawGizmo(nodeData);

        ImGui::Dummy(ImVec2(0.0f, 10.0f));

        std::vector<std::string> components;
        for (auto component : Factory::getRegistry()) {
            components.push_back(component.first);
        }

        if (components.size() > 0) {
            if (ImGui::BeginCombo("##ComponentsList", components[m_componentSelect].c_str())) {
                for (int i = 0; i < components.size(); i++) {
                    bool isSelected = (m_componentSelect == i);
                    if (ImGui::Selectable(components[i].c_str(), isSelected)) {
                        // Update the selected index when the item is clicked
                        m_componentSelect = i;
                    }
                    // Set the initial focus when opening the combo box
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }
        }

        if (ImGui::Button("Add component")) {
            if (componentAdding) {
                componentAdding->addComponent(Factory::createInstance(components[m_componentSelect]));
            } else {
                m_current->addComponent(Factory::createInstance(components[m_componentSelect]));
            }
        }
        if (showData) {
            showComponents(m_current);
        }
        if (end) {
            ImGui::End();
        }
    }
}

const std::shared_ptr<Prisma::Texture>& Prisma::GUI::NodeViewer::rotateTexture() const {
    return m_rotateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::GUI::NodeViewer::translateTexture() const {
    return m_translateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::GUI::NodeViewer::scaleTexture() const {
    return m_scaleTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::GUI::NodeViewer::eyeOpenTexture() const {
    return m_eyeOpen;
}

const std::shared_ptr<Prisma::Texture>& Prisma::GUI::NodeViewer::eyeCloseTexture() const {
    return m_eyeClose;
}

void Prisma::GUI::NodeViewer::drawGizmo(const NodeData& nodeData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    if (m_current) {
        glm::mat4 model = m_current->finalMatrix();
        auto inverseParent = glm::mat4(1.0f);
        if (m_current->parent()) {
            inverseParent = inverse(m_current->parent()->finalMatrix());
        }

        Manipulate(value_ptr(nodeData.camera->matrix()), value_ptr(nodeData.projection),
                   privateImguizmo->currentGizmoOperation, privateImguizmo->currentGizmoMode, value_ptr(model));
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ImGuizmo::DecomposeMatrixToComponents(value_ptr(model), value_ptr(m_translation),
                                                  value_ptr(m_rotation), value_ptr(m_scale));
        }

        m_current->matrix(inverseParent * model);
    }
}

void Prisma::GUI::NodeViewer::hideChilds(std::shared_ptr<Node> root, bool hide) {
    for (auto child : root->children()) {
        child->visible(hide);
        hideChilds(child, hide);
    }
}

void Prisma::GUI::NodeViewer::recompose(const NodeData& nodeData) {
    glm::mat4 model = m_current->finalMatrix();
    auto inverseParent = glm::mat4(1.0f);
    if (m_current->parent()) {
        inverseParent = inverse(m_current->parent()->finalMatrix());
    }

    model = recomposeTransform(m_translation, m_rotation, m_scale);

    ImGuizmo::RecomposeMatrixFromComponents(value_ptr(m_translation), value_ptr(m_rotation), value_ptr(m_scale),
                                            value_ptr(model));

    m_current->matrix(inverseParent * model);
}