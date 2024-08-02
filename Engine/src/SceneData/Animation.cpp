#include "../../include/SceneData/Animation.h"

static unsigned int uid = 0;

Prisma::Animation::Animation()
{
	this->m_id = uid;
	uid++;
}

Prisma::Animation::Animation(const std::string& animationPath, std::shared_ptr<Prisma::AnimatedMesh> model):m_animationPath{animationPath}
{
	Assimp::Importer importer;
	m_BoneInfoMap = std::make_shared<std::map<std::string, Prisma::BoneInfo>>();
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	m_Duration = animation->mDuration;
	m_TicksPerSecond = animation->mTicksPerSecond;
	m_inverseTransform = glm::inverse(model->parent()->finalMatrix());
	ReadHierarchyData(m_RootNode, scene->mRootNode);
	ReadMissingBones(animation, model);
	this->m_id = uid;
	uid++;
}

Prisma::Animation::~Animation()
{
}

std::shared_ptr<Prisma::Bone> Prisma::Animation::FindBone(const std::string& name)
{
	return m_Bones[name];
}

std::shared_ptr<std::map<std::string, Prisma::BoneInfo>> Prisma::Animation::GetBoneIDMap()
{
	return m_BoneInfoMap;
}

std::string Prisma::Animation::name() const {
	return m_animationPath;
}

unsigned int Prisma::Animation::id()
{
	return m_id;
}

void Prisma::Animation::ReadMissingBones(const aiAnimation* animation, std::shared_ptr<Prisma::AnimatedMesh> model)
{
	int size = animation->mNumChannels;

	std::map<std::string, Prisma::BoneInfo>& boneInfoMap = model->boneInfoMap();//getting m_BoneInfoMap from Model class
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
		const std::string name = channel->mNodeName.data;
		m_Bones[name]=std::make_shared<Bone>(channel->mNodeName.data,
			boneInfoMap[channel->mNodeName.data].id, channel);
	}

	*m_BoneInfoMap = boneInfoMap;
}

void Prisma::Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
	assert(src);

	dest.name = src->mName.data;
	dest.transformation = getTransform(src->mTransformation) * m_inverseTransform;
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		ReadHierarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}
