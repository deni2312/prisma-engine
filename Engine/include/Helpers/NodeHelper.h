#pragma once
#include "../SceneObjects/Node.h"
#include <memory>
#include <string>
#include <functional>

namespace Prisma
{
	class NodeHelper
	{
	public:
		template <typename T>
		int findUUID(std::vector<std::shared_ptr<T>> data, uint64_t uuid)
		{
			int index = -1;
			for (int i = 0; i < data.size(); i++)
			{
				if (data[i]->uuid() == uuid)
				{
					index = i;
					break;
				}
			}
			return index;
		}

		std::shared_ptr<Node> find(std::shared_ptr<Node> root, const std::string& name);
		void nodeIterator(std::shared_ptr<Node> root,
		                  std::function<void(std::shared_ptr<Node>, std::shared_ptr<Node>)> current);

	private:
		std::shared_ptr<Node> findRecursive(const std::shared_ptr<Node>& currentNode, const std::string& nodeName);
	};
}
