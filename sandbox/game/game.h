#include "core/application.h"
#include "core/engine.h"


class SandboxApplication : public golias::Application {
public:
    bool Initialize() override;

    void Update(float deltaTime) override;

    void Destroy() override;

private:
    golias::Material material;
    std::shared_ptr<golias::Mesh> mesh;
};
