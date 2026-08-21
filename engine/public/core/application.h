#pragma once

namespace golias {

    class Application {
    public:
        virtual bool Initialize() = 0;

        virtual void Update(float deltaTime) = 0;

        virtual void Shutdown() = 0;

        bool ShouldClose() const;

        void Close();
        
    private:
        bool mIsRunning = true;
    };

} // namespace golias
