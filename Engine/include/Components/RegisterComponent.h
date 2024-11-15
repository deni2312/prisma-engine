#pragma once
#include <map>
#include <memory>
#include <functional>
#include <string>
#include "Component.h"

namespace Prisma
{
	class Factory
	{
	public:
		using Creator = std::function<std::shared_ptr<Prisma::Component>()>;

		// Method to register a class type with a name
		static void registerClass(const std::string& className, Creator creator)
		{
			getRegistry()[className] = creator;
		}

		// Method to create an instance of a class by name
		static std::shared_ptr<Prisma::Component> createInstance(const std::string& className)
		{
			auto it = getRegistry().find(className);
			if (it != getRegistry().end())
			{
				return it->second();
			}
			return nullptr; // Return null if class name not found
		}

		// Registry storing class name to creator function mappings
		static std::unordered_map<std::string, Creator>& getRegistry()
		{
			static std::unordered_map<std::string, Creator> registry;
			return registry;
		}
	};

	// Helper template to register classes
	template <typename T>
	class Registrar
	{
	public:
		explicit Registrar(const std::string& className)
		{
			Factory::registerClass(className, []() -> std::shared_ptr<Prisma::Component>
			{
				return std::make_shared<T>();
			});
		}
	};
}
