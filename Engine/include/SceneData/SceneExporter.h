#include <memory>
#include "../SceneObjects/Node.h"
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Prisma {

    using json = nlohmann::json;

    enum ExportTypes {
        NODE,
        MESH,
        MESH_ANIM,
        LIGHT_OMNI,
        LIGHT_DIR
    };

    // Define a Transform structure
    struct Transform {
        glm::mat4 transform;

        // Serialize Transform to JSON
        friend void to_json(json& j, const Transform& t) {
            std::vector<float> transformData(16);
            const float* matrix = glm::value_ptr(t.transform);
            for (int i = 0; i < 16; ++i) {
                transformData[i] = matrix[i];
            }
            j = json{
                {"t", transformData}
            };
        }

        // Deserialize Transform from JSON
        friend void from_json(const json& j, Transform& t) {
            std::vector<float> transformData;
            j.at("t").get_to(transformData);
            t.transform = glm::make_mat4(transformData.data());
        }
    };

    struct NodeExport {
        std::string name;
        Transform transform;
        std::vector<std::shared_ptr<NodeExport>> children;
        std::vector<Prisma::Mesh::Vertex> vertices;
        std::vector<unsigned int> faces;
        std::vector<std::pair<std::string, std::string>> textures; // Texture directory and type pairs
        ExportTypes type = ExportTypes::NODE;

        // Serialize NodeExport to JSON
        friend void to_json(json& j, const NodeExport& n) {
            switch (n.type) {
            case ExportTypes::NODE: {
                std::vector<NodeExport> nodes;

                for (auto node : n.children) {
                    nodes.push_back(*node);
                }

                j = json{
                    {"name", n.name},
                    {"type", n.type},
                    {"t", n.transform},
                    {"c", nodes}
                };
            }
                                  break;
            case ExportTypes::MESH: {
                std::vector<NodeExport> nodes;

                for (auto node : n.children) {
                    nodes.push_back(*node);
                }

                j = json{
                    {"name", n.name},
                    {"type", n.type},
                    {"t", n.transform},
                    {"c", nodes},
                    {"textures", n.textures} // Serialize textures
                };

                // Convert Vertex properties to arrays of floats
                std::vector<json> verticesJson;
                for (const auto& vertex : n.vertices) {
                    verticesJson.push_back({
                        {"p", {vertex.position.x, vertex.position.y, vertex.position.z}},
                        {"n", {vertex.normal.x, vertex.normal.y, vertex.normal.z}},
                        {"texCoords", {vertex.texCoords.x, vertex.texCoords.y}},
                        {"ta", {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z}},
                        {"bi", {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z}}
                        });
                }

                j["vertices"] = verticesJson;
                j["faces"] = n.faces;
            }
                                  break;
            }
        }

        // Deserialize NodeExport from JSON
        friend void from_json(const json& j, NodeExport& n) {
            switch (n.type) {
            case ExportTypes::NODE: {
                j.at("name").get_to(n.name);
                j.at("type").get_to(n.type);
                j.at("t").get_to(n.transform);
                std::vector<json> childrenJson;
                j.at("c").get_to(childrenJson);
                for (const auto& childJson : childrenJson) {
                    auto child = std::make_shared<NodeExport>();
                    from_json(childJson, *child);
                    n.children.push_back(child);
                }
            }
            break;
            case ExportTypes::MESH: {
                j.at("name").get_to(n.name);
                j.at("type").get_to(n.type);
                j.at("t").get_to(n.transform);
                std::vector<json> childrenJson;
                j.at("c").get_to(childrenJson);
                for (const auto& childJson : childrenJson) {
                    auto child = std::make_shared<NodeExport>();
                    from_json(childJson, *child);
                    n.children.push_back(child);
                }

                // Deserialize textures
                if (j.contains("textures")) {
                    n.textures.clear(); // Clear existing textures in case of update
                    auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
                    n.textures.insert(n.textures.end(), texturesJson.begin(), texturesJson.end());
                }

                // Convert arrays of floats back to Vertex properties
                auto verticesJson = j.at("vertices").get<std::vector<json>>();
                n.vertices.resize(verticesJson.size());
                for (size_t i = 0; i < verticesJson.size(); ++i) {
                    auto& vertexJson = verticesJson[i];
                    n.vertices[i].position = glm::vec3(vertexJson.at("p").get<std::vector<float>>().at(0),
                        vertexJson.at("p").get<std::vector<float>>().at(1),
                        vertexJson.at("p").get<std::vector<float>>().at(2));
                    n.vertices[i].normal = glm::vec3(vertexJson.at("n").get<std::vector<float>>().at(0),
                        vertexJson.at("n").get<std::vector<float>>().at(1),
                        vertexJson.at("n").get<std::vector<float>>().at(2));
                    n.vertices[i].texCoords = glm::vec2(vertexJson.at("texCoords").get<std::vector<float>>().at(0),
                        vertexJson.at("texCoords").get<std::vector<float>>().at(1));
                    n.vertices[i].tangent = glm::vec3(vertexJson.at("ta").get<std::vector<float>>().at(0),
                        vertexJson.at("ta").get<std::vector<float>>().at(1),
                        vertexJson.at("ta").get<std::vector<float>>().at(2));
                    n.vertices[i].bitangent = glm::vec3(vertexJson.at("bi").get<std::vector<float>>().at(0),
                        vertexJson.at("bi").get<std::vector<float>>().at(1),
                        vertexJson.at("bi").get<std::vector<float>>().at(2));
                }

                n.faces = j.at("faces").get<std::vector<unsigned int>>();
            }
            break;
            }
        }
    };

	class Exporter {
	public:

		Exporter();

		void exportScene();

		static Exporter& getInstance();
		Exporter(const Exporter&) = delete;
		Exporter& operator=(const Exporter&) = delete;
	private:
		static std::shared_ptr<Exporter> instance;
        void addNodesRecursively(const std::shared_ptr<Prisma::Node>& sceneNode);
		std::string getFileName(const std::string& filePath);
		void addNodesExport(const std::shared_ptr<Prisma::Node>& sceneNode, std::shared_ptr<NodeExport> nodeNext);
	};
}