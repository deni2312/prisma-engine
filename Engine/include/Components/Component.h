#pragma once

#include <memory>
#include <tuple>
#include "../SceneObjects/Node.h"
#include <nlohmann/json.hpp>

#include "../Containers/Texture.h"

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
			VEC3,
			COLOR,
			TEXTURE,
			TEXTURE_BUTTON
		};

		struct ImageButton
		{
			std::function<void()> handler;
			Prisma::Texture texture;
		};

		using ComponentType = std::tuple<TYPES, std::string, void*>;

		struct Options
		{
			ComponentType type;
			bool sameline = false;
			glm::vec2 size=glm::vec2(256,256);
		};

		using ComponentTypeVector = std::vector<Options>;

		virtual void ui();

		virtual void start();

		virtual void update();

		//virtual void updateRender(std::shared_ptr<FBO> fbo = nullptr);

		//virtual void updatePreRender(std::shared_ptr<FBO> fbo = nullptr);

		//virtual void updatePostRender(std::shared_ptr<FBO> fbo = nullptr);

		virtual void onParent(Node* parent);

		virtual void destroy();

		void parent(Node* parent);

		Node* parent();

		void isStart(bool start);

		bool isStart() const;

		void isUi(bool ui);

		bool isUi() const;

		void addGlobal(Options globalVar);

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
	protected:
		ComponentTypeVector m_globalVars;

	private:
		Node* m_parent = nullptr;
		bool m_start = false;
		bool m_ui = false;
		std::string m_name;
		uint64_t m_uuid;
		bool m_uiRemovable = true;
	};
}
