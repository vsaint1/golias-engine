#pragma once

#include "core/application.h"
#include "main_menu.h"
#include "player_obj.h"


class SandboxApplication : public golias::Application {
public:
    SandboxApplication()
        : golias::Application("Golias Engine - Sandbox", 1280, 720, golias::ERenderingDeviceType::COMPATIBILITY) {
    }

    void RegisterTypes() override;

    bool Initialize() override;

    void Update(float deltaTime) override;

    void Destroy() override;
};
