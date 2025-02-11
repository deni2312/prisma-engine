#pragma once

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Components/RegisterComponent.h"
#include "../SceneObjects/Node.h"
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Light.h"
#include <glm/gtx/string_cast.hpp>

namespace Prisma
{
	using json = nlohmann::json;

	// Define a Transform structure
	struct Transform
	{
		glm::mat4 transform;

		// Serialize Transform to JSON
		friend void to_json(json& j, const Transform& t)
		{
			j = json{ {"t", std::vector<float>(value_ptr(t.transform), value_ptr(t.transform) + 16)} };
		}

		// Deserialize Transform from JSON
		friend void from_json(const json& j, Transform& t)
		{
			auto transformData = j.at("t").get<std::vector<float>>();
			t.transform = glm::make_mat4(transformData.data());
		}
	};

	namespace SceneExporterLayout
	{
		static std::pair<std::string, int> status;
		static int counter;
		static int percentage;
		static std::mutex mutex;
	}

	void to_json(json& j, std::shared_ptr<Node> n)
	{
		Transform t;
		Transform k;
		t.transform = n->matrix();
		k.transform = n->finalMatrix();
		j = json{
			{"name", n->name()},
			{"t", t},
			{"k", k},
			{"c", n->children()}
		};
		j["type"] = "NODE";
		std::vector<std::pair<std::string, std::string>> textures;
		if (std::dynamic_pointer_cast<AnimatedMesh>(n))
		{
			auto mesh = std::dynamic_pointer_cast<AnimatedMesh>(n);

			// Add the diffuse texture property
			if (mesh->material()->diffuse().size() > 0)
			{
				std::string textureName = mesh->material()->diffuse()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "DIFFUSE", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "DIFFUSE", textureName });
				}
			}

			// Add the normal texture property
			if (mesh->material()->normal().size() > 0)
			{
				std::string textureName = mesh->material()->normal()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "NORMAL", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "NORMAL", textureName });
				}
			}

			// Add the roughness/metalness texture property
			if (mesh->material()->roughnessMetalness().size() > 0)
			{
				std::string textureName = mesh->material()->roughnessMetalness()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "ROUGHNESS", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "ROUGHNESS", textureName });
				}
			}

			if (mesh->material()->specular().size() > 0)
			{
				std::string textureName = mesh->material()->specular()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "SPECULAR", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "SPECULAR", textureName });
				}
			}
			if (mesh->material()->ambientOcclusion().size() > 0)
			{
				std::string textureName = mesh->material()->ambientOcclusion()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "AMBIENT_OCCLUSION", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "AMBIENT_OCCLUSION", textureName });
				}
			}

			j["textures"] = textures;
			j["plain"] = mesh->material()->plain();
			j["color"] = { mesh->material()->color().x,mesh->material()->color().y ,mesh->material()->color().z ,mesh->material()->color().w };
			// Convert Vertex properties to arrays of floats
			std::vector<json> verticesJson;
			for (const auto& vertex : mesh->animateVerticesData()->vertices)
			{
				verticesJson.push_back({
					{"p", {vertex.position.x, vertex.position.y, vertex.position.z}},
					{
						"boneId",
						{vertex.m_BoneIDs[0], vertex.m_BoneIDs[1], vertex.m_BoneIDs[2], vertex.m_BoneIDs[3]}
					},
					{
						"weight",
						{vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2], vertex.m_Weights[3]}
					},
					{"n", {vertex.normal.x, vertex.normal.y, vertex.normal.z}},
					{"texCoords", {vertex.texCoords.x, vertex.texCoords.y}},
					{"ta", {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z}},
					{"bi", {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z}}
					});
			}
			std::vector<std::vector<float>> data;

			for (const auto& vertex : mesh->animateVerticesData()->vertices)
			{
				data.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });

				data.push_back({ vertex.normal.x, vertex.normal.y, vertex.normal.z });

				data.push_back({ vertex.texCoords.x, vertex.texCoords.y });

				data.push_back({ vertex.tangent.x, vertex.tangent.y, vertex.tangent.z });

				data.push_back({ vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z });

				data.push_back({
					static_cast<float>(vertex.m_BoneIDs[0]), static_cast<float>(vertex.m_BoneIDs[1]),
					static_cast<float>(vertex.m_BoneIDs[2]), static_cast<float>(vertex.m_BoneIDs[3])
					});

				data.push_back({
					vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2], vertex.m_Weights[3]
					});
			}

			j["type"] = "MESH_ANIMATE";
			j["vertices"] = data;
			j["boneCount"] = mesh->boneInfoCounter();

			int i = 0;

			for (const auto& boneData : mesh->boneInfoMap())
			{
				j["boneData"][i]["name"] = boneData.first;
				auto data = boneData.second;
				j["boneData"][i]["data"]["id"] = data.id;
				Transform transform;
				transform.transform = data.offset;
				j["boneData"][i]["data"]["offset"] = transform;
				i++;
			}

			j["faces"] = mesh->animateVerticesData()->indices;
		}
		else if (std::dynamic_pointer_cast<Mesh>(n))
		{
			auto mesh = std::dynamic_pointer_cast<Mesh>(n);

			// Add the diffuse texture property
			if (mesh->material()->diffuse().size() > 0)
			{
				std::string textureName = mesh->material()->diffuse()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "DIFFUSE", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "DIFFUSE", textureName });
				}
			}

			// Add the normal texture property
			if (mesh->material()->normal().size() > 0)
			{
				std::string textureName = mesh->material()->normal()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "NORMAL", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "NORMAL", textureName });
				}
			}

			// Add the roughness/metalness texture property
			if (mesh->material()->roughnessMetalness().size() > 0)
			{
				std::string textureName = mesh->material()->roughnessMetalness()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "ROUGHNESS", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "ROUGHNESS", textureName });
				}
			}

			if (mesh->material()->specular().size() > 0)
			{
				std::string textureName = mesh->material()->specular()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "SPECULAR", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "SPECULAR", textureName });
				}
			}
			if (mesh->material()->ambientOcclusion().size() > 0)
			{
				std::string textureName = mesh->material()->ambientOcclusion()[0].name();
				if (textureName == "")
				{
					textures.push_back({ "AMBIENT_OCCLUSION", "NO_TEXTURE" });
				}
				else
				{
					textures.push_back({ "AMBIENT_OCCLUSION", textureName });
				}
			}

			j["textures"] = textures;
			// Convert Vertex properties to arrays of floats
			std::vector<std::vector<float>> data;

			for (const auto& vertex : mesh->verticesData().vertices)
			{
				data.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });

				data.push_back({ vertex.normal.x, vertex.normal.y, vertex.normal.z });

				data.push_back({ vertex.texCoords.x, vertex.texCoords.y });

				data.push_back({ vertex.tangent.x, vertex.tangent.y, vertex.tangent.z });

				data.push_back({ vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z });
			}
			j["type"] = "MESH";
			j["vertices"] = data;
			j["faces"] = mesh->verticesData().indices;
			j["plain"] = mesh->material()->plain();
			j["color"] = { mesh->material()->color().x,mesh->material()->color().y ,mesh->material()->color().z ,mesh->material()->color().w };
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightDir>>(n))
		{
			j["type"] = "LIGHT_DIRECTIONAL";
			auto light = std::dynamic_pointer_cast<Light<LightType::LightDir>>(n);
			j["direction"] = { light->type().direction.x, light->type().direction.y, light->type().direction.z };
			j["diffuse"] = { light->type().diffuse.x, light->type().diffuse.y, light->type().diffuse.z };
			j["specular"] = { light->type().specular.x, light->type().specular.y, light->type().specular.z };
			j["padding"] = { light->type().padding.x, light->type().padding.y };
			j["shadow"] = light->hasShadow();
			j["near"] = light->shadow()->nearPlane();
			j["far"] = light->shadow()->farPlane();
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(n))
		{
			j["type"] = "LIGHT_OMNI";
			auto light = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(n);
			j["position"] = { light->type().position.x, light->type().position.y, light->type().position.z };
			j["diffuse"] = { light->type().diffuse.x, light->type().diffuse.y, light->type().diffuse.z };
			j["specular"] = { light->type().specular.x, light->type().specular.y, light->type().specular.z };
			j["radius"] = light->type().radius;
			j["attenuation"] = {
				light->type().attenuation.x, light->type().attenuation.y, light->type().attenuation.z,
				light->type().attenuation.w
			};
			j["farPlane"] = light->type().farPlane.x;
			j["shadow"] = light->hasShadow();
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightArea>>(n))
		{
			j["type"] = "LIGHT_AREA";
			auto light = std::dynamic_pointer_cast<Light<LightType::LightArea>>(n);
			j["position"][0] = {light->type().position[0].x, light->type().position[0].y, light->type().position[0].z};
			j["position"][1] = { light->type().position[1].x, light->type().position[1].y, light->type().position[1].z };
			j["position"][2] = { light->type().position[2].x, light->type().position[2].y, light->type().position[2].z };
			j["position"][3] = { light->type().position[3].x, light->type().position[3].y, light->type().position[3].z };
			j["diffuse"] = { light->type().diffuse.x, light->type().diffuse.y, light->type().diffuse.z };
			j["doubleSide"] = light->type().doubleSide;
			j["shadow"] = light->hasShadow();
		}

		std::vector<std::pair<std::string, json>> componentJson;

		for (auto& [name, component] : n->components())
		{
			componentJson.push_back({ name, component->serialize() });
		}

		j["components"] = componentJson;
		j["visible"] = n->visible();
	}

	// Deserialize NodeExport from JSON
	void from_json(json& j, std::shared_ptr<Node> n)
	{
		std::string name;
		Transform t;
		Transform k;
		std::string type;
		j.at("name").get_to(name);
		j.at("t").get_to(t);
		j.at("k").get_to(k);
		j.at("type").get_to(type);
		n->name(name);
		n->matrix(t.transform);
		n->finalMatrix(k.transform);
		n->loadingComponent(false);
		std::vector<json> childrenJson;
		j.at("c").get_to(childrenJson);
		SceneExporterLayout::mutex.lock();
		SceneExporterLayout::status = std::make_pair(
			name, (static_cast<float>(SceneExporterLayout::percentage) / static_cast<float>(
				SceneExporterLayout::counter)) * 100);
		SceneExporterLayout::percentage++;
		SceneExporterLayout::mutex.unlock();
		for (json& childJson : childrenJson)
		{
			auto child = std::make_shared<Node>();
			if (childJson["type"] == "MESH")
			{
				child = std::make_shared<Mesh>();
			}
			else if (childJson["type"] == "LIGHT_DIRECTIONAL")
			{
				child = std::make_shared<Light<LightType::LightDir>>();
			}
			else if (childJson["type"] == "LIGHT_OMNI")
			{
				child = std::make_shared<Light<LightType::LightOmni>>();
			}
			else if (childJson["type"] == "LIGHT_AREA")
			{
				child = std::make_shared<Light<LightType::LightArea>>();
			}
			else if (childJson["type"] == "MESH_ANIMATE")
			{
				child = std::make_shared<AnimatedMesh>();
			}
			child->parent(n.get());
			from_json(childJson, child);
			n->addChild(child, false);
		}
		if (type == "MESH")
		{
			auto mesh = std::dynamic_pointer_cast<Mesh>(n);
			// Deserialize textures
			if (j.contains("textures"))
			{
				auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
				auto material = std::make_shared<MaterialComponent>();
				for (const auto& t : texturesJson)
				{
					if (t.first == "DIFFUSE")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultBlack());
							Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
								"No diffuse texture " + mesh->name());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second, true});
							textures.push_back(texture);
						}
						material->diffuse(textures);
					}
					else if (t.first == "NORMAL")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultNormal());
							Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
								"No normal texture " + mesh->name());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->normal(textures);
					}
					else if (t.first == "ROUGHNESS")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultBlack());
							Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
								"No roughness or metalness texture " + mesh->name());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->roughnessMetalness(textures);
					}
					else if (t.first == "SPECULAR")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultWhite());
							Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
								"No specular texture " + mesh->name());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->specular(textures);
					}
					else if (t.first == "AMBIENT_OCCLUSION")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultWhite());
							Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
								"No ambient occlusion texture " + mesh->name());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->ambientOcclusion(textures);
					}
				}
				glm::vec4 color;
				color.x = j.at("color").at(0);
				color.y = j.at("color").at(1);
				color.z = j.at("color").at(2);
				color.w = j.at("color").at(3);

				int plain = 0;
				plain=j.at("plain");

				material->plain(plain);
				material->color(color);
				mesh->material(material);
			}


			// Convert arrays of floats back to Vertex properties
			auto verticesJson = j.at("vertices").get<std::vector<std::vector<float>>>();
			std::vector<Mesh::Vertex> vertices;
			vertices.resize(verticesJson.size() / 5);
			for (size_t i = 0; i < verticesJson.size(); i = i + 5)
			{
				int vertexIndex = i / 5;

				vertices[vertexIndex].position = glm::vec3(verticesJson[i][0], verticesJson[i][1], verticesJson[i][2]);
				vertices[vertexIndex].normal = glm::vec3(verticesJson[i + 1][0], verticesJson[i + 1][1],
					verticesJson[i + 1][2]);
				vertices[vertexIndex].texCoords = glm::vec2(verticesJson[i + 2][0], verticesJson[i + 2][1]);
				vertices[vertexIndex].tangent = glm::vec3(verticesJson[i + 3][0], verticesJson[i + 3][1],
					verticesJson[i + 3][2]);
				vertices[vertexIndex].bitangent = glm::vec3(verticesJson[i + 4][0], verticesJson[i + 4][1],
					verticesJson[i + 4][2]);
			}

			auto verticesData = std::make_shared<Mesh::VerticesData>();
			verticesData->vertices = vertices;
			verticesData->indices = j.at("faces").get<std::vector<unsigned int>>();
			mesh->loadModel(verticesData);
		}
		else if (type == "LIGHT_DIRECTIONAL")
		{
			auto light = std::dynamic_pointer_cast<Light<LightType::LightDir>>(n);
			LightType::LightDir lightType;
			lightType.direction = glm::vec4(j.at("direction").get<std::vector<float>>().at(0),
				j.at("direction").get<std::vector<float>>().at(1),
				j.at("direction").get<std::vector<float>>().at(2), 1.0);
			lightType.diffuse = glm::vec4(j.at("diffuse").get<std::vector<float>>().at(0),
				j.at("diffuse").get<std::vector<float>>().at(1),
				j.at("diffuse").get<std::vector<float>>().at(2), 1.0);
			lightType.specular = glm::vec4(j.at("specular").get<std::vector<float>>().at(0),
				j.at("specular").get<std::vector<float>>().at(1),
				j.at("specular").get<std::vector<float>>().at(2), 1.0);
			lightType.padding = glm::vec2(j.at("padding").get<std::vector<float>>().at(0),
				j.at("padding").get<std::vector<float>>().at(1));
			bool hasShadow = false;
			j.at("shadow").get_to(hasShadow);

			float nearPlane = 1;
			float farPlane = 200;
			j.at("near").get_to(nearPlane);
			j.at("far").get_to(farPlane);

			light->hasShadow(hasShadow);
			light->type(lightType);
			light->createShadow(MAX_SHADOW_DIR, MAX_SHADOW_DIR,true);
			light->shadow()->nearPlane(nearPlane);
			light->shadow()->farPlane(farPlane);
		}
		else if (type == "LIGHT_OMNI")
		{
			auto light = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(n);
			LightType::LightOmni lightType;
			lightType.position = glm::vec4(j.at("position").get<std::vector<float>>().at(0),
				j.at("position").get<std::vector<float>>().at(1),
				j.at("position").get<std::vector<float>>().at(2), 1.0);
			lightType.diffuse = glm::vec4(j.at("diffuse").get<std::vector<float>>().at(0),
				j.at("diffuse").get<std::vector<float>>().at(1),
				j.at("diffuse").get<std::vector<float>>().at(2), 1.0);
			lightType.specular = glm::vec4(j.at("specular").get<std::vector<float>>().at(0),
				j.at("specular").get<std::vector<float>>().at(1),
				j.at("specular").get<std::vector<float>>().at(2), 1.0);
			lightType.radius = j.at("radius").get<float>();
			lightType.attenuation = glm::vec4(j.at("attenuation").get<std::vector<float>>().at(0),
				j.at("attenuation").get<std::vector<float>>().at(1),
				j.at("attenuation").get<std::vector<float>>().at(2),
				j.at("attenuation").get<std::vector<float>>().at(3));
			lightType.farPlane.x = j.at("farPlane").get<float>();
			bool hasShadow = false;
			j.at("shadow").get_to(hasShadow);
			light->hasShadow(hasShadow);
			light->type(lightType);
			light->createShadow(MAX_SHADOW_OMNI, MAX_SHADOW_OMNI,true);
		}
		else if (type == "LIGHT_AREA")
		{
			auto light = std::dynamic_pointer_cast<Light<LightType::LightArea>>(n);
			LightType::LightArea lightType;
			lightType.position[0] = glm::vec4(j.at("position").at(0).get<std::vector<float>>().at(0),
				j.at("position").at(0).get<std::vector<float>>().at(1),
				j.at("position").at(0).get<std::vector<float>>().at(2), 1.0);
			lightType.position[1] = glm::vec4(j.at("position").at(1).get<std::vector<float>>().at(0),
				j.at("position").at(1).get<std::vector<float>>().at(1),
				j.at("position").at(1).get<std::vector<float>>().at(2), 1.0);
			lightType.position[2] = glm::vec4(j.at("position").at(2).get<std::vector<float>>().at(0),
				j.at("position").at(2).get<std::vector<float>>().at(1),
				j.at("position").at(2).get<std::vector<float>>().at(2), 1.0);
			lightType.position[3] = glm::vec4(j.at("position").at(3).get<std::vector<float>>().at(0),
				j.at("position").at(3).get<std::vector<float>>().at(1),
				j.at("position").at(3).get<std::vector<float>>().at(2), 1.0);
			lightType.diffuse = glm::vec4(j.at("diffuse").get<std::vector<float>>().at(0),
				j.at("diffuse").get<std::vector<float>>().at(1),
				j.at("diffuse").get<std::vector<float>>().at(2), 1.0);

			bool hasShadow = false;
			j.at("shadow").get_to(hasShadow);

			int doubleSide = 0;

			j.at("doubleSide").get_to(doubleSide);
			lightType.doubleSide = doubleSide;
			light->hasShadow(hasShadow);
			light->type(lightType);
		}
		else if (type == "MESH_ANIMATE")
		{
			auto mesh = std::dynamic_pointer_cast<AnimatedMesh>(n);
			// Deserialize textures
			if (j.contains("textures"))
			{
				auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
				auto material = std::make_shared<MaterialComponent>();
				for (const auto& t : texturesJson)
				{
					if (t.first == "DIFFUSE")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultBlack());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second, true});
							textures.push_back(texture);
						}
						material->diffuse(textures);
					}
					else if (t.first == "NORMAL")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultNormal());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->normal(textures);
					}
					else if (t.first == "ROUGHNESS")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultBlack());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->roughnessMetalness(textures);
					}
					else if (t.first == "SPECULAR")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultWhite());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->specular(textures);
					}
					else if (t.first == "AMBIENT_OCCLUSION")
					{
						std::vector<Texture> textures;
						Texture texture;
						if (t.second == "NO_TEXTURE")
						{
							textures.push_back(Prisma::GlobalData::getInstance().defaultWhite());
						}
						else
						{
							texture.name(t.second);
							//texture.loadTexture({t.second});
							textures.push_back(texture);
						}
						material->ambientOcclusion(textures);
					}
				}
				glm::vec4 color;
				color.x = j.at("color").at(0);
				color.y = j.at("color").at(1);
				color.z = j.at("color").at(2);
				color.w = j.at("color").at(3);

				int plain = 0;
				plain = j.at("plain");

				material->plain(plain);
				material->color(color);
				mesh->material(material);
			}
			// Convert arrays of floats back to Vertex properties
			auto verticesJson = j.at("vertices").get<std::vector<json>>();


			std::vector<AnimatedMesh::AnimateVertex> vertices;
			vertices.resize(verticesJson.size() / 7);
			for (size_t i = 0; i < verticesJson.size(); i = i + 7)
			{
				int vertexIndex = i / 7;

				vertices[vertexIndex].position = glm::vec3(verticesJson[i][0], verticesJson[i][1], verticesJson[i][2]);
				vertices[vertexIndex].normal = glm::vec3(verticesJson[i + 1][0], verticesJson[i + 1][1],
					verticesJson[i + 1][2]);
				vertices[vertexIndex].texCoords = glm::vec2(verticesJson[i + 2][0], verticesJson[i + 2][1]);
				vertices[vertexIndex].tangent = glm::vec3(verticesJson[i + 3][0], verticesJson[i + 3][1],
					verticesJson[i + 3][2]);
				vertices[vertexIndex].bitangent = glm::vec3(verticesJson[i + 4][0], verticesJson[i + 4][1],
					verticesJson[i + 4][2]);
				vertices[vertexIndex].m_BoneIDs[0] = verticesJson[i + 5][0];
				vertices[vertexIndex].m_BoneIDs[1] = verticesJson[i + 5][1];
				vertices[vertexIndex].m_BoneIDs[2] = verticesJson[i + 5][2];
				vertices[vertexIndex].m_BoneIDs[3] = verticesJson[i + 5][3];

				vertices[vertexIndex].m_Weights[0] = verticesJson[i + 6][0];
				vertices[vertexIndex].m_Weights[1] = verticesJson[i + 6][1];
				vertices[vertexIndex].m_Weights[2] = verticesJson[i + 6][2];
				vertices[vertexIndex].m_Weights[3] = verticesJson[i + 6][3];
			}

			auto verticesData = std::make_shared<AnimatedMesh::AnimateVerticesData>();
			verticesData->vertices = vertices;
			verticesData->indices = j.at("faces").get<std::vector<unsigned int>>();
			auto& counter = mesh->boneInfoCounter();
			counter = j.at("boneCount").get<int>();

			auto boneDataJson = j.at("boneData");
			for (auto it = boneDataJson.begin(); it != boneDataJson.end(); ++it)
			{
				std::string boneName = it.value().at("name");
				BoneInfo data;
				data.id = it.value().at("data").at("id");
				Transform transform;
				from_json(it.value().at("data").at("offset"), transform);
				data.offset = transform.transform;

				auto& boneMap = mesh->boneInfoMap();

				boneMap[boneName] = data;
			}


			mesh->loadAnimateModel(verticesData);
		}
		if (j.contains("visible"))
		{
			bool visible = true;
			j.at("visible").get_to(visible);
			n->visible(visible);
		}

		if (!j.contains("components") || !j["components"].is_array())
			return;

		for (const auto& componentData : j["components"])
		{
			std::string componentName = componentData[0];
			if (!name.empty())
			{
				json componentJson = componentData[1];
				auto component = Prisma::Factory::createInstance(componentName);
				if (component)
				{
					component->deserialize(componentJson);
					n->addComponent(component);
				}
			}
		}
	}
}