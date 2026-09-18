#pragma once

#include "math/vector2.h"
#include "math/vector3.h"
#include <pocketpy.h>

namespace golias {

    class GameObject;

    void register_golias_py_module();

    void push_vector3(py_OutRef out, Vector3 v);
    bool vector3_repr(int argc, py_Ref val);
    bool unpack_vector3(py_Ref val, Vector3& out);
    py_Type get_vector3_type();

    void push_vector2(py_OutRef out, Vector2 v);
    bool vector2_repr(int argc, py_Ref val);
    bool unpack_vector2(py_Ref val, Vector2& out);
    py_Type get_vector2_type();


    /// Attaches the owning GameObject to a script instance.
    void inject_game_object(void* instance, GameObject* go);

} // namespace golias
