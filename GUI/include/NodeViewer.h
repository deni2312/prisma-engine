#pragma once
#include "../../Engine/include/GlobalData/InstanceData.h"
#include "../../Engine/include/Components/Component.h"
#include <imgui.h>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Containers/Texture.h"
#include <ImGuizmo.h>
#include <iostream>
#include "../../Engine/include/Helpers/NodeHelper.h"

namespace Prisma {
	class NodeViewer : public Prisma::InstanceData<NodeViewer>{
	public:
        struct NodeData {
            Prisma::Node* node;
            std::shared_ptr<Prisma::Camera> camera;
            glm::mat4 projection;
            float translate;
            float width;
            float height;
            float scale;
            float initOffset;
        };

        void varsDispatcher(Prisma::Component::ComponentType types);

        NodeViewer();

        void showComponents(Prisma::Node* nodeData);

        void showSelected(const NodeData& nodeData,bool end=true);
        // Getters for textures
        const std::shared_ptr<Prisma::Texture>& rotateTexture() const;

        const std::shared_ptr<Prisma::Texture>& translateTexture() const;

        const std::shared_ptr<Prisma::Texture>& scaleTexture() const;

        const std::shared_ptr<Prisma::Texture>& eyeOpenTexture() const;

        const std::shared_ptr<Prisma::Texture>& eyeCloseTexture() const;

        void drawGizmo(const NodeData& nodeData);

    private:
        glm::vec3 m_scale;
        glm::vec3 m_rotation;
        glm::vec3 m_translation;

        glm::mat4 m_currentModel;
        void hideChilds(Prisma::Node* root,bool hide);
        std::shared_ptr<Prisma::Texture> m_rotateTexture;
        std::shared_ptr<Prisma::Texture> m_translateTexture;
        std::shared_ptr<Prisma::Texture> m_scaleTexture;
        std::shared_ptr<Prisma::Texture> m_eyeOpen;
        std::shared_ptr<Prisma::Texture> m_eyeClose;

        ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
    };
}