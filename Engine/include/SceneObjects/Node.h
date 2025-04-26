#pragma once
#include <string>
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <map>
#include "../Components/Component.h"

namespace Prisma {
class Component;

class Node : public std::enable_shared_from_this<Node> {
public:
        Node();
        void name(const std::string& name);
        std::string name() const;
        const std::vector<std::shared_ptr<Node>>& children() const;
        void addChild(std::shared_ptr<Node> child, bool updateScene = true);
        void removeChild(uint64_t uuid, bool removeRecursive = true);
        virtual void matrix(const glm::mat4& matrix, bool updateChildren = true);
        virtual glm::mat4 matrix() const;
        virtual void finalMatrix(const glm::mat4& matrix, bool update = true);
        virtual glm::mat4 finalMatrix() const;
        void parent(std::shared_ptr<Node> parent, bool update = false);
        std::shared_ptr<Node> parent() const;
        virtual void istantiate(std::shared_ptr<Node> node);
        uint64_t uuid();
        void visible(bool visible);
        bool visible();


        void addComponent(std::shared_ptr<Component> component);
        void removeComponent(const std::string& name);

        std::map<std::string, std::shared_ptr<Component>> components();

        bool loadingComponent();

        void loadingComponent(bool loadingComponent);

        void loadComponents();

        void addGlobalList(bool globalList);
        bool addGlobalList() const;

        const char* strUUID();

        ~Node();

private:
        void updateCaches(std::shared_ptr<Node> child);
        void updateParent(std::shared_ptr<Node> parent);
        void dispatch(std::shared_ptr<Node> child);
        std::map<std::string, std::shared_ptr<Component>> m_components;

        uint64_t m_uuid;
        std::string m_name;
        std::vector<std::shared_ptr<Node>> m_children;
        glm::mat4 m_matrix = glm::mat4(1.0f);
        glm::mat4 m_finalMatrix = glm::mat4(1.0f);
        std::weak_ptr<Node> m_parent;
        void updateChild(Node* node);
        bool m_visible = true;
        bool m_loadingComponent = true;
        bool m_addGlobal = true;
        const char* m_strUUID;
        std::string m_strrUUID;
};
}