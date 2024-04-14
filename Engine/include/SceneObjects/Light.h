#pragma once
#include "Node.h"

#include "../GlobalData/GlobalData.h"

#include "../Pipelines/GenericShadow.h"
#include "../Pipelines/PipelineCSM.h"
#include "../Pipelines/PipelineOmniShadow.h"

namespace Prisma {
	namespace LightType {
		struct LightDir {
			glm::vec4 direction;
			glm::vec4 diffuse;
			glm::vec4 specular;
			uint64_t shadowMap = 0;
			glm::vec2 padding;
		};

		struct LightOmni {
			glm::vec4 position;
			glm::vec4 diffuse;
			glm::vec4 specular;
            glm::vec4 farPlane;
            uint64_t shadowMap = 0;
            float padding = 0;
			float radius = 1;
        };
	};

	template <typename T> class Light :public Prisma::Node {
	public:
		Light() {

		}
		T type() const
		{
			return m_type;
		}
		void type(T p_type) 
		{
			m_type = p_type;
			updateLights = true;
		}
		~Light() {

		}

        std::shared_ptr<GenericShadow> shadow(){
            return m_shadow;
        }

        void createShadow(unsigned int width, unsigned int height){
            if(std::is_same<LightType::LightOmni,T>()){
                m_shadow=std::make_shared<PipelineOmniShadow>(width,height);
            }

			if (std::is_same<LightType::LightDir, T>()) {
				m_shadow = std::make_shared<PipelineCSM>(width, height);
			}
			updateLights = true;
        }

		void hasShadow(bool hasShadow) {
			m_hasShadow = hasShadow;
			updateLights = true;
		}

		bool hasShadow() {
			return m_hasShadow;
		}

	private:
		T m_type;
        std::shared_ptr<GenericShadow> m_shadow;
		bool m_hasShadow = false;
	};
}