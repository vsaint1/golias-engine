#pragma once

namespace golias {

    class Application {
    public:
        virtual bool Initialize() = 0;

        virtual void RegisterTypes() = 0;
        
        virtual void Update(float deltaTime) = 0;

        virtual void Shutdown() = 0;
    };

} // namespace golias
