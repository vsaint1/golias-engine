#pragma once

namespace golias {

    class Application {
    public:
        virtual bool Initialize()            = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void Destroy()               = 0;


        bool ShouldClose() const;

        void Close();

    protected:
        bool is_running = true;
    };


}; // namespace golias
