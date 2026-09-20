#pragma once

#include "stdafx.h"
#include <LinearMath/btIDebugDraw.h>

namespace golias {

    enum class PhysicsDebugMode {
        None        = 0,
        Wireframe   = btIDebugDraw::DBG_DrawWireframe,
        Constraints = btIDebugDraw::DBG_DrawConstraints,
        AABB        = btIDebugDraw::DBG_DrawAabb
    };

    inline PhysicsDebugMode operator|(PhysicsDebugMode a, PhysicsDebugMode b) {
        return static_cast<PhysicsDebugMode>(static_cast<int>(a) | static_cast<int>(b));
    }

    class PhysicsDebugDrawer : public btIDebugDraw {
    public:
        PhysicsDebugDrawer() = default;
        ~PhysicsDebugDrawer() override;

        void Begin();

        void Render();

        void End();

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

        void drawContactPoint(
            const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

        void reportErrorWarning(const char* warningString) override;

        void draw3dText(const btVector3& location, const char* textString) override;

        void SetDebugMode(PhysicsDebugMode debugMode);

        PhysicsDebugMode GetDebugMode() const;

    private:
        void setDebugMode(int debugMode) override;

        int getDebugMode() const override;

    private:
        glm::vec3 btToGlm(const btVector3& vec) const;

    private:
        PhysicsDebugMode mDebugMode = PhysicsDebugMode::None;

        struct DebugLine {
            glm::vec3 from;
            glm::vec3 to;
            glm::vec4 color;
        };

        std::vector<DebugLine> mDebugLines = {};
    };
} // namespace golias
