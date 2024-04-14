#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Containers/Ubo.h"

namespace Prisma {
	class MeshHandler {
	public:
		struct UBOData {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
		};
		static const unsigned int VIEW_OFFSET = 0;
		static const unsigned int PROJECTION_OFFSET = sizeof(glm::mat4);

		std::shared_ptr<UBOData> data() const;
		std::shared_ptr<Ubo> ubo() const;
		MeshHandler(const MeshHandler&) = delete;
		MeshHandler& operator=(const MeshHandler&) = delete;
		void updateCamera();

		static MeshHandler& getInstance();

        MeshHandler();

    private:
        std::shared_ptr<UBOData> m_uboData;
		std::shared_ptr<Ubo> m_ubo;
		static std::shared_ptr<MeshHandler> instance;
	};
}