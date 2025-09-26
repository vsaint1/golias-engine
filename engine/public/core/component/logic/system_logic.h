#pragma once

#include "core/component/components.h"
#include "core/io/file_system.h"

// =======================================================
// FLECS SYSTEM LOGIC PROTOTYPES                         |
// =======================================================
void setup_scripts_system(flecs::entity e,Script& script);

void process_scripts_system(Script& script);

void render_primitives_system(Transform2D& t, Shape& s);

void render_labels_system(Transform2D& t, Label2D& l);

void render_sprites_system(Transform2D& t,  Sprite2D& s);

void update_transforms_system(flecs::entity e, Transform2D& t);

void scene_manager_system(flecs::world& world);
