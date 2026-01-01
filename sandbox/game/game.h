#include "core/application.h"
#include "player_obj.h"


#define SCENE_LOAD_FROM_FILE 1

class SandboxApplication : public golias::Application {
public:
    void RegisterTypes() override;

    bool Initialize() override;

    void Update(float deltaTime) override;

    void Destroy() override;


};
