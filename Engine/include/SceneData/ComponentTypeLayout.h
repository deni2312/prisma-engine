#pragma once
#include <nlohmann/json.hpp>
#include "../Physics/Physics.h"
#include "../Components/CloudComponent.h"


namespace Prisma
{
	using json = nlohmann::json;


	// Conversion functions to/from JSON for Collider enum
	NLOHMANN_JSON_SERIALIZE_ENUM(Prisma::Physics::Collider, {
	                             {Prisma::Physics::Collider::BOX_COLLIDER, "BOX_COLLIDER"},
	                             {Prisma::Physics::Collider::SPHERE_COLLIDER, "SPHERE_COLLIDER"},
	                             {Prisma::Physics::Collider::LANDSCAPE_COLLIDER, "LANDSCAPE_COLLIDER"},
	                             {Prisma::Physics::Collider::CONVEX_COLLIDER, "CONVEX_COLLIDER"}
	                             })

	// Conversion functions to/from JSON for CollisionData
	void to_json(json& j, const Prisma::Physics::CollisionData& data)
	{
		j = json{
			{"collider", data.collider},
			{"mass", data.mass},
			{"dynamic", data.dynamic},
			{"softBody", data.softBody},
			{"friction", data.friction},
			{"restitution", data.restitution},
			{"pressure", data.pressure}
		};
	}

	void from_json(const json& j, Prisma::Physics::CollisionData& data)
	{
		j.at("collider").get_to(data.collider);
		j.at("mass").get_to(data.mass);
		j.at("dynamic").get_to(data.dynamic);
		j.at("softBody").get_to(data.softBody);
		j.at("friction").get_to(data.friction);
		j.at("restitution").get_to(data.restitution);
		j.at("pressure").get_to(data.pressure);
	}

	// Conversion functions to/from JSON for SoftBodySettings
	void to_json(json& j, const Prisma::Physics::SoftBodySettings& settings)
	{
		std::vector<std::pair<std::array<float, 3>, float>> data;
		for (auto v : settings.customVertices)
		{
			data.push_back({{v.first.x, v.first.y, v.first.z}, v.second});
		}

		j = json{
			{"numIteration", settings.numIteration},
			{"sleep", settings.sleep},
			{"updatePosition", settings.updatePosition},
			{"customVertices", data},
			{
				"vertexAttributes", {
					{"attribute1", settings.vertexAttributes.mBendCompliance},
					{"attribute2", settings.vertexAttributes.mCompliance},
					{"attribute3", settings.vertexAttributes.mShearCompliance}
				}
			}
		};
	}

	void from_json(const json& j, Prisma::Physics::SoftBodySettings& settings)
	{
		std::vector<std::pair<std::array<float, 3>, float>> data;

		j.at("numIteration").get_to(settings.numIteration);
		j.at("sleep").get_to(settings.sleep);
		j.at("updatePosition").get_to(settings.updatePosition);
		j.at("vertexAttributes").at("attribute1").get_to(settings.vertexAttributes.mBendCompliance);
		j.at("vertexAttributes").at("attribute2").get_to(settings.vertexAttributes.mCompliance);
		j.at("vertexAttributes").at("attribute3").get_to(settings.vertexAttributes.mShearCompliance);
		j.at("customVertices").get_to(data);
		for (auto v : data)
		{
			settings.customVertices.push_back({glm::vec3(v.first[0], v.first[1], v.first[2]), v.second});
		}
	}

	// Conversion functions to/from JSON for LandscapeData
	void to_json(json& j, const Prisma::Physics::LandscapeData& data)
	{
		std::vector<float> landscapeData;
		for (auto landscape : data.landscape)
		{
			landscapeData.push_back(landscape);
		}
		j = json{
			{"landscape", landscapeData},
			{"offset", {data.offset.GetX(), data.offset.GetY(), data.offset.GetZ()}},
			{"scale", {data.scale.GetX(), data.scale.GetY(), data.scale.GetZ()}},
			{"width", data.width}
		};
	}

	void from_json(const json& j, Prisma::Physics::LandscapeData& data)
	{
		std::vector<float> landscapeData;
		j.at("landscape").get_to(landscapeData);
		for (auto landscape : landscapeData)
		{
			data.landscape.push_back(landscape);
		}
		auto offset = j.at("offset");
		data.offset.SetX(offset[0]);
		data.offset.SetY(offset[1]);
		data.offset.SetZ(offset[2]);
		auto scale = j.at("scale");
		data.scale.SetX(scale[0]);
		data.scale.SetY(scale[1]);
		data.scale.SetZ(scale[2]);
		j.at("width").get_to(data.width);
	}

	// Serialization for glm::vec3
	void to_json(json& j, const glm::vec3& vec)
	{
		j = json{vec.x, vec.y, vec.z};
	}
}
