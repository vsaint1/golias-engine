#pragma once
#include "golias.h"

using namespace golias;

class GameApplication : public Application {

public:
    bool Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;

private:
   
    Scene mScene;
};
