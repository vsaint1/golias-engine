#pragma once

#include "physics/3d/physics_debug_drawer.h"

namespace golias {

    class PhysicsDebugDrawerGLES3 : public PhysicsDebugDrawer {
    public:
        PhysicsDebugDrawerGLES3();
        ~PhysicsDebugDrawerGLES3() override;

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime,
                              const btVector3& color) override;
        void reportErrorWarning(const char* warningString) override;
        void draw3dText(const btVector3& location, const char* textString) override;
        void setDebugMode(int debugMode) override;
        int getDebugMode() const override;
       
        void Initialize() override;

        void Begin() override;
        void End() override;
        void Render(const glm::mat4& viewProjection) override;

        void Clear() override;

    private:
        struct DebugLine {
            glm::vec3 from;
            glm::vec3 to;
            glm::vec3 color;
        };

        std::vector<DebugLine> lines;
        int debugMode;

        unsigned int VAO;
        unsigned int VBO;
        unsigned int shaderProgram;

        void Cleanup();
        glm::vec3 btToGlm(const btVector3& vec) const;
    };

} // namespace golias
