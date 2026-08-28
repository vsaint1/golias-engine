#pragma once

#include "core/application.h"
#include "core/engine.h"

#include "core/io/file_system.h"

#include "core/input/input_manager.h"

#include "graphics/graphics_device.h"
#include "graphics/render_types.h"
#include "graphics/gpu_types.h"
#include "graphics/texture_slots.h"
#include "graphics/vertex_layout.h"
#include "graphics/shader.h"
#include "graphics/texture_2d.h"
#include "graphics/texture_2d_array.h"
#include "graphics/texture_cube.h"
#include "graphics/framebuffer.h"

#include "math/aabb.h"
#include "math/frustum.h"

#include "render/command_queue.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/model.h"
#include "render/csm.h"

#include "scene/scene.h"
#include "scene/game_object.h"

#include "physics/collider.h"
#include "physics/collision.h"
#include "physics/rigid_body.h"
#include "physics/kinematic_character_controller.h"

#include "scene/components/component.h"
#include "scene/components/static_mesh_component.h"
#include "scene/components/camera_component.h"
#include "scene/components/player_controller_component.h"
#include "scene/components/light_component.h"
#include "scene/components/physics_component.h"
#include "scene/components/audio_listener_component.h"
#include "scene/components/audio_source_component.h"
