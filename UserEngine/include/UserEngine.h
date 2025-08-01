#include "engine.h"

#include <memory>
#include "PlayerController.h"

class UserEngine : public Prisma::UserData {
public:
    void start() override;

    void update() override;

    void finish() override;

    std::shared_ptr<Prisma::CallbackHandler> callbacks() override;

private:
    std::shared_ptr<PlayerController> m_player;
    std::atomic_bool m_init = false;
};