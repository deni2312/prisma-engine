#pragma once

#include <memory>
#include <tuple>
#include "../SceneObjects/Node.h"
#include "../Containers/FBO.h"
#include <nlohmann/json.hpp>

namespace Prisma
{
	class Node;

	class Component
	{
	public:
		struct ComponentList
		{
			std::vector<const char*> items;
			int currentitem = 0;
		};

		enum class TYPES
		{
			INT,
			FLOAT,
			STRING,
			BOOL,
			STRINGLIST,
			BUTTON,
			VEC2,
			VEC3
		};

		using ComponentType = std::tuple<TYPES, std::string, void*>;

		using ComponentTypeVector = std::vector<ComponentType>;

		virtual void ui();

		virtual void start();

		virtual void update();

		virtual void updateRender(std::shared_ptr<FBO> fbo = nullptr);

		virtual void updatePreRender(std::shared_ptr<FBO> fbo = nullptr);

		virtual void updatePostRender(std::shared_ptr<FBO> fbo = nullptr);

		virtual void onParent(Node* parent);

		virtual void destroy();

		void parent(Node* parent);

		Node* parent();

		void isStart(bool start);

		bool isStart() const;

		void isUi(bool ui);

		bool isUi() const;

		void addGlobal(ComponentType globalVar);

		ComponentTypeVector globalVars();

		void name(std::string name);

		std::string name();

		uint64_t uuid();

		Component();

		virtual nlohmann::json serialize();

		virtual void deserialize(nlohmann::json& data);

		virtual ~Component();

		void uiRemovable(bool uiRemovable);

		bool uiRemovable();

	private:
		Node* m_parent = nullptr;
		bool m_start = false;
		bool m_ui = false;
		ComponentTypeVector m_globalVars;
		std::string m_name;
		uint64_t m_uuid;
		bool m_uiRemovable = true;
	};
}
