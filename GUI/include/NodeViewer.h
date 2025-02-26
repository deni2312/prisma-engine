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

namespace Prisma
{
	class NodeViewer : public InstanceData<NodeViewer>
	{
	public:
		struct NodeData
		{
			Node* node;
			std::shared_ptr<Camera> camera;
			glm::mat4 projection;
			float translate;
			float width;
			float height;
			float scale;
			float initOffset;
		};

		void varsDispatcher(Component::Options types, int index);

		NodeViewer();

		void showComponents(Node* nodeData);

		void showSelected(const NodeData& nodeData, bool end = true, bool showData = true,
		                  Node* componentAdding = nullptr);
		// Getters for textures
		const std::shared_ptr<Texture>& rotateTexture() const;

		const std::shared_ptr<Texture>& translateTexture() const;

		const std::shared_ptr<Texture>& scaleTexture() const;

		const std::shared_ptr<Texture>& eyeOpenTexture() const;

		const std::shared_ptr<Texture>& eyeCloseTexture() const;

		void drawGizmo(const NodeData& nodeData);

	private:
		glm::vec3 m_scale;
		glm::vec3 m_rotation;
		glm::vec3 m_translation;

		glm::mat4 m_currentModel;
		void hideChilds(Node* root, bool hide);
		std::shared_ptr<Texture> m_rotateTexture;
		std::shared_ptr<Texture> m_translateTexture;
		std::shared_ptr<Texture> m_scaleTexture;
		std::shared_ptr<Texture> m_eyeOpen;
		std::shared_ptr<Texture> m_eyeClose;
		int m_componentSelect = 0;
		std::vector<const char*> m_components;

		Prisma::Node* m_currentNode = nullptr;

		ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;

		Prisma::Node* m_current = nullptr;

		void recompose(const NodeData& nodeData);
	};
}
