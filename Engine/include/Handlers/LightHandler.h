#pragma once
#include "../SceneData/SceneLoader.h"
#include "../GlobalData/Defines.h"
#include "../GlobalData/InstanceData.h"
#include <vector>

namespace Prisma
{
	class LightHandler : public InstanceData<LightHandler>
	{
		struct SSBODataDirectional
		{
			std::vector<LightType::LightDir> lights;
		};

		struct SSBODataOmni
		{
			std::vector<LightType::LightOmni> lights;
		};

		struct SSBODataArea
		{
			std::vector<LightType::LightArea> lights;
		};

		struct LightSizes
		{
			int omni = 0;
			int dir = 0;
			int area = 0;
			int padding = 0;
		};

		std::shared_ptr<SSBODataDirectional> m_dataDirectional;
		std::shared_ptr<SSBODataOmni> m_dataOmni;
		std::shared_ptr<SSBODataArea> m_dataArea;


		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_omniLights;

		LightSizes m_sizes;

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
		//std::shared_ptr<SSBO> ssboDirectional() const;
		std::shared_ptr<SSBODataOmni> dataOmni() const;
		void updateLightBindings(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb);
		std::shared_ptr<SSBODataArea> dataArea() const;
		//std::shared_ptr<SSBO> ssboArea() const;

		//std::shared_ptr<ClusterCalculation> m_clusterCalculation;

		static constexpr unsigned int DIRECTIONAL_OFFSET = sizeof(glm::vec4);
		static constexpr unsigned int OMNI_OFFSET = sizeof(glm::vec4);

		LightHandler();
	};
}
