#include "../../include/SceneData/SceneExporter.h"
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../include/Helpers/NodeHelper.h"
#include "../../include/Pipelines/PipelineHandler.h"
#include "../../include/SceneData/SceneExporterLayout.h"


Assimp::Importer importer;
Assimp::Exporter exporter;
std::map<unsigned int, std::shared_ptr<Prisma::MaterialComponent>> materials;


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

void Prisma::Exporter::postLoad(std::shared_ptr<Prisma::Node> node, bool loadCubemap)
{
	Prisma::NodeHelper nodeHelper;
	std::map<std::string, Texture> texturesLoaded;
	if (loadCubemap && !Prisma::SceneExporterLayout::skybox.empty()) {
		Texture texture;
		texture.loadTexture({ Prisma::SceneExporterLayout::skybox,true });
		texture.data({ 4096, 4096, 3 });
		PipelineSkybox::getInstance().texture(texture);
	}
	nodeHelper.nodeIterator(node, [&](auto node, auto parent)
	{
		auto mesh = std::dynamic_pointer_cast<Mesh>(node);
		if (mesh)
		{
			if (!mesh->material()->diffuse().empty() && !mesh->material()->diffuse()[0].name().empty())
			{
				if (texturesLoaded.find(mesh->material()->diffuse()[0].name()) == texturesLoaded.end())
				{
					Texture texture;
					texture.name(mesh->material()->diffuse()[0].name());
					if (!texture.loadTexture({ mesh->material()->diffuse()[0].name(),true,Prisma::Define::DEFAULT_MIPS,true }))
					{
						texture = Prisma::GlobalData::getInstance().defaultBlack();
						std::cout << mesh->material()->diffuse()[0].name() << std::endl;
					}
					texturesLoaded[mesh->material()->diffuse()[0].name()] = texture;
					mesh->material()->diffuse({ texture });
				}
				else
				{
					mesh->material()->diffuse({ texturesLoaded[mesh->material()->diffuse()[0].name()] });
				}
			}
			else
			{
				mesh->material()->diffuse({ Prisma::GlobalData::getInstance().defaultBlack() });
			}

			if (!mesh->material()->normal().empty() && !mesh->material()->normal()[0].name().empty())
			{
				if (texturesLoaded.find(mesh->material()->normal()[0].name()) == texturesLoaded.end())
				{
					Texture texture;
					texture.name(mesh->material()->normal()[0].name());
					if (!texture.loadTexture({ mesh->material()->normal()[0].name(),false ,Prisma::Define::DEFAULT_MIPS,true })) {
						texture = Prisma::GlobalData::getInstance().defaultNormal();
					}
					texturesLoaded[mesh->material()->normal()[0].name()] = texture;
					mesh->material()->normal({ texture });
				}
				else
				{
					mesh->material()->normal({ texturesLoaded[mesh->material()->normal()[0].name()] });
				}
			}
			else
			{
				mesh->material()->normal({ Prisma::GlobalData::getInstance().defaultNormal() });
			}

			if (!mesh->material()->roughnessMetalness().empty() && !mesh->material()->roughnessMetalness()[0].name().empty())
			{
				if (texturesLoaded.find(mesh->material()->roughnessMetalness()[0].name()) == texturesLoaded.end())
				{
					Texture texture;
					texture.name(mesh->material()->roughnessMetalness()[0].name());
					if (!texture.loadTexture({ mesh->material()->roughnessMetalness()[0].name(),false ,Prisma::Define::DEFAULT_MIPS,true })) {
						texture = Prisma::GlobalData::getInstance().defaultBlack();
					}
					texturesLoaded[mesh->material()->roughnessMetalness()[0].name()] = texture;
					mesh->material()->roughnessMetalness({ texture });
				}
				else
				{
					mesh->material()->roughnessMetalness({
						texturesLoaded[mesh->material()->roughnessMetalness()[0].name()]
					});
				}
			}
			else
			{
				mesh->material()->roughnessMetalness({ Prisma::GlobalData::getInstance().defaultBlack() });
			}

			if (!mesh->material()->specular().empty() && !mesh->material()->specular()[0].name().empty())
			{
				if (texturesLoaded.find(mesh->material()->specular()[0].name()) == texturesLoaded.end())
				{
					Texture texture;
					texture.name(mesh->material()->specular()[0].name());
					if (!texture.loadTexture({ mesh->material()->specular()[0].name(),false ,Prisma::Define::DEFAULT_MIPS,true })) {
						texture = Prisma::GlobalData::getInstance().defaultWhite();
					}
					texturesLoaded[mesh->material()->specular()[0].name()] = texture;
					mesh->material()->specular({ texture });
				}
				else
				{
					mesh->material()->specular({ texturesLoaded[mesh->material()->specular()[0].name()] });
				}
			}
			else
			{
				mesh->material()->specular({ Prisma::GlobalData::getInstance().defaultWhite() });
			}

			if (!mesh->material()->ambientOcclusion().empty() && !mesh->material()->ambientOcclusion()[0].name().empty())
			{
				if (texturesLoaded.find(mesh->material()->ambientOcclusion()[0].name()) == texturesLoaded.end())
				{
					Texture texture;
					texture.name(mesh->material()->ambientOcclusion()[0].name());
					if (texture.loadTexture({ mesh->material()->ambientOcclusion()[0].name(),false ,Prisma::Define::DEFAULT_MIPS,true })) {
						texture = Prisma::GlobalData::getInstance().defaultWhite();
					}
					texturesLoaded[mesh->material()->ambientOcclusion()[0].name()] = texture;
					mesh->material()->ambientOcclusion({ texture });
				}
				else
				{
					mesh->material()->ambientOcclusion({
						texturesLoaded[mesh->material()->ambientOcclusion()[0].name()]
					});
				}
			}
			else
			{
				mesh->material()->ambientOcclusion({ Prisma::GlobalData::getInstance().defaultWhite() });
			}
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightDir>>(node))
		{
			/*auto light = std::dynamic_pointer_cast<Light<LightType::LightDir>>(node);
			light->shadow()->init();
			auto type = light->type();
			type.shadowMap = light->shadow()->id();
			light->type(type);*/
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node))
		{
			/*auto light = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node);
			light->shadow()->init();
			auto type = light->type();
			type.shadowMap = light->shadow()->id();
			light->type(type);*/
		}
		node->loadComponents();
	});
}

void Prisma::Exporter::countNodes(std::shared_ptr<Node> next, int& counter)
{
	counter = counter + 1;
	for (auto child : next->children())
	{
		countNodes(child, counter);
	}
}

void printScene(std::shared_ptr<Prisma::Node> nodeNext, int depth = 0)
{
	if (!nodeNext)
	{
		return;
	}

	// Print the current node's name with indentation based on the depth
	for (int i = 0; i < depth; ++i)
	{
		std::cout << " "; // Print a tab for each level of depth
	}
	std::cout << nodeNext->name() << std::endl;

	// Recursively print each child node, increasing the depth
	unsigned int childrenSize = nodeNext->children().size();
	for (unsigned int i = 0; i < childrenSize; i++)
	{
		printScene(nodeNext->children()[i], depth + 1);
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
