#pragma once
#include <LinearMath/btIDebugDraw.h>
#include <glm/glm.hpp>
#include <vector>

namespace golias {
    class PhysicsDebugDrawer : public btIDebugDraw {
    public:
        PhysicsDebugDrawer()          = default;
        virtual ~PhysicsDebugDrawer() = default;

        virtual void Initialize() = 0;
        virtual void Cleanup()    = 0;
        virtual void Begin()      = 0;
        virtual void End()        = 0;

        virtual void Render(const glm::mat4& viewProjection) = 0;

        virtual void Clear() = 0;

    private:
        glm::vec3 btToGlm(const btVector3& vec) const;
    };
} // namespace golias
