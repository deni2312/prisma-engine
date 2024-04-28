#pragma once
#include <vector>
#include <memory>
#include "Mesh.h"
#include <map>
#include "../GlobalData/Defines.h"
#include "../SceneData/Animator.h"

namespace Prisma {

    struct BoneInfo
    {
        int id;
        glm::mat4 offset;
    };

    class Animator;

    class AnimatedMesh : public Prisma::Mesh {
    public:

        struct AnimateVertex{
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texCoords;
            glm::vec3 tangent;
            glm::vec3 bitangent;
            int m_BoneIDs[MAX_BONE_INFLUENCE];
            //weights from each bone
            float m_Weights[MAX_BONE_INFLUENCE];
        };

        struct AnimateVerticesData{
            std::vector<AnimateVertex> vertices;
            std::vector<unsigned int> indices;
        };

        void loadAnimateModel(std::shared_ptr<AnimateVerticesData> vertices);

        std::shared_ptr<AnimateVerticesData> animateVerticesData();

        std::map<std::string, Prisma::BoneInfo>& boneInfoMap();
        int& boneInfoCounter();

        void animator(std::shared_ptr<Animator> animator);

        std::shared_ptr<Animator> animator();

    private:
        std::map<std::string, Prisma::BoneInfo> m_BoneInfoMap;
        std::shared_ptr<AnimateVerticesData> m_animateVertices;
        std::shared_ptr<Animator> m_animator;

        int m_BoneCounter = 0;
    };
}
