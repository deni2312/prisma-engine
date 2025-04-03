#include "Handlers/LightHandler.h"
#include "GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include "GlobalData/CacheScene.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"


Prisma::LightHandler::LightHandler()
{
	/*m_dirLights = std::make_shared<SSBO>(2);
	m_dirLights->resize(MAX_DIR_LIGHTS * sizeof(LightType::LightDir) + sizeof(glm::vec4));

	m_omniLights = std::make_shared<SSBO>(3);
	m_omniLights->resize(MAX_OMNI_LIGHTS * sizeof(LightType::LightOmni) + sizeof(glm::vec4));

	m_dirCSM = std::make_shared<SSBO>(9);
	m_dirCSM->resize(16 * sizeof(float) + sizeof(glm::vec4));

	m_areaLights = std::make_shared<SSBO>(31);
	m_areaLights->resize(MAX_AREA_LIGHTS * sizeof(LightType::LightArea) + sizeof(glm::vec4));
	*/

	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	Diligent::BufferDesc OmniDesc;
	OmniDesc.Name = "Omni Light Buffer";
	OmniDesc.Usage = Diligent::USAGE_DEFAULT;
	OmniDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	OmniDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	OmniDesc.ElementByteStride = sizeof(LightType::LightOmni);
	OmniDesc.Size = Define::MAX_OMNI_LIGHTS * sizeof(LightType::LightOmni);
	contextData.m_pDevice->CreateBuffer(OmniDesc, nullptr, &m_omniLights);

	Diligent::BufferDesc DirDesc;
	DirDesc.Name = "Dir Light Buffer";
	DirDesc.Usage = Diligent::USAGE_DEFAULT;
	DirDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
	DirDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	DirDesc.ElementByteStride = sizeof(LightType::LightDir);
	DirDesc.Size = Define::MAX_DIR_LIGHTS * sizeof(LightType::LightDir);
	contextData.m_pDevice->CreateBuffer(DirDesc, nullptr, &m_dirLights);

	Diligent::BufferDesc LightSizeDesc;
	LightSizeDesc.Name = "Light sizes";
	LightSizeDesc.Usage = Diligent::USAGE_DEFAULT;
	LightSizeDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	LightSizeDesc.Size = sizeof(LightSizes);
	contextData.m_pDevice->CreateBuffer(LightSizeDesc, nullptr, &m_lightSizes);
	m_clusterCalculation = std::make_shared<ClusterCalculation>(m_omniLights,m_lightSizes);

	m_init = true;
}

void Prisma::LightHandler::updateDirectional()
{
	const auto& scene = Prisma::GlobalData::getInstance().currentGlobalScene();
	int numVisible = 0;

	if (scene->dirLights.size() > 0)
	{
		auto shadow = Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights[0]->shadow();

		if (shadow && Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights[0]->hasShadow())
		{
			auto& levels = std::dynamic_pointer_cast<PipelineCSM>(shadow)->cascadeLevels();
			glm::vec4 length;
			length.x = levels.size();
			length.y = shadow->farPlane();

			//m_dirCSM->modifyData(0, 16 * sizeof(float), levels.data());
			//m_dirCSM->modifyData(16 * sizeof(float), sizeof(glm::vec4), value_ptr(length));
		}
	}

	m_dataDirectional = std::make_shared<SSBODataDirectional>();
	for (int i = 0; i < scene->dirLights.size(); i++)
	{
		if (scene->dirLights[i]->visible())
		{
			const auto& light = scene->dirLights[i];
			m_dataDirectional->lights.push_back(scene->dirLights[i]->type());
			const auto& dirMatrix = scene->dirLights[i]->finalMatrix();
			auto shadow = std::dynamic_pointer_cast<PipelineCSM>(light->shadow());
			const auto& dirMult = glm::normalize(dirMatrix * m_dataDirectional->lights[i].direction);
			m_dataDirectional->lights[i].diffuse = m_dataDirectional->lights[i].diffuse * light->intensity();
			m_dataDirectional->lights[i].direction = dirMult;
			m_dataDirectional->lights[i].padding.x = scene->dirLights[i]->hasShadow() ? 2.0f : 0.0f;
			m_dataDirectional->lights[i].padding.y = shadow->bias();
			numVisible++;
		}
	}

	m_sizes.dir = numVisible;

	if (!m_dataDirectional->lights.empty()) {
		auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
		contextData.m_pImmediateContext->UpdateBuffer(m_dirLights, 0, numVisible * sizeof(LightType::LightDir), m_dataDirectional->lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
}

void Prisma::LightHandler::updateArea()
{
	const auto& scene = Prisma::GlobalData::getInstance().currentGlobalScene();

	m_dataArea = std::make_shared<SSBODataArea>();
	int numVisible = 0;
	for (int i = 0; i < scene->areaLights.size(); i++)
	{
		const auto& light = scene->areaLights[i];
		if (light->visible())
		{
			m_dataArea->lights.push_back(light->type());
			glm::mat4 areaMatrix;
			if (light->parent())
			{
				areaMatrix = light->parent()->finalMatrix();
			}
			else
			{
				areaMatrix = light->matrix();
			}
			for (int j = 0; j < 4; j++) {
				m_dataArea->lights[i].position[j] = areaMatrix * m_dataArea->lights[i].position[j];
			}
			m_dataArea->lights[i].diffuse = m_dataArea->lights[i].diffuse * light->intensity();

			numVisible++;
		}
	}

	glm::ivec4 areaLength;
	areaLength.r = numVisible;
	//m_areaLights->modifyData(0, sizeof(glm::vec4),value_ptr(areaLength));
	//m_areaLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightArea),m_dataArea->lights.data());
}

void Prisma::LightHandler::updateOmni()
{
	const auto& scene = Prisma::GlobalData::getInstance().currentGlobalScene();
	m_dataOmni = std::make_shared<SSBODataOmni>();
	int numVisible = 0;
	for (int i = 0; i < scene->omniLights.size(); i++)
	{
		const auto& light = scene->omniLights[i];
		if (light->visible())
		{
			m_dataOmni->lights.push_back(light->type());
			glm::mat4 omniMatrix;
			if (light->parent())
			{
				omniMatrix = light->parent()->finalMatrix();
			}
			else
			{
				omniMatrix = light->matrix();
			}
			m_dataOmni->lights[i].position = omniMatrix * m_dataOmni->lights[i].position;
			if (light->shadow() && light->hasShadow())
			{
				light->shadow()->update(m_dataOmni->lights[i].position);
				m_dataOmni->lights[i].farPlane.x = light->shadow()->farPlane();
			}
			m_dataOmni->lights[i].hasShadow = light->hasShadow() ? 2.0f : 0.0f;
			m_dataOmni->lights[i].diffuse = m_dataOmni->lights[i].diffuse * light->intensity();
			numVisible++;
		}
	}

	m_sizes.omni = numVisible;

	if (!m_dataOmni->lights.empty()) {
		auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
		contextData.m_pImmediateContext->UpdateBuffer(m_omniLights,0, numVisible * sizeof(LightType::LightOmni), m_dataOmni->lights.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
	//m_omniLights->modifyData(0, sizeof(glm::vec4), value_ptr(omniLength));
	//m_omniLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightOmni),m_dataOmni->lights.data());
}

void Prisma::LightHandler::updateCSM()
{
	const auto& dirLights = Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights;

	if (dirLights.size() > 0 && dirLights[0]->shadow() && dirLights[0]->hasShadow())
	{
		auto shadow = dirLights[0]->shadow();

		const auto& dirMatrix = dirLights[0]->finalMatrix();

		const auto& dirMult = normalize(dirMatrix * dirLights[0]->type().direction);

		shadow->update(dirMult);
	}
}


void Prisma::LightHandler::updateSizes()
{
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
	contextData.m_pImmediateContext->UpdateBuffer(m_lightSizes, 0, sizeof(LightSizes), &m_sizes, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

bool Prisma::LightHandler::updateCascade()
{
	return m_updateCascade;
}

void Prisma::LightHandler::updateCascade(bool updateCascade)
{
	m_updateCascade;
}

void Prisma::LightHandler::update()
{
	const auto& scene = Prisma::GlobalData::getInstance().currentGlobalScene();
	if (m_updateCascade)
	{
		updateCSM();
	}
	for (int i = 0; i < scene->omniLights.size(); i++)
	{
		const auto& light = scene->omniLights[i];
		if (light->shadow() && light->hasShadow())
		{
			light->shadow()->update(m_dataOmni->lights[i].position);
		}
	}

	if (m_init || CacheScene::getInstance().updateData() || CacheScene::getInstance().updateSizes() ||
		CacheScene::getInstance().updateLights() || CacheScene::getInstance().updateStatus())
	{
		if (scene->dirLights.size() < Define::MAX_DIR_LIGHTS && scene->omniLights.size() < Define::MAX_OMNI_LIGHTS && scene->areaLights.size() < Define::MAX_AREA_LIGHTS)
		{
			updateDirectional();
			updateOmni();
			updateArea();
			updateSizes();
		}
		else
		{
			std::cerr << "Too many lights" << std::endl;
		}
	}

	m_clusterCalculation->updateCamera();
	m_clusterCalculation->updateLights();
	CacheScene::getInstance().resetCaches();

	m_init = false;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::lightSizes() const
{
	return m_lightSizes;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::dirLights() const
{
	return m_dirLights;
}

std::shared_ptr<Prisma::LightHandler::SSBODataArea> Prisma::LightHandler::dataArea() const
{
	return m_dataArea;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::omniLights()
{
	return m_omniLights;
}

Prisma::LightHandler::ClusterData Prisma::LightHandler::clusters()
{
	return { m_clusterCalculation->clusters(),m_clusterCalculation->clusterData() };
}

