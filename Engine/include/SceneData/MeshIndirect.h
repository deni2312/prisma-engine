#pragma once
#include <vector>
#include <memory>
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Camera.h"
#include "../Components/MaterialComponent.h"
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	struct DrawElementsIndirectCommand
	{
		unsigned int count;
		unsigned int instanceCount;
		unsigned int firstIndex;
		unsigned int baseVertex;
		unsigned int baseInstance;
	};
	class MeshIndirect : public InstanceData<MeshIndirect>
	{
	public:
		struct MaterialView
		{
			std::vector<Diligent::IDeviceObject*> diffuse;
			std::vector<Diligent::IDeviceObject*> normal;
			std::vector<Diligent::IDeviceObject*> rm;
		};

		//std::shared_ptr<VAO> vao();

		//std::shared_ptr<VBO> vbo();

		//std::shared_ptr<EBO> ebo();

		Prisma::Mesh::VerticesData& verticesData();

		Diligent::RefCntAutoPtr<Diligent::IBuffer> modelsBuffer();

		void load();

		void init();

		void add(unsigned int add);

		void remove(unsigned int remove);

		void updateModels(int model);

		void addAnimate(unsigned int add);

		void removeAnimate(unsigned int remove);

		void updateModelsAnimate(int model);

		void renderMeshes() const;
		void renderMeshesCopy() const;
		void renderAnimateMeshes() const;

		void update();
		void updateSize();
		void updateModels();
		void updateTextureSize();
		void updateStatus() const;

		void setupBuffers();

		void addResizeHandler(std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, MaterialView&)> resizeHandler);

		MeshIndirect();


	private:

		//BINDING DATA

		MaterialView m_textureViews;

		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indirectBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelBuffer;
		Diligent::DrawIndexedIndirectAttribs m_commandsBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBuffer;

		void resizeModels(std::vector<Prisma::Mesh::MeshData>& models);

		std::vector<std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, MaterialView&)>> m_resizeHandler;

		//INDIRECT INDEX

		unsigned int m_indirectDraw;
		unsigned int m_indirectDrawCopy;
		unsigned int m_currentIndex;
		unsigned int m_currentVertex;

		unsigned int m_indirectDrawAnimation;
		unsigned int m_currentIndexAnimation;
		unsigned int m_currentVertexAnimation;

		//CURRENT CACHE DATA

		const unsigned int m_cacheSize = 1000;

		uint64_t m_currentVertexMax = 0;
		uint64_t m_currentIndexMax = 0;

		uint64_t m_currentVertexMaxAnimation = 0;
		uint64_t m_currentIndexMaxAnimation = 0;

		//VERTICES DATA

		Mesh::VerticesData m_verticesData;
		std::vector<DrawElementsIndirectCommand> m_drawCommands;


		AnimatedMesh::AnimateVerticesData m_verticesDataAnimation;
		std::vector<DrawElementsIndirectCommand> m_drawCommandsAnimation;

		std::vector<MaterialData> m_materialData;
		std::vector<MaterialData> m_materialDataAnimation;

		void updateAnimation();

		std::vector<unsigned int> m_cacheAddAnimate;
		std::vector<unsigned int> m_cacheRemoveAnimate;


		std::vector<unsigned int> m_cacheAdd;
		std::vector<unsigned int> m_cacheRemove;

		void sort() const;
		void updateStatusShader() const;


		unsigned int m_sizeLocation;
		unsigned int m_indicesCopyLocation;

		std::map<int,int> m_updateModels;
		std::map<int,int> m_updateModelsAnimate;

		struct CameraData
		{
			float zNear;
			float zFar;
			float fovY;
			float aspect;
		};

		struct StatusData {
			unsigned int status;
			int plainMaterial;
			glm::vec2 padding;
		};

		void updatePso();
	
	};
}
