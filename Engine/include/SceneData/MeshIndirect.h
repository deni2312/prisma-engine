#pragma once
#include <vector>
#include <memory>
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/AnimatedMesh.h"
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

	class MeshIndirect {
	private:
		//BINDING DATA

		std::shared_ptr<Prisma::VAO> m_vao;
        std::shared_ptr<Prisma::VBO> m_vbo;
        std::shared_ptr<Prisma::EBO> m_ebo;

		std::shared_ptr<Prisma::VAO> m_vaoAnimation;
		std::shared_ptr<Prisma::VBO> m_vboAnimation;
		std::shared_ptr<Prisma::EBO> m_eboAnimation;

		//INDIRECT INDEX

		unsigned int m_indirectDraw;
		unsigned int m_currentIndex;
		unsigned int m_currentVertex;

		unsigned int m_indirectDrawAnimation;
		unsigned int m_currentIndexAnimation;
		unsigned int m_currentVertexAnimation;

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


		Prisma::AnimatedMesh::AnimateVerticesData m_verticesDataAnimation;
		std::vector<DrawElementsIndirectCommand> m_drawCommandsAnimation;
		std::shared_ptr<Prisma::SSBO> m_ssboModelAnimation;
		std::shared_ptr<Prisma::SSBO> m_ssboMaterialAnimation;

		std::vector<Prisma::MaterialData> m_materialData;
		std::vector<Prisma::MaterialData> m_materialDataAnimation;

        static std::shared_ptr<MeshIndirect> instance;

		void updateAnimation();

		std::vector<unsigned int> m_cacheAdd;
		std::vector<unsigned int> m_cacheRemove;


	public:
		static MeshIndirect& getInstance();

		MeshIndirect(const MeshIndirect&) = delete;
		MeshIndirect& operator=(const MeshIndirect&) = delete;
		void load();

		void add(unsigned int add);

		void remove(unsigned int remove);

		void renderMeshes();
		void renderAnimateMeshes();

		void update();
		void updateSize();
		void updateModels();
        void updateTextureSize();

        MeshIndirect();
    };

}