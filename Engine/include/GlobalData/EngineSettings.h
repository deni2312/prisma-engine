#pragma once

namespace Prisma {
	namespace EngineSettings {
		enum class Pipeline {
			FORWARD,
			DEFERRED,
			DEFERRED_FORWARD
		};
		struct Settings {
			Pipeline pipeline;
			bool ssr;
		};
	}
}