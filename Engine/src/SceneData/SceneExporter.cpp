#include "../../include/SceneData/SceneExporter.h"
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../include/SceneData/SceneExporterLayout.h"



Assimp::Importer importer;
Assimp::Exporter exporter;
std::map<unsigned int, std::shared_ptr<Prisma::MaterialComponent>> materials;


Prisma::Exporter::Exporter()
{

}

std::string Prisma::Exporter::getFileName(const std::string& filePath) {
    size_t pos = filePath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }
    return filePath;
}

void printScene(std::shared_ptr<Prisma::Node> nodeNext, int depth = 0) {
    if (!nodeNext) {
        return;
    }

    // Print the current node's name with indentation based on the depth
    for (int i = 0; i < depth; ++i) {
        std::cout << " "; // Print a tab for each level of depth
    }
    std::cout << nodeNext->name() << std::endl;

    // Recursively print each child node, increasing the depth
    unsigned int childrenSize = nodeNext->children().size();
    for (unsigned int i = 0; i < childrenSize; i++) {
        printScene(nodeNext->children()[i], depth + 1);
    }
}

void Prisma::Exporter::exportScene(const std::string& sceneName)
{
    if (!currentGlobalScene || !currentGlobalScene->root) {
        std::cerr << "Error: No scene data available to export." << std::endl;
        return;
    }
    // Serialize rootNode to JSON
    json j = currentGlobalScene->root;

    // Write JSON to file
    std::ofstream outFile(sceneName);
    outFile << j.dump();
    outFile.close();

}

std::shared_ptr<Prisma::Node> Prisma::Exporter::importScene(const std::string& sceneName)
{
    // Read JSON from file
    std::ifstream inFile(sceneName);
    json jIn;
    inFile >> jIn;

    auto newRootNode = std::make_shared<Prisma::Node>();
    from_json(jIn, newRootNode);
    return newRootNode;
}
