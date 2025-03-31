#pragma once
#include "Node.h"

#include "../GlobalData/GlobalData.h"
#include "../GlobalData/CacheScene.h"

#include "../Pipelines/GenericShadow.h"
#include "../Pipelines/PipelineCSM.h"
#include "../Pipelines/PipelineOmniShadow.h"

namespace Prisma
{
	namespace LightType
	{
		struct LightDir
		{
			glm::vec4 direction = glm::vec4(0,0.9,0,0);
			glm::vec4 diffuse = glm::vec4(1, 1, 1, 0);
			glm::vec4 specular = glm::vec4(1, 1, 1, 0);
			glm::vec4 padding;
		};

		struct LightOmni
		{
			glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec4 diffuse = glm::vec4(1.0f);
			glm::vec4 specular = glm::vec4(1.0f);
			glm::vec4 farPlane = glm::vec4(100.0f, 0.0f, 0.0f, 0.0f);
			glm::vec4 attenuation = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec2 padding;
			float hasShadow = 0;
			float radius = 1;
		};

		struct LightArea
		{
			glm::vec4 position[4] = {
				glm::vec4(1.0f, -1.0f, 0.0f, 1.0f),  // Bottom-right
				glm::vec4(1.0f,  1.0f, 0.0f, 1.0f),   // Top-right
				glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f),  // Top-left
				glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f)  // Bottom-left
			};
			glm::vec4 diffuse = glm::vec4(1, 1, 1, 0);
			uint64_t shadowMap = 0;
			int doubleSide = 1;
			float padding;
		};
	};

	template <typename T>
	class Light : public Node
	{
	public:
		Light()
		{
		}

		T type() const
		{
			return m_type;
		}

		void type(T p_type)
		{
			m_type = p_type;
			CacheScene::getInstance().updateLights(true);
		}

		~Light()
		{
		}

		void matrix(const glm::mat4& matrix, bool update) override
		{
			Node::matrix(matrix);
			CacheScene::getInstance().updateLights(true);
		}

		void finalMatrix(const glm::mat4& matrix, bool update) override
		{
			Node::finalMatrix(matrix);
			CacheScene::getInstance().updateLights(true);
		}

		glm::mat4 matrix() const override
		{
			return Node::matrix();
		}

		glm::mat4 finalMatrix() const override
		{
			return Node::finalMatrix();
		}

		std::shared_ptr<GenericShadow> shadow()
		{
			return m_shadow;
		}

		void createShadow(unsigned int width, unsigned int height,bool post=false)
		{
			if (std::is_same<LightType::LightOmni, T>())
			{
				m_shadow = std::make_shared<PipelineOmniShadow>(width, height,post);
			}

			if (std::is_same<LightType::LightDir, T>())
			{
				m_shadow = std::make_shared<PipelineCSM>(width, height,post);
			}
			CacheScene::getInstance().updateLights(true);
		}

		void hasShadow(bool hasShadow)
		{
			m_hasShadow = hasShadow;
			CacheScene::getInstance().updateLights(true);
		}

		bool hasShadow()
		{
			return m_hasShadow;
		}

		static std::shared_ptr<Light<T>> instantiate(std::shared_ptr<Light<T>> light)
		{
			std::shared_ptr<Light<T>> newInstance = nullptr;
			if (light)
			{
				newInstance = std::make_shared<Light<T>>();
				newInstance->hasShadow(light->hasShadow());
				newInstance->matrix(light->matrix());
				newInstance->name(light->name() + std::to_string(newInstance->uuid()));
				auto parent = std::make_shared<Node>();
				parent->name(light->parent()->name() + std::to_string(parent->uuid()));
				parent->matrix(light->parent()->matrix());
				parent->addChild(newInstance);
				parent->parent(light->parent()->parent());
				newInstance->parent(parent);
				Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
			}
		}

		void intensity(float intensity)
		{
			m_intensity = intensity;
			CacheScene::getInstance().updateLights(true);
		}

		float intensity()
		{
			return m_intensity;
		}

	private:
		T m_type;
		std::shared_ptr<GenericShadow> m_shadow = nullptr;
		bool m_hasShadow = false;
		float m_intensity=1;
	};
}
