#pragma once
#include <vector>
#include <memory>
#include "Mesh.h"
#include <map>
#include "../GlobalData/Defines.h"

namespace Prisma {
    class AnimatedMesh : public Prisma::Node {
    public:
        struct BoneInfo
        {
            int id;
            glm::mat4 offset;
        };

        struct AnimateVertex {
            // position
            glm::vec3 Position;
            // normal
            glm::vec3 Normal;
            // texCoords
            glm::vec2 TexCoords;
            // tangent
            glm::vec3 Tangent;
            // bitangentW
            glm::vec3 Bitangent;
            //bone indexes which will influence this vertex
            int m_BoneIDs[MAX_BONE_INFLUENCE];
            //weights from each bone
            float m_Weights[MAX_BONE_INFLUENCE];
        };


        std::map<std::string, BoneInfo>& boneInfoMap();
        int& boneInfoCounter();
    private:
        std::map<std::string, BoneInfo> m_BoneInfoMap;
        int m_BoneCounter = 0;
    };
}
