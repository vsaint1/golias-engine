#include "core/engine.h"
#include "core/application.h"


class GameApplication : public golias::Application {
public:
    
    bool Initialize() override;

    void Update(float deltaTime) override;

    void Destroy() override;
};