#include "../include/NodeViewer.h"
#include "../../Engine/include/Components/Component.h"
#include "../../Engine/include/Components/RegisterComponent.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../include/ImGuiDebug.h"
#include "../include/ImGuiStyle.h"

void Prisma::NodeViewer::varsDispatcher(Component::Options types, int index)
{

	auto type = std::get<0>(types.type);
	auto name = std::get<1>(types.type);
	auto variable = std::get<2>(types.type);
	if (variable)
	{
		std::string label = "##dispatcherlabel" + std::to_string(index) + name;
		switch (type)
		{
		case Component::TYPES::BUTTON:
		{
			if (ImGui::Button(name.c_str()))
			{
				(*static_cast<std::function<void()>*>(variable))();
			}
		}
		break;
		case Component::TYPES::VEC3:
		{
			ImGui::Text(name.c_str());
			ImGui::InputFloat3(label.c_str(), value_ptr(*static_cast<glm::vec3*>(variable)));
		}
		break;
		case Component::TYPES::VEC2:
		{
			ImGui::Text(name.c_str());
			ImGui::InputFloat2(label.c_str(), value_ptr(*static_cast<glm::vec2*>(variable)));
		}
		break;
		case Component::TYPES::INT:
		{
			ImGui::Text(name.c_str());
			ImGui::InputInt(name.c_str(), static_cast<int*>(variable));
		}
		break;
		case Component::TYPES::FLOAT:
		{
			ImGui::Text(name.c_str());
			ImGui::InputFloat(label.c_str(), static_cast<float*>(variable));
		}
		break;
		case Component::TYPES::STRING:
		{
			ImGui::Text("%s", static_cast<std::string*>(variable)->c_str());
		}
		break;
		case Component::TYPES::BOOL:
		{
			ImGui::Checkbox(name.c_str(), static_cast<bool*>(variable));
		}
		break;
		case Component::TYPES::STRINGLIST:
		{
			ImGui::Text(name.c_str());
			auto comboData = static_cast<Component::ComponentList*>(variable);
			ImGui::Combo(label.c_str(), &comboData->currentitem, comboData->items.data(), comboData->items.size());
		}
		break;
		case Component::TYPES::COLOR:
		{
			ImGui::Text(name.c_str());
			ImGui::ColorPicker3(label.c_str(), value_ptr(*static_cast<glm::vec3*>(variable)));
		}
		break;
		case Component::TYPES::TEXTURE:
		{
			auto id = static_cast<Prisma::Texture*>(variable)->rawId();
			ImGui::Image((void*)static_cast<intptr_t>(id), ImVec2(types.size.x*Prisma::ImguiDebug::getInstance().globalSize().x, types.size.y * Prisma::ImguiDebug::getInstance().globalSize().y));
		}
		break;
		case Component::TYPES::TEXTURE_BUTTON:
		{
			if (ImGui::ImageButton((void*)static_cast<intptr_t>(stoi(name)), ImVec2(types.size.x * Prisma::ImguiDebug::getInstance().globalSize().x, types.size.y * Prisma::ImguiDebug::getInstance().globalSize().y)))
			{
				(*static_cast<std::function<void()>*>(variable))();
			}
		}
		break;
		}
	}
	if (types.sameline)
	{
		ImGui::SameLine();
	}
}

Prisma::NodeViewer::NodeViewer()
{
	m_rotateTexture = std::make_shared<Texture>();
	m_rotateTexture->loadTexture({ "../../../GUI/icons/rotate.png", false, false, false });

	m_translateTexture = std::make_shared<Texture>();
	m_translateTexture->loadTexture({ "../../../GUI/icons/move.png", false, false, false });

	m_scaleTexture = std::make_shared<Texture>();
	m_scaleTexture->loadTexture({ "../../../GUI/icons/scale.png", false, false, false });

	m_eyeOpen = std::make_shared<Texture>();
	m_eyeOpen->loadTexture({ "../../../GUI/icons/eyeopen.png", false, false, false });

	m_eyeClose = std::make_shared<Texture>();
	m_eyeClose->loadTexture({ "../../../GUI/icons/eyeclose.png", false, false, false });
}

void Prisma::NodeViewer::showComponents(Node* nodeData)
{
	auto components = nodeData->components();
	int i = 0;
	std::string indexRemove = "";
	bool visible = true;


	for (const auto& component : components)
	{
		ImGui::Separator();
		if (component.second->isUi())
		{
			auto fields = component.second->globalVars();
			auto dispatch = [&]()
				{
					Prisma::ImGuiStyles::getInstance().clearTreeStyle();
					for (auto field : fields)
					{
						getInstance().varsDispatcher(field, i);
					}
				};

			Prisma::ImGuiStyles::getInstance().treeStyle();

			if (component.second->uiRemovable())
			{
				if (ImGui::CollapsingHeader((component.second->name()).c_str(), &visible))
				{
					dispatch();
				}else
				{
					Prisma::ImGuiStyles::getInstance().clearTreeStyle();
				}
				if (!visible)
				{
					indexRemove = component.first;
				}
			}else
			{
				if (ImGui::CollapsingHeader((component.second->name()).c_str()))
				{
					dispatch();
				}
				else
				{
					Prisma::ImGuiStyles::getInstance().clearTreeStyle();
				}
			}

		}
		std::string nameRemove = "Remove Component##" + std::to_string(i);
		i++;
	}
	if (!indexRemove.empty())
	{
		nodeData->removeComponent(indexRemove);
	}
}

// Getters for textures

void Prisma::NodeViewer::showSelected(const NodeData& nodeData, bool end, bool showData,
	Node* componentAdding)
{
	if (nodeData.node)
	{
		float windowWidth = nodeData.translate * nodeData.width / 2.0f;
		auto nextRight = [&](float pos)
			{
				ImGui::SetNextWindowPos(ImVec2(windowWidth + nodeData.scale * nodeData.width, pos));
				ImGui::SetNextWindowSize(ImVec2(windowWidth, nodeData.height * nodeData.scale + 44 - pos));
			};
		nextRight(nodeData.initOffset);
		ImGui::Begin(nodeData.node->name().c_str(), nullptr,
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::ImageButton((void*)m_rotateTexture->id(), ImVec2(24, 24)))
		{
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();

		if (ImGui::ImageButton((void*)m_translateTexture->id(), ImVec2(24, 24)))
		{
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();

		if (ImGui::ImageButton((void*)m_scaleTexture->id(), ImVec2(24, 24)))
		{
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::SameLine();

		auto textureId = 0;

		if (nodeData.node->visible())
		{
			textureId = m_eyeOpen->id();
		}
		else
		{
			textureId = m_eyeClose->id();
		}


		if (ImGui::ImageButton((void*)textureId, ImVec2(24, 24)))
		{
			nodeData.node->visible(!nodeData.node->visible());
			hideChilds(nodeData.node, nodeData.node->visible());
		}

		if (ImGui::InputFloat3("Translation", value_ptr(m_translation), "%.3f"))
		{
			recompose(nodeData);
		}

		if (ImGui::InputFloat3("Rotation", value_ptr(m_rotation), "%.3f"))
		{
			recompose(nodeData);
		}

		if (ImGui::InputFloat3("Scale", value_ptr(m_scale), "%.3f"))
		{
			recompose(nodeData);
		}

		drawGizmo(nodeData);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));


		std::vector<std::string> components;
		for (auto component : Prisma::Factory::getRegistry())
		{
			components.push_back(component.first);
		}

		if (components.size() > 0)
		{
			if (ImGui::BeginCombo("##ComponentsList", components[m_componentSelect].c_str()))
			{
				for (int i = 0; i < components.size(); i++)
				{
					bool isSelected = (m_componentSelect == i);
					if (ImGui::Selectable(components[i].c_str(), isSelected))
					{
						// Update the selected index when the item is clicked
						m_componentSelect = i;
					}
					// Set the initial focus when opening the combo box
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}

		std::string addComponent = "Add component";

		const char* label = addComponent.c_str();
		ImVec2 text_size = ImGui::CalcTextSize(label);
		ImVec2 widget_size = ImVec2(text_size.x + ImGui::GetStyle().FramePadding.x * 2,
			text_size.y + ImGui::GetStyle().FramePadding.y * 2);
		ImVec2 widget_pos = ImGui::GetCursorScreenPos();
		ImVec2 screen_size = ImGui::GetIO().DisplaySize;

		bool widget_out_of_screen =
			(widget_pos.x + widget_size.x < 0 || widget_pos.x > screen_size.x ||
				widget_pos.y + widget_size.y < 0 || widget_pos.y > screen_size.y);

		if (!widget_out_of_screen)
		{
			ImGui::SameLine();
		}
		if (ImGui::Button(addComponent.c_str()))
		{
			if (componentAdding)
			{
				componentAdding->addComponent(Prisma::Factory::createInstance(components[m_componentSelect]));
			}
			else
			{
				nodeData.node->addComponent(Prisma::Factory::createInstance(components[m_componentSelect]));
			}
		}
		if (showData)
		{
			showComponents(nodeData.node);
		}
		if (end)
		{
			ImGui::End();
		}
	}
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::rotateTexture() const
{
	return m_rotateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::translateTexture() const
{
	return m_translateTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::scaleTexture() const
{
	return m_scaleTexture;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::eyeOpenTexture() const
{
	return m_eyeOpen;
}

const std::shared_ptr<Prisma::Texture>& Prisma::NodeViewer::eyeCloseTexture() const
{
	return m_eyeClose;
}

void Prisma::NodeViewer::drawGizmo(const NodeData& nodeData)
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	if (nodeData.node) {
		glm::mat4 model = nodeData.node->finalMatrix();
		auto inverseParent = glm::mat4(1.0f);
		if (nodeData.node->parent())
		{
			inverseParent = inverse(nodeData.node->parent()->finalMatrix());
		}

		Manipulate(value_ptr(nodeData.camera->matrix()), value_ptr(nodeData.projection), mCurrentGizmoOperation,
			mCurrentGizmoMode, value_ptr(model));

		ImGuizmo::DecomposeMatrixToComponents(value_ptr(model), value_ptr(m_translation), value_ptr(m_rotation),
			value_ptr(m_scale));

		nodeData.node->matrix(inverseParent * model);
	}
}

void Prisma::NodeViewer::hideChilds(Node* root, bool hide)
{
	for (auto child : root->children())
	{
		child->visible(hide);
		hideChilds(child.get(), hide);
	}
}

void Prisma::NodeViewer::recompose(const NodeData& nodeData)
{
	glm::mat4 model = nodeData.node->finalMatrix();
	auto inverseParent = glm::mat4(1.0f);
	if (nodeData.node->parent())
	{
		inverseParent = inverse(nodeData.node->parent()->finalMatrix());
	}
	ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale), glm::value_ptr(model));

	nodeData.node->matrix(inverseParent * model);
}
