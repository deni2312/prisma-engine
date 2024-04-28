#pragma once
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "../SceneObjects/AnimatedMesh.h"
#include "Bone.h"

namespace Prisma {

	struct AssimpNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};


	class Animation {
		Animation() = default;

		Animation(const std::string& animationPath, std::shared_ptr<Prisma::AnimatedMesh> model)
		{
			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
			assert(scene && scene->mRootNode);
			auto animation = scene->mAnimations[0];
			m_Duration = animation->mDuration;
			m_TicksPerSecond = animation->mTicksPerSecond;
			aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
			globalTransformation = globalTransformation.Inverse();
			ReadHierarchyData(m_RootNode, scene->mRootNode);
			ReadMissingBones(animation, model);
		}

		~Animation()
		{
		}

		Bone* FindBone(const std::string& name)
		{
			auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
				[&](const Bone& Bone)
				{
					return Bone.GetBoneName() == name;
				}
			);
			if (iter == m_Bones.end()) return nullptr;
			else return &(*iter);
		}


		inline float GetTicksPerSecond() { return m_TicksPerSecond; }
		inline float GetDuration() { return m_Duration; }
		inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
		inline const std::map<std::string, Prisma::AnimatedMesh::BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}

	private:
		void ReadMissingBones(const aiAnimation* animation, std::shared_ptr<Prisma::AnimatedMesh> model)
		{
			int size = animation->mNumChannels;

			auto& boneInfoMap = model->boneInfoMap();//getting m_BoneInfoMap from Model class
			int& boneCount = model->boneInfoCounter(); //getting the m_BoneCounter from Model class

			//reading channels(bones engaged in an animation and their keyframes)
			for (int i = 0; i < size; i++)
			{
				auto channel = animation->mChannels[i];
				std::string boneName = channel->mNodeName.data;

				if (boneInfoMap.find(boneName) == boneInfoMap.end())
				{
					boneInfoMap[boneName].id = boneCount;
					boneCount++;
				}
				m_Bones.push_back(Bone(channel->mNodeName.data,
					boneInfoMap[channel->mNodeName.data].id, channel));
			}

			m_BoneInfoMap = boneInfoMap;
		}

		void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
		{
			assert(src);

			dest.name = src->mName.data;
			dest.transformation = getTransform(src->mTransformation);
			dest.childrenCount = src->mNumChildren;

			for (int i = 0; i < src->mNumChildren; i++)
			{
				AssimpNodeData newData;
				ReadHierarchyData(newData, src->mChildren[i]);
				dest.children.push_back(newData);
			}
		}
		float m_Duration;
		int m_TicksPerSecond;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::map<std::string, Prisma::AnimatedMesh::BoneInfo> m_BoneInfoMap;
	};

}