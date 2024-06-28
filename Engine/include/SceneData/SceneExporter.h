#include <memory>

#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"

namespace Prisma{

	class Exporter {
	public:

		Exporter();

		void exportScene();

		static Exporter& getInstance();
		Exporter(const Exporter&) = delete;
		Exporter& operator=(const Exporter&) = delete;
	private:
		static std::shared_ptr<Exporter> instance;
		std::string getFileName(const std::string& filePath);
	};
}