#pragma once

#include <memory>
#include <tuple>
#include "../SceneObjects/Node.h"
#include "../Containers/FBO.h"

namespace Prisma {

    class Node;

    class Component {
    public:

        struct ComponentList {
            std::vector<const char*> items;
            int currentitem = 0;
        };

        enum class TYPES{
            INT,
            FLOAT,
            STRING,
            BOOL,
            STRINGLIST,
            BUTTON,
            VEC2,
            VEC3
        };

        using ComponentType = std::tuple<TYPES,std::string,void*>;

        using ComponentTypeVector = std::vector<ComponentType>;

        virtual void ui();

        virtual void start();

        virtual void update();

        virtual void updateRender(std::shared_ptr<Prisma::FBO> fbo = 0);

        virtual void updatePreRender(std::shared_ptr<Prisma::FBO> fbo = 0);

        virtual void destroy();

        void parent(Prisma::Node* parent);

        Prisma::Node* parent();

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

        virtual ~Component();

    private:

        Prisma::Node* m_parent = nullptr;
        bool m_start=false;
        bool m_ui = false;
        ComponentTypeVector m_globalVars;
        std::string m_name;
        uint64_t m_uuid;
    };

}

