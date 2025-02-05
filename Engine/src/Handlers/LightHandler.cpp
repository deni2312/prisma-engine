#include "../../include/Handlers/LightHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include "../../include/GlobalData/CacheScene.h"


Prisma::LightHandler::LightHandler()
{
	m_dirLights = std::make_shared<SSBO>(2);
	m_dirLights->resize(MAX_DIR_LIGHTS * sizeof(LightType::LightDir) + sizeof(glm::vec4));

	m_omniLights = std::make_shared<SSBO>(3);
	m_omniLights->resize(MAX_OMNI_LIGHTS * sizeof(LightType::LightOmni) + sizeof(glm::vec4));

	m_dirCSM = std::make_shared<SSBO>(9);
	m_dirCSM->resize(16 * sizeof(float) + sizeof(glm::vec4));

	m_areaLights = std::make_shared<SSBO>(31);
	m_areaLights->resize(MAX_AREA_LIGHTS * sizeof(LightType::LightArea) + sizeof(glm::vec4));

	glm::vec3 size = ClusterCalculation::grids();

	m_clusterCalculation = std::make_shared<ClusterCalculation>(size.x * size.y * size.z);

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

			m_dirCSM->modifyData(0, 16 * sizeof(float), levels.data());
			m_dirCSM->modifyData(16 * sizeof(float), sizeof(glm::vec4), value_ptr(length));
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
			m_dataDirectional->lights[i].direction = dirMult;
			m_dataDirectional->lights[i].padding.x = scene->dirLights[i]->hasShadow() ? 2.0f : 0.0f;
			m_dataDirectional->lights[i].padding.y = shadow->bias();
			numVisible++;
		}
	}
	glm::ivec4 dirLength;
	dirLength.r = numVisible;
	m_dirLights->modifyData(0, sizeof(glm::vec4),
	                        value_ptr(dirLength));
	m_dirLights->modifyData(sizeof(glm::vec4), scene->dirLights.size() * sizeof(LightType::LightDir),
	                        m_dataDirectional->lights.data());
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
			m_dataArea->lights[i].position[0] = areaMatrix * m_dataArea->lights[i].position[3];
			m_dataArea->lights[i].position[1] = areaMatrix * m_dataArea->lights[i].position[2];
			m_dataArea->lights[i].position[2] = areaMatrix * m_dataArea->lights[i].position[1];
			m_dataArea->lights[i].position[3] = areaMatrix * m_dataArea->lights[i].position[0];

			numVisible++;
		}
	}

	glm::ivec4 areaLength;
	areaLength.r = numVisible;
	m_areaLights->modifyData(0, sizeof(glm::vec4),
		value_ptr(areaLength));
	m_areaLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightArea),
		m_dataArea->lights.data());
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
			m_dataOmni->lights[i].padding = light->hasShadow() ? 2.0f : 0.0f;
			numVisible++;
		}
	}

	glm::ivec4 omniLength;
	omniLength.r = numVisible;
	m_omniLights->modifyData(0, sizeof(glm::vec4),
	                         value_ptr(omniLength));

	m_omniLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightOmni),
	                         m_dataOmni->lights.data());
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
	if (m_init || CacheScene::getInstance().updateData() || CacheScene::getInstance().updateSizes() ||
		CacheScene::getInstance().updateLights() || CacheScene::getInstance().updateStatus())
	{
		if (scene->dirLights.size() < MAX_DIR_LIGHTS && scene->omniLights.size() < MAX_OMNI_LIGHTS && scene->areaLights.size() < MAX_AREA_LIGHTS)
		{
			updateDirectional();
			updateOmni();
			updateArea();
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

void Prisma::LightHandler::bind()
{
	m_dirLights->bind();
}

std::shared_ptr<Prisma::LightHandler::SSBODataDirectional> Prisma::LightHandler::dataDirectional() const
{
	return m_dataDirectional;
}

std::shared_ptr<Prisma::SSBO> Prisma::LightHandler::ssboDirectional() const
{
	return m_dirLights;
}

std::shared_ptr<Prisma::LightHandler::SSBODataOmni> Prisma::LightHandler::dataOmni() const
{
	return m_dataOmni;
}

std::shared_ptr<Prisma::SSBO> Prisma::LightHandler::ssboOmni() const
{
	return m_omniLights;
}

std::shared_ptr<Prisma::LightHandler::SSBODataArea> Prisma::LightHandler::dataArea() const
{
	return m_dataArea;
}

std::shared_ptr<Prisma::SSBO> Prisma::LightHandler::ssboArea() const
{
	return m_areaLights;
}
