#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "../Containers/Texture.h"
#include "../Components/MaterialComponent.h"
#include "../Physics/Physics.h"
#include "../Components/Component.h"
#include <map>

namespace Prisma {
    class Mesh : public Prisma::Node{
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoords;
            glm::vec3 tangent;
            glm::vec3 bitangent;
        };
        struct VerticesData {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
        };
        struct AABBData{
            glm::vec3 min;
            glm::vec3 max;
            glm::vec3 center;
        };
        struct BoneInfo
        {
            int id;
            glm::mat4 offset;
        };

        void loadModel(std::shared_ptr<VerticesData> vertices);
        const VerticesData& verticesData() const;
        void matrix(const glm::mat4& matrix, bool update = true) override;
        glm::mat4 matrix() const override;
        void finalMatrix(const glm::mat4& matrix, bool update = true) override;
        glm::mat4 finalMatrix() const override;

        unsigned int vao() const;
        static std::shared_ptr<Prisma::Mesh> instantiate(std::shared_ptr<Mesh> mesh);
        void material(std::shared_ptr<Prisma::MaterialComponent> material);
        std::shared_ptr<Prisma::MaterialComponent> material();
        void vectorId(unsigned int vectorId);
        virtual void computeAABB();
        AABBData aabbData();

        unsigned int vectorId();
    protected:
        std::shared_ptr<VerticesData> m_vertices;
        std::shared_ptr<Prisma::MaterialComponent> m_material;

        unsigned int m_vectorId;
        AABBData m_aabbData;
    };
}
