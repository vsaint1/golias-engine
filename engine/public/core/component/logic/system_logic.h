#pragma once

#include "core/component/components.h"
#include "core/io/file_system.h"

// =======================================================
// FLECS SYSTEM LOGIC PROTOTYPES                         |
// =======================================================
#pragma region 2D SYSTEMS

/*!
* @brief System to render 2D primitives like rectangles, circles, lines, and polygons.
* @ingroup Systems

*/
void render_primitives_system(Transform2D& t, Shape2D& s);

/*!

@brief System to render text labels in 2D space.
@ingroup Systems

*/
void render_labels_system(Transform2D& t, Label2D& l);

/*!
@brief System to render 2D sprites.
@ingroup Systems

*/
void render_sprites_system(Transform2D& t, Sprite2D& s);

/*!
 * @brief System to update 2D transforms.
 * @ingroup Systems

 */
void update_transforms_system(flecs::entity e, Transform2D& t);

/*!

@brief System to render the entire 2D world based on the active camera.
@ingroup Systems
*/
void render_world_2d_system(flecs::entity e, Camera2D& camera);

#pragma endregion

#pragma region 3D SYSTEMS

/*!

@brief System to render the entire 3D world based on the active camera.
@ingroup Systems
*/
void render_world_3d_system(flecs::entity e, Camera3D& camera);

/*!
@brief System to update and render animated 3D models.
@ingroup Systems
*/
void animation_system(flecs::entity e, Model& model, Animation3D& anim, Transform3D& transform);

#pragma endregion


// =======================================================
// COMMON LOGIC 2D/3D SYSTEM PROTOTYPES                  |
// =======================================================

/*!
@brief System to manage scene transitions.
@ingroup Systems
*/
void scene_manager_system(flecs::world& world);

/*!
@brief System to setup scripts.
@ingroup Systems

*/
void setup_scripts_system(flecs::entity e, Script& script);

/*!
@brief System to process scripts.
@ingroup Systems

*/
void process_scripts_system(Script& script);

/*!
@brief System to process events in scripts.
@ingroup Systems
*/
void process_event_scripts_system(const Script&, const SDL_Event& event);
