#include "golias_module.h"

#include "core/engine.h"
#include "core/input/input_manager.h"
#include "core/time.h"
#include "math/vector3.h"
#include "scene/game_object.h"
#include <pocketpy.h>
#include <unordered_map>

#include <glm/glm.hpp>

namespace golias {


    namespace {

        py_Type gVector3;
        py_Type gVector2;
        py_Type gTransform;
        py_Type gGameObject;
        py_Type gTime;
        py_Type gInput;

        bool parse_number(py_Ref ref, double& out) {
            if (py_isfloat(ref)) {
                out = py_tofloat(ref);
                return true;
            }

            if (py_isint(ref)) {
                out = (double) py_toint(ref);
                return true;
            }

            return false;
        }

        GameObject* self_gameobject(py_Ref self) {
            return *(GameObject**) py_touserdata(self);
        }

        void push_gameobject(py_OutRef out, GameObject* go) {
            void* ud           = py_newobject(out, gGameObject, -1, sizeof(void*));
            *(GameObject**) ud = go;
        }

#pragma region Vector3

        bool vector3_factory(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(3);
            double x, y, z;
            if (!parse_number(py_arg(0), x) || !parse_number(py_arg(1), y) || !parse_number(py_arg(2), z)) {
                return TypeError("Vector3() expected numbers for x, y, z");
            }

            push_vector3(py_retval(), Vector3((float) x, (float) y, (float) z));

            return true;
        }

#pragma endregion

#pragma region Vector2

        bool vector2_factory(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(2);
            double x, y;
            if (!parse_number(py_arg(0), x) || !parse_number(py_arg(1), y)) {
                return TypeError("Vector2() expected numbers for x, y");
            }

            push_vector2(py_retval(), Vector2((float) x, (float) y));

            return true;
        }
#pragma endregion

#pragma region Mathf

        bool mathf_deg_to_rad(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            double degrees;
            if (!parse_number(py_arg(0), degrees)) {
                return false;
            }

            const float radians = (float) (degrees * 0.0174532925f);

            py_newfloat(py_retval(), radians);
            return true;
        }

#pragma endregion

#pragma region Transform

        bool transform_get_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            const glm::vec3 p = self_gameobject(py_arg(0))->GetPosition();
            push_vector3(py_retval(), Vector3(p.x, p.y, p.z));
            return true;
        }

        bool transform_set_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(2);
            Vector3 v;
            if (!unpack_vector3(py_arg(1), v)) {
                return false;
            }

            self_gameobject(py_arg(0))->SetPosition(glm::vec3(v.x, v.y, v.z));
            py_newnone(py_retval());
            return true;
        }

        bool transform_get_world_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            const glm::vec3 p = self_gameobject(py_arg(0))->GetWorldPosition();
            push_vector3(py_retval(), Vector3(p.x, p.y, p.z));
            return true;
        }

        bool transform_set_world_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(2);
            Vector3 v;
            if (!unpack_vector3(py_arg(1), v)) {
                return false;
            }
            self_gameobject(py_arg(0))->SetWorldPosition(glm::vec3(v.x, v.y, v.z));
            py_newnone(py_retval());
            return true;
        }

        bool transform_get_forward(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            const glm::vec3 f = self_gameobject(py_arg(0))->GetForward();
            push_vector3(py_retval(), Vector3(f.x, f.y, f.z));
            return true;
        }

        bool transform_rotate_local(int argc, py_StackRef argv) {
            // rotate_local(axis, radians) — self, axis, angle
            PY_CHECK_ARGC(3);
            Vector3 v;
            if (!unpack_vector3(py_arg(1), v)) {
                return false;
            }

            double angle;
            if (!parse_number(py_arg(2), angle)) {
                return false;
            }

            self_gameobject(py_arg(0))->RotateLocal(glm::vec3(v.x, v.y, v.z), (float) angle);
            py_newnone(py_retval());
            return true;
        }
#pragma endregion

#pragma region GameObject

        bool gameobject_get_name(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            const String name = self_gameobject(py_arg(0))->GetName();
            py_newstr(py_retval(), name.c_str());
            return true;
        }

        bool gameobject_get_transform(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            void* ud           = py_newobject(py_retval(), gTransform, -1, sizeof(void*));
            *(GameObject**) ud = self_gameobject(py_arg(0));
            return true;
        }

        bool gameobject_get_active(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            py_newbool(py_retval(), self_gameobject(py_arg(0))->IsActiveSelf());
            return true;
        }

        bool gameobject_set_active(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(2);
            PY_CHECK_ARG_TYPE(1, tp_bool);
            self_gameobject(py_arg(0))->SetActive(py_tobool(py_arg(1)));
            py_newnone(py_retval());
            return true;
        }
#pragma endregion

#pragma region Time

        bool time_get_delta_time(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(0);
            py_newfloat(py_retval(), Time::GetDeltaTime());
            return true;
        }

        bool time_get_elapsed_time(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(0);
            py_newfloat(py_retval(), Time::GetElapsedTime());
            return true;
        }

#pragma endregion

#pragma region Input

        std::unordered_map<std::string, KeyCode> MakeKeyCodeMap() {
            std::unordered_map<std::string, KeyCode> map;

            // Letters A..Z
            for (int i = 0; i <= static_cast<int>(KeyCode::Z) - static_cast<int>(KeyCode::A); i++) {
                std::string name(1, static_cast<char>('A' + i));
                map[name] = static_cast<KeyCode>(static_cast<int>(KeyCode::A) + i);
            }

            // Digit row
            for (int i = 0; i <= 9; i++) {
                map[std::string(1, static_cast<char>('0' + i))] = static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + i);
            }

            // Function keys
            for (int i = 1; i <= static_cast<int>(KeyCode::F25) - static_cast<int>(KeyCode::F1) + 1; i++) {
                map["F" + std::to_string(i)] = static_cast<KeyCode>(static_cast<int>(KeyCode::F1) + i - 1);
            }

            // Modifiers
            map["LeftShift"]    = KeyCode::LeftShift;
            map["RightShift"]   = KeyCode::RightShift;
            map["LeftControl"]  = KeyCode::LeftControl;
            map["RightControl"] = KeyCode::RightControl;
            map["LeftAlt"]      = KeyCode::LeftAlt;
            map["RightAlt"]     = KeyCode::RightAlt;
            map["LeftSuper"]    = KeyCode::LeftSuper;
            map["RightSuper"]   = KeyCode::RightSuper;

            // Navigation
            map["Escape"]      = KeyCode::Escape;
            map["Enter"]       = KeyCode::Enter;
            map["Tab"]         = KeyCode::Tab;
            map["Backspace"]   = KeyCode::Backspace;
            map["Insert"]      = KeyCode::Insert;
            map["Delete"]      = KeyCode::Delete;
            map["Home"]        = KeyCode::Home;
            map["End"]         = KeyCode::End;
            map["PageUp"]      = KeyCode::PageUp;
            map["PageDown"]    = KeyCode::PageDown;
            map["Space"]       = KeyCode::Space;
            map["Left"]        = KeyCode::Left;
            map["Right"]       = KeyCode::Right;
            map["Up"]          = KeyCode::Up;
            map["Down"]        = KeyCode::Down;
            map["CapsLock"]    = KeyCode::CapsLock;
            map["NumLock"]     = KeyCode::NumLock;
            map["ScrollLock"]  = KeyCode::ScrollLock;
            map["PrintScreen"] = KeyCode::PrintScreen;
            map["Pause"]       = KeyCode::Pause;
            map["Menu"]        = KeyCode::Menu;
            return map;
        }

        const std::unordered_map<std::string, KeyCode> gKeyCodeMap = MakeKeyCodeMap();

        // Input.is_key_pressed("W") — key_name only
        bool input_is_key_pressed(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            PY_CHECK_ARG_TYPE(0, tp_str);
            auto it = gKeyCodeMap.find(py_tostr(py_arg(0)));
            py_newbool(py_retval(), it != gKeyCodeMap.end() && Engine::GetInstance().GetInputManager().IsKeyPressed(it->second));
            return true;
        }

        bool input_get_mouse_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(0);
            const glm::vec2 p = Engine::GetInstance().GetInputManager().GetMousePosition();
            push_vector2(py_retval(), Vector2(p.x, p.y));
            return true;
        }

        const char* kBootstrap = R"py(
class PythonBehavior:

    def start(self):              pass
    def update(self, delta_time): pass
    def on_enable(self):          pass
    def on_disable(self):         pass
    def on_destroy(self):         pass

    @property
    def transform(self):
        return self.get_transform()

    # `self` proxies the owning GameObject: self.get_name(), self.set_active(...), ...
    def __getattr__(self, name):
        owner = self.__dict__.get("_owner", None)
        if owner is None:
            raise AttributeError(name)
        return getattr(owner, name)

golias.PythonBehavior = PythonBehavior


from golias import *
)py";

    } // namespace

    void push_vector2(py_OutRef out, Vector2 v) {
        py_newobject(out, gVector2, -1, 0);
        py_newfloat(py_r0(), v.x);
        py_setdict(out, py_name("x"), py_r0());
        py_newfloat(py_r0(), v.y);
        py_setdict(out, py_name("y"), py_r0());
    }

    bool vector2_repr(int argc, py_Ref val) {
        UNUSED_PARAMETER(argc);

        if (!py_isinstance(val, gVector2)) {
            return false;
        }

        Vector2 v;
        unpack_vector2(val, v);
        py_newfstr(py_retval(), "Vector2(%f, %f)", v.x, v.y);

        return true;
    }

    bool unpack_vector2(py_Ref val, Vector2& out) {
        if (!py_isinstance(val, gVector2)) {
            return false;
        }

        py_ItemRef x = py_getdict(val, py_name("x"));
        py_ItemRef y = py_getdict(val, py_name("y"));
        if (!x || !y) {
            return false;
        }

        out.x = (float) py_tofloat(x);
        out.y = (float) py_tofloat(y);
        return true;
    }

    py_Type get_vector2_type() {
        return gVector2;
    }

    void push_vector3(py_OutRef out, Vector3 v) {
        py_newobject(out, gVector3, -1, 0);
        py_newfloat(py_r0(), v.x);
        py_setdict(out, py_name("x"), py_r0());
        py_newfloat(py_r0(), v.y);
        py_setdict(out, py_name("y"), py_r0());
        py_newfloat(py_r0(), v.z);
        py_setdict(out, py_name("z"), py_r0());
    }

    bool vector3_repr(int argc, py_Ref val) {
        UNUSED_PARAMETER(argc);

        if (!py_isinstance(val, gVector3)) {
            return false;
        }

        Vector3 v;
        unpack_vector3(val, v);
        py_newfstr(py_retval(), "Vector3(%f, %f, %f)", v.x, v.y, v.z);
        return true;
    }

    bool unpack_vector3(py_Ref val, Vector3& out) {
        if (!py_isinstance(val, gVector3)) {
            return false;
        }

        py_ItemRef x = py_getdict(val, py_name("x"));
        py_ItemRef y = py_getdict(val, py_name("y"));
        py_ItemRef z = py_getdict(val, py_name("z"));
        if (!x || !y || !z) {
            return false;
        }

        out.x = (float) py_tofloat(x);
        out.y = (float) py_tofloat(y);
        out.z = (float) py_tofloat(z);
        return true;
    }

    py_Type get_vector3_type() {
        return gVector3;
    }

    void inject_game_object(void* instance, GameObject* go) {
        // PythonBehavior `self` -> GameObject proxy.
        push_gameobject(py_r0(), go);
        py_setdict((py_Ref) instance, py_name("_owner"), py_r0());
    }

    void register_golias_py_module() {
        py_GlobalRef golias = py_newmodule("golias");

        gVector3    = py_newtype("Vector3", 0, golias, nullptr);
        gVector2    = py_newtype("Vector2", 0, golias, nullptr);
        gTransform  = py_newtype("Transform", 0, golias, nullptr);
        gGameObject = py_newtype("GameObject", 0, golias, nullptr);
        gTime       = py_newtype("Time", 0, golias, nullptr);
        gInput      = py_newtype("Input", 0, golias, nullptr);

        // module-level functions
        py_bindfunc(golias, "Vector3", vector3_factory);
        py_bindfunc(golias, "Vector2", vector2_factory);
        py_bindfunc(golias, "deg_to_rad", mathf_deg_to_rad);

        py_bindmagic(gVector2, py_name("__repr__"), vector2_repr);
        py_bindmagic(gVector3, py_name("__repr__"), vector3_repr);

        // class members
        py_bindmethod(gTransform, "rotate_local", transform_rotate_local);
        py_bindmethod(gTransform, "get_position", transform_get_position);
        py_bindmethod(gTransform, "set_position", transform_set_position);
        py_bindmethod(gTransform, "get_world_position", transform_get_world_position);
        py_bindmethod(gTransform, "set_world_position", transform_set_world_position);
        py_bindmethod(gTransform, "get_forward", transform_get_forward);

        py_bindmethod(gGameObject, "get_name", gameobject_get_name);
        py_bindmethod(gGameObject, "get_transform", gameobject_get_transform);
        py_bindmethod(gGameObject, "is_active", gameobject_get_active);
        py_bindmethod(gGameObject, "set_active", gameobject_set_active);

        // static helper classes: Time.get_delta_time(), Input.is_key_pressed("W")
        py_bindstaticmethod(gTime, "get_delta_time", time_get_delta_time);
        py_bindstaticmethod(gTime, "get_elapsed_time", time_get_elapsed_time);
        py_bindstaticmethod(gInput, "is_key_pressed", input_is_key_pressed);
        py_bindstaticmethod(gInput, "get_mouse_position", input_get_mouse_position);

        // instance store — script instances survive the tracing GC
        py_newdict(py_r0());
        py_setattr(golias, py_name("_instances"), py_r0());

        // scripts can `import golias`
        py_setglobal(py_name("golias"), golias);

        // PythonBehavior + unqualified names (input, time, Vector3, deg_to_rad, ...)
        if (!py_exec(kBootstrap, "<golias bootstrap>", EXEC_MODE, nullptr)) {
            GOLIAS_LOG_ERROR("Python bootstrap failed to execute.");
            py_printexc();
            py_clearexc(nullptr);
        }
    }

} // namespace golias
