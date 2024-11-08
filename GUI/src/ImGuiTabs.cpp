#include "../include/ImGuiTabs.h"
#include "imgui.h"
#include <iostream> // For std::cout

#include "imgui_internal.h"
#include "../../Engine/include/Helpers/NodeHelper.h"

Prisma::ImGuiTabs::ImGuiTabs()
{
}

void Prisma::ImGuiTabs::showCurrentNodes(std::shared_ptr<Node> root, int depth, ImGuiCamera& camera)
{
	if (root)
	{
		// Iterate through children of the current node
		for (const auto& child : root->children())
		{
			m_index++;
			// Add spacing based on depth
			ImGui::Indent(depth * 20.0f); // Indent by 20 pixels per depth level
			// Create a button without a label
			std::string finalText = child->name() + "##" + std::to_string(m_index);
			if (ImGui::Selectable(finalText.c_str()))
			{
				camera.currentSelect(child.get());
			}

			ImGuiDragDropFlags src_flags = 0;
			src_flags |= ImGuiDragDropFlags_SourceNoDisableHover; // Keep the source displayed as hovered
			src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;
			// Because our dragging is local, we disable the feature of opening foreign treenodes/tabs while dragging
			//src_flags |= ImGuiDragDropFlags_SourceNoPreviewTooltip; // Hide the tooltip
			if (ImGui::BeginDragDropSource(src_flags))
			{
				ImGui::SetDragDropPayload("DND_DEMO_NAME", child.get(), sizeof(child));
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				ImGuiDragDropFlags target_flags = 0;
				target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
				// Don't wait until the delivery (release mouse button on a target) to do something
				target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_NAME", target_flags))
				{
					m_current = (Node*)payload->Data;
					m_parent = child;
				}
				ImGui::EndDragDropTarget();
			}

			// Remove the indent
			ImGui::Unindent(depth * 20.0f);
			// Recur for children with increased depth
			showCurrentNodes(child, depth + 1, camera);
		}
	}
}

void Prisma::ImGuiTabs::showNodes(std::shared_ptr<Node> root, int depth, ImGuiCamera& camera)
{
	m_current = nullptr;
	m_parent = nullptr;
	showCurrentNodes(root, depth, camera);

	if (m_current && m_parent && m_current->uuid() == m_parent->uuid())
	{
		m_current = nullptr;
		m_parent = nullptr;
	}
	if (m_current && m_parent)
	{
		std::cout << m_current->name() << " " << m_parent->name() << std::endl;
	}
	if (m_current && m_parent)
	{
		Prisma::NodeHelper nodeHelper;
		m_current->parent()->removeChild(m_current->uuid());
		m_parent->addChild(nodeHelper.find(root, m_current->name()));
		m_current = nullptr;
		m_parent = nullptr;
	}
	m_index = 0;
}
