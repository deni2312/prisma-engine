#include "Handlers/AnimationHandler.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/Defines.h"
#include "Helpers/TimeCounter.h"
#include "SceneData/MeshIndirect.h"


void Prisma::AnimationHandler::fill()
{
	for (auto mesh : Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes)
	{
		if (mesh->animator())
		{
			mesh->animator()->updateAnimation(0);
		}
	}
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::AnimationHandler::animation()
{
	return m_animation;
}

Prisma::AnimationHandler::AnimationHandler()
{
	std::vector<SSBOAnimation> animations;
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
	Diligent::BufferDesc MatBufferDesc;

	MatBufferDesc.Name = "Mesh Transform Buffer";
	MatBufferDesc.Usage = Diligent::USAGE_DEFAULT;
	MatBufferDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	MatBufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	MatBufferDesc.ElementByteStride = sizeof(Prisma::Mesh::MeshData);
	auto size = sizeof(SSBOAnimation) * Define::MAX_ANIMATION_MESHES;
	MatBufferDesc.Size = size; // Ensure enough space
	contextData.m_pDevice->CreateBuffer(MatBufferDesc, nullptr, &m_animation);
	animations.resize(Define::MAX_ANIMATION_MESHES);
	//m_ssboAnimation = std::make_shared<SSBO>(8);
	//m_ssboAnimation->resize(sizeof(SSBOAnimation) * MAX_ANIMATION_MESHES);
	for (int i = 0; i < animations.size(); i++)
	{
		for (int j = 0; j < Define::MAX_BONES; j++)
		{
			animations[i].animations[j] = glm::mat4(1.0f);
		}
	}
}
