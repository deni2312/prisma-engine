#pragma once
#include "../SceneData/SceneLoader.h"
#include "../Containers/SSBO.h"
#include "../GlobalData/Defines.h"
#include "../Helpers/ClusterCalculation.h"
#include "../GlobalData/InstanceData.h"
#include <vector>

namespace Prisma
{
	class LightHandler : public InstanceData<LightHandler>
	{
		struct SSBODataDirectional
		{
			glm::ivec4 size;
			std::vector<LightType::LightDir> lights;
		};

		struct SSBODataOmni
		{
			glm::ivec4 size;
			std::vector<LightType::LightOmni> lights;
		};

		struct SSBODataArea
		{
			glm::ivec4 size;
			std::vector<LightType::LightArea> lights;
		};

		std::shared_ptr<SSBODataDirectional> m_dataDirectional;
		std::shared_ptr<SSBODataOmni> m_dataOmni;
		std::shared_ptr<SSBODataArea> m_dataArea;

		std::shared_ptr<SSBO> m_omniLights;
		std::shared_ptr<SSBO> m_dirLights;
		std::shared_ptr<SSBO> m_dirCSM;
		std::shared_ptr<SSBO> m_areaLights;

		void updateDirectional();

		void updateArea();

		void updateOmni();

		void updateCSM();

		bool m_init;

		bool m_updateCascade = true;

	public:
		bool updateCascade();

		void updateCascade(bool updateCascade);

		void update();
		void bind();
		std::shared_ptr<SSBODataDirectional> dataDirectional() const;
		std::shared_ptr<SSBO> ssboDirectional() const;
		std::shared_ptr<SSBODataOmni> dataOmni() const;
		std::shared_ptr<SSBO> ssboOmni() const;
		std::shared_ptr<SSBODataArea> dataArea() const;
		std::shared_ptr<SSBO> ssboArea() const;

		std::shared_ptr<ClusterCalculation> m_clusterCalculation;

		static constexpr unsigned int DIRECTIONAL_OFFSET = sizeof(glm::vec4);
		static constexpr unsigned int OMNI_OFFSET = sizeof(glm::vec4);

		LightHandler();
	};
}
