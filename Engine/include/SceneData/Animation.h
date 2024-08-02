#pragma once
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../SceneObjects/AnimatedMesh.h"
#include "../GlobalData/Defines.h"
#include "Bone.h"

namespace Prisma {
	class AnimatedMesh;
}

namespace Prisma {

	struct BoneInfo;

	struct AssimpNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};

	class Animation {
	public:
		Animation();

		Animation(const std::string& animationPath, std::shared_ptr<Prisma::AnimatedMesh> model);

		~Animation();

		std::shared_ptr<Bone> FindBone(const std::string& name);

		int ticksPerSecond();
		float duration();
		const AssimpNodeData& rootNode();
		std::shared_ptr<std::map<std::string, Prisma::BoneInfo>> boneIdMap();
		std::string name() const;

		unsigned int id();

	private:
		std::string m_animationPath;
		void ReadMissingBones(const aiAnimation* animation, std::shared_ptr<Prisma::AnimatedMesh> model);
		glm::mat4 m_inverseTransform;
		void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
		float m_Duration;
		int m_TicksPerSecond;
		std::map<const std::string, std::shared_ptr<Bone>> m_Bones;
		AssimpNodeData m_RootNode;
		std::shared_ptr<std::map<std::string, Prisma::BoneInfo>> m_BoneInfoMap;
		unsigned int m_id;
	};

}