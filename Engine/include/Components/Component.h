#pragma once

#include <memory>
#include <tuple>
#include "../SceneObjects/Node.h"

namespace Prisma {

    class Component {
    public:

        enum class TYPES{
            INT,
            FLOAT,
            STRING,
            BOOL,
            STRINGLIST,
            BUTTON
        };

        using ComponentType = std::tuple<TYPES,std::string,void*>;

        using ComponentTypeVector = std::vector<ComponentType>;

        virtual void start() = 0;

        virtual void update()= 0;

        void parent(Prisma::Node* parent);

        Prisma::Node* parent();

        void isStart(bool start);

        bool isStart() const;

        void addGlobal(ComponentType globalVar);

        ComponentTypeVector globalVars();

        void name(std::string name);

        std::string name();



    private:

        Prisma::Node* m_parent = nullptr;
        bool m_start=false;
        ComponentTypeVector m_globalVars;
        std::string m_name;

    };

}

