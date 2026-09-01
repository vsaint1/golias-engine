#pragma once
#include "golias.h"

using namespace golias;

class GameApplication : public Application {

public:
    void RegisterTypes() override;

    bool Initialize() override;

    void Update(float deltaTime) override;

    void Shutdown() override;

private:
    GameObject* mRoot   = nullptr;

    GameObject* mCanvas = nullptr;

    GameObject* mSettingsCanvas = nullptr;
};
