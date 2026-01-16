#define GLM_FORCE_INTRINSICS
#define GLM_ENABLE_EXPERIMENTAL
#include "game/game.h"
#include  <SDL3/SDL_main.h>


int main(int argc, char* argv[]) {


    SandboxApplication* gameApp = new SandboxApplication();

    golias::Engine& engine = golias::Engine::GetInstance();
    engine.SetApplication(gameApp);
    
    if (!engine.Initialize()) {
        return -1;
    }

    engine.Run();
    engine.Destroy();

    return 0;
}