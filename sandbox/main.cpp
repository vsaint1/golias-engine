
#include "core/engine.h"
#include "game.h"
#include "stdafx.h"

int main(int argc, char* argv[]) {


    GameApplication* gameApp = new GameApplication();
    golias::Engine& engine = golias::Engine::GetInstance();
    engine.SetApplication(gameApp);

    if (!engine.Initialize(800, 600, "Golias Engine")) {
        GOLIAS_LOG_ERROR("Failed to initialize the engine.");
        return -1;
    }

    engine.Run();

    engine.Shutdown();

    return 0;
}
