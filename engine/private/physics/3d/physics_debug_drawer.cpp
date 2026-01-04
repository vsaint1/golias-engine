#include "physics/3d/physics_debug_drawer.h"

namespace golias {
    glm::vec3 PhysicsDebugDrawer::btToGlm(const btVector3& vec) const {
        return glm::vec3(vec.x(), vec.y(), vec.z());
    }
} // namespace golias
