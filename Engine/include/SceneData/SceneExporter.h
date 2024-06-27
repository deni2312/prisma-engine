#include <memory>
#include "../SceneObjects/Node.h"
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"

namespace Prisma {
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
	};
}