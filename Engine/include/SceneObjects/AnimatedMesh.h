#pragma once
#include <vector>
#include <memory>
#include "Mesh.h"
#include <map>
#include "../GlobalData/Defines.h"

namespace Prisma {
    class AnimatedMesh : public Prisma::Mesh {
    public:
        struct BoneInfo
        {
            int id;
            glm::mat4 offset;
        };


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

        std::map<std::string, BoneInfo>& boneInfoMap();
        int& boneInfoCounter();
    private:
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        std::shared_ptr<AnimateVerticesData> m_animateVertices;
        int m_BoneCounter = 0;
    };
}