#include "physics/physics_debug_drawer.h"

#include "core/engine.h"
#include "render/command_queue.h"

namespace golias {


    PhysicsDebugDrawer::~PhysicsDebugDrawer() {
        mDebugLines.clear();
    }

    void PhysicsDebugDrawer::Begin() {
        mDebugLines.clear();
    }

    void PhysicsDebugDrawer::End() {
    }

    void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
        DebugLine line;
        line.from  = btToGlm(from);
        line.to    = btToGlm(to);
        line.color = glm::vec4(btToGlm(color), 1.0f);
        mDebugLines.push_back(line);
    }

    void PhysicsDebugDrawer::drawContactPoint(
        const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {

        const btScalar kCossSize = 0.1f;
        btVector3 normal         = normalOnB.normalized();
        btVector3 tangent1, tangent2;

        if (btFabs(normal.x()) < 0.9f) {
            tangent1 = btVector3(1, 0, 0).cross(normal);
        } else {
            tangent1 = btVector3(0, 1, 0).cross(normal);
        }

        tangent1.normalize();
        tangent2 = normal.cross(tangent1);

        drawLine(PointOnB - tangent1 * kCossSize, PointOnB + tangent1 * kCossSize, color);
        drawLine(PointOnB - tangent2 * kCossSize, PointOnB + tangent2 * kCossSize, color);

        drawLine(PointOnB, PointOnB + normal * 0.3f, btVector3(1, 1, 0));
    }

    void PhysicsDebugDrawer::reportErrorWarning(const char* warningString) {
        GOLIAS_LOG_WARN("%s", warningString);
    }

    void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString) {
    }

    void PhysicsDebugDrawer::SetDebugMode(PhysicsDebugMode debugMode) {
        mDebugMode = debugMode;
        mDebugLines.clear();
    }

    PhysicsDebugMode PhysicsDebugDrawer::GetDebugMode() const {
        return mDebugMode;
    }

    void PhysicsDebugDrawer::setDebugMode(int debugMode) {
       
    }

    int PhysicsDebugDrawer::getDebugMode() const {
        return static_cast<int>(mDebugMode);
    }

    void PhysicsDebugDrawer::Render() {
        if (mDebugLines.empty()) {
            return;
        }

        constexpr size_t kFloatsPerVertex = 9; // vec3 position + vec4 color + vec2 texCoord

        std::vector<float> vertices;
        vertices.reserve(mDebugLines.size() * 2 * kFloatsPerVertex);

        for (const DebugLine& line : mDebugLines) {
            // clang-format off
            vertices.insert(vertices.end(), {
                line.from.x, line.from.y, line.from.z,
                line.color.r, line.color.g, line.color.b, line.color.a,
                0.0f, 0.0f
            });
            
            vertices.insert(vertices.end(), {
                line.to.x, line.to.y, line.to.z,
                line.color.r, line.color.g, line.color.b, line.color.a,
                1.0f, 0.0f
            });
            // clang-format on
        }

        UnlitCommand command;
        command.Vertices  = std::move(vertices);
        command.Primitive = PrimitiveType::Lines;
        Engine::GetInstance().GetCommandQueue().Submit(command);
    }


    glm::vec3 PhysicsDebugDrawer::btToGlm(const btVector3& vec) const {
        return glm::vec3(vec.x(), vec.y(), vec.z());
    }

} // namespace golias
