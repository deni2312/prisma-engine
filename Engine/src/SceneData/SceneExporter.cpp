#include "../../include/SceneData/SceneExporter.h"
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../include/SceneData/SceneExporterLayout.h"
#include "../../include/Helpers/NodeHelper.h"


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

	auto writeData = [&]()
	{
		// Serialize rootNode to JSON
		json j = Prisma::GlobalData::getInstance().currentGlobalScene()->root;
		// Serialize JSON to MessagePack format and write to binary file
		std::ofstream outFile(sceneName, std::ios::binary); // Open in binary mode
		std::vector<std::uint8_t> msgpackData = json::to_msgpack(j);
		outFile.write(reinterpret_cast<const char*>(msgpackData.data()), msgpackData.size());
		outFile.close();
	};
	auto threadData = std::thread(writeData);
	threadData.detach();
}

std::shared_ptr<Prisma::Node> Prisma::Exporter::importScene(const std::string& sceneName)
{
	// Read binary MessagePack data from file
	std::ifstream inFile(sceneName, std::ios::binary); // Open in binary mode
	std::vector<std::uint8_t> msgpackData((std::istreambuf_iterator<char>(inFile)), {});

	// Deserialize MessagePack data to JSON
	json jIn = json::from_msgpack(msgpackData);

	// Convert JSON to Node (assuming `from_json` function exists for Node type)
	auto newRootNode = std::make_shared<Node>();
	from_json(jIn, newRootNode); // Make sure this function is implemented for Node type

	/*Prisma::NodeHelper nodeHelper;

	nodeHelper.nodeIterator(newRootNode, [](auto node, auto parent)
	{
		auto mesh = std::dynamic_pointer_cast<Mesh>(node);
		if (mesh)
		{
			if (mesh->material()->diffuse()[0].name() != "")
			{
				mesh->material()->diffuse()[0].loadTexture({mesh->material()->diffuse()[0].name(), true});
			}

			if (mesh->material()->normal()[0].name() != "")
			{
				mesh->material()->normal()[0].loadTexture({mesh->material()->normal()[0].name()});
			}

			if (mesh->material()->roughness_metalness()[0].name() != "")
			{
				mesh->material()->roughness_metalness()[0].loadTexture({
					mesh->material()->roughness_metalness()[0].name()
				});
			}

			if (mesh->material()->specular()[0].name() != "")
			{
				mesh->material()->specular()[0].loadTexture({mesh->material()->specular()[0].name()});
			}

			if (mesh->material()->ambientOcclusion()[0].name() != "")
			{
				mesh->material()->ambientOcclusion()[0].loadTexture({mesh->material()->ambientOcclusion()[0].name()});
			}
		}
	});*/

	return newRootNode;
}
