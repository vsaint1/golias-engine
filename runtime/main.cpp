#define GLM_FORCE_INTRINSICS
#include "core/gstl/str.h"
#include "servers/rendering/rendering_canvas.h"
#include "stdafx.h"
#include <SDL3/SDL_main.h>
#include <nuklear.h>
#include <nuklear_sdl3_ogl3.h>

namespace golias {



    class SceneTree;

    class Node {
    protected:
        String name;
        Node* parent = nullptr;
        Vector<Node*> children;
        bool active = true;

    public:
        explicit Node(const String& n = "") : name(n) {
        }

        virtual ~Node() {
            for (auto* child : children) {
                delete child;
            }
        }

        void add_child(Node* child) {
            child->parent = this;
            children.push_back(child);
        }

        void remove_child(Node* child) {
            auto position = children.find(child);

            if (position == -1) {
                return;
            }

            children.erase(position);

            child->parent = nullptr;
        }

        Node* get_parent() const {
            return parent;
        }

        const std::vector<Node*>& get_children() const {
            return children;
        }

        void set_active(bool a) {
            active = a;
        }

        bool is_active() const {
            return active;
        }

        const String& get_name() const {
            return name;
        }

        void set_name(const String& n) {
            name = n;
        }

        virtual void _process(float delta) {
        }

        virtual void _draw(RenderingCanvas* canvas) {
        }

        void process_tree(float delta) {
            if (!active) {
                return;
            }

            _process(delta);

            for (auto* child : children) {
                child->process_tree(delta);
            }
        }

        void draw_tree(RenderingCanvas* canvas) {
            if (!active) {
                return;
            }

            _draw(canvas);

            for (auto* child : children) {
                child->draw_tree(canvas);
            }
        }

        // Find nodes
        Node* find_child(const String& child_name, bool recursive = false) {
            for (auto* child : children) {

                if (child->name == child_name) {
                    return child;
                }

                if (recursive) {
                    if (auto* found = child->find_child(child_name, true)) {
                        return found;
                    }
                }
            }

            return nullptr;
        }

        template <typename T>
        T* cast() {
            return dynamic_cast<T*>(this);
        }
    };

    // CanvasItem - Base for all 2D drawable nodes (like Godot's CanvasItem)
    class CanvasItem : public Node {
    public:
        glm::vec2 position = {0.0f, 0.0f};
        float rotation     = 0.0f;
        glm::vec2 scale    = {1.0f, 1.0f};
        Color modulate     = Color::WHITE;
        CanvasMaterial* material = nullptr; /// Custom material (albedo, texture, shader, params)
        int z_index        = 0;
        bool visible       = true;

        using Node::Node;

        // Transform helpers
        void set_position(float x, float y) {
            position = {x, y};
        }

        void set_rotation(float r) {
            rotation = r;
        }

        void set_scale(float x, float y) {
            scale = {x, y};
        }

        glm::vec2 get_global_position() const {
            if (auto* canvas_parent = dynamic_cast<CanvasItem*>(parent)) {
                return canvas_parent->get_global_position() + position;
            }
            return position;
        }

        float get_global_rotation() const {
            if (auto* canvas_parent = dynamic_cast<CanvasItem*>(parent)) {
                return canvas_parent->get_global_rotation() + rotation;
            }
            return rotation;
        }

        void render_tree(RenderingCanvas* canvas) {
            if (!active || !visible) {
                return;
            }
            _draw(canvas);
            for (auto* child : children) {
                child->draw_tree(canvas);
            }
        }
    };


    class Node2D : public CanvasItem {
    public:
        using CanvasItem::CanvasItem;
    };

    class Sprite2D : public Node2D {
    public:
        RID texture      = INVALID_RID;
        glm::vec4 region = {0, 0, 0, 0};
        bool flip_h      = false;
        bool flip_v      = false;
        bool centered    = true;
        glm::vec2 offset = {0, 0};

        explicit Sprite2D(const String& n = "") : Node2D(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            if (texture == INVALID_RID && material == nullptr) {
                return;
            }

            Rect src = {region.x, region.y, region.z, region.w};

            float w = 100.0f;
            float h = 100.0f;

            if (texture != INVALID_RID) {
                uint32_t tex_width = 0, tex_height = 0;
                canvas->get_texture_size(texture, tex_width, tex_height);

                if (src.width > 0 && src.height > 0) {
                    w = src.width;
                    h = src.height;
                } else if (tex_width > 0 && tex_height > 0) {
                    w = static_cast<float>(tex_width);
                    h = static_cast<float>(tex_height);
                }
            } else if (src.width > 0 && src.height > 0) {
                w = src.width;
                h = src.height;
            }

            w *= scale.x;
            h *= scale.y;

            float x = position.x + offset.x;
            float y = position.y + offset.y;

            if (centered) {
                x -= w * 0.5f;
                y -= h * 0.5f;
            }

            canvas->draw_texture_ex(x, y, w, h, texture, src, modulate, rotation, flip_h, flip_v, material);
        }
    };


    class ColorRect2D : public CanvasItem {
    public:
        Color color        = Color::WHITE;
        glm::vec2 size     = {100, 100};
        bool filled        = true;
        float border_width = 1.0f;

        explicit ColorRect2D(const String& n = "") : CanvasItem(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            Color final_color(color.r * modulate.r, color.g * modulate.g, color.b * modulate.b, color.a * modulate.a);

            if (filled) {
                canvas->draw_rect(position.x, position.y, size.x * scale.x, size.y * scale.y, final_color, rotation);
            } else {
                canvas->draw_rect_outlined(position.x, position.y, size.x * scale.x, size.y * scale.y, final_color, border_width);
            }
        }
    };

    class Circle2D : public CanvasItem {
    public:
        float radius       = 50.0f;
        Color color        = Color::WHITE;
        bool filled        = true;
        float border_width = 1.0f;

        explicit Circle2D(const String& n = "") : CanvasItem(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            Color final_color(color.r * modulate.r, color.g * modulate.g, color.b * modulate.b, color.a * modulate.a);

            if (filled) {
                canvas->draw_circle(position.x, position.y, radius * scale.x, final_color);
            } else {
                canvas->draw_circle_outlined(position.x, position.y, radius * scale.x, final_color, border_width);
            }
        }
    };

    class Line2D : public CanvasItem {
    public:
        std::vector<glm::vec2> points;
        Color color = Color::WHITE;
        float width = 1.0f;

        explicit Line2D(const String& n = "") : CanvasItem(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            if (points.size() < 2) {
                return;
            }

            Color line_color(color.r * modulate.r, color.g * modulate.g, color.b * modulate.b, color.a * modulate.a);

            for (size_t i = 0; i < points.size() - 1; i++) {
                glm::vec2 p1 = position + points[i] * scale;
                glm::vec2 p2 = position + points[i + 1] * scale;
                canvas->draw_line(p1.x, p1.y, p2.x, p2.y, line_color, width);
            }
        }
    };

    class Polygon2D : public CanvasItem {
    public:
        std::vector<glm::vec2> polygon;
        Color color = Color::WHITE;

        explicit Polygon2D(const String& n = "") : CanvasItem(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            if (polygon.size() < 3) {
                return;
            }

            Color poly_color(color.r * modulate.r, color.g * modulate.g, color.b * modulate.b, color.a * modulate.a);

            if (polygon.size() == 3) {
                glm::vec2 p1 = position + polygon[0] * scale;
                glm::vec2 p2 = position + polygon[1] * scale;
                glm::vec2 p3 = position + polygon[2] * scale;
                canvas->draw_triangle(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, poly_color);
            }
        }
    };

    class Label2D : public CanvasItem {
    public:
        Font* font = nullptr;
        String text;

        explicit Label2D(const String& n = "") : CanvasItem(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            if (font && !text.empty()) {
                canvas->draw_text(font, position.x, position.y, Color(modulate.r, modulate.g, modulate.b, modulate.a), text);
            }
        }
    };

    class Camera2D : public Node2D {
    public:
        float zoom          = 1.0f;
        bool current        = false;
        int viewport_width  = 1280;
        int viewport_height = 720;
        glm::vec2 offset    = {0, 0};

        explicit Camera2D(const String& n = "") : Node2D(n) {
        }

        glm::mat4 get_camera_matrix() const {
            glm::mat4 view = glm::mat4(1.0f);
            view           = glm::translate(view, glm::vec3(-(position.x + offset.x), -(position.y + offset.y), 0.0f));
            view           = glm::rotate(view, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            view           = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));

            glm::mat4 projection =
                glm::ortho(0.0f, static_cast<float>(viewport_width), static_cast<float>(viewport_height), 0.0f, -1.0f, 1.0f);
            return projection * view;
        }

        void make_current() {
            current = true;
        }
    };

    class CanvasLayer : public Node {
    public:
        int layer            = 0;
        glm::vec2 offset     = {0, 0};
        float rotation       = 0.0f;
        glm::vec2 scale      = {1.0f, 1.0f};
        bool follow_viewport = true;

        explicit CanvasLayer(const String& n = "") : Node(n) {
        }

        void _draw(RenderingCanvas* canvas) override {
            // CanvasLayer doesn't render itself, just its children
        }
    };

    // SceneTree - Manages the node hierarchy (like Godot's SceneTree)
    class SceneTree {
        Node* root               = nullptr;
        RenderingCanvas* canvas  = nullptr;
        Camera2D* current_camera = nullptr;
        float delta              = 0.016f;
        float elapsed            = 0.0f;

    public:
        SceneTree() {
            root = new Node("root");
        }

        ~SceneTree() {
            delete root;
        }

        void initialize(RenderingCanvas* r) {
            canvas = r;
        }

        Node* get_root() {
            return root;
        }

        // Add node to root
        template <typename T>
        T* add_node(const String& name = "") {
            T* node = new T(name);
            root->add_child(node);
            return node;
        }

        // Add node to specific parent
        template <typename T>
        T* add_node_to(Node* parent, const String& name = "") {
            T* node = new T(name);
            parent->add_child(node);
            return node;
        }

        // Find node by name
        Node* find_node(const String& name) {
            return root->find_child(name, true);
        }

        // Find all nodes of type
        template <typename T>
        std::vector<T*> find_nodes_of_type() {
            std::vector<T*> results;
            find_nodes_recursive<T>(root, results);
            return results;
        }

        void set_current_camera(Camera2D* cam) {
            if (current_camera) {
                current_camera->current = false;
            }
            current_camera = cam;
            if (cam) {
                cam->current = true;
            }
        }

        Camera2D* get_current_camera() {
            return current_camera;
        }

        void update(float dt) {
            delta = dt;
            elapsed += dt;
            root->process_tree(dt);
        }

        void render() {
            if (current_camera) {
                canvas->set_camera(current_camera->get_camera_matrix());
            } else {
                canvas->reset_camera();
            }
            root->draw_tree(canvas);
        }

        float get_delta() const {
            return delta;
        }
        float get_time() const {
            return elapsed;
        }

    private:
        template <typename T>
        void find_nodes_recursive(Node* node, std::vector<T*>& results) {
            if (auto* typed = node->cast<T>()) {
                results.push_back(typed);
            }
            for (auto* child : node->get_children()) {
                find_nodes_recursive<T>(child, results);
            }
        }
    };

} // namespace golias


int main(int argc, char* argv[]) {

#if defined(NDEBUG)
    constexpr auto LOG_LEVEL = spdlog::level::info;
#else
    constexpr auto LOG_LEVEL = spdlog::level::debug;
#endif

#if defined(__ANDROID__)
    auto android_sink = std::make_shared<spdlog::sinks::android_sink_mt>("GoliasEngine");
    std::vector<spdlog::sink_ptr> sinks{android_sink};
#else
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/output.log", true);
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
#endif

    auto logger = std::make_shared<spdlog::logger>("GoliasEngine", sinks.begin(), sinks.end());
    logger->set_level(LOG_LEVEL);
    logger->flush_on(LOG_LEVEL);
    spdlog::set_default_logger(logger);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    SDL_Window* window       = SDL_CreateWindow("Golias Engine", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if (!gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
        spdlog::error("Failed to initialize OpenGL/ES Loader (GLAD)");
        return -1;
    }

    SDL_GL_SetSwapInterval(1);

    if (!TTF_Init()) {
        spdlog::error("Failed to initialize SDL_TTF: {}", SDL_GetError());
        return -1;
    }

    RenderingDeviceGLES3 rd;
    rd.initialize();

    RenderingCanvas renderer(&rd);
    renderer.initialize(1280, 720);
    renderer.set_viewport_size(1280, 720);
    renderer.set_scale_mode(ScaleMode::KEEP);


    golias::SceneTree scene;
    scene.initialize(&renderer);

    RID icon_tex     = renderer.load_texture_from_file("res/icon.png");
    RID monsters_tex = renderer.load_texture_from_file("res/test/sprites/monsters.png");

    TextureDescription pixel_desc;
    pixel_desc.min_filter = TextureFilter::NEAREST;
    pixel_desc.mag_filter = TextureFilter::NEAREST;
    pixel_desc.wrap_u     = TextureWrap::CLAMP_TO_EDGE;
    pixel_desc.wrap_v     = TextureWrap::CLAMP_TO_EDGE;
    RID pixel_tex         = renderer.load_texture_from_file("res/test/sprites/monsters.png", pixel_desc);

    RID wavy_shader    = renderer.create_shader_from_file("res/test/shaders/wave.glsl");
    RID rainbow_shader = renderer.create_shader_from_file("res/test/shaders/rainbow.glsl");
    RID retro_shader   = renderer.create_shader_from_file("res/test/shaders/retro.glsl");

    auto wavy_material = CanvasMaterial(wavy_shader)
      .set_float("amplitude", 10.0f)
      .set_float("frequency", 5.0f);

    auto rainbow_material = CanvasMaterial(rainbow_shader)
      .set_float("speed", 2.0f);

    auto retro_material = CanvasMaterial(retro_shader)
        .set_float("pixel_size", 64.0f);

    Font* title_font = renderer.load_font_from_file("res/test/fonts/Minecraft.ttf", 24.0f);
    Font* ui_font    = renderer.load_font_from_file("res/test/fonts/Default.ttf", 24.0f);

    auto* camera = scene.add_node<golias::Camera2D>("camera");
    camera->set_position(0, 0);
    camera->zoom            = 1.0f;
    camera->viewport_width  = 1280;
    camera->viewport_height = 720;


    auto* red_rect = scene.add_node<golias::ColorRect2D>("red_rect");
    red_rect->set_position(50, 50);
    red_rect->size  = {100, 80};
    red_rect->color = Color::RED;

    auto* green_circle = scene.add_node<golias::Circle2D>("green_circle");
    green_circle->set_position(200, 90);
    green_circle->radius = 40.0f;
    green_circle->color  = Color::GREEN;

    auto* blue_triangle    = scene.add_node<golias::Polygon2D>("blue_triangle");
    blue_triangle->polygon = {{40, 0}, {80, 60}, {0, 60}};
    blue_triangle->set_position(290, 50);
    blue_triangle->color = Color::BLUE;

    auto* magenta_outline = scene.add_node<golias::ColorRect2D>("magenta_outline");
    magenta_outline->set_position(400, 50);
    magenta_outline->size         = {90, 70};
    magenta_outline->color        = Color::MAGENTA;
    magenta_outline->filled       = false;
    magenta_outline->border_width = 3.0f;

    auto* yellow_circle = scene.add_node<golias::Circle2D>("yellow_circle");
    yellow_circle->set_position(540, 90);
    yellow_circle->radius = 40.0f;
    yellow_circle->color  = Color::YELLOW;
    yellow_circle->filled = false;

    auto* cyan_rect = scene.add_node<golias::ColorRect2D>("cyan_rect");
    cyan_rect->set_position(620, 50);
    cyan_rect->size  = {80, 80};
    cyan_rect->color = Color::CYAN;

    // --- ROW 2: LINES ---
    auto* yellow_line   = scene.add_node<golias::Line2D>("yellow_line");
    yellow_line->points = {{0, 0}, {180, 0}};
    yellow_line->set_position(50, 170);
    yellow_line->color = Color::YELLOW;
    yellow_line->width = 4.0f;

    auto* cyan_line   = scene.add_node<golias::Line2D>("cyan_line");
    cyan_line->points = {{0, 0}, {130, 40}};
    cyan_line->set_position(250, 160);
    cyan_line->color = Color::CYAN;
    cyan_line->width = 2.0f;

    auto* multi_line   = scene.add_node<golias::Line2D>("multi_line");
    multi_line->points = {{0, 0}, {40, 25}, {80, 5}, {120, 30}};
    multi_line->set_position(400, 160);
    multi_line->color = Color(1.0f, 0.5f, 0.0f, 1.0f);
    multi_line->width = 3.0f;

    auto* monster_sprite = scene.add_node<golias::Sprite2D>("monster_sprite");
    monster_sprite->set_position(80, 280);
    monster_sprite->set_scale(1.2f, 1.2f);
    monster_sprite->texture = monsters_tex;

    auto* icon_sprite1 = scene.add_node<golias::Sprite2D>("icon_sprite1");
    icon_sprite1->set_position(1000, 700);
    icon_sprite1->set_scale(1.0f, 1.0f);
    icon_sprite1->texture = icon_tex;

    auto* rotating_icon = scene.add_node<golias::Sprite2D>("rotating_icon");
    rotating_icon->set_position(800, 400);
    rotating_icon->set_scale(0.5f, 0.5f);
    rotating_icon->texture = icon_tex;

    auto* pixel_sprite = scene.add_node<golias::Sprite2D>("pixel_sprite");
    pixel_sprite->set_position(520, 280);
    pixel_sprite->set_scale(1.2f, 1.2f);
    pixel_sprite->texture = pixel_tex;

    auto* normal_sprite = scene.add_node<golias::Sprite2D>("normal");
    normal_sprite->set_position(70, 400);
    normal_sprite->set_scale(0.2f, 0.2f);
    normal_sprite->texture = icon_tex;

    auto* flip_h_sprite = scene.add_node<golias::Sprite2D>("flip_h");
    flip_h_sprite->set_position(70, 450);
    flip_h_sprite->set_scale(0.2f, 0.2f);
    flip_h_sprite->texture = icon_tex;
    flip_h_sprite->flip_h  = true;

    auto* flip_v_sprite = scene.add_node<golias::Sprite2D>("flip_v");
    flip_v_sprite->set_position(140, 400);
    flip_v_sprite->set_scale(0.2f, 0.2f);
    flip_v_sprite->texture = icon_tex;
    flip_v_sprite->flip_v  = true;

    auto* flip_both_sprite = scene.add_node<golias::Sprite2D>("flip_both");
    flip_both_sprite->set_position(140, 450);
    flip_both_sprite->set_scale(0.2f, 0.2f);
    flip_both_sprite->texture = icon_tex;
    flip_both_sprite->flip_h  = true;
    flip_both_sprite->flip_v  = true;

    auto* wavy_sprite = scene.add_node<golias::Sprite2D>("wavy");
    wavy_sprite->set_position(800, 520);
    wavy_sprite->set_scale(0.5f, 0.5f);
    wavy_sprite->texture  = icon_tex;
    wavy_sprite->material = &wavy_material;

    auto* rainbow_sprite = scene.add_node<golias::Sprite2D>("rainbow");
    rainbow_sprite->set_position(680, 520);
    rainbow_sprite->set_scale(1.0f, 1.0f);
    rainbow_sprite->material = &rainbow_material;


    auto* retro_sprite = scene.add_node<golias::Sprite2D>("retro");
    retro_sprite->set_position(500, 520);
    retro_sprite->set_scale(0.3f, 0.3f);
    retro_sprite->texture  = icon_tex;
    retro_sprite->material = &retro_material;

    auto* rotating_wavy = scene.add_node<golias::Sprite2D>("rotating_wavy");
    rotating_wavy->set_position(900, 520);
    rotating_wavy->set_scale(0.2f, 0.2f);
    rotating_wavy->texture  = icon_tex;
    rotating_wavy->material = &wavy_material;

    // --- SPRITE REGIONS ---
    auto* cropped_sprite = scene.add_node<golias::Sprite2D>("cropped");
    cropped_sprite->set_position(100, 640);
    cropped_sprite->set_scale(2.0f, 2.0f);
    cropped_sprite->texture = pixel_tex;
    cropped_sprite->region  = {0, 0, 32, 32};

    auto* cropped_sprite2 = scene.add_node<golias::Sprite2D>("cropped2");
    cropped_sprite2->set_position(230, 640);
    cropped_sprite2->set_scale(2.0f, 2.0f);
    cropped_sprite2->texture = pixel_tex;
    cropped_sprite2->region  = {32, 0, 32, 32};

    // --- UI TEXT (Right side) ---
    auto* title_label = scene.add_node<golias::Label2D>("title");
    title_label->set_position(750, 50);
    title_label->font = title_font;
    title_label->text = "We love Fish Salad 🥰";

    auto* frame_label = scene.add_node<golias::Label2D>("frame_text");
    frame_label->set_position(750, 100);
    frame_label->font = ui_font;
    frame_label->text = "Frame: 0";

    auto* time_label = scene.add_node<golias::Label2D>("time_text");
    time_label->set_position(750, 130);
    time_label->font = ui_font;
    time_label->text = "Time: 0.00s ⏱️";

    auto* health_label = scene.add_node<golias::Label2D>("health_text");
    health_label->set_position(750, 160);
    health_label->font     = ui_font;
    health_label->text     = "Health: 100";
    health_label->modulate = Color::GREEN;

    auto* score_label = scene.add_node<golias::Label2D>("score_text");
    score_label->set_position(750, 190);
    score_label->font     = ui_font;
    score_label->text     = "Score: 1,000 🏆";
    score_label->modulate = Color::YELLOW;

    auto* info_label = scene.add_node<golias::Label2D>("info_text");
    info_label->set_position(750, 240);
    info_label->font     = ui_font;
    info_label->text     = "Press C for Camera";
    info_label->modulate = Color(0.7f, 0.7f, 0.7f, 1.0f);

    auto* info_label2 = scene.add_node<golias::Label2D>("info_text2");
    info_label2->set_position(750, 265);
    info_label2->font     = ui_font;
    info_label2->text     = "Arrow Keys to Move";
    info_label2->modulate = Color(0.7f, 0.7f, 0.7f, 1.0f);

    auto* canvas_label = scene.add_node<golias::Label2D>("canvas_label");
    canvas_label->set_position(750, 310);
    canvas_label->font     = ui_font;
    canvas_label->text     = "(1) No Scale  (2) Keep  (3) Expand";
    canvas_label->modulate = Color(0.8f, 0.8f, 0.8f, 1.0f);


    auto* filter_label = scene.add_node<golias::Label2D>("filter_label");
    filter_label->set_position(750, 335);
    filter_label->font     = ui_font;
    filter_label->text     = "Texture Filtering: (Linear/Nearest)";
    filter_label->modulate = Color(0.8f, 0.8f, 0.8f, 1.0f);


    nk_context* nk_ctx = nk_sdl_init(window);

    // Load font for Nuklear
    nk_font_atlas* atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();


    // UI state variables
    bool show_ui = true;
    bool show_shader_editor = true;
    bool show_object_inspector = true;

    // Shader uniform controls
    float wavy_amplitude = 10.0f;
    float wavy_frequency = 5.0f;
    float rainbow_speed = 2.0f;
    float retro_pixel_size = 64.0f;

    // Object transform controls
    float selected_pos_x = 0.0f;
    float selected_pos_y = 0.0f;
    float selected_rotation = 0.0f;
    float selected_scale_x = 1.0f;
    float selected_scale_y = 1.0f;
    golias::Sprite2D* selected_sprite = wavy_sprite;

    struct nk_colorf bg_color = {0.1f, 0.1f, 0.15f, 1.0f};

    bool running = true;
    SDL_Event event;

    while (running) {
        nk_input_begin(nk_ctx);
        while (SDL_PollEvent(&event)) {
            nk_sdl_handle_event(&event);

            if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                running = false;
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                int new_width  = event.window.data1;
                int new_height = event.window.data2;
                renderer.set_viewport_size(new_width, new_height);
                if (camera) {
                    camera->viewport_width  = new_width;
                    camera->viewport_height = new_height;
                }
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_H) {
                    show_ui = !show_ui;
                    spdlog::info("UI: {}", show_ui ? "SHOWN" : "HIDDEN");
                } else if (event.key.key == SDLK_1) {
                    renderer.set_scale_mode(ScaleMode::NONE);
                    spdlog::info("Scale Mode: NONE");
                } else if (event.key.key == SDLK_2) {
                    renderer.set_scale_mode(ScaleMode::KEEP);
                    spdlog::info("Scale Mode: KEEP");
                } else if (event.key.key == SDLK_3) {
                    renderer.set_scale_mode(ScaleMode::EXPAND);
                    spdlog::info("Scale Mode: EXPAND");
                } else if (event.key.key == SDLK_C) {
                    if (camera) {
                        camera->current = !camera->current;
                        if (camera->current) {
                            scene.set_current_camera(camera);
                        } else {
                            scene.set_current_camera(nullptr);
                        }
                        spdlog::info("Camera: {}", camera->current ? "ENABLED" : "DISABLED");
                    }
                }
            }
        }
        nk_input_end(nk_ctx);

        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (camera && camera->current) {
            float camera_speed = 200.0f * scene.get_delta();

            if (keys[SDL_SCANCODE_LEFT]) {
                camera->position.x -= camera_speed;
            }
            if (keys[SDL_SCANCODE_RIGHT]) {
                camera->position.x += camera_speed;
            }
            if (keys[SDL_SCANCODE_UP]) {
                camera->position.y -= camera_speed;
            }
            if (keys[SDL_SCANCODE_DOWN]) {
                camera->position.y += camera_speed;
            }

            if (keys[SDL_SCANCODE_EQUALS] || keys[SDL_SCANCODE_KP_PLUS]) {
                camera->zoom = std::min(camera->zoom + 0.5f * scene.get_delta(), 3.0f);
            }
            if (keys[SDL_SCANCODE_MINUS] || keys[SDL_SCANCODE_KP_MINUS]) {
                camera->zoom = std::max(camera->zoom - 0.5f * scene.get_delta(), 0.1f);
            }

            if (keys[SDL_SCANCODE_R]) {
                camera->rotation += 1.0f * scene.get_delta();
            }
            if (keys[SDL_SCANCODE_T]) {
                camera->rotation -= 1.0f * scene.get_delta();
            }
        }

        rotating_wavy->rotation += 2.0f * scene.get_delta();
        rotating_icon->rotation += 3.0f * scene.get_delta();
        cyan_rect->rotation += 2.0f * scene.get_delta();

        frame_label->text = std::format("Frame: {}", static_cast<int>(scene.get_time() * 60));
        time_label->text  = std::format("Time: {:.2f}s", scene.get_time());

        if (show_ui) {
            // Shader Editor Window
            if (show_shader_editor && nk_begin(nk_ctx, "Shader Editor", nk_rect(10, 10, 350, 450),
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "WAVY SHADER", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_property_float(nk_ctx, "Amplitude:", 0.0f, &wavy_amplitude, 50.0f, 0.5f, 0.5f);
                nk_property_float(nk_ctx, "Frequency:", 0.0f, &wavy_frequency, 20.0f, 0.5f, 0.5f);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "RAINBOW SHADER", NK_TEXT_CENTERED);
                nk_property_float(nk_ctx, "Speed:", 0.0f, &rainbow_speed, 10.0f, 0.1f, 0.1f);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "RETRO SHADER", NK_TEXT_CENTERED);
                nk_property_float(nk_ctx, "Pixel Size:", 1.0f, &retro_pixel_size, 128.0f, 1.0f, 1.0f);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "BACKGROUND", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(nk_ctx, 120, 1);
                bg_color = nk_color_picker(nk_ctx, bg_color, NK_RGBA);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                if (nk_button_label(nk_ctx, "Reset Shaders")) {
                    wavy_amplitude = 10.0f;
                    wavy_frequency = 5.0f;
                    rainbow_speed = 2.0f;
                    retro_pixel_size = 64.0f;
                }
            }
            nk_end(nk_ctx);

            // Object Inspector Window
            if (show_object_inspector && nk_begin(nk_ctx, "Object Inspector", nk_rect(370, 10, 350, 400),
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "Selected: Wavy Sprite", NK_TEXT_CENTERED);

                // Update controls from selected sprite
                selected_pos_x = wavy_sprite->position.x;
                selected_pos_y = wavy_sprite->position.y;
                selected_rotation = wavy_sprite->rotation;
                selected_scale_x = wavy_sprite->scale.x;
                selected_scale_y = wavy_sprite->scale.y;

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_label(nk_ctx, "TRANSFORM", NK_TEXT_CENTERED);

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_property_float(nk_ctx, "Position X:", 0.0f, &selected_pos_x, 1280.0f, 1.0f, 1.0f);
                nk_property_float(nk_ctx, "Position Y:", 0.0f, &selected_pos_y, 720.0f, 1.0f, 1.0f);
                nk_property_float(nk_ctx, "Rotation:", -10.0f, &selected_rotation, 10.0f, 0.01f, 0.01f);
                nk_property_float(nk_ctx, "Scale X:", 0.1f, &selected_scale_x, 3.0f, 0.05f, 0.05f);
                nk_property_float(nk_ctx, "Scale Y:", 0.1f, &selected_scale_y, 3.0f, 0.05f, 0.05f);

                // Apply changes to sprite
                wavy_sprite->position.x = selected_pos_x;
                wavy_sprite->position.y = selected_pos_y;
                wavy_sprite->rotation = selected_rotation;
                wavy_sprite->scale.x = selected_scale_x;
                wavy_sprite->scale.y = selected_scale_y;

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                if (nk_button_label(nk_ctx, "Reset Transform")) {
                    wavy_sprite->position = {800, 520};
                    wavy_sprite->rotation = 0.0f;
                    wavy_sprite->scale = {0.5f, 0.5f};
                }

                nk_layout_row_dynamic(nk_ctx, 25, 1);
                nk_checkbox_label(nk_ctx, "Flip Horizontal", reinterpret_cast<nk_bool*>(&wavy_sprite->flip_h));
                nk_checkbox_label(nk_ctx, "Flip Vertical", reinterpret_cast<nk_bool*>(&wavy_sprite->flip_v));
            }
            nk_end(nk_ctx);

            // Info Panel
            if (nk_begin(nk_ctx, "Info", nk_rect(730, 10, 250, 150),
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
                nk_layout_row_dynamic(nk_ctx, 20, 1);
                nk_labelf(nk_ctx, NK_TEXT_LEFT, "FPS: %.1f", 1.0f / scene.get_delta());
                nk_labelf(nk_ctx, NK_TEXT_LEFT, "Frame: %d", static_cast<int>(scene.get_time() * 60));
                nk_labelf(nk_ctx, NK_TEXT_LEFT, "Time: %.2fs", scene.get_time());
                nk_layout_row_dynamic(nk_ctx, 20, 1);
                nk_label(nk_ctx, "Press H to toggle UI", NK_TEXT_LEFT);
                nk_label(nk_ctx, "Press C for Camera", NK_TEXT_LEFT);
            }
            nk_end(nk_ctx);
        }

        wavy_material.set_float("amplitude", wavy_amplitude);
        wavy_material.set_float("frequency", wavy_frequency);
        rainbow_material.set_float("speed", rainbow_speed);
        retro_material.set_float("pixel_size", retro_pixel_size);

        scene.update(0.016f);

        renderer.begin(Color(bg_color.r, bg_color.g, bg_color.b, bg_color.a));
        scene.render();
        renderer.end();

        nk_sdl_render(NK_ANTI_ALIASING_ON, 512 * 1024, 128 * 1024);

        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    nk_sdl_shutdown();
    renderer.shutdown();
    rd.shutdown();
    TTF_Quit();
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
