#pragma once

namespace Prisma::EngineSettings
{
	enum class Pipeline
	{
		FORWARD,
		DEFERRED,
		DEFERRED_FORWARD,
		RAYTRACING
	};

	struct Settings
	{
		Pipeline pipeline;
		bool ssr;
		bool ssao;
	};
}
