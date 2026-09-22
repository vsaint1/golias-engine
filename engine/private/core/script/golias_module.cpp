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
        py_Type gKeyCode;

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

            const float radians = Math_Radians((float) degrees);

            py_newfloat(py_retval(), radians);
            return true;
        }

        bool mathf_rad_to_deg(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            double radians;
            if (!parse_number(py_arg(0), radians)) {
                return false;
            }

            const float degrees = Math_Degrees((float) radians);

            py_newfloat(py_retval(), degrees);
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

        struct KeyCodeEntry {
            std::string name;
            int value;
        };

        std::vector<KeyCodeEntry> KeyCodeEntries() {
            std::vector<KeyCodeEntry> entries;

            // Letters A..Z
            for (int i = 0; i <= static_cast<int>(KeyCode::Z) - static_cast<int>(KeyCode::A); i++) {
                std::string name(1, static_cast<char>('A' + i));
                entries.push_back({name, static_cast<int>(KeyCode::A) + i});
            }

            // Digit row -> Num0..Num9
            for (int i = 0; i <= 9; i++) {
                entries.push_back({"Num" + std::to_string(i), static_cast<int>(KeyCode::Num0) + i});
            }

            // Function keys
            int fCount = static_cast<int>(KeyCode::F25) - static_cast<int>(KeyCode::F1) + 1;
            for (int i = 1; i <= fCount; i++) {
                entries.push_back({"F" + std::to_string(i), static_cast<int>(KeyCode::F1) + i - 1});
            }

            auto add = [&](const char* name, KeyCode kc) { entries.push_back({name, static_cast<int>(kc)}); };

            add("LeftShift", KeyCode::LeftShift);
            add("RightShift", KeyCode::RightShift);
            add("LeftControl", KeyCode::LeftControl);
            add("RightControl", KeyCode::RightControl);
            add("LeftAlt", KeyCode::LeftAlt);
            add("RightAlt", KeyCode::RightAlt);
            add("LeftSuper", KeyCode::LeftSuper);
            add("RightSuper", KeyCode::RightSuper);
            add("Escape", KeyCode::Escape);
            add("Enter", KeyCode::Enter);
            add("Tab", KeyCode::Tab);
            add("Backspace", KeyCode::Backspace);
            add("Insert", KeyCode::Insert);
            add("Delete", KeyCode::Delete);
            add("Home", KeyCode::Home);
            add("End", KeyCode::End);
            add("PageUp", KeyCode::PageUp);
            add("PageDown", KeyCode::PageDown);
            add("Space", KeyCode::Space);
            add("Left", KeyCode::Left);
            add("Right", KeyCode::Right);
            add("Up", KeyCode::Up);
            add("Down", KeyCode::Down);
            add("CapsLock", KeyCode::CapsLock);
            add("NumLock", KeyCode::NumLock);
            add("ScrollLock", KeyCode::ScrollLock);
            add("PrintScreen", KeyCode::PrintScreen);
            add("Pause", KeyCode::Pause);
            add("Menu", KeyCode::Menu);

            add("Space", KeyCode::Space);
            add("Apostrophe", KeyCode::Apostrophe);
            add("Comma", KeyCode::Comma);
            add("Minus", KeyCode::Minus);
            add("Period", KeyCode::Period);
            add("Slash", KeyCode::Slash);
            add("Semicolon", KeyCode::Semicolon);
            add("Equal", KeyCode::Equal);
            add("LeftBracket", KeyCode::LeftBracket);
            add("Backslash", KeyCode::Backslash);
            add("RightBracket", KeyCode::RightBracket);
            add("GraveAccent", KeyCode::GraveAccent);

            for (int i = 0; i <= 9; i++) {
                entries.push_back({"KP" + std::to_string(i), static_cast<int>(KeyCode::KP0) + i});
            }

            add("KPDecimal", KeyCode::KPDecimal);
            add("KPDivide", KeyCode::KPDivide);
            add("KPMultiply", KeyCode::KPMultiply);
            add("KPSubtract", KeyCode::KPSubtract);
            add("KPAdd", KeyCode::KPAdd);
            add("KPEnter", KeyCode::KPEnter);
            add("KPEqual", KeyCode::KPEqual);
            add("KPInsert", KeyCode::KPInsert);
            add("KPDelete", KeyCode::KPDelete);
            add("KPHome", KeyCode::KPHome);
            add("KPEnd", KeyCode::KPEnd);
            add("KPPageUp", KeyCode::KPPageUp);
            add("KPPageDown", KeyCode::KPPageDown);
            add("KPLeft", KeyCode::KPLeft);
            add("KPRight", KeyCode::KPRight);
            add("KPUp", KeyCode::KPUp);
            add("KPDown", KeyCode::KPDown);

            add("World1", KeyCode::World1);
            add("World2", KeyCode::World2);

            return entries;
        }

        // Input.is_key_pressed(KeyCode.W)
        bool input_is_key_pressed(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(1);
            py_Ref arg = py_arg(0);

            if (!py_isint(arg)) {
                return TypeError("Input.is_key_pressed() expects a KeyCode (e.g. KeyCode.W)");
            }

            InputManager& input = Engine::GetInstance().GetInputManager();
            py_newbool(py_retval(), input.IsKeyPressed((KeyCode) py_toint(arg)));
            return true;
        }

        bool input_get_mouse_position(int argc, py_StackRef argv) {
            PY_CHECK_ARGC(0);
            const glm::vec2 p = Engine::GetInstance().GetInputManager().GetMousePosition();
            push_vector2(py_retval(), Vector2(p.x, p.y));
            return true;
        }

    } // namespace

#pragma endregion Input

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

        py_GlobalRef engine = py_newmodule("golias.engine");

        py_setattr(golias, py_name("engine"), engine);

        gVector3    = py_newtype("Vector3", 0, engine, nullptr);
        gVector2    = py_newtype("Vector2", 0, engine, nullptr);
        gTransform  = py_newtype("Transform", 0, engine, nullptr);
        gGameObject = py_newtype("GameObject", 0, engine, nullptr);
        gTime       = py_newtype("Time", 0, engine, nullptr);
        gInput      = py_newtype("Input", 0, engine, nullptr);
        gKeyCode    = py_newtype("KeyCode", 0, engine, nullptr);

        // KeyCode enum members -> class attributes (int values match C++ `KeyCode`).
        for (const auto& e : KeyCodeEntries()) {
            py_newint(py_r0(), e.value);
            py_setdict(py_tpobject(gKeyCode), py_name(e.name.c_str()), py_r0());
        }

        // module-level functions
        py_bindfunc(engine, "Vector3", vector3_factory);
        py_bindfunc(engine, "Vector2", vector2_factory);
        py_bindfunc(engine, "deg_to_rad", mathf_deg_to_rad);
        py_bindfunc(engine, "rad_to_deg", mathf_rad_to_deg);

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

        // static helper classes: Time.get_delta_time(), Input.is_key_pressed(KeyCode.W)
        py_bindstaticmethod(gTime, "get_delta_time", time_get_delta_time);
        py_bindstaticmethod(gTime, "get_elapsed_time", time_get_elapsed_time);
        py_bindstaticmethod(gInput, "is_key_pressed", input_is_key_pressed);
        py_bindstaticmethod(gInput, "get_mouse_position", input_get_mouse_position);

        // instance store — script instances survive the tracing GC
        py_newdict(py_r0());
        py_setattr(engine, py_name("_instances"), py_r0());

        py_setglobal(py_name("golias"), golias);

        py_setglobal(py_name("engine"), engine);

        String bootstrap = Engine::GetInstance().GetFileSystem().LoadAssetFileText("golias/engine/__init__.py");

        // PythonBehavior + unqualified names (input, time, Vector3, deg_to_rad, ...)
        if (!py_exec(bootstrap.c_str(), "<golias bootstrap>", EXEC_MODE, nullptr)) {
            GOLIAS_LOG_ERROR("Python bootstrap failed to execute.");
            py_printexc();
            py_clearexc(nullptr);
        }
    }

} // namespace golias