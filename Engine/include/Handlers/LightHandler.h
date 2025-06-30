#pragma once
#include "../SceneData/SceneLoader.h"
#include "../GlobalData/Defines.h"
#include "../GlobalData/InstanceData.h"
#include <vector>
#include "../Helpers/ClusterCalculation.h"

namespace Prisma {
class LightHandler : public InstanceData<LightHandler> {
    struct SSBODataDirectional {
        std::vector<LightType::LightDir> lights;
    };

    struct SSBODataOmni {
        std::vector<LightType::LightOmni> lights;
    };

    struct SSBODataArea {
        std::vector<LightType::LightArea> lights;
    };

    struct LightSizes {
        int omni = 0;
        int dir = 0;
        int area = 0;
        int padding = 0;
    };

    std::shared_ptr<SSBODataDirectional> m_dataDirectional;
    std::shared_ptr<SSBODataOmni> m_dataOmni;
    std::shared_ptr<SSBODataArea> m_dataArea;


    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_omniLights;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_dirLights;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightSizes;

    LightSizes m_sizes;

    void updateDirectional();

    void updateArea();

    void updateOmni();

    void updateCSM();

    bool m_init;

    bool m_updateCascade = true;

    void updateSizes();

    std::vector<Diligent::IDeviceObject*> m_omniData;

    std::map<std::string,std::function<void()>> m_updates;

public:
    struct ClusterData {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> clusters;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> clustersData;
    };

    std::vector<Diligent::IDeviceObject*>& omniData();

    Diligent::IDeviceObject* dirShadowData();

    void addLightHandler(std::pair<std::string,std::function<void()>> update);
    void removeLightHandler(const std::string& update);

    bool updateCascade();

    void updateCascade(bool updateCascade);

    void update();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> lightSizes() const;
    std::shared_ptr<SSBODataArea> dataArea() const;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> omniLights();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> dirLights() const;

    ClusterData clusters();

    std::shared_ptr<ClusterCalculation> m_clusterCalculation;
    LightHandler();
};
}