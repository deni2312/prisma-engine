#pragma once
#include "../SceneObjects/Node.h"
#include <memory>
#include <string>
#include <functional>

namespace Prisma {
	class NodeHelper {
	public:
		std::shared_ptr<Prisma::Node> find(std::shared_ptr<Prisma::Node> root,const std::string& name);
        void nodeIterator(std::shared_ptr<Prisma::Node> root,std::function<void(std::shared_ptr<Prisma::Node>,std::shared_ptr<Prisma::Node>)> current);
	private:
		std::shared_ptr<Prisma::Node> findRecursive(const std::shared_ptr<Node>& currentNode, const std::string& nodeName);
	};
}