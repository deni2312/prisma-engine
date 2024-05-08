#pragma once
#include "../SceneData/SceneLoader.h"
#include "../Containers/SSBO.h"
#include "../GlobalData/Defines.h"
#include "../Helpers/ClusterCalculation.h"
#include <vector>

namespace Prisma {
	class LightHandler {
	private:
		struct SSBODataDirectional {
			glm::vec4 size;
			std::vector<Prisma::LightType::LightDir> lights;
		};

		struct SSBODataOmni {
			glm::vec4 size;
			std::vector<Prisma::LightType::LightOmni> lights;
		};

		std::shared_ptr<Prisma::LightHandler::SSBODataDirectional> m_dataDirectional;
		std::shared_ptr<Prisma::LightHandler::SSBODataOmni> m_dataOmni;

		std::shared_ptr<Prisma::SSBO> m_omniLights;
		std::shared_ptr<Prisma::SSBO> m_dirLights;
		std::shared_ptr<Prisma::SSBO> m_dirCSM;


        static std::shared_ptr<LightHandler> instance;

		void updateDirectional();

		void updateOmni();

        bool m_init;
	public:
		static LightHandler& getInstance();

		

		LightHandler(const LightHandler&) = delete;
		LightHandler& operator=(const LightHandler&) = delete;
		void update();
		void bind();
		std::shared_ptr<Prisma::LightHandler::SSBODataDirectional> dataDirectional() const;
		std::shared_ptr<Prisma::SSBO> ssboDirectional() const;
		std::shared_ptr<Prisma::LightHandler::SSBODataOmni> dataOmni() const;
		std::shared_ptr<Prisma::SSBO> ssboOmni() const;

		std::shared_ptr<ClusterCalculation> m_clusterCalculation;

		static const unsigned int DIRECTIONAL_OFFSET = sizeof(glm::vec4);
		static const unsigned int OMNI_OFFSET = sizeof(glm::vec4);

        LightHandler();
    };
}