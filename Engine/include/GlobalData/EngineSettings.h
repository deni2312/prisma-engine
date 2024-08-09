#pragma once

namespace Prisma {
	namespace EngineSettings {
		enum class Pipeline {
			FORWARD,
			DEFERRED
		};
		struct Settings {
			Pipeline pipeline;
			bool ssr;
		};
	}
}