#include "core/application.h"
#include "stdafx.h"

using namespace golias;

class GameApplication : public Application {

public:
    bool Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;
};