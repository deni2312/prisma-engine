#include "SceneData/SceneExporter.h"
#include "GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "Helpers/NodeHelper.h"
#include "Pipelines/PipelineHandler.h"
#include "SceneData/SceneExporterLayout.h"
#include <future>


Prisma::Exporter::Exporter()
{
}

std::string Prisma::Exporter::getFileName(const std::string& filePath)
{
	size_t pos = filePath.find_last_of("/\\");
	if (pos != std::string::npos)
	{
		return filePath.substr(pos + 1);
	}
	return filePath;
}
namespace Prisma {
	std::mutex textureMutex;

	void processMeshChunk(std::vector<std::shared_ptr<Prisma::Mesh>>::iterator start, std::vector<std::shared_ptr<Prisma::Mesh>>::iterator end, std::unordered_map<std::string, Prisma::Texture>& texturesLoaded) {
		auto processTexture = [&](std::vector<Prisma::Texture>& textureList, Prisma::Texture defaultTexture, bool srgb) {
			if (!textureList.empty() && !textureList[0].name().empty()) {
				std::string textureName = textureList[0].name();

				textureMutex.lock();
				auto notFind = texturesLoaded.find(textureName) == texturesLoaded.end();
				textureMutex.unlock();

				if (notFind) {
					Prisma::Texture texture;
					texture.name(textureName);
					if (!texture.loadTexture({ textureName, srgb, Prisma::Define::DEFAULT_MIPS, true ,false})) {
						texture = defaultTexture;
					}
					textureMutex.lock();
					texturesLoaded[textureName] = texture;
					textureMutex.unlock();
				}
				textureMutex.lock();
				textureList = { texturesLoaded[textureName] };
				textureMutex.unlock();
			}
			else {
				textureList = { defaultTexture };
			}
			};
		for (auto it = start; it != end; ++it) {
			std::shared_ptr<Prisma::Mesh> mesh = *it;
			auto mat = mesh->material();

			processTexture(mat->diffuse(), Prisma::GlobalData::getInstance().defaultBlack(), true);
			processTexture(mat->normal(), Prisma::GlobalData::getInstance().defaultNormal(), false);
			processTexture(mat->roughnessMetalness(), Prisma::GlobalData::getInstance().defaultBlack(), false);
			processTexture(mat->specular(), Prisma::GlobalData::getInstance().defaultWhite(), false);
			processTexture(mat->ambientOcclusion(), Prisma::GlobalData::getInstance().defaultWhite(), false);
		}
	}
}

void Prisma::Exporter::loadTexturesMultithreaded(std::vector<std::shared_ptr<Prisma::Mesh>>& meshes, std::unordered_map<std::string, Prisma::Texture>& texturesLoaded, int numThreads) {
	std::vector<std::future<void>> futures;
	size_t chunkSize = meshes.size() / numThreads;
	auto it = meshes.begin();

	for (int i = 0; i < numThreads; ++i) {
		auto start = it;
		auto remaining = std::distance(it, meshes.end());
		auto safeChunkSize = std::min<size_t>(chunkSize, static_cast<size_t>(remaining));
		auto end = (i == numThreads - 1) ? meshes.end() : std::next(it, safeChunkSize);
		futures.push_back(std::async(std::launch::async, processMeshChunk, start, end, std::ref(texturesLoaded)));
		it = end;
	}

	for (auto& fut : futures) {
		fut.get();
	}

	for (auto texture : texturesLoaded)
	{
		auto t=texture.second.texture();
		if (t) {
			Prisma::GlobalData::getInstance().addGlobalTexture({ t,texture.first });
		}
	}
	for (auto mesh:meshes)
	{
		auto material = mesh->material();
		if (material->roughnessMetalness()[0].name()==DIR_DEFAULT_BLACK && material->specular()[0].name()!=DIR_DEFAULT_WHITE)
		{
			material->roughnessMetalness(material->specular());
			mesh->material()->isSpecular(true);
			mesh->material(material);
		}
	}
}

void Prisma::Exporter::postLoad(std::shared_ptr<Prisma::Node> node, bool loadCubemap)
{
	Prisma::NodeHelper nodeHelper;
	std::unordered_map<std::string, Texture> texturesLoaded;
	if (loadCubemap && !Prisma::SceneExporterLayout::skybox.empty()) {
		Texture texture;
		texture.loadTexture({ Prisma::SceneExporterLayout::skybox,true });
		PipelineSkybox::getInstance().texture(texture);
	}
	std::vector<std::shared_ptr<Mesh>> meshes;
	nodeHelper.nodeIterator(node, [&](auto node, auto parent)
	{
		auto mesh = std::dynamic_pointer_cast<Mesh>(node);

		if (mesh)
		{
			meshes.push_back(mesh);
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightDir>>(node))
		{
			auto light = std::dynamic_pointer_cast<Light<LightType::LightDir>>(node);
			light->shadow()->init();
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node))
		{
			auto light = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node);
			light->shadow()->init();
		}
		node->loadComponents();
	});


	int numThreads = std::thread::hardware_concurrency();
	loadTexturesMultithreaded(meshes, texturesLoaded, numThreads);

}

void Prisma::Exporter::countNodes(std::shared_ptr<Node> next, int& counter)
{
	counter = counter + 1;
	for (auto child : next->children())
	{
		countNodes(child, counter);
	}
}

void Prisma::Exporter::exportScene(const std::string& sceneName)
{
	if (!Prisma::GlobalData::getInstance().currentGlobalScene() || !Prisma::GlobalData::getInstance().
	                                                                currentGlobalScene()->root)
	{
		std::cerr << "Error: No scene data available to export." << std::endl;
		return;
	}

	int counter = 0;
	countNodes(Prisma::GlobalData::getInstance().currentGlobalScene()->root, counter);
	// Serialize rootNode to JSON
	json j = Prisma::GlobalData::getInstance().currentGlobalScene()->root;
	j["Counter"] = counter;
	// Serialize JSON to MessagePack format and write to binary file
	std::ofstream outFile(sceneName, std::ios::binary); // Open in binary mode
	std::vector<std::uint8_t> msgpackData = json::to_msgpack(j);
	outFile.write(reinterpret_cast<const char*>(msgpackData.data()), msgpackData.size());
	outFile.close();
}

void Prisma::Exporter::importSceneAsync(const std::string& sceneName)
{
	auto loadData = [&](std::string name)
	{
		Prisma::SceneExporterLayout::mutex.lock();
		Prisma::SceneExporterLayout::counter = 0;
		Prisma::SceneExporterLayout::status = std::make_pair("", 0);
		Prisma::SceneExporterLayout::mutex.unlock();
		// Read binary MessagePack data from file
		std::ifstream inFile(name, std::ios::binary); // Open in binary mode
		std::vector<std::uint8_t> msgpackData((std::istreambuf_iterator<char>(inFile)), {});
		// Deserialize MessagePack data to JSON
		json jIn = json::from_msgpack(msgpackData);
		Prisma::SceneExporterLayout::mutex.lock();
		Prisma::SceneExporterLayout::percentage = 0;
		jIn.at("Counter").get_to(Prisma::SceneExporterLayout::counter);
		Prisma::SceneExporterLayout::mutex.unlock();
		m_newRootNode = std::make_shared<Prisma::Node>();
		// Convert JSON to Node (assuming `from_json` function exists for Node type)
		from_json(jIn, m_newRootNode); // Make sure this function is implemented for Node type
		m_finish = true;
	};
	auto threadData = std::thread(loadData, sceneName);
	threadData.detach();
}

std::shared_ptr<Prisma::Node> Prisma::Exporter::importScene(const std::string& sceneName)
{
	// Read binary MessagePack data from file
	std::ifstream inFile(sceneName, std::ios::binary); // Open in binary mode
	std::vector<std::uint8_t> msgpackData((std::istreambuf_iterator<char>(inFile)), {});
	// Deserialize MessagePack data to JSON
	json jIn = json::from_msgpack(msgpackData);

	auto newRootNode = std::make_shared<Prisma::Node>();

	// Convert JSON to Node (assuming `from_json` function exists for Node type)
	from_json(jIn, newRootNode); // Make sure this function is implemented for Node type

	postLoad(newRootNode);

	return newRootNode;
}

bool Prisma::Exporter::hasFinish()
{
	if (m_finish)
	{
		postLoad(m_newRootNode);
		m_finish = false;
		CacheScene::getInstance().updateAllCaches();
		return true;
	}
	return false;
}

std::shared_ptr<Prisma::Node> Prisma::Exporter::newRootNode()
{
	return m_newRootNode;
}

std::mutex& Prisma::Exporter::mutexData()
{
	return Prisma::SceneExporterLayout::mutex;
}

std::pair<std::string, int> Prisma::Exporter::status()
{
	return Prisma::SceneExporterLayout::status;
}
