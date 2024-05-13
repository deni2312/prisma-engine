#pragma once
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../SceneObjects/AnimatedMesh.h"
#include "../GlobalData/Defines.h"
#include "Bone.h"
#include "../Containers/SSBO.h"

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
		Animation() = default;

		Animation(const std::string& animationPath, std::shared_ptr<Prisma::AnimatedMesh> model);

		~Animation();

		std::pair<std::shared_ptr<Bone>, int> FindBone(const std::string& name);

		int GetTicksPerSecond() { return m_TicksPerSecond; }
		float GetDuration() { return m_Duration; }
		const AssimpNodeData& GetRootNode() { return m_RootNode; }
		const std::map<std::string, Prisma::BoneInfo>& GetBoneIDMap() const;
		std::string name() const;

	private:
		std::string m_animationPath;
		void ReadMissingBones(const aiAnimation* animation, std::shared_ptr<Prisma::AnimatedMesh> model);
		glm::mat4 m_inverseTransform;
		void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
		float m_Duration;
		int m_TicksPerSecond;
		std::map<const std::string, std::pair<std::shared_ptr<Bone>,int>> m_Bones;
		AssimpNodeData m_RootNode;
		std::shared_ptr<Prisma::SSBO> m_ssbo;
		std::shared_ptr<Prisma::SSBO> m_ssboData;
		std::map<std::string, Prisma::BoneInfo> m_BoneInfoMap;
	};

}