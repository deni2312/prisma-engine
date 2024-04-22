#pragma once
#include <vector>
#include <memory>
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Camera.h"
#include "../Helpers/Shader.h"
#include "../Containers/SSBO.h"
#include "../Components/MaterialComponent.h"
#include "../Containers/VAO.h"
#include "../Containers/VBO.h"
#include "../Containers/EBO.h"

namespace Prisma {

	struct DrawElementsIndirectCommand
	{
		unsigned int  count;
		unsigned int  instanceCount;
		unsigned int  firstIndex;
		unsigned int  baseVertex;
		unsigned int  baseInstance;
	};

	struct IndirectLoaded {
		unsigned int m_vao;
		unsigned int m_drawBuffer;
	};

	class MeshIndirect {
	private:
		std::shared_ptr<Shader> m_shader;

		//BINDING DATA

		std::shared_ptr<Prisma::VAO> m_vao;
        std::shared_ptr<Prisma::VBO> m_vbo;
        std::shared_ptr<Prisma::EBO> m_ebo;

		//INDIRECT INDEX

		unsigned int m_indirectDraw;
		unsigned int m_currentIndex;
		unsigned int m_currentVertex;

		//CURRENT CACHE DATA

		const unsigned int m_cacheSize = 1000;
		uint64_t m_currentVertexSize;
		uint64_t m_currentIndexSize;
		uint64_t m_currentVertexMax;
		uint64_t m_currentIndexMax;
		unsigned int m_meshSize;

		//VERTICES DATA

		Prisma::Mesh::VerticesData m_verticesData;
		std::vector<DrawElementsIndirectCommand> m_drawCommands;
		std::shared_ptr<Prisma::SSBO> m_ssboModel;
		std::shared_ptr<Prisma::SSBO> m_ssboMaterial;
		std::vector<Prisma::MaterialData> m_materialData;

        static std::shared_ptr<MeshIndirect> instance;

		Prisma::IndirectLoaded m_indirectLoaded;

	public:
		static MeshIndirect& getInstance();

		MeshIndirect(const MeshIndirect&) = delete;
		MeshIndirect& operator=(const MeshIndirect&) = delete;
		void load();

		void renderMeshes();

		void update();
		void updateSize();
		void updateModels();
        void updateTextureSize();

        MeshIndirect();
    };

}