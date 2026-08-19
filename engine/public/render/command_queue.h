#pragma once
#include <vector>

namespace golias {

    class Mesh;
    class Material;

    struct RenderCommand {
        Mesh* Mesh = nullptr;
        Material* Material = nullptr;
    };

    class CommandQueue {
    public:

        void Submit(const RenderCommand& command);

        void Execute();

    private:
        std::vector<RenderCommand> mCommands;
    };
} // namespace golias
