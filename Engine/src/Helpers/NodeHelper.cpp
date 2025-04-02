#include "Helpers/NodeHelper.h"
#include "GlobalData/GlobalData.h"

std::shared_ptr<Prisma::Node> Prisma::NodeHelper::find(std::shared_ptr<Node> root, const std::string& name)
{
	return findRecursive(root, name);
}

std::shared_ptr<Prisma::Node> Prisma::NodeHelper::find(uint64_t uuid)
{
	return Prisma::GlobalData::getInstance().sceneNodes()[uuid];
}

std::shared_ptr<Prisma::Node> Prisma::NodeHelper::findRecursive(const std::shared_ptr<Node>& currentNode,
                                                                const std::string& nodeName)
{
	if (currentNode && currentNode->name() == nodeName)
	{
		return currentNode;
	}

	for (const auto& child : currentNode->children())
	{
		auto result = findRecursive(child, nodeName);
		if (result)
		{
			return result;
		}
	}

	return nullptr;
}

void Prisma::NodeHelper::nodeIterator(std::shared_ptr<Node> root,
                                      std::function<void(std::shared_ptr<Node>, std::shared_ptr<Node>)> current)
{
	for (const auto& child : root->children())
	{
		current(child, root);
		nodeIterator(child, current);
	}
}
