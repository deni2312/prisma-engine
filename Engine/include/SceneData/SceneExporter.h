#include <memory>

#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class Exporter : public InstanceData<Exporter>
	{
	public:
		Exporter();

		void exportScene(const std::string& sceneName = "output.prisma");

		std::shared_ptr<Node> importScene(const std::string& sceneName);

	private:
		std::string getFileName(const std::string& filePath);
	};
}
