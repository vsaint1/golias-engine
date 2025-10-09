#pragma once

#include "core/component/components.h"
#include "core/io/file_system.h"

// =======================================================
// FLECS SYSTEM LOGIC PROTOTYPES                         |
// =======================================================
#if defined(EMBER_2D)
void render_primitives_system(Transform2D& t, Shape2D& s);

void render_labels_system(Transform2D& t, Label2D& l);

void render_sprites_system(Transform2D& t, Sprite2D& s);

void update_transforms_system(flecs::entity e, Transform2D& t);

void render_world_2d_system(flecs::entity e, Camera2D& camera);


#endif

#if defined(EMBER_3D)
// NOTE: This system assumes there's only one main camera in the scene
void render_world_3d_system(flecs::entity e, Camera3D& camera);


// NOTE: this systems are used more like helpers for camera control
//       they should be replaced with proper input handling system
//       that can be bound to different actions
// systems: camera_touch_system/camera_keyboard_system
void camera_touch_system(flecs::entity e, Camera3D& camera, const SDL_Event& event);

#endif

// =======================================================
// COMMON LOGIC 2D/3D SYSTEM PROTOTYPES                  |
// =======================================================
void scene_manager_system(flecs::world& world);

void setup_scripts_system(flecs::entity e, Script& script);

void process_scripts_system(Script& script);


void init_lua_vm();

void shutdown_lua_vm();