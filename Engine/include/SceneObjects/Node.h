#pragma once
#include <string>
#include <vector>
#include <memory>
#include "glm/glm.hpp"

namespace Prisma {
	class Node {
	public:
		Node();
		void name(std::string name);
		std::string name() const;
		const std::vector<std::shared_ptr<Node>>& children() const;
		void addChild(std::shared_ptr<Node> child,bool updateScene=true);
		void removeChild(uint64_t uuid);
		virtual void matrix(glm::mat4 matrix,bool updateChildren=true);
		virtual glm::mat4 matrix() const;
		virtual void finalMatrix(glm::mat4 matrix,bool update=true);
		virtual glm::mat4 finalMatrix() const;
		void parent(std::shared_ptr<Node> parent);
		std::shared_ptr<Node> parent() const;
		virtual void istantiate(std::shared_ptr<Node> node);
		uint64_t uuid();
		~Node();
	private:
		uint64_t m_uuid;
		std::string m_name;
		std::vector<std::shared_ptr<Node>> m_children;
		glm::mat4 m_matrix=glm::mat4(1.0f);
		glm::mat4 m_finalMatrix = glm::mat4(1.0f);
		std::shared_ptr<Node> m_parent;
		void updateChild(Node* node);
	};
}