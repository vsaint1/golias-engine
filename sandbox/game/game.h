#include "core/application.h"
#include "test_obj.h"

class SandboxApplication : public golias::Application {
public:
    bool Initialize() override;

    void Update(float deltaTime) override;

    void Destroy() override;

private:
    golias::Scene scene;
};
